# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Main Goal**: Build a high-performance C++ implementation of the drawing data model capable of handling millions of objects efficiently, with Python bindings for ease of use.

This project is transitioning from a pure Python implementation (using Pydantic) to a hybrid C++/Python architecture. The Python prototype has proven the API design and functionality, but performance analysis shows it cannot efficiently handle millions of objects due to memory overhead (~800 bytes per simple shape) and computational bottlenecks.

### Performance Targets
- Handle 1-10 million drawable objects in a single drawing
- Load 1M objects in <100ms
- Render 1M objects at 60 FPS
- Save 1M objects in <50ms  
- Average memory usage: <100 bytes per object

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

### C++ Development (Coming Soon)
```bash
# Build C++ library
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run C++ tests
./tests/run_tests

# Build Python bindings
python setup.py build_ext --inplace

# Run benchmarks
python benchmarks/compare_performance.py
```

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

### Planned C++ Architecture

#### Memory Layout
```cpp
// Compact representation for millions of objects
struct CompactObject {
    ObjectType type;        // 1 byte enum
    uint16_t layer_id;      // 2 bytes
    uint16_t flags;         // 2 bytes (visible, locked, selected, etc.)
    uint32_t fill_color;    // 4 bytes RGBA
    uint32_t stroke_color;  // 4 bytes RGBA
    float opacity;          // 4 bytes
    // Type-specific data follows (union or variant)
};

// Example: Circle uses only 32 bytes total
struct CompactCircle {
    CompactObject base;     // 17 bytes
    float x, y, radius;     // 12 bytes
    // 3 bytes padding
};
```

#### Key Components
1. **Object Storage**: Structure-of-arrays for cache efficiency
2. **Spatial Index**: R-tree for fast spatial queries
3. **Memory Pool**: Custom allocator for fixed-size objects
4. **Serialization**: MessagePack or custom binary format
5. **Python Interface**: Zero-copy views where possible

### Testing Strategy
- Maintain Python tests for API compatibility
- Add C++ unit tests for performance-critical paths
- Benchmark suite comparing Python vs C++ performance
- Stress tests with millions of objects

## Roadmap

### Phase 1: C++ Core Implementation (Current Focus)
- [ ] Design compact object representation (32-64 bytes per object)
- [ ] Implement basic shapes (Circle, Rectangle, Line, Polygon)
- [ ] Create efficient serialization (MessagePack/custom binary)
- [ ] Build spatial indexing (R-tree)
- [ ] Memory pool allocator

### Phase 2: Python Integration
- [ ] pybind11 bindings for core classes
- [ ] Compatibility layer for existing Pydantic models
- [ ] JSON import/export bridge
- [ ] Zero-copy NumPy integration

### Phase 3: Performance Optimization
- [ ] SIMD optimizations for batch operations
- [ ] Parallel algorithms for rendering
- [ ] GPU compute support (optional)
- [ ] Memory-mapped file support

### Phase 4: Advanced Features
- [ ] Streaming API for huge files
- [ ] Progressive loading/rendering
- [ ] Distributed rendering support
- [ ] WebAssembly compilation

## Recent Updates

### C++ Architecture Planning (Latest)
- Defined main project goal: C++ implementation for handling millions of objects
- Set performance targets: <100ms load time, 60 FPS rendering for 1M objects
- Chosen implementation strategy: Pure C++ with pybind11 bindings
- Designed compact memory layout: 32 bytes for simple shapes vs 800 bytes in Python

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

### Saving Drawings

The library supports saving drawings to disk in multiple formats:

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

### Loading Drawings

- **From JSON**: Use `Drawing.model_validate()` to deserialize
  ```python
  with open("drawing.json", "r") as f:
      data = json.load(f)
  drawing = Drawing.model_validate(data)
  ```

See `create_drawing.py` for a complete example of creating, saving, and loading drawings with various shapes and styles.
