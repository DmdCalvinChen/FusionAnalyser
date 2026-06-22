import open3d as o3d
import numpy as np
import torch
import sys
import os

# Add parent directory to path to allow importing dl_models
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from ai_segmentation.dl_models import MeshSegNet

model_path = os.path.join(os.path.dirname(__file__), "models", "model.tar")
device = torch.device("mps" if torch.backends.mps.is_available() else "cpu")
model = MeshSegNet(num_classes=17, num_channels=15).to(device, dtype=torch.float)
model.load_state_dict(torch.load(model_path, map_location=device)['model_state_dict'])
model.eval()

mesh_path = os.path.join(os.path.dirname(__file__), "..", "orthodontic-analyzer", "example", "upper.stl")
mesh = o3d.io.read_triangle_mesh(mesh_path)
mesh = mesh.simplify_quadric_decimation(10000)
vertices = np.asarray(mesh.vertices)
triangles = np.asarray(mesh.triangles)

cells = vertices[triangles].reshape(-1, 9)
mean_center = mesh.get_center()
cells_centered = cells - np.tile(mean_center, 3)

v1 = cells_centered[:, 0:3] - cells_centered[:, 3:6]
v2 = cells_centered[:, 3:6] - cells_centered[:, 6:9]
normals = np.cross(v1, v2)
lengths = np.linalg.norm(normals, axis=1, keepdims=True)
lengths[lengths==0] = 1e-10
normals /= lengths

barycenters = (cells_centered[:, 0:3] + cells_centered[:, 3:6] + cells_centered[:, 6:9]) / 3.0

means = cells_centered[:, 0:3].mean(axis=0)
stds = cells_centered[:, 0:3].std(axis=0)
stds[stds==0]=1e-10
nmeans = normals.mean(axis=0)
nstds = normals.std(axis=0)
nstds[nstds==0]=1e-10

cells_norm = np.zeros_like(cells_centered)
for i in range(3):
    cells_norm[:, i] = (cells_centered[:, i] - means[i]) / stds[i]
    cells_norm[:, i+3] = (cells_centered[:, i+3] - means[i]) / stds[i]
    cells_norm[:, i+6] = (cells_centered[:, i+6] - means[i]) / stds[i]

mins = cells_centered[:, 0:3].min(axis=0)
maxs = cells_centered[:, 0:3].max(axis=0)
bary_norm = (barycenters - mins) / (maxs - mins + 1e-10)
normals_norm = (normals - nmeans) / nstds

X = np.column_stack((cells_norm, bary_norm, normals_norm))
# Dummy A_S, A_L
A_S = np.zeros([1, X.shape[0], X.shape[0]], dtype='float32')
A_L = np.zeros([1, X.shape[0], X.shape[0]], dtype='float32')
X = X.transpose(1, 0).reshape(1, 15, -1)

with torch.no_grad():
    out = model(torch.tensor(X, dtype=torch.float32).to(device), 
                torch.tensor(A_S).to(device), 
                torch.tensor(A_L).to(device)).cpu().numpy()[0]

labels = np.argmax(out, axis=0)
for c in range(1, 17):
    idx = np.where(labels == c)[0]
    if len(idx) > 0:
        pts = vertices[np.unique(triangles[idx].flatten())]
        center = pts.mean(axis=0)
        spread = pts.max(axis=0) - pts.min(axis=0)
        print(f"Class {c:2d}: Center=[{center[0]:6.1f}, {center[1]:6.1f}, {center[2]:6.1f}], Points={len(pts):4d}, Size=[{spread[0]:4.1f}, {spread[1]:4.1f}, {spread[2]:4.1f}]")
