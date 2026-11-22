#!/usr/bin/env python3
"""Execute the editable offline content source."""
from pathlib import Path
import runpy
runpy.run_path(str(Path(__file__).resolve().parents[1]/'assets_src/generate.py'),run_name='__main__')
