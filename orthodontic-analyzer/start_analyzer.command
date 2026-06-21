#!/bin/bash
cd "$(dirname "$0")"

PORT=8001

echo "====================================="
echo "  Starting FusionAnalyser WebUI..."
echo "====================================="

# Kill port
lsof -ti:$PORT | xargs kill -9 2>/dev/null

echo "Opening browser at http://localhost:$PORT"
sleep 1
open "http://localhost:$PORT"

echo ""
echo "⚠️  Server is running... (Do not close this terminal window to keep the server alive)"
echo "====================================="
echo "Starting Orthodontic Analyzer Python API Server..."

# Run server
python3 server.py
