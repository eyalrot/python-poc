"""
Pytest configuration and shared fixtures
"""

import pytest

from python.data import Color, FillStyle, Point


@pytest.fixture
def basic_point():
    """Fixture for a basic point"""
    return Point(x=50, y=50)


@pytest.fixture
def origin_point():
    """Fixture for origin point"""
    return Point(x=0, y=0)


@pytest.fixture
def red_color():
    """Fixture for red color"""
    return Color(r=255, g=0, b=0)


@pytest.fixture
def blue_color():
    """Fixture for blue color"""
    return Color(r=0, g=0, b=255)


@pytest.fixture
def semi_transparent_green():
    """Fixture for semi-transparent green color"""
    return Color(r=0, g=255, b=0, a=0.5)


@pytest.fixture
def solid_red_fill():
    """Fixture for solid red fill"""
    return FillStyle(color=Color(r=255, g=0, b=0))


@pytest.fixture
def solid_blue_fill():
    """Fixture for solid blue fill"""
    return FillStyle(color=Color(r=0, g=0, b=255))
