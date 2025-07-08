"""
SVG rendering functionality for drawable objects
"""

import math
from typing import Union

from python.data.models import (
    Arc,
    Circle,
    Color,
    DrawableObject,
    Drawing,
    Ellipse,
    FillStyle,
    Group,
    Layer,
    Line,
    LineStyle,
    Path,
    Polygon,
    Polyline,
    Rectangle,
    Text,
    TextAlignment,
    VerticalAlignment,
)


def color_to_svg(color: Color) -> str:
    """Convert Color to SVG color string"""
    if color.a < 1.0:
        return f"rgba({color.r},{color.g},{color.b},{color.a})"
    return color.to_hex()


def line_style_to_svg(style: LineStyle) -> str:
    """Convert LineStyle to SVG stroke-dasharray"""
    patterns = {
        LineStyle.SOLID: "",
        LineStyle.DASHED: "10,5",
        LineStyle.DOTTED: "2,3",
        LineStyle.DASH_DOT: "10,5,2,5",
    }
    return patterns.get(style, "")


def fill_to_svg(fill: FillStyle, defs: list[str], obj_id: str) -> str:
    """Convert FillStyle to SVG fill attribute"""
    if not fill:
        return "none"

    if fill.color:
        return color_to_svg(fill.color)

    if fill.gradient:
        gradient = fill.gradient
        grad_id = f"gradient_{obj_id}"

        if gradient.type.value == "linear":
            angle = gradient.angle or 0
            x1 = 50 + 50 * math.cos(math.radians(angle + 180))
            y1 = 50 + 50 * math.sin(math.radians(angle + 180))
            x2 = 50 + 50 * math.cos(math.radians(angle))
            y2 = 50 + 50 * math.sin(math.radians(angle))

            grad_def = (
                f'<linearGradient id="{grad_id}" x1="{x1}%" y1="{y1}%" x2="{x2}%" y2="{y2}%">'
            )
        else:  # radial
            cx = gradient.center.x if gradient.center else 50
            cy = gradient.center.y if gradient.center else 50
            r = gradient.radius if gradient.radius else 50
            grad_def = f'<radialGradient id="{grad_id}" cx="{cx}%" cy="{cy}%" r="{r}%">'

        for stop in gradient.stops:
            stop_color = color_to_svg(stop.color)
            grad_def += f'<stop offset="{stop.offset * 100}%" stop-color="{stop_color}"/>'

        grad_def += f"</{gradient.type.value}Gradient>"
        defs.append(grad_def)

        return f"url(#{grad_id})"

    return "none"


def render_line(line: Line, defs: list[str]) -> str:
    """Render Line to SVG"""
    attrs = []
    attrs.append(f'x1="{line.start_point.x}"')
    attrs.append(f'y1="{line.start_point.y}"')
    attrs.append(f'x2="{line.end_point.x}"')
    attrs.append(f'y2="{line.end_point.y}"')

    if line.stroke_color:
        attrs.append(f'stroke="{color_to_svg(line.stroke_color)}"')
    attrs.append(f'stroke-width="{line.stroke_width}"')

    dash = line_style_to_svg(line.line_style)
    if dash:
        attrs.append(f'stroke-dasharray="{dash}"')

    if line.opacity < 1.0:
        attrs.append(f'opacity="{line.opacity}"')

    return f'<line {" ".join(attrs)}/>'


def render_circle(circle: Circle, defs: list[str]) -> str:
    """Render Circle to SVG"""
    attrs = []
    attrs.append(f'cx="{circle.center.x}"')
    attrs.append(f'cy="{circle.center.y}"')
    attrs.append(f'r="{circle.radius}"')

    if circle.fill:
        fill = fill_to_svg(circle.fill, defs, str(circle.id))
        attrs.append(f'fill="{fill}"')
    else:
        attrs.append('fill="none"')

    if circle.stroke_color:
        attrs.append(f'stroke="{color_to_svg(circle.stroke_color)}"')
    attrs.append(f'stroke-width="{circle.stroke_width}"')

    if circle.opacity < 1.0:
        attrs.append(f'opacity="{circle.opacity}"')

    return f'<circle {" ".join(attrs)}/>'


def render_ellipse(ellipse: Ellipse, defs: list[str]) -> str:
    """Render Ellipse to SVG"""
    attrs = []
    attrs.append(f'cx="{ellipse.center.x}"')
    attrs.append(f'cy="{ellipse.center.y}"')
    attrs.append(f'rx="{ellipse.rx}"')
    attrs.append(f'ry="{ellipse.ry}"')

    if ellipse.rotation != 0:
        attrs.append(
            f'transform="rotate({ellipse.rotation} {ellipse.center.x} {ellipse.center.y})"'
        )

    if ellipse.fill:
        fill = fill_to_svg(ellipse.fill, defs, str(ellipse.id))
        attrs.append(f'fill="{fill}"')
    else:
        attrs.append('fill="none"')

    if ellipse.stroke_color:
        attrs.append(f'stroke="{color_to_svg(ellipse.stroke_color)}"')
    attrs.append(f'stroke-width="{ellipse.stroke_width}"')

    if ellipse.opacity < 1.0:
        attrs.append(f'opacity="{ellipse.opacity}"')

    return f'<ellipse {" ".join(attrs)}/>'


