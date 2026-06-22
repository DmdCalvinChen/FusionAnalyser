import os
import json
import numpy as np
from scipy.spatial import distance_matrix
from scipy.spatial.distance import pdist
import torch
import open3d as o3d
from pygco import cut_from_graph

try:
    from .dl_models import MeshSegNet
except ImportError:
    from dl_models import MeshSegNet

def calculate_tooth_width(points):
    if len(points) < 2:
        return 0.0
    if len(points) > 1000:
        indices = np.random.choice(len(points), 1000, replace=False)
        pts = points[indices]
    else:
        pts = points
        
    # Use DBSCAN to remove stray noise points that inflate the bounding box
    from sklearn.cluster import DBSCAN
    clustering = DBSCAN(eps=2.0, min_samples=10).fit(pts)
    labels = clustering.labels_
    unique_labels, counts = np.unique(labels[labels >= 0], return_counts=True)
    if len(unique_labels) > 0:
        largest_cluster = unique_labels[np.argmax(counts)]
        pts = pts[labels == largest_cluster]
    
    # Fast PCA to find the longest axis in XY plane (mesiodistal width)
    # Ignore Z to prevent measuring tooth height
    pts_xy = pts[:, 0:2]
    centroid = np.mean(pts_xy, axis=0)
    centered = pts_xy - centroid
    cov = np.cov(centered.T)
    eigenvalues, eigenvectors = np.linalg.eigh(cov)
    # The eigenvector with the largest eigenvalue is the principal axis
    principal_axis = eigenvectors[:, np.argmax(eigenvalues)]
    # Project points onto the principal axis in XY
    projections = np.dot(pts_xy, principal_axis)
    # The width is the spread along this axis
    width = float(np.max(projections) - np.min(projections))
    spread = np.max(pts, axis=0) - np.min(pts, axis=0)
    print(f"[DEBUG width] points: {len(pts)}, bounds_spread: {spread}, PCA width: {width}")
    return width

