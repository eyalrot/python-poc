#!/usr/bin/env python3
"""Runner for C++ integration tests."""

import sys
import os
import subprocess
import importlib.util

# Check if drawing_cpp module is available
cpp_module_path = os.path.join(
    os.path.dirname(__file__), "..", "..", "cpp", "drawing_cpp.cpython*.so"
)
import glob

cpp_modules = glob.glob(cpp_module_path)
if not cpp_modules:
    print("ERROR: C++ module not found. Please build it first:")
    print("  cd cpp")
    print("  python setup.py build_ext --inplace")
    sys.exit(1)

# Add cpp directory to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "cpp"))

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
for test_file in test_files:
    print(f"\n{'='*60}")
    print(f"Running {test_file}")
    print(f"{'='*60}")

    result = subprocess.run([sys.executable, test_file], cwd=os.path.dirname(__file__))
    if result.returncode != 0:
        failed = True
        print(f"FAILED: {test_file}")

if failed:
    print("\nSome tests failed!")
    sys.exit(1)
else:
    print("\nAll tests passed!")

