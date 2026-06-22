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

# We will modify process_mesh locally to dump tooth 22
import inference
mesh = o3d.io.read_triangle_mesh("../orthodontic-analyzer/example/upper.stl")
mesh = mesh.simplify_quadric_decimation(10000)
# Instead of full inference, let's just run it
metrics = inference.process_mesh("../orthodontic-analyzer/example/upper.stl", 'upper', model, device)
print(metrics)
