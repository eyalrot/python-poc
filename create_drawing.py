"""
Create a sample drawing and save it to disk in various formats
"""
import json
from pathlib import Path
from python.data import (
    Drawing, Layer, Color, Point, 
    Line, Circle, Rectangle, Text, Group,
    Polygon, FillStyle, LineStyle, Ellipse,
    Arc, Polyline, GradientType, Gradient, GradientStop,
    TextAlignment
)


def create_sample_drawing():
    """Create a complex drawing with various shapes and styles"""
    # Create main drawing
    drawing = Drawing(
        name="Sample Vector Drawing",
        width=800,
        height=600,
        background_color=Color(r=250, g=250, b=250)
    )
    
    # Layer 1: Background elements
    background_layer = Layer(name="Background", z_index=0)
    
    # Add gradient background rectangle
    gradient_bg = Rectangle(
        name="Gradient Background",
        top_left=Point(x=0, y=0),
        width=800,
        height=600,
        fill=FillStyle(
            gradient=Gradient(
                type=GradientType.LINEAR,
                stops=[
                    GradientStop(offset=0.0, color=Color(r=200, g=230, b=255)),
                    GradientStop(offset=1.0, color=Color(r=255, g=255, b=255))
                ],
                angle=90
            )
        ),
        stroke_width=0
    )
    background_layer.add_object(gradient_bg)
    
    # Layer 2: Shapes
    shapes_layer = Layer(name="Shapes", z_index=1)
    
    # Create a sun-like shape using circles and lines
    sun_group = Group(name="Sun")
    
    # Sun center
    sun_circle = Circle(
        name="Sun Body",
        center=Point(x=650, y=100),
        radius=40,
        fill=FillStyle(color=Color(r=255, g=200, b=0)),
        stroke_color=Color(r=255, g=150, b=0),
        stroke_width=2
    )
    sun_group.add_object(sun_circle)
    
    # Sun rays
    import math
    for i in range(8):
        angle = i * math.pi / 4
        start_x = 650 + 50 * math.cos(angle)
        start_y = 100 + 50 * math.sin(angle)
        end_x = 650 + 70 * math.cos(angle)
        end_y = 100 + 70 * math.sin(angle)
        
        ray = Line(
            start_point=Point(x=start_x, y=start_y),
            end_point=Point(x=end_x, y=end_y),
            stroke_color=Color(r=255, g=200, b=0),
            stroke_width=3
        )
        sun_group.add_object(ray)
    
    shapes_layer.add_object(sun_group)
    
    # House shape
    house_group = Group(name="House")
    
    # House body
    house_body = Rectangle(
        name="House Body",
        top_left=Point(x=100, y=300),
        width=200,
        height=150,
        fill=FillStyle(color=Color(r=220, g=180, b=140)),
        stroke_color=Color(r=150, g=100, b=60),
        stroke_width=2
    )
    house_group.add_object(house_body)
    
    # Roof
    roof = Polygon(
        name="Roof",
        points=[
            Point(x=80, y=300),
            Point(x=200, y=200),
            Point(x=320, y=300)
        ],
        fill=FillStyle(color=Color(r=180, g=60, b=60)),
        stroke_color=Color(r=120, g=40, b=40),
        stroke_width=2
    )
    house_group.add_object(roof)
    
    # Door
    door = Rectangle(
        name="Door",
        top_left=Point(x=180, y=380),
        width=40,
        height=70,
        fill=FillStyle(color=Color(r=100, g=60, b=40)),
        stroke_color=Color(r=60, g=40, b=20),
        stroke_width=2
    )
    house_group.add_object(door)
    
    # Windows
    window1 = Rectangle(
        name="Window 1",
        top_left=Point(x=130, y=330),
        width=30,
        height=30,
        fill=FillStyle(color=Color(r=180, g=220, b=255, a=0.8)),
        stroke_color=Color(r=100, g=100, b=100),
        stroke_width=2
    )
    window2 = Rectangle(
        name="Window 2",
        top_left=Point(x=240, y=330),
        width=30,
        height=30,
        fill=FillStyle(color=Color(r=180, g=220, b=255, a=0.8)),
        stroke_color=Color(r=100, g=100, b=100),
        stroke_width=2
    )
    house_group.add_object(window1)
    house_group.add_object(window2)
    
    shapes_layer.add_object(house_group)
    
    # Tree
    tree_group = Group(name="Tree")
    
    # Tree trunk
    trunk = Rectangle(
        name="Tree Trunk",
        top_left=Point(x=480, y=380),
        width=40,
        height=80,
        fill=FillStyle(color=Color(r=101, g=67, b=33)),
        stroke_color=Color(r=60, g=40, b=20),
        stroke_width=2
    )
    tree_group.add_object(trunk)
    
    # Tree crown (multiple circles)
    crown_positions = [
        (500, 340, 35),
        (480, 350, 30),
        (520, 350, 30),
        (500, 320, 30),
        (485, 330, 25),
        (515, 330, 25)
    ]
    
    for x, y, r in crown_positions:
        crown = Circle(
            center=Point(x=x, y=y),
            radius=r,
            fill=FillStyle(color=Color(r=34, g=139, b=34)),
            stroke_color=Color(r=20, g=90, b=20),
            stroke_width=1
        )
        tree_group.add_object(crown)
    
    shapes_layer.add_object(tree_group)
    
    # Abstract shapes
    # Ellipse
    ellipse = Ellipse(
        name="Ellipse",
        center=Point(x=400, y=150),
        rx=60,
        ry=30,
        rotation=30,
        fill=FillStyle(color=Color(r=255, g=100, b=100, a=0.6)),
        stroke_color=Color(r=200, g=50, b=50),
        stroke_width=2
    )
    shapes_layer.add_object(ellipse)
    
    # Arc (rainbow)
    rainbow = Arc(
        name="Rainbow",
        center=Point(x=400, y=500),
        radius=150,
        start_angle=180,
        end_angle=360,
        stroke_color=Color(r=147, g=112, b=219),
        stroke_width=20,
        fill=None
    )
    shapes_layer.add_object(rainbow)
    
    # Polyline (mountain outline)
    mountain = Polyline(
        name="Mountain",
        points=[
            Point(x=50, y=500),
            Point(x=150, y=400),
            Point(x=200, y=450),
            Point(x=300, y=350),
            Point(x=350, y=400),
            Point(x=450, y=500)
        ],
        stroke_color=Color(r=100, g=100, b=100),
        stroke_width=3,
        line_style=LineStyle.SOLID
    )
    shapes_layer.add_object(mountain)
    
    # Layer 3: Text and labels
    text_layer = Layer(name="Text", z_index=2)
    
    # Title
    title = Text(
        name="Title",
        position=Point(x=400, y=50),
        content="My Vector Drawing",
        font_size=32,
        font_weight="bold",
        font_family="Arial",
        stroke_color=Color(r=50, g=50, b=150),
        text_alignment=TextAlignment.CENTER
    )
    text_layer.add_object(title)
    
    # Labels
    house_label = Text(
        position=Point(x=200, y=480),
        content="House",
        font_size=14,
        stroke_color=Color(r=100, g=100, b=100),
        text_alignment=TextAlignment.CENTER
    )
    tree_label = Text(
        position=Point(x=500, y=480),
        content="Tree",
        font_size=14,
        stroke_color=Color(r=100, g=100, b=100),
        text_alignment=TextAlignment.CENTER
    )
    text_layer.add_object(house_label)
    text_layer.add_object(tree_label)
    
    # Add signature
    signature = Text(
        position=Point(x=700, y=580),
        content="Created with Python",
        font_size=10,
        font_style="italic",
        stroke_color=Color(r=150, g=150, b=150),
        text_alignment=TextAlignment.RIGHT
    )
    text_layer.add_object(signature)
    
    # Add all layers to drawing
    drawing.add_layer(background_layer)
    drawing.add_layer(shapes_layer)
    drawing.add_layer(text_layer)
    
    return drawing


