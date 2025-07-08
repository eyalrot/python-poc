# C++ Integration Tests

This directory contains tests that require the C++ bindings to be built. These tests are separated from the main Python tests to allow the Python CI to run without building C++ dependencies.

## Running These Tests

### Prerequisites

1. Build the C++ library:
```bash
cd cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
cd ../..
```

2. Build the Python bindings:
```bash
cd cpp
python setup.py build_ext --inplace
cd ..
```

### Run the tests

```bash
# Run all C++ integration tests
pytest tests/cpp_integration/

# Run a specific test
python tests/cpp_integration/test_cpp_bindings.py
```

## Tests in this directory

- `test_cpp_bindings.py` - Basic C++ binding functionality
- `test_batch_operations.py` - Batch operation performance tests
- `test_group_functionality.py` - Group object tests
- `test_load_save.py` - Serialization tests
- `test_new_features.py` - Tests for new C++ features
- `test_path_functionality.py` - Path object tests
- `test_python_cpp_compatibility.py` - Comprehensive compatibility tests

## CI/CD

These tests run in a separate GitHub Actions workflow (`cpp-integration-tests.yml`) that builds the C++ bindings before running the tests.