import struct
import os

def get_stl_bounds(filepath):
    min_x, max_x = float('inf'), float('-inf')
    min_y, max_y = float('inf'), float('-inf')
    min_z, max_z = float('inf'), float('-inf')
    
    with open(filepath, 'rb') as f:
        header = f.read(80)
        if header[0:5] == b'solid':
            return "ASCII STL not supported by this simple script"
        try:
            num_triangles = struct.unpack('<I', f.read(4))[0]
            for _ in range(num_triangles):
                f.read(12) # skip normal
                for _ in range(3):
                    v = struct.unpack('<3f', f.read(12))
                    min_x, max_x = min(min_x, v[0]), max(max_x, v[0])
                    min_y, max_y = min(min_y, v[1]), max(max_y, v[1])
                    min_z, max_z = min(min_z, v[2]), max(max_z, v[2])
                f.read(2) # attribute byte count
        except Exception as e:
            return f"Error: {e}"
            
    return (min_x, max_x, min_y, max_y, min_z, max_z)

print("Upper bounds:", get_stl_bounds('example/upper.stl'))
print("Lower bounds:", get_stl_bounds('example/lower.stl'))