def render_rectangle(rect: Rectangle, defs: list[str]) -> str:
    """Render Rectangle to SVG"""
    attrs = []
    attrs.append(f'x="{rect.top_left.x}"')
    attrs.append(f'y="{rect.top_left.y}"')
    attrs.append(f'width="{rect.width}"')
    attrs.append(f'height="{rect.height}"')

    if rect.corner_radius > 0:
        attrs.append(f'rx="{rect.corner_radius}"')
        attrs.append(f'ry="{rect.corner_radius}"')

    if rect.fill:
        fill = fill_to_svg(rect.fill, defs, str(rect.id))
        attrs.append(f'fill="{fill}"')
    else:
        attrs.append('fill="none"')

    if rect.stroke_color:
        attrs.append(f'stroke="{color_to_svg(rect.stroke_color)}"')
    attrs.append(f'stroke-width="{rect.stroke_width}"')

    if rect.opacity < 1.0:
        attrs.append(f'opacity="{rect.opacity}"')

    return f'<rect {" ".join(attrs)}/>'


def render_polygon(polygon: Polygon, defs: list[str]) -> str:
    """Render Polygon to SVG"""
    points = " ".join([f"{p.x},{p.y}" for p in polygon.points])

    attrs = []
    attrs.append(f'points="{points}"')

    if polygon.fill:
        fill = fill_to_svg(polygon.fill, defs, str(polygon.id))
        attrs.append(f'fill="{fill}"')
    else:
        attrs.append('fill="none"')

    if polygon.stroke_color:
        attrs.append(f'stroke="{color_to_svg(polygon.stroke_color)}"')
    attrs.append(f'stroke-width="{polygon.stroke_width}"')

    if polygon.opacity < 1.0:
        attrs.append(f'opacity="{polygon.opacity}"')

    return f'<polygon {" ".join(attrs)}/>'


def render_polyline(polyline: Polyline, defs: list[str]) -> str:
    """Render Polyline to SVG"""
    points = " ".join([f"{p.x},{p.y}" for p in polyline.points])

    attrs = []
    attrs.append(f'points="{points}"')
    attrs.append('fill="none"')

    if polyline.stroke_color:
        attrs.append(f'stroke="{color_to_svg(polyline.stroke_color)}"')
    attrs.append(f'stroke-width="{polyline.stroke_width}"')

    dash = line_style_to_svg(polyline.line_style)
    if dash:
        attrs.append(f'stroke-dasharray="{dash}"')

    if polyline.opacity < 1.0:
        attrs.append(f'opacity="{polyline.opacity}"')

    return f'<polyline {" ".join(attrs)}/>'


def render_arc(arc: Arc, defs: list[str]) -> str:
    """Render Arc to SVG using path"""
    # Convert angles to radians
    start_rad = math.radians(arc.start_angle)
    end_rad = math.radians(arc.end_angle)

    # Calculate start and end points
    start_x = arc.center.x + arc.radius * math.cos(start_rad)
    start_y = arc.center.y + arc.radius * math.sin(start_rad)
    end_x = arc.center.x + arc.radius * math.cos(end_rad)
    end_y = arc.center.y + arc.radius * math.sin(end_rad)

    # Determine if we need the large arc flag
    angle_diff = end_rad - start_rad
    if angle_diff < 0:
        angle_diff += 2 * math.pi
    large_arc = 1 if angle_diff > math.pi else 0

    # Create path data
    path_data = f"M {start_x} {start_y} A {arc.radius} {arc.radius} 0 {large_arc} 1 {end_x} {end_y}"

    attrs = []
    attrs.append(f'd="{path_data}"')
    attrs.append('fill="none"')

    if arc.stroke_color:
        attrs.append(f'stroke="{color_to_svg(arc.stroke_color)}"')
    attrs.append(f'stroke-width="{arc.stroke_width}"')

    if arc.opacity < 1.0:
        attrs.append(f'opacity="{arc.opacity}"')

    return f'<path {" ".join(attrs)}/>'