def process_mesh(stl_path, jaw_type, model, device):
    """
    jaw_type: 'upper' or 'lower'
    Returns: a dictionary of {FDI_label: width}
    """
    print(f"Loading mesh: {stl_path} for jaw: {jaw_type}")
    from scipy.spatial import KDTree
    
    mesh = o3d.io.read_triangle_mesh(stl_path)
    original_vertices = np.asarray(mesh.vertices)
    original_triangles = np.asarray(mesh.triangles)
    
    if len(original_triangles) == 0:
        return {}

    # Downsample points for network input using voxel downsample to ensure uniform triangles
    # MeshSegNet is sensitive to irregular triangulation from quadric decimation
    pcd = o3d.geometry.PointCloud()
    pcd.points = o3d.utility.Vector3dVector(original_vertices)
    voxel_size = 0.6
    pcd_down = pcd.voxel_down_sample(voxel_size)
    pcd_down.estimate_normals()
    
    # Create uniform mesh using Ball Pivoting Algorithm
    distances = pcd_down.compute_nearest_neighbor_distance()
    avg_dist = np.mean(distances)
    radius = 3 * avg_dist
    mesh_down = o3d.geometry.TriangleMesh.create_from_point_cloud_ball_pivoting(
        pcd_down,
        o3d.utility.DoubleVector([radius, radius * 2]))
    
    vertices_points = np.asarray(mesh_down.vertices)
    triangles_points = np.asarray(mesh_down.triangles)
    
    if len(triangles_points) == 0:
        # Fallback to original if BPA fails
        vertices_points = original_vertices
        triangles_points = original_triangles

    cells = vertices_points[triangles_points].reshape(triangles_points.shape[0], 9)
    mean_cell_centers = mesh_down.get_center()
    cells[:, 0:3] -= mean_cell_centers[0:3]
    cells[:, 3:6] -= mean_cell_centers[0:3]
    cells[:, 6:9] -= mean_cell_centers[0:3]

    v1 = np.zeros([triangles_points.shape[0], 3], dtype='float32')
    v2 = np.zeros([triangles_points.shape[0], 3], dtype='float32')
    v1[:, 0] = cells[:, 0] - cells[:, 3]
    v1[:, 1] = cells[:, 1] - cells[:, 4]
    v1[:, 2] = cells[:, 2] - cells[:, 5]
    v2[:, 0] = cells[:, 3] - cells[:, 6]
    v2[:, 1] = cells[:, 4] - cells[:, 7]
    v2[:, 2] = cells[:, 5] - cells[:, 8]
    mesh_normals = np.cross(v1, v2)
    mesh_normal_length = np.linalg.norm(mesh_normals, axis=1)
    # Avoid division by zero
    mesh_normal_length[mesh_normal_length == 0] = 1e-10
    mesh_normals[:, 0] /= mesh_normal_length[:]
    mesh_normals[:, 1] /= mesh_normal_length[:]
    mesh_normals[:, 2] /= mesh_normal_length[:]

    points = vertices_points.copy()
    points[:, 0:3] -= mean_cell_centers[0:3]
    normals = np.nan_to_num(mesh_normals).copy()
    
    s = np.sum(vertices_points[triangles_points], 1)
    barycenters = 1 / 3 * s
    barycenters -= mean_cell_centers[0:3]

    maxs = points.max(axis=0)
    mins = points.min(axis=0)
    means = points.mean(axis=0)
    stds = points.std(axis=0)
    stds[stds == 0] = 1e-10
    
    nmeans = normals.mean(axis=0)
    nstds = normals.std(axis=0)
    nstds[nstds == 0] = 1e-10

    for i in range(3):
        cells[:, i] = (cells[:, i] - means[i]) / stds[i]
        cells[:, i + 3] = (cells[:, i + 3] - means[i]) / stds[i]
        cells[:, i + 6] = (cells[:, i + 6] - means[i]) / stds[i]
        barycenters[:, i] = (barycenters[:, i] - mins[i]) / (maxs[i] - mins[i] + 1e-10)
        normals[:, i] = (normals[:, i] - nmeans[i]) / nstds[i]

    X = np.column_stack((cells, barycenters, normals))

    # Downsample for A_S and A_L computation to avoid memory exhaustion on large meshes
    # Original paper limits this. If mesh is > 10000 faces, this D matrix is 10000x10000 (400MB)
    # If 100k faces, it's 100k x 100k = 40GB! We MUST chunk or simplify.
    # MeshSegNet originally downsamples mesh to 10k faces.
    if X.shape[0] > 10000:
        print(f"Mesh too large ({X.shape[0]} faces), simplifying...")    # No quadric decimation here anymore, mesh is already uniform

    A_S = np.zeros([X.shape[0], X.shape[0]], dtype='float32')
    A_L = np.zeros([X.shape[0], X.shape[0]], dtype='float32')
    # Use KDTree instead of full distance matrix for performance and memory
    tree_bary = KDTree(X[:, 9:12])
    
    k_S = min(16, X.shape[0])
    _, idx_S = tree_bary.query(X[:, 9:12], k=k_S)
    for i in range(X.shape[0]):
        A_S[i, idx_S[i]] = 1.0
    A_S = A_S / np.sum(A_S, axis=1, keepdims=True)

    k_L = min(64, X.shape[0])
    _, idx_L = tree_bary.query(X[:, 9:12], k=k_L)
    for i in range(X.shape[0]):
        A_L[i, idx_L[i]] = 1.0
    A_L = A_L / np.sum(A_L, axis=1, keepdims=True)

    X = X.transpose(1, 0).reshape([1, X.shape[1], X.shape[0]])
    A_S = A_S.reshape([1, A_S.shape[0], A_S.shape[1]])
    A_L = A_L.reshape([1, A_L.shape[0], A_L.shape[1]])

    X = torch.from_numpy(X).to(device, dtype=torch.float)
    A_S = torch.from_numpy(A_S).to(device, dtype=torch.float)
    A_L = torch.from_numpy(A_L).to(device, dtype=torch.float)

    print("Running MeshSegNet inference...")
    with torch.no_grad():
        tensor_prob_output = model(X, A_S, A_L).to(device, dtype=torch.float)
        patch_prob_output = tensor_prob_output.cpu().numpy()

    round_factor = 100
    patch_prob_output[patch_prob_output < 1.0e-6] = 1.0e-6

    # unaries
    num_classes = 17
    unaries = -round_factor * np.log10(patch_prob_output)
    unaries = unaries.astype(np.int32)
    unaries = unaries.reshape(-1, num_classes)

    # pairwise
    pairwise = (1 - np.eye(num_classes, dtype=np.int32))

    cells_copy = cells.copy()
    cell_ids = np.asarray(triangles_points)
    lambda_c = 5.0  # Reduced from 20 to prevent over-smoothing bleeding
    edges = np.empty([1, 3], order='C')
    
    print("Computing edges for graph cut...")
    # Find neighbors - optimize this using a faster approach if possible, but keep original for now
    # Original approach is O(N^2) which is very slow. 
    # Let's use open3d adjacency
    mesh.compute_adjacency_list()
    adjacency = mesh.adjacency_list
    
    edge_list = []
    # Actually MeshSegNet uses faces adjacency. 
    # Two faces are adjacent if they share 2 vertices.
    # To find face adjacency fast:
    edge_to_faces = {}
    for face_idx, face in enumerate(triangles_points):
        for i in range(3):
            edge = tuple(sorted((face[i], face[(i+1)%3])))
            if edge not in edge_to_faces:
                edge_to_faces[edge] = []
            edge_to_faces[edge].append(face_idx)
            
    for edge, faces in edge_to_faces.items():
        if len(faces) == 2:
            i_node = faces[0]
            i_nei = faces[1]
            if i_node > i_nei:
                i_node, i_nei = i_nei, i_node
            
            cos_theta = np.dot(normals[i_node, 0:3], normals[i_nei, 0:3]) / (np.linalg.norm(normals[i_node, 0:3]) * np.linalg.norm(normals[i_nei, 0:3]) + 1e-10)
            if cos_theta >= 1.0: cos_theta = 0.9999
            if cos_theta <= -1.0: cos_theta = -0.9999
            theta = np.arccos(cos_theta)
            mesh_diagonal = np.linalg.norm(maxs - mins) + 1e-10
            phi = np.linalg.norm(barycenters[i_node, :] - barycenters[i_nei, :]) / mesh_diagonal
            if theta > np.pi / 2.0:
                weight = -np.log10(theta / np.pi) * phi
            else:
                beta = 1 + np.linalg.norm(np.dot(normals[i_node, 0:3], normals[i_nei, 0:3]))
                weight = -beta * np.log10(theta / np.pi) * phi
            
            edge_list.append([i_node, i_nei, weight])
            
    if len(edge_list) > 0:
        edges = np.array(edge_list)
        edges[:, 2] *= lambda_c * round_factor
        edges = edges.astype(np.int32)
    else:
        edges = np.empty([0, 3], dtype=np.int32)

    print("Running Graph Cut...")
    if len(edges) > 0:
        refine_labels = cut_from_graph(edges, unaries, pairwise)
        predicted_labels = refine_labels.reshape(-1)
    else:
        predicted_labels = np.argmin(unaries, axis=1)

    # Map labels back to dense original vertices using KDTree
    barycenters_world = (cells[:, 0:3] + cells[:, 3:6] + cells[:, 6:9]) / 3.0
    for i in range(3):
        barycenters_world[:, i] = barycenters_world[:, i] * stds[i] + means[i]
    barycenters_world += mean_cell_centers[0:3]

    tree = KDTree(barycenters_world)
    _, nearest_face_idx = tree.query(original_vertices)
    dense_vertex_labels = predicted_labels[nearest_face_idx]

    # Convert MeshSegNet labels (1-16) to FDI
    metrics = {}
    for class_idx in range(1, 17):
        # find dense vertices with this label
        vert_indices = np.where(dense_vertex_labels == class_idx)[0]
        if len(vert_indices) > 0:
            tooth_points = original_vertices[vert_indices]
            
            # map class to FDI
            fdi_label = 0
            if jaw_type == "upper":
                mapping = [0, 11, 12, 13, 14, 15, 16, 17, 21, 22, 23, 24, 25, 26, 27]
                if class_idx < len(mapping):
                    fdi_label = mapping[class_idx]
            else:
                mapping = [0, 31, 32, 33, 34, 35, 36, 37, 41, 42, 43, 44, 45, 46, 47]
                if class_idx < len(mapping):
                    fdi_label = mapping[class_idx]
            
            if fdi_label > 0:
                width = calculate_tooth_width(tooth_points)
                metrics[str(fdi_label)] = round(width, 2)
                
    return metrics

