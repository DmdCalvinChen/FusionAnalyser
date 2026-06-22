import http.server
import socketserver
import json
import base64
import os
import subprocess
import tempfile
import sys

# Add parent dir to python path to import ai_segmentation
PARENT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(PARENT_DIR)
sys.path.append(PROJECT_ROOT)

try:
    from ai_segmentation.inference import run_inference
except ImportError as e:
    print(f"Warning: Could not import ai_segmentation: {e}")
    run_inference = None


PORT = 8001
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def do_POST(self):
        if self.path == '/api/analyze':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            
            try:
                payload = json.loads(post_data.decode('utf-8'))
                upper_b64 = payload.get('upper_stl_b64')
                lower_b64 = payload.get('lower_stl_b64')

                # Create temp files for the C++ engine to read
                temp_dir = os.path.join(DIRECTORY, "temp")
                os.makedirs(temp_dir, exist_ok=True)
                
                upper_path_mac = ""
                lower_path_mac = ""
                upper_path_vm = ""
                lower_path_vm = ""
                
                if upper_b64:
                    upper_path_mac = os.path.join(temp_dir, "temp_upper.stl")
                    with open(upper_path_mac, "wb") as f:
                        f.write(base64.b64decode(upper_b64))
                    upper_path_vm = "/home/ubuntu/fusionanalyser/orthodontic-analyzer/temp/temp_upper.stl"
                else:
                    upper_path_vm = "/home/ubuntu/fusionanalyser/orthodontic-analyzer/example/upper.stl"
                    upper_path_mac = os.path.join(DIRECTORY, "example", "upper.stl")
                
                if lower_b64:
                    lower_path_mac = os.path.join(temp_dir, "temp_lower.stl")
                    with open(lower_path_mac, "wb") as f:
                        f.write(base64.b64decode(lower_b64))
                    lower_path_vm = "/home/ubuntu/fusionanalyser/orthodontic-analyzer/temp/temp_lower.stl"
                else:
                    lower_path_vm = "/home/ubuntu/fusionanalyser/orthodontic-analyzer/example/lower.stl"
                    lower_path_mac = os.path.join(DIRECTORY, "example", "lower.stl")

                metrics = payload.get('metrics', {})
                # Run AI inference to compute actual metrics
                if run_inference is not None and (upper_path_mac or lower_path_mac):
                    print("Running AI inference to calculate metrics...")
                    ai_metrics = run_inference(upper_path_mac, lower_path_mac)
                    metrics.update(ai_metrics)
                    print(f"AI computed metrics: {ai_metrics}")
                else:
                    print("Skipping AI inference (module not found or no meshes uploaded). Using mock metrics.")

                # Format for C++ backend which expects 'upperWidths' and 'lowerWidths'
                if "tooth_widths" in metrics:
                    upper_w = {}
                    lower_w = {}
                    for fdi, w in metrics["tooth_widths"].items():
                        if str(fdi).startswith('1') or str(fdi).startswith('2'):
                            upper_w[str(fdi)] = w
                        elif str(fdi).startswith('3') or str(fdi).startswith('4'):
                            lower_w[str(fdi)] = w
                    metrics["upperWidths"] = upper_w
                    metrics["lowerWidths"] = lower_w

                # Write the updated metrics to temp dir for C++ engine
                metrics_path_mac = os.path.join(temp_dir, "metrics.json")
                with open(metrics_path_mac, "w") as f:
                    json.dump(metrics, f)
                metrics_path_vm = "/home/ubuntu/fusionanalyser/orthodontic-analyzer/temp/metrics.json"

                # ==========================================
                # Call the C++ Engine via Multipass
                # ==========================================
                
                engine_cmd = f"export LD_LIBRARY_PATH=/home/ubuntu/fusionanalyser/src/distrib:$LD_LIBRARY_PATH && /home/ubuntu/fusionanalyser/orthodontic-analyzer/backend_engine --metrics {metrics_path_vm}"
                
                cmd = ["multipass", "exec", "fusion", "--", "bash", "-c", engine_cmd]
                print(f"Executing: {' '.join(cmd)}")
                result = subprocess.run(cmd, capture_output=True, text=True)
                
                if result.returncode != 0:
                    raise Exception(f"C++ 引擎崩溃或报错: {result.stderr}")
                    
                # We expect the C++ engine to print pure JSON to stdout, but it might have qDebug output
                try:
                    json_str = result.stdout[result.stdout.find('{'):]
                    engine_data = json.loads(json_str)
                except Exception as e:
                    raise Exception(f"C++ 引擎输出不是有效的 JSON: {result.stdout}")

                response = {
                    "status": "success",
                    "data": engine_data
                }
                
                self._send_json(200, response)

            except Exception as e:
                self._send_json(500, {"status": "error", "message": str(e)})
        else:
            self.send_response(404)
            self.end_headers()

    def _send_json(self, code, data):
        self.send_response(code)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode('utf-8'))

if __name__ == '__main__':
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer(("", PORT), Handler) as httpd:
        print(f"API Server listening on port {PORT}")
        httpd.serve_forever()
