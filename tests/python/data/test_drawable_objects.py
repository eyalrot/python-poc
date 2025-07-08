import pytest
from datetime import datetime
from uuid import UUID
from pydantic import ValidationError
from python.data import (
    Point,
    Color,
    Line,
    Circle,
    Ellipse,
    Rectangle,
    Polygon,
    Polyline,
    Arc,
    Text,
    Path,
    PathCommand,
    LineStyle,
    TextAlignment,
    VerticalAlignment,
    FillStyle,
)


class TestLine:
    def test_line_creation(self):
        line = Line(start_point=Point(x=0, y=0), end_point=Point(x=100, y=100))
        assert line.start_point.x == 0
        assert line.start_point.y == 0
        assert line.end_point.x == 100
        assert line.end_point.y == 100
        assert line.line_style == LineStyle.SOLID

    def test_line_with_style(self):
        line = Line(
            start_point=Point(x=10, y=20),
            end_point=Point(x=50, y=60),
            line_style=LineStyle.DASHED,
            stroke_color=Color(r=255, g=0, b=0),
            stroke_width=3,
        )
        assert line.line_style == LineStyle.DASHED
        assert line.stroke_color.r == 255
        assert line.stroke_width == 3

    def test_line_bounding_box(self):
        line = Line(start_point=Point(x=20, y=30), end_point=Point(x=10, y=50))
        bbox = line.get_bounding_box()
        assert bbox.min_x == 10
        assert bbox.min_y == 30
        assert bbox.max_x == 20
        assert bbox.max_y == 50

    def test_line_default_properties(self):
        line = Line(start_point=Point(x=0, y=0), end_point=Point(x=10, y=10))
        assert isinstance(line.id, UUID)
        assert line.name is None
        assert line.visible is True
        assert line.locked is False
        assert line.opacity == 1.0


class TestCircle:
    def test_circle_creation(self):
        circle = Circle(center=Point(x=50, y=50), radius=25)
        assert circle.center.x == 50
        assert circle.center.y == 50
        assert circle.radius == 25

    def test_circle_validation(self):
        with pytest.raises(ValidationError):
            Circle(center=Point(x=0, y=0), radius=0)  # radius must be > 0

        with pytest.raises(ValidationError):
            Circle(center=Point(x=0, y=0), radius=-10)  # negative radius

    def test_circle_bounding_box(self):
        circle = Circle(center=Point(x=100, y=100), radius=50)
        bbox = circle.get_bounding_box()
        assert bbox.min_x == 50
        assert bbox.min_y == 50
        assert bbox.max_x == 150
        assert bbox.max_y == 150
        assert bbox.width == 100
        assert bbox.height == 100

    def test_circle_with_fill(self):
        circle = Circle(
            center=Point(x=0, y=0),
            radius=10,
            fill=FillStyle(color=Color(r=255, g=200, b=100, a=0.5)),
        )
        assert circle.fill.color.r == 255
        assert circle.fill.color.a == 0.5


class TestEllipse:
    def test_ellipse_creation(self):
        ellipse = Ellipse(center=Point(x=50, y=50), rx=30, ry=20)
        assert ellipse.rx == 30
        assert ellipse.ry == 20
        assert ellipse.rotation == 0.0

    def test_ellipse_with_rotation(self):
        ellipse = Ellipse(center=Point(x=0, y=0), rx=40, ry=20, rotation=45)
        assert ellipse.rotation == 45

    def test_ellipse_validation(self):
        with pytest.raises(ValidationError):
            Ellipse(center=Point(x=0, y=0), rx=0, ry=10)  # rx must be > 0

        with pytest.raises(ValidationError):
            Ellipse(center=Point(x=0, y=0), rx=10, ry=-5)  # negative ry

    def test_ellipse_bounding_box(self):
        ellipse = Ellipse(center=Point(x=100, y=100), rx=50, ry=30)
        bbox = ellipse.get_bounding_box()
        assert bbox.min_x == 50
        assert bbox.min_y == 70
        assert bbox.max_x == 150
        assert bbox.max_y == 130


class TestRectangle:
    def test_rectangle_creation(self):
        rect = Rectangle(top_left=Point(x=10, y=20), width=100, height=50)
        assert rect.top_left.x == 10
        assert rect.top_left.y == 20
        assert rect.width == 100
        assert rect.height == 50
        assert rect.corner_radius == 0

    def test_rectangle_with_corner_radius(self):
        rect = Rectangle(top_left=Point(x=0, y=0), width=100, height=100, corner_radius=10)
        assert rect.corner_radius == 10

    def test_rectangle_validation(self):
        with pytest.raises(ValidationError):
            Rectangle(top_left=Point(x=0, y=0), width=0, height=50)  # width must be > 0

        with pytest.raises(ValidationError):
            Rectangle(top_left=Point(x=0, y=0), width=50, height=-10)  # negative height

        with pytest.raises(ValidationError):
            Rectangle(
                top_left=Point(x=0, y=0), width=50, height=50, corner_radius=-5
            )  # negative corner radius

    def test_rectangle_bounding_box(self):
        rect = Rectangle(top_left=Point(x=20, y=30), width=80, height=40)
        bbox = rect.get_bounding_box()
        assert bbox.min_x == 20
        assert bbox.min_y == 30
        assert bbox.max_x == 100
        assert bbox.max_y == 70


