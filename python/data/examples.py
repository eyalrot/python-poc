"""
Example usage of the drawing data structures
"""

from python.data import (
    Circle,
    Color,
    Drawing,
    FillStyle,
    Group,
    Layer,
    Line,
    LineStyle,
    Point,
    Polygon,
    Rectangle,
    Text,
)


def create_sample_drawing() -> Drawing:
    # Create a new drawing
    drawing = Drawing(name="Sample Drawing", width=1000, height=800)

    # Create layers
    background_layer = Layer(name="Background", z_index=0)
    shapes_layer = Layer(name="Shapes", z_index=1)
    text_layer = Layer(name="Text", z_index=2)

    # Add background rectangle
    background = Rectangle(
        name="Background",
        top_left=Point(x=0, y=0),
        width=1000,
        height=800,
        fill=FillStyle(color=Color(r=240, g=240, b=240)),
    )
    background_layer.add_object(background)

    # Create some shapes
    # Red circle
    circle = Circle(
        name="Red Circle",
        center=Point(x=200, y=200),
        radius=50,
        fill=FillStyle(color=Color(r=255, g=0, b=0, a=0.8)),
        stroke_color=Color(r=100, g=0, b=0),
        stroke_width=2,
    )
    shapes_layer.add_object(circle)

    # Blue line
    line = Line(
        name="Blue Line",
        start_point=Point(x=100, y=100),
        end_point=Point(x=300, y=300),
        stroke_color=Color(r=0, g=0, b=255),
        stroke_width=3,
        line_style=LineStyle.DASHED,
    )
    shapes_layer.add_object(line)

    # Green polygon (triangle)
    triangle = Polygon(
        name="Green Triangle",
        points=[Point(x=500, y=100), Point(x=600, y=250), Point(x=400, y=250)],
        fill=FillStyle(color=Color(r=0, g=255, b=0, a=0.6)),
        stroke_color=Color(r=0, g=100, b=0),
        stroke_width=2,
    )
    shapes_layer.add_object(triangle)

    # Create a group of shapes
    group = Group(name="Grouped Shapes")

    # Add rectangle to group
    rect1 = Rectangle(
        top_left=Point(x=700, y=100),
        width=100,
        height=80,
        fill=FillStyle(color=Color(r=255, g=165, b=0)),
        corner_radius=10,
    )
    group.add_object(rect1)

    # Add circle to group
    circle2 = Circle(
        center=Point(x=750, y=140), radius=30, fill=FillStyle(color=Color(r=255, g=255, b=0, a=0.7))
    )
    group.add_object(circle2)

    shapes_layer.add_object(group)

    # Add text
    title = Text(
        name="Title",
        position=Point(x=500, y=50),
        content="Sample Drawing",
        font_size=24,
        font_weight="bold",
        stroke_color=Color(r=0, g=0, b=0),
    )
    text_layer.add_object(title)

    # Add layers to drawing
    drawing.add_layer(background_layer)
    drawing.add_layer(shapes_layer)
    drawing.add_layer(text_layer)

    return drawing


def demonstrate_usage() -> None:
    # Create drawing
    drawing = create_sample_drawing()

    print(f"Drawing: {drawing.name}")
    print(f"Size: {drawing.width}x{drawing.height}")
    print(f"Layers: {len(drawing.layers)}")

    # List all objects
    for layer in drawing.layers:
        print(f"\nLayer: {layer.name} (visible: {layer.visible})")
        for obj in layer.objects:
            if hasattr(obj, "name") and obj.name:
                print(f"  - {obj.__class__.__name__}: {obj.name}")
                bbox = obj.get_bounding_box()
                print(
                    f"    Bounding box: ({bbox.min_x}, {bbox.min_y}) to ({bbox.max_x}, {bbox.max_y})"
                )

    # Export to dict (JSON-serializable)
    drawing_dict = drawing.model_dump()
    print(f"\nSerialized drawing has {len(str(drawing_dict))} characters")

    # Find specific object
    shapes_layer = drawing.get_layer_by_id(drawing.layers[1].id)
    if shapes_layer:
        print(f"\nFound layer: {shapes_layer.name}")

    # Modify object
    all_objects = drawing.get_all_objects()
    if all_objects:
        first_object = all_objects[0]
        first_object.opacity = 0.5
        print(f"\nModified {first_object.__class__.__name__} opacity to 0.5")


if __name__ == "__main__":
    demonstrate_usage()
