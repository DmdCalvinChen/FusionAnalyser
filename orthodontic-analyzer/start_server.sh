#!/bin/bash
# Script to start the orthodontic-analyzer server with the correct Conda environment
PYTHONPATH=$(dirname $(pwd)) /Users/chen/miniforge3/envs/ai_env/bin/python3 server.py
