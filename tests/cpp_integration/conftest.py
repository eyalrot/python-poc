"""Configuration for C++ integration tests."""

import pytest
import sys


def pytest_configure(config):
    """Check if C++ bindings are available."""
    try:
        sys.path.insert(0, 'cpp/build')
        import drawing_cpp
    except ImportError:
        pytest.exit("C++ bindings not available. Please build them first with: cd cpp && mkdir build && cd build && cmake .. && make")