import open3d as o3d
import numpy as np
from inference import process_mesh
from dl_models import MeshSegNet
import torch
import os

model_path = "models/model.tar"
device = torch.device("mps" if torch.backends.mps.is_available() else "cpu")
model = MeshSegNet(num_classes=17, num_channels=15).to(device, dtype=torch.float)
checkpoint = torch.load(model_path, map_location=device)
model.load_state_dict(checkpoint['model_state_dict'])
model.eval()

mesh = o3d.io.read_triangle_mesh("../orthodontic-analyzer/example/upper.stl")
mesh = mesh.simplify_quadric_decimation(10000)
vertices_points = np.asarray(mesh.vertices)
triangles_points = np.asarray(mesh.triangles)

import inference
# We'll hijack the end of process_mesh to see the labels
# Actually, let's just do inference here
cells = vertices_points[triangles_points].reshape(triangles_points.shape[0], 9)
mean_cell_centers = mesh.get_center()
cells[:, 0:3] -= mean_cell_centers[0:3]
cells[:, 3:6] -= mean_cell_centers[0:3]
cells[:, 6:9] -= mean_cell_centers[0:3]
v1 = np.zeros([triangles_points.shape[0], 3], dtype='float32')
v2 = np.zeros([triangles_points.shape[0], 3], dtype='float32')
v1[:, 0] = cells[:, 0] - cells[:, 3]; v1[:, 1] = cells[:, 1] - cells[:, 4]; v1[:, 2] = cells[:, 2] - cells[:, 5]
v2[:, 0] = cells[:, 3] - cells[:, 6]; v2[:, 1] = cells[:, 4] - cells[:, 7]; v2[:, 2] = cells[:, 5] - cells[:, 8]
mesh_normals = np.cross(v1, v2)
mesh_normal_length = np.linalg.norm(mesh_normals, axis=1)
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
A_S = np.zeros([X.shape[0], X.shape[0]], dtype='float32')
A_L = np.zeros([X.shape[0], X.shape[0]], dtype='float32')
X = X.transpose(1, 0).reshape([1, X.shape[1], X.shape[0]])
A_S = A_S.reshape([1, A_S.shape[0], A_S.shape[1]])
A_L = A_L.reshape([1, A_L.shape[0], A_L.shape[1]])

X = torch.from_numpy(X).to(device, dtype=torch.float)
A_S = torch.from_numpy(A_S).to(device, dtype=torch.float)
A_L = torch.from_numpy(A_L).to(device, dtype=torch.float)

with torch.no_grad():
    tensor_prob_output = model(X, A_S, A_L).to(device, dtype=torch.float)
    patch_prob_output = tensor_prob_output.cpu().numpy()

unaries = -100 * np.log10(np.clip(patch_prob_output, 1e-6, None))
unaries = unaries.astype(np.int32).reshape(-1, 17)
predicted_labels = np.argmin(unaries, axis=1)

# Check label 10 (which corresponds to 22 if 9->21, 10->22)
face_indices = np.where(predicted_labels == 10)[0]
vert_indices = np.unique(triangles_points[face_indices].flatten())
tooth_points = vertices_points[vert_indices]

print(f"Class 10 (22) points shape: {tooth_points.shape}")
if len(tooth_points) > 0:
    min_pt = np.min(tooth_points, axis=0)
    max_pt = np.max(tooth_points, axis=0)
    print(f"Bounding box min: {min_pt}, max: {max_pt}, size: {max_pt - min_pt}")

# Check class 0 (background)
bg_faces = np.where(predicted_labels == 0)[0]
print(f"Background (class 0) faces: {len(bg_faces)}")

# Check label 15?
face_15 = np.where(predicted_labels == 15)[0]
print(f"Class 15 faces: {len(face_15)}")
