"""
Setup script for building the C++ drawing library Python bindings.
"""

from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
import pybind11

ext_modules = [
    Pybind11Extension(
        "drawing_cpp",
        ["python/bindings.cpp",
         "src/objects.cpp",
         "src/drawing.cpp", 
         "src/serialization.cpp",
         "src/json_serialization.cpp",
         "src/batch_operations.cpp"],
        include_dirs=["include"],
        cxx_std=17,
        extra_compile_args=["-O3"],
    ),
]

setup(
    name="drawing_cpp",
    version="0.1.0",
    author="Drawing Library Team",
    description="High-performance C++ drawing library with Python bindings",
    long_description="A fast C++ implementation of a 2D drawing library capable of handling millions of objects efficiently",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
    install_requires=[
        "pybind11>=2.11.0",
    ],
)