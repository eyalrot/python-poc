# Python Drawing Models

[![CI](https://github.com/eyalrot/python-poc/actions/workflows/ci.yml/badge.svg)](https://github.com/eyalrot/python-poc/actions/workflows/ci.yml)
[![Python Version](https://img.shields.io/badge/python-3.9%20|%203.10%20|%203.11%20|%203.12-blue)](https://www.python.org)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A comprehensive Python library for 2D vector graphics data models using Pydantic. This library provides type-safe, validated data structures for building drawing applications, CAD software, or vector graphics editors.

## Features

- 🎨 **Complete set of 2D shapes**: Line, Circle, Ellipse, Rectangle, Polygon, Arc, Text, Path
- 📐 **Type-safe models** with Pydantic v2 validation
- 🎭 **Layer-based organization** with z-indexing
- 🌈 **Advanced styling**: Colors (RGBA/Hex), gradients, patterns, transparency
- 🔄 **Transformations**: Translation, rotation, scaling
- 📦 **Groups and composition** for complex drawings
- 💾 **JSON serialization/deserialization**
- 🖼️ **SVG export** with full feature support
- ✅ **Comprehensive test suite** (95% coverage)

## Installation

```bash
pip install -r requirements.txt
```

## Quick Start

```python
from python.data import (
    Drawing, Layer, Color, Point,
    Circle, Rectangle, Text, FillStyle
)

# Create a new drawing
drawing = Drawing(name="My Drawing", width=800, height=600)

# Create a layer
layer = Layer(name="Shapes")

# Add a circle
circle = Circle(
    center=Point(x=400, y=300),
    radius=50,
    fill=FillStyle(color=Color(r=255, g=0, b=0, a=0.8)),
    stroke_color=Color(r=100, g=0, b=0),
    stroke_width=2
)
layer.add_object(circle)

# Add text
text = Text(
    position=Point(x=400, y=400),
    content="Hello, World!",
    font_size=24,
    font_family="Arial"
)
layer.add_object(text)

# Add layer to drawing
drawing.add_layer(layer)

# Export to SVG
svg_content = drawing.to_svg()
with open("my_drawing.svg", "w") as f:
    f.write(svg_content)

# Save as JSON
import json
with open("my_drawing.json", "w") as f:
    json.dump(drawing.model_dump(), f, indent=2)
```

## Project Structure

```
python-poc/
├── python/data/          # Main package
│   ├── models.py        # Core Pydantic models
│   ├── svg_renderer.py  # SVG export functionality
│   └── examples.py      # Usage examples
├── tests/               # Comprehensive test suite
│   └── python/data/
│       ├── test_basic_models.py
│       ├── test_drawable_objects.py
│       └── test_containers.py
├── output/              # Example outputs
└── create_drawing.py    # Example script
```

## Core Components

### Basic Types
- `Point`: 2D/3D coordinates
- `Color`: RGBA color with hex conversion
- `BoundingBox`: Object boundaries with calculated properties
- `Transform`: Translation, rotation, and scaling

### Drawable Objects
All objects inherit from `DrawableObject` base class:
- `Line`: Start and end points with line styles
- `Circle`: Center and radius
- `Ellipse`: Center, radii, and rotation
- `Rectangle`: Position, dimensions, optional corner radius
- `Polygon`: List of points (closed)
- `Polyline`: List of points (open)
- `Arc`: Center, radius, start/end angles
- `Text`: Position, content, font properties
- `Path`: SVG-style path commands

### Container Types
- `Group`: Collection of objects
- `Layer`: Named, z-indexed container
- `Drawing`: Top-level container with dimensions

## Development

### Install development dependencies
```bash
pip install -r requirements.txt
pip install black ruff mypy
```

### Run tests
```bash
pytest                           # Run all tests
pytest -v                        # Verbose output
pytest --cov=python.data         # With coverage
```

### Format code
```bash
black python tests
ruff check python tests --fix
```

### Type checking
```bash
mypy python
```

## GitHub Actions CI

The project includes comprehensive CI/CD workflows:
- **Testing**: Multi-OS, multi-Python version testing
- **Linting**: Black, Ruff, MyPy
- **Security**: Safety and Bandit checks
- **Coverage**: Automated coverage reports
- **Dependencies**: Dependabot updates

## License

MIT License - see LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Author

- **eyal.rot1** - [GitHub](https://github.com/eyalrot)

## Acknowledgments

- Built with [Pydantic](https://pydantic.dev/) for robust data validation
- SVG rendering for universal vector graphics support
- Comprehensive test suite with [pytest](https://pytest.org/)