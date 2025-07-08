# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Python library implementing a comprehensive data model for 2D vector graphics using Pydantic. It provides type-safe models for drawings, shapes, text, and styling that can be used to build drawing applications, CAD software, or vector graphics editors.

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