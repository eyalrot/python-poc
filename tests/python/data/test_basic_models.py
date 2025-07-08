import pytest
from pydantic import ValidationError

from python.data import (
    BoundingBox,
    Color,
    FillStyle,
    Gradient,
    GradientStop,
    GradientType,
    Point,
    Transform,
)


class TestPoint:
    def test_point_creation(self):
        point = Point(x=10.5, y=20.3)
        assert point.x == 10.5
        assert point.y == 20.3
        assert point.z == 0.0

    def test_point_with_z(self):
        point = Point(x=1.0, y=2.0, z=3.0)
        assert point.z == 3.0

    def test_point_negative_values(self):
        point = Point(x=-10, y=-20)
        assert point.x == -10
        assert point.y == -20


class TestColor:
    def test_color_creation(self):
        color = Color(r=255, g=128, b=64)
        assert color.r == 255
        assert color.g == 128
        assert color.b == 64
        assert color.a == 1.0

    def test_color_with_alpha(self):
        color = Color(r=255, g=255, b=255, a=0.5)
        assert color.a == 0.5

    def test_color_validation_error(self):
        with pytest.raises(ValidationError):
            Color(r=256, g=0, b=0)  # r > 255

        with pytest.raises(ValidationError):
            Color(r=-1, g=0, b=0)  # r < 0

        with pytest.raises(ValidationError):
            Color(r=0, g=0, b=0, a=1.5)  # a > 1.0

    def test_color_to_hex(self):
        color = Color(r=255, g=128, b=64)
        assert color.to_hex() == "#ff8040"

        color2 = Color(r=0, g=0, b=0)
        assert color2.to_hex() == "#000000"

    def test_color_from_hex(self):
        color = Color.from_hex("#ff8040")
        assert color.r == 255
        assert color.g == 128
        assert color.b == 64

        color2 = Color.from_hex("ff8040")  # without #
        assert color2.r == 255
        assert color2.g == 128
        assert color2.b == 64

    @pytest.mark.parametrize(
        "r,g,b,expected",
        [
            (0, 0, 0, "#000000"),
            (255, 255, 255, "#ffffff"),
            (128, 128, 128, "#808080"),
            (255, 0, 0, "#ff0000"),
            (0, 255, 0, "#00ff00"),
            (0, 0, 255, "#0000ff"),
        ],
    )
    def test_color_hex_conversion(self, r, g, b, expected):
        color = Color(r=r, g=g, b=b)
        assert color.to_hex() == expected

        color_from_hex = Color.from_hex(expected)
        assert color_from_hex.r == r
        assert color_from_hex.g == g
        assert color_from_hex.b == b


class TestBoundingBox:
    def test_bounding_box_creation(self):
        bbox = BoundingBox(min_x=0, min_y=0, max_x=100, max_y=50)
        assert bbox.min_x == 0
        assert bbox.min_y == 0
        assert bbox.max_x == 100
        assert bbox.max_y == 50

    def test_bounding_box_properties(self):
        bbox = BoundingBox(min_x=10, min_y=20, max_x=110, max_y=70)
        assert bbox.width == 100
        assert bbox.height == 50

        center = bbox.center
        assert center.x == 60
        assert center.y == 45

    def test_bounding_box_negative_coords(self):
        bbox = BoundingBox(min_x=-50, min_y=-30, max_x=50, max_y=30)
        assert bbox.width == 100
        assert bbox.height == 60
        assert bbox.center.x == 0
        assert bbox.center.y == 0


class TestTransform:
    def test_transform_defaults(self):
        transform = Transform()
        assert transform.translate_x == 0.0
        assert transform.translate_y == 0.0
        assert transform.translate_z == 0.0
        assert transform.rotate_x == 0.0
        assert transform.rotate_y == 0.0
        assert transform.rotate_z == 0.0
        assert transform.scale_x == 1.0
        assert transform.scale_y == 1.0
        assert transform.scale_z == 1.0

    def test_transform_custom_values(self):
        transform = Transform(translate_x=10, translate_y=20, rotate_z=45, scale_x=2, scale_y=2)
        assert transform.translate_x == 10
        assert transform.translate_y == 20
        assert transform.rotate_z == 45
        assert transform.scale_x == 2
        assert transform.scale_y == 2


class TestGradient:
    def test_gradient_stop(self):
        stop = GradientStop(offset=0.5, color=Color(r=255, g=0, b=0))
        assert stop.offset == 0.5
        assert stop.color.r == 255

    def test_gradient_stop_validation(self):
        with pytest.raises(ValidationError):
            GradientStop(offset=1.5, color=Color(r=0, g=0, b=0))  # offset > 1

        with pytest.raises(ValidationError):
            GradientStop(offset=-0.1, color=Color(r=0, g=0, b=0))  # offset < 0

    def test_linear_gradient(self):
        gradient = Gradient(
            type=GradientType.LINEAR,
            stops=[
                GradientStop(offset=0.0, color=Color(r=255, g=0, b=0)),
                GradientStop(offset=1.0, color=Color(r=0, g=0, b=255)),
            ],
            angle=45,
        )
        assert gradient.type == GradientType.LINEAR
        assert len(gradient.stops) == 2
        assert gradient.angle == 45

    def test_radial_gradient(self):
        gradient = Gradient(
            type=GradientType.RADIAL,
            stops=[
                GradientStop(offset=0.0, color=Color(r=255, g=255, b=255)),
                GradientStop(offset=1.0, color=Color(r=0, g=0, b=0)),
            ],
            center=Point(x=50, y=50),
            radius=100,
        )
        assert gradient.type == GradientType.RADIAL
        assert gradient.center.x == 50
        assert gradient.center.y == 50
        assert gradient.radius == 100


class TestFillStyle:
    def test_fill_style_with_color(self):
        fill = FillStyle(color=Color(r=100, g=150, b=200))
        assert fill.color.r == 100
        assert fill.gradient is None
        assert fill.pattern is None

    def test_fill_style_with_gradient(self):
        gradient = Gradient(
            type=GradientType.LINEAR,
            stops=[
                GradientStop(offset=0.0, color=Color(r=255, g=0, b=0)),
                GradientStop(offset=1.0, color=Color(r=0, g=0, b=255)),
            ],
        )
        fill = FillStyle(gradient=gradient)
        assert fill.color is None
        assert fill.gradient is not None
        assert fill.gradient.type == GradientType.LINEAR

    def test_fill_style_empty(self):
        fill = FillStyle()
        assert fill.color is None
        assert fill.gradient is None
        assert fill.pattern is None
