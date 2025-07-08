# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Main Goal**: Build a high-performance C++ implementation of the drawing data model capable of handling millions of objects efficiently, with Python bindings for ease of use.

This project is transitioning from a pure Python implementation (using Pydantic) to a hybrid C++/Python architecture. The Python prototype has proven the API design and functionality, but performance analysis shows it cannot efficiently handle millions of objects due to memory overhead (~800 bytes per simple shape) and computational bottlenecks.

### Performance Targets (✅ ACHIEVED)
- Handle 1-10 million drawable objects in a single drawing ✓
- Load 1M objects in <100ms ✓ (Achieved: 20ms)
- Render 1M objects at 60 FPS ✓ 
- Save 1M objects in <50ms ✓ (Achieved: 20ms)
- Average memory usage: <100 bytes per object ✓ (Achieved: 32 bytes)

### Implementation Strategy (Option A: Pure C++ with Python Bindings)
- **Core C++ library**: Compact data structures, efficient memory layout, fast serialization
- **Python bindings**: Using pybind11 for seamless Python integration
- **Compatibility**: Maintain ability to import/export the existing Python JSON format
- **Spatial indexing**: R-tree/quadtree for O(log n) spatial queries

## Platform Requirements

This project is designed to work on **Linux platforms only**. While the code may work on other platforms, testing and CI/CD are configured specifically for Linux environments.

## Development Commands

### Python Development
```bash
# Activate virtual environment
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt

# Run all tests
pytest

# Run tests with coverage
pytest --cov=python.data --cov-report=term-missing

# Run specific test file
pytest tests/python/data/test_basic_models.py

# Run tests matching a pattern
pytest -k "test_color"

# Run with verbose output
pytest -v

# Format code with Black
black .

# Check code formatting
black . --check

# Run linter
ruff check .

# Type checking
mypy python/
```

### C++ Development
```bash
# Build C++ library and tests
cd cpp
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Run C++ tests
./tests/test_drawing

# Build Python bindings
cd ../
source ../venv/bin/activate
pip install pybind11 setuptools wheel
python setup.py build_ext --inplace

# Run Python integration tests
cd ..
python test_cpp_bindings.py
python test_batch_operations.py
```

## Project Structure

### C++ Library (`cpp/`)
```
cpp/
├── include/drawing/          # Header files
│   ├── types.hpp            # Basic types (Color, Point, BoundingBox)
│   ├── objects.hpp          # Object storage and compact representations
│   ├── drawing.hpp          # Main Drawing and Layer classes
│   ├── batch_operations.hpp # High-performance batch operations
│   └── serialization.hpp    # Binary and JSON serialization
├── src/                     # Implementation files
│   ├── objects.cpp
│   ├── drawing.cpp
│   ├── batch_operations.cpp # SIMD-optimized operations
│   ├── serialization.cpp
│   └── json_serialization.cpp
├── python/                  # Python bindings
│   ├── bindings.cpp        # pybind11 bindings
│   └── CMakeLists.txt
├── tests/                   # C++ unit tests
├── examples/                # Example programs
├── CMakeLists.txt          # Build configuration
└── setup.py                # Python package build
```

### Python Wrapper (`python/`)
- `drawing_cpp_wrapper.py`: Pythonic interface to C++ library
- Original Pydantic models for compatibility

## Architecture

### Current Python Design (Prototype)
The library uses a hierarchical structure:
- **Drawing** (top-level container) → contains multiple **Layers**
- **Layer** (z-indexed, can be hidden/locked) → contains **Objects**
- **Objects** can be shapes (Line, Circle, etc.), Text, or **Groups**
- **Groups** can contain other objects for composition

Key characteristics:
- `DrawableObject`: Base class with common properties (id, color, opacity, transform, metadata)
- Pydantic v2 for validation and serialization
- UUID-based identification for all objects
- ~800 bytes per simple object due to Python overhead

### Implemented C++ Architecture

#### Memory Layout (✅ Implemented)
```cpp
// Compact representation achieving 32 bytes per circle
struct CompactObject {
    ObjectType type;        // 1 byte enum
    uint8_t layer_id;       // 1 byte
    ObjectFlags flags;      // 2 bytes (visible, locked, selected, etc.)
    Color fill_color;       // 4 bytes RGBA
    Color stroke_color;     // 4 bytes RGBA
    float stroke_width;     // 4 bytes
    float opacity;          // 4 bytes
};

// Circle: exactly 32 bytes
struct CompactCircle {
    CompactObject base;     // 20 bytes
    float x, y, radius;     // 12 bytes
};
```

#### Key Components
1. **Object Storage**: ✅ Structure-of-arrays for cache efficiency
2. **Batch Operations**: ✅ SIMD-optimized operations (147M objects/second)
3. **Serialization**: ✅ Binary format (36 bytes/object on disk) + JSON export
4. **Python Interface**: ✅ Full pybind11 bindings with zero-copy where possible
5. **Spatial Queries**: ✅ Basic implementation (pending R-tree optimization)

### Testing Strategy
- Maintain Python tests for API compatibility
- Add C++ unit tests for performance-critical paths
- Benchmark suite comparing Python vs C++ performance
- Stress tests with millions of objects

## Roadmap

### Phase 1: C++ Core Implementation (✅ COMPLETED)
- [x] Design compact object representation (32 bytes achieved)
- [x] Implement basic shapes (Circle, Rectangle, Line, Polygon)
- [x] Create efficient serialization (Binary format: 36 bytes/object)
- [x] Basic spatial queries (pending R-tree optimization)
- [x] Structure-of-arrays storage

### Phase 2: Python Integration (✅ COMPLETED)
- [x] pybind11 bindings for core classes
- [x] Python wrapper class for ease of use
- [x] JSON import/export bridge
- [x] Performance comparison tools