def run_inference(upper_stl_path, lower_stl_path):
    device = torch.device("mps" if torch.backends.mps.is_available() else "cpu")
    print(f"Using device: {device}")
    
    model_path = os.path.join(os.path.dirname(__file__), "models", "model.tar")
    if not os.path.exists(model_path):
        print(f"Model weight not found at {model_path}. Returning empty metrics.")
        return {"tooth_widths": {}}
        
    model = MeshSegNet(num_classes=17, num_channels=15).to(device, dtype=torch.float)
    checkpoint = torch.load(model_path, map_location=device)
    model.load_state_dict(checkpoint['model_state_dict'])
    model.eval()

    tooth_widths = {}
    
    if os.path.exists(upper_stl_path):
        upper_metrics = process_mesh(upper_stl_path, 'upper', model, device)
        tooth_widths.update(upper_metrics)
        
    if os.path.exists(lower_stl_path):
        lower_metrics = process_mesh(lower_stl_path, 'lower', model, device)
        tooth_widths.update(lower_metrics)
        
    return {
        "tooth_widths": tooth_widths
    }

if __name__ == "__main__":
    # Test script
    import sys
    if len(sys.argv) > 2:
        metrics = run_inference(sys.argv[1], sys.argv[2])
        print(json.dumps(metrics, indent=2))
