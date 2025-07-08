#!/usr/bin/env python3
"""Runner for C++ integration tests."""

import sys
import os
import subprocess
import importlib.util
import glob

# Add project root and cpp directory to Python path
project_root = os.path.join(os.path.dirname(__file__), "..", "..")
cpp_dir = os.path.join(project_root, "cpp")
sys.path.insert(0, cpp_dir)
sys.path.insert(0, project_root)

# Check if drawing_cpp module is available
cpp_module_patterns = [
    os.path.join(cpp_dir, "drawing_cpp.cpython*.so"),
    os.path.join(cpp_dir, "build", "lib.*", "drawing_cpp.cpython*.so"),
]

cpp_modules = []
for pattern in cpp_module_patterns:
    cpp_modules.extend(glob.glob(pattern))

if not cpp_modules:
    print("ERROR: C++ module not found. Please build it first:")
    print("  cd cpp")
    print("  python setup.py build_ext --inplace")
    print("\nSearched in:")
    for pattern in cpp_module_patterns:
        print(f"  {pattern}")
    sys.exit(1)

print(f"Found C++ module: {cpp_modules[0]}")

# Run all test files
test_files = [
    "test_cpp_bindings.py",
    "test_batch_operations.py",
    "test_group_functionality.py",
    "test_load_save.py",
    "test_new_features.py",
    "test_path_functionality.py",
    "test_python_cpp_compatibility.py",
]

failed = False

# Set up environment with both project root and cpp directory in PYTHONPATH
env = os.environ.copy()
python_paths = f"{project_root}{os.pathsep}{cpp_dir}"
if 'PYTHONPATH' in env:
    env['PYTHONPATH'] = f"{python_paths}{os.pathsep}{env['PYTHONPATH']}"
else:
    env['PYTHONPATH'] = python_paths

for test_file in test_files:
    print(f"\n{'='*60}")
    print(f"Running {test_file}")
    print(f"{'='*60}")

    result = subprocess.run([sys.executable, test_file], cwd=os.path.dirname(__file__), env=env)
    if result.returncode != 0:
        failed = True
        print(f"FAILED: {test_file}")

if failed:
    print("\nSome tests failed!")
    sys.exit(1)
else:
    print("\nAll tests passed!")