def save_drawing_to_disk(drawing: Drawing, output_dir: str = "output"):
    """Save the drawing in various formats"""
    # Create output directory
    output_path = Path(output_dir)
    output_path.mkdir(exist_ok=True)
    
    # Save as JSON
    json_path = output_path / f"{drawing.name.replace(' ', '_')}.json"
    with open(json_path, 'w') as f:
        json.dump(drawing.model_dump(), f, indent=2, default=str)
    print(f"Saved JSON to: {json_path}")
    
    # Save as pretty-printed JSON
    pretty_json_path = output_path / f"{drawing.name.replace(' ', '_')}_pretty.json"
    with open(pretty_json_path, 'w') as f:
        json.dump(drawing.model_dump(), f, indent=4, default=str)
    print(f"Saved pretty JSON to: {pretty_json_path}")
    
    # Save basic SVG
    svg_path = output_path / f"{drawing.name.replace(' ', '_')}.svg"
    with open(svg_path, 'w') as f:
        f.write(drawing.to_svg())
    print(f"Saved SVG to: {svg_path}")
    
    # Save drawing statistics
    stats_path = output_path / f"{drawing.name.replace(' ', '_')}_stats.txt"
    with open(stats_path, 'w') as f:
        f.write(f"Drawing: {drawing.name}\n")
        f.write(f"Size: {drawing.width}x{drawing.height}\n")
        f.write(f"Layers: {len(drawing.layers)}\n")
        f.write(f"Total objects: {len(drawing.get_all_objects())}\n\n")
        
        for layer in drawing.layers:
            f.write(f"\nLayer: {layer.name}\n")
            f.write(f"  Visible: {layer.visible}\n")
            f.write(f"  Z-index: {layer.z_index}\n")
            f.write(f"  Objects: {len(layer.objects)}\n")
            
            for obj in layer.objects:
                obj_type = obj.__class__.__name__
                obj_name = obj.name if obj.name else "Unnamed"
                f.write(f"    - {obj_type}: {obj_name}\n")
                
                if isinstance(obj, Group):
                    for sub_obj in obj.objects:
                        sub_type = sub_obj.__class__.__name__
                        sub_name = sub_obj.name if sub_obj.name else "Unnamed"
                        f.write(f"      - {sub_type}: {sub_name}\n")
    
    print(f"Saved statistics to: {stats_path}")
    
    return {
        'json': json_path,
        'pretty_json': pretty_json_path,
        'svg': svg_path,
        'stats': stats_path
    }