### Phase 3: Performance Optimization (🚧 IN PROGRESS)
- [x] SIMD optimizations for batch operations (147M objects/sec)
- [x] Batch geometric operations (translate, scale, rotate, align)
- [ ] R-tree spatial indexing
- [ ] Parallel algorithms for rendering
- [ ] Memory-mapped file support

### Phase 4: Full Object Compatibility (✅ CORE TYPES COMPLETED)
All 10 core object types are now implemented:
- [x] Circle (32 bytes), Rectangle (36 bytes), Line (36 bytes)
- [x] Polygon (28 bytes + point data) - Full support with Python bindings
- [x] Ellipse (40 bytes) - Rotation, rx/ry radii, point detection
- [x] Polyline (28 bytes + point data) - Open path support
- [x] Arc (36 bytes) - Partial circles with angle range checking
- [x] Text (40 bytes + strings) - Font support, alignment, UTF-8 compatible
- [x] Path (32 bytes + commands) - SVG-style path with M,L,C,Q,A,Z commands
- [x] Group (36 bytes + children) - Hierarchical object containers

Remaining features to implement:
- [ ] Gradient/Pattern fills
- [ ] Line styles (dashed, dotted)
- [ ] 3D transforms (currently 2D only)
- [ ] Metadata support per object

### Phase 5: Advanced Features (📋 FUTURE)
- [ ] Streaming API for huge files
- [ ] Progressive loading/rendering
- [ ] GPU compute support
- [ ] WebAssembly compilation

## Recent Updates

### Object Implementation Progress (January 2025)
**Current Status**: C++ now implements all 10 drawable object types:
- ✅ Circle (32 bytes), Rectangle (36 bytes), Line (36 bytes)
- ✅ Polygon (28 bytes + point data) - Full support with Python bindings
- ✅ Ellipse (40 bytes) - Rotation, rx/ry radii, point detection
- ✅ Polyline (28 bytes + point data) - Open path support
- ✅ Arc (36 bytes) - Partial circles with angle range checking
- ✅ Text (40 bytes + strings) - Font support, alignment, UTF-8 compatible
- ✅ Path (32 bytes + commands) - SVG path parser supporting M,L,C,Q,A,Z commands
- ✅ Group (36 bytes + children) - Hierarchical containers with nested support

**Memory Efficiency Maintained**:
- Base objects: 28-40 bytes each (highly optimized)
- Variable data (points, strings, path commands) stored separately
- Group children stored in dedicated array for cache efficiency
- Total memory usage tracked including all auxiliary data

**Implementation Highlights**:
- Path: Full SVG path string parser with command interpretation
- Group: Recursive bounding box calculation, dynamic child management
- JSON serialization: All object types properly exported
- Binary serialization: Compact format with type-specific chunks
- Spatial queries: All objects support find_in_rect and find_at_point

**Next Steps**: 
1. Add gradient/pattern fill support
2. Implement line styles (dashed, dotted)
3. Add metadata support per object
4. Optimize batch operations for newer object types

### C++ Performance Targets Achieved
- ✅ Memory: 32 bytes/object (25x reduction from Python's 800 bytes)
- ✅ Creation: 1.4M objects/second
- ✅ Batch operations: 147M objects/second with SIMD optimization
- ✅ Serialization: 20ms to save/load 1M objects (16.7M objects/sec throughput)
- ✅ Python bindings: Full integration with pybind11
- ✅ Binary format: 36 bytes/object on disk with JSON export compatibility

### SVG Rendering

- Implemented complete SVG rendering functionality in `svg_renderer.py`
- All drawable objects can now be rendered to SVG format
- Supports gradients, patterns, transforms, and all shape types
- Fixed issue where SVG files appeared empty/white

### CI/CD Configuration

- GitHub Actions workflow configured for Linux only
- Automated testing with pytest on Python 3.9, 3.10, 3.11, and 3.12
- Code quality checks include:
  - Black formatter for consistent code style
  - Ruff linter for code quality
  - MyPy for type checking
  - Bandit for security analysis
- Test coverage reporting with pytest-cov

### Code Quality Standards

- All Python code is formatted with Black
- Type hints are used throughout the codebase
- Pydantic v2 for data validation
- Comprehensive test suite with 87 tests

## File I/O Capabilities

### Python (Pydantic) Version

- **JSON Format**: Use `drawing.model_dump()` to serialize to JSON
  ```python
  with open("drawing.json", "w") as f:
      json.dump(drawing.model_dump(), f, indent=2, default=str)
  ```

- **SVG Format**: Use `drawing.to_svg()` to export as SVG
  ```python
  with open("drawing.svg", "w") as f:
      f.write(drawing.to_svg())
  ```

### C++ Version (High Performance)

- **Binary Format**: 36 bytes/object on disk, 20ms for 1M objects
  ```python
  from python.drawing_cpp_wrapper import DrawingCpp
  
  drawing = DrawingCpp(800, 600)
  drawing.add_circle(100, 100, 50)
  drawing.save_binary("drawing.bin")  # Ultra-fast binary format
  
  loaded = DrawingCpp.load_binary("drawing.bin")
  ```

- **JSON Export**: Compatible with Python version
  ```python
  drawing.save_json("drawing.json")  # For compatibility
  ```

### Performance Comparison

| Operation | Python Version | C++ Version | Speedup |
|-----------|---------------|-------------|---------|
| Memory/object | 800 bytes | 32 bytes | 25x |
| Create 100k objects | 720ms | 72ms | 10x |
| Save 100k objects | ~1000ms | 3ms | 333x |
| Batch translate | N/A | 147M obj/sec | ∞ |

See `test_cpp_bindings.py` and `test_batch_operations.py` for performance comparisons.
