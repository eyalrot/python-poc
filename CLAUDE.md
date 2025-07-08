# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Python library implementing a comprehensive data model for 2D vector graphics using Pydantic. It provides type-safe models for drawings, shapes, text, and styling that can be used to build drawing applications, CAD software, or vector graphics editors.

## Platform Requirements

This project is designed to work on **Linux platforms only**. While the code may work on other platforms, testing and CI/CD are configured specifically for Linux environments.

## Development Commands

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

## Architecture

### Core Design
The library uses a hierarchical structure:
- **Drawing** (top-level container) → contains multiple **Layers**
- **Layer** (z-indexed, can be hidden/locked) → contains **Objects**
- **Objects** can be shapes (Line, Circle, etc.), Text, or **Groups**
- **Groups** can contain other objects for composition

### Key Base Classes
- `DrawableObject`: Base class for all drawable items with common properties (id, color, opacity, transform, metadata)
- `Point`, `Color`, `BoundingBox`: Fundamental value objects used throughout
- All objects can calculate their bounding box via `get_bounding_box()`

### Type System
- Uses Pydantic v2 for validation and serialization
- Extensive type hints with Union types for collections
- Enums for predefined values (LineStyle, TextAlignment, etc.)
- UUID-based identification for all objects

### Testing
- Tests organized by model type in `tests/python/data/`
- Shared fixtures in `tests/conftest.py`
- 95% code coverage on core models
- Tests validate both success cases and error conditions

## Recent Updates

### SVG Rendering (Latest)

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