def render_text(text: Text, defs: list[str]) -> str:
    """Render Text to SVG"""
    attrs = []
    attrs.append(f'x="{text.position.x}"')
    attrs.append(f'y="{text.position.y}"')

    # Text alignment
    anchor = {
        TextAlignment.LEFT: "start",
        TextAlignment.CENTER: "middle",
        TextAlignment.RIGHT: "end",
    }
    attrs.append(f'text-anchor="{anchor.get(text.text_alignment, "start")}"')

    # Vertical alignment
    baseline = {
        VerticalAlignment.TOP: "text-before-edge",
        VerticalAlignment.MIDDLE: "middle",
        VerticalAlignment.BOTTOM: "text-after-edge",
    }
    attrs.append(f'dominant-baseline="{baseline.get(text.vertical_alignment, "auto")}"')

    # Font properties
    attrs.append(f'font-family="{text.font_family}"')
    attrs.append(f'font-size="{text.font_size}"')
    if text.font_weight != "normal":
        attrs.append(f'font-weight="{text.font_weight}"')
    if text.font_style != "normal":
        attrs.append(f'font-style="{text.font_style}"')

    if text.stroke_color:
        attrs.append(f'fill="{color_to_svg(text.stroke_color)}"')

    if text.opacity < 1.0:
        attrs.append(f'opacity="{text.opacity}"')

    # Escape special characters
    content = text.content.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

    return f'<text {" ".join(attrs)}>{content}</text>'


def render_path(path: Path, defs: list[str]) -> str:
    """Render Path to SVG"""
    # Build path data from commands
    path_data = []
    for cmd in path.commands:
        path_data.append(cmd.command)
        path_data.extend([str(p) for p in cmd.params])

    attrs = []
    attrs.append(f'd="{" ".join(path_data)}"')

    if path.fill:
        fill = fill_to_svg(path.fill, defs, str(path.id))
        attrs.append(f'fill="{fill}"')
    else:
        attrs.append('fill="none"')

    if path.stroke_color:
        attrs.append(f'stroke="{color_to_svg(path.stroke_color)}"')
    attrs.append(f'stroke-width="{path.stroke_width}"')

    if path.opacity < 1.0:
        attrs.append(f'opacity="{path.opacity}"')

    return f'<path {" ".join(attrs)}/>'


def render_object(obj: Union[DrawableObject, Group], defs: list[str]) -> str:
    """Render any drawable object to SVG"""
    if isinstance(obj, Line):
        return render_line(obj, defs)
    elif isinstance(obj, Circle):
        return render_circle(obj, defs)
    elif isinstance(obj, Ellipse):
        return render_ellipse(obj, defs)
    elif isinstance(obj, Rectangle):
        return render_rectangle(obj, defs)
    elif isinstance(obj, Polygon):
        return render_polygon(obj, defs)
    elif isinstance(obj, Polyline):
        return render_polyline(obj, defs)
    elif isinstance(obj, Arc):
        return render_arc(obj, defs)
    elif isinstance(obj, Text):
        return render_text(obj, defs)
    elif isinstance(obj, Path):
        return render_path(obj, defs)
    elif isinstance(obj, Group):
        return render_group(obj, defs)
    else:
        return f"<!-- Unsupported object type: {type(obj).__name__} -->"


def render_group(group: Group, defs: list[str]) -> str:
    """Render Group to SVG"""
    group_parts = [f'<g id="{group.id}">']

    for obj in group.objects:
        svg = render_object(obj, defs)
        if svg:
            group_parts.append(f"  {svg}")

    group_parts.append("</g>")
    return "\n".join(group_parts)


def render_layer(layer: Layer, defs: list[str]) -> str:
    """Render Layer to SVG"""
    if not layer.visible:
        return ""

    layer_parts = [f'<g id="{layer.id}" opacity="{layer.opacity}">']

    for obj in layer.objects:
        svg = render_object(obj, defs)
        if svg:
            layer_parts.append(f"  {svg}")

    layer_parts.append("</g>")
    return "\n".join(layer_parts)


def drawing_to_svg(drawing: Drawing) -> str:
    """Convert Drawing to complete SVG string"""
    defs: list[str] = []
    svg_parts = [
        f'<svg width="{drawing.width}" height="{drawing.height}" xmlns="http://www.w3.org/2000/svg">',
        f'<rect width="{drawing.width}" height="{drawing.height}" fill="{drawing.background_color.to_hex()}"/>',
    ]

    # Add defs section if needed (will be populated by gradients)
    defs_placeholder = len(svg_parts)

    # Render layers in z-order
    for layer in sorted(drawing.layers, key=lambda layer: layer.z_index):
        layer_svg = render_layer(layer, defs)
        if layer_svg:
            svg_parts.append(layer_svg)

    # Insert defs if any were created
    if defs:
        svg_parts.insert(defs_placeholder, "<defs>")
        for i, def_item in enumerate(defs):
            svg_parts.insert(defs_placeholder + 1 + i, f"  {def_item}")
        svg_parts.insert(defs_placeholder + 1 + len(defs), "</defs>")

    svg_parts.append("</svg>")
    return "\n".join(svg_parts)