def main():
    print("Creating sample drawing...")
    drawing = create_sample_drawing()
    
    print(f"\nDrawing created: {drawing.name}")
    print(f"Size: {drawing.width}x{drawing.height}")
    print(f"Layers: {len(drawing.layers)}")
    print(f"Total objects: {len(drawing.get_all_objects())}")
    
    print("\nSaving to disk...")
    saved_files = save_drawing_to_disk(drawing)
    
    print("\nAll files saved successfully!")
    print("\nYou can now:")
    print("1. View the JSON files to see the data structure")
    print("2. Open the SVG file in a browser (basic rendering)")
    print("3. Check the statistics file for a summary")
    
    # Also create a simpler example
    print("\n" + "="*50)
    print("Creating a simpler example drawing...")
    
    simple = Drawing(name="Simple Example", width=400, height=300)
    simple_layer = Layer(name="Main")
    
    # Add a few basic shapes
    circle = Circle(
        center=Point(x=100, y=150),
        radius=50,
        fill=FillStyle(color=Color(r=255, g=0, b=0, a=0.7)),
        stroke_color=Color(r=100, g=0, b=0),
        stroke_width=2
    )
    
    rect = Rectangle(
        top_left=Point(x=200, y=100),
        width=100,
        height=100,
        corner_radius=10,
        fill=FillStyle(color=Color(r=0, g=255, b=0, a=0.7)),
        stroke_color=Color(r=0, g=100, b=0),
        stroke_width=2
    )
    
    line = Line(
        start_point=Point(x=50, y=250),
        end_point=Point(x=350, y=50),
        stroke_color=Color(r=0, g=0, b=255),
        stroke_width=3,
        line_style=LineStyle.DASHED
    )
    
    simple_layer.add_object(circle)
    simple_layer.add_object(rect)
    simple_layer.add_object(line)
    simple.add_layer(simple_layer)
    
    save_drawing_to_disk(simple)
    print("Simple example saved!")


if __name__ == "__main__":
    main()