class TestPolygon:
    def test_polygon_creation(self):
        polygon = Polygon(points=[Point(x=0, y=0), Point(x=100, y=0), Point(x=50, y=100)])
        assert len(polygon.points) == 3
        assert polygon.closed is True

    def test_polygon_validation(self):
        with pytest.raises(ValidationError):
            # Less than 3 points
            Polygon(points=[Point(x=0, y=0), Point(x=10, y=10)])

    def test_polygon_bounding_box(self):
        polygon = Polygon(
            points=[Point(x=10, y=20), Point(x=50, y=10), Point(x=30, y=60), Point(x=5, y=40)]
        )
        bbox = polygon.get_bounding_box()
        assert bbox.min_x == 5
        assert bbox.min_y == 10
        assert bbox.max_x == 50
        assert bbox.max_y == 60

    def test_polygon_open(self):
        polygon = Polygon(
            points=[Point(x=0, y=0), Point(x=10, y=0), Point(x=10, y=10)], closed=False
        )
        assert polygon.closed is False


class TestPolyline:
    def test_polyline_creation(self):
        polyline = Polyline(points=[Point(x=0, y=0), Point(x=50, y=50), Point(x=100, y=0)])
        assert len(polyline.points) == 3
        assert polyline.line_style == LineStyle.SOLID

    def test_polyline_validation(self):
        with pytest.raises(ValidationError):
            # Less than 2 points
            Polyline(points=[Point(x=0, y=0)])

    def test_polyline_with_style(self):
        polyline = Polyline(
            points=[Point(x=0, y=0), Point(x=100, y=100)],
            line_style=LineStyle.DOTTED,
            stroke_width=2.5,
        )
        assert polyline.line_style == LineStyle.DOTTED
        assert polyline.stroke_width == 2.5


class TestArc:
    def test_arc_creation(self):
        arc = Arc(center=Point(x=50, y=50), radius=30, start_angle=0, end_angle=90)
        assert arc.radius == 30
        assert arc.start_angle == 0
        assert arc.end_angle == 90

    def test_arc_angles(self):
        arc = Arc(center=Point(x=0, y=0), radius=50, start_angle=-45, end_angle=270)
        assert arc.start_angle == -45
        assert arc.end_angle == 270

    def test_arc_validation(self):
        with pytest.raises(ValidationError):
            Arc(center=Point(x=0, y=0), radius=0, start_angle=0, end_angle=90)  # radius must be > 0


class TestText:
    def test_text_creation(self):
        text = Text(position=Point(x=100, y=100), content="Hello World")
        assert text.content == "Hello World"
        assert text.font_family == "Arial"
        assert text.font_size == 12.0
        assert text.font_weight == "normal"
        assert text.font_style == "normal"

    def test_text_with_custom_font(self):
        text = Text(
            position=Point(x=0, y=0),
            content="Custom Text",
            font_family="Helvetica",
            font_size=24,
            font_weight="bold",
            font_style="italic",
        )
        assert text.font_family == "Helvetica"
        assert text.font_size == 24
        assert text.font_weight == "bold"
        assert text.font_style == "italic"

    def test_text_alignment(self):
        text = Text(
            position=Point(x=50, y=50),
            content="Aligned",
            text_alignment=TextAlignment.CENTER,
            vertical_alignment=VerticalAlignment.MIDDLE,
        )
        assert text.text_alignment == TextAlignment.CENTER
        assert text.vertical_alignment == VerticalAlignment.MIDDLE

    def test_text_validation(self):
        with pytest.raises(ValidationError):
            Text(position=Point(x=0, y=0), content="Test", font_size=0)  # font_size must be > 0

    def test_text_bounding_box(self):
        text = Text(position=Point(x=10, y=20), content="Test", font_size=20)
        bbox = text.get_bounding_box()
        # Approximate calculations
        assert bbox.min_x == 10
        assert bbox.max_x > bbox.min_x
        assert bbox.max_y == 20
        assert bbox.min_y < bbox.max_y


class TestPath:
    def test_path_creation(self):
        path = Path(
            commands=[
                PathCommand(command="M", params=[0, 0]),
                PathCommand(command="L", params=[100, 100]),
                PathCommand(command="Z"),
            ]
        )
        assert len(path.commands) == 3
        assert path.commands[0].command == "M"
        assert path.commands[0].params == [0, 0]
        assert path.commands[2].command == "Z"
        assert path.commands[2].params == []

    def test_path_complex(self):
        path = Path(
            commands=[
                PathCommand(command="M", params=[10, 10]),
                PathCommand(command="C", params=[20, 20, 40, 20, 50, 10]),
                PathCommand(command="L", params=[90, 90]),
                PathCommand(command="Z"),
            ]
        )
        assert len(path.commands) == 4
        assert path.commands[1].command == "C"
        assert len(path.commands[1].params) == 6  # Cubic bezier has 6 params


class TestDrawableObjectCommon:
    def test_metadata(self):
        circle = Circle(
            center=Point(x=0, y=0), radius=10, metadata={"author": "test", "version": 1}
        )
        assert circle.metadata["author"] == "test"
        assert circle.metadata["version"] == 1

    def test_timestamps(self):
        before = datetime.now()
        rect = Rectangle(top_left=Point(x=0, y=0), width=100, height=100)
        after = datetime.now()

        assert before <= rect.created_at <= after
        assert before <= rect.updated_at <= after
        # Timestamps might differ by microseconds
        time_diff = abs((rect.updated_at - rect.created_at).total_seconds())
        assert time_diff < 0.001  # Less than 1 millisecond

    def test_opacity_validation(self):
        with pytest.raises(ValidationError):
            Circle(center=Point(x=0, y=0), radius=10, opacity=1.5)  # opacity must be <= 1.0

        with pytest.raises(ValidationError):
            Circle(center=Point(x=0, y=0), radius=10, opacity=-0.1)  # opacity must be >= 0.0
