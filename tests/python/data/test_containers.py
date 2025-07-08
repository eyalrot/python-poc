import pytest
from datetime import datetime
from uuid import UUID
from python.data import (
    Point, Color, Line, Circle, Rectangle, Text,
    Group, Layer, Drawing, FillStyle
)


class TestGroup:
    def test_group_creation(self):
        group = Group(name="Test Group")
        assert group.name == "Test Group"
        assert len(group.objects) == 0
        assert isinstance(group.id, UUID)
    
    def test_group_add_objects(self):
        group = Group()
        
        circle = Circle(center=Point(x=50, y=50), radius=25)
        rect = Rectangle(top_left=Point(x=0, y=0), width=100, height=100)
        
        group.add_object(circle)
        group.add_object(rect)
        
        assert len(group.objects) == 2
        assert group.objects[0] == circle
        assert group.objects[1] == rect
    
    def test_group_remove_object(self):
        group = Group()
        
        circle = Circle(center=Point(x=50, y=50), radius=25)
        rect = Rectangle(top_left=Point(x=0, y=0), width=100, height=100)
        
        group.add_object(circle)
        group.add_object(rect)
        
        # Remove by ID
        removed = group.remove_object(circle.id)
        assert removed is True
        assert len(group.objects) == 1
        assert group.objects[0] == rect
        
        # Try to remove non-existent object
        removed = group.remove_object(circle.id)
        assert removed is False
    
    def test_group_bounding_box_empty(self):
        group = Group()
        bbox = group.get_bounding_box()
        assert bbox.min_x == 0
        assert bbox.min_y == 0
        assert bbox.max_x == 0
        assert bbox.max_y == 0
    
    def test_group_bounding_box_with_objects(self):
        group = Group()
        
        # Add circle at (50, 50) with radius 25
        circle = Circle(center=Point(x=50, y=50), radius=25)
        # Add rectangle from (100, 100) to (200, 150)
        rect = Rectangle(top_left=Point(x=100, y=100), width=100, height=50)
        
        group.add_object(circle)
        group.add_object(rect)
        
        bbox = group.get_bounding_box()
        assert bbox.min_x == 25  # circle left edge
        assert bbox.min_y == 25  # circle top edge
        assert bbox.max_x == 200  # rectangle right edge
        assert bbox.max_y == 150  # rectangle bottom edge
    
    def test_group_updated_timestamp(self):
        group = Group()
        initial_updated = group.updated_at
        
        circle = Circle(center=Point(x=0, y=0), radius=10)
        group.add_object(circle)
        
        assert group.updated_at > initial_updated
    
    def test_nested_groups(self):
        outer_group = Group(name="Outer")
        inner_group = Group(name="Inner")
        
        circle = Circle(center=Point(x=0, y=0), radius=10)
        inner_group.add_object(circle)
        outer_group.add_object(inner_group)
        
        assert len(outer_group.objects) == 1
        assert isinstance(outer_group.objects[0], Group)
        assert outer_group.objects[0].name == "Inner"


class TestLayer:
    def test_layer_creation(self):
        layer = Layer(name="Test Layer")
        assert layer.name == "Test Layer"
        assert layer.visible is True
        assert layer.locked is False
        assert layer.opacity == 1.0
        assert layer.z_index == 0
        assert len(layer.objects) == 0
    
    def test_layer_with_properties(self):
        layer = Layer(
            name="Background",
            visible=False,
            locked=True,
            opacity=0.5,
            z_index=10
        )
        assert layer.visible is False
        assert layer.locked is True
        assert layer.opacity == 0.5
        assert layer.z_index == 10
    
    def test_layer_add_object(self):
        layer = Layer(name="Shapes")
        circle = Circle(center=Point(x=50, y=50), radius=25)
        
        layer.add_object(circle)
        
        assert len(layer.objects) == 1
        assert layer.objects[0] == circle
        assert circle.layer_id == layer.id
    
    def test_layer_remove_object(self):
        layer = Layer(name="Test")
        circle = Circle(center=Point(x=0, y=0), radius=10)
        rect = Rectangle(top_left=Point(x=0, y=0), width=50, height=50)
        
        layer.add_object(circle)
        layer.add_object(rect)
        
        removed = layer.remove_object(circle.id)
        assert removed is True
        assert len(layer.objects) == 1
        
        removed = layer.remove_object(circle.id)  # Try again
        assert removed is False
    
    def test_layer_get_object_by_id(self):
        layer = Layer(name="Test")
        circle = Circle(center=Point(x=0, y=0), radius=10)
        rect = Rectangle(top_left=Point(x=0, y=0), width=50, height=50)
        
        layer.add_object(circle)
        layer.add_object(rect)
        
        found = layer.get_object_by_id(circle.id)
        assert found == circle
        
        not_found = layer.get_object_by_id(UUID('00000000-0000-0000-0000-000000000000'))
        assert not_found is None
    
    def test_layer_get_object_in_group(self):
        layer = Layer(name="Test")
        group = Group(name="Group")
        circle = Circle(center=Point(x=0, y=0), radius=10)
        
        group.add_object(circle)
        layer.add_object(group)
        
        # Should find object inside group
        found = layer.get_object_by_id(circle.id)
        assert found == circle
    
    def test_layer_mixed_objects(self):
        layer = Layer(name="Mixed")
        
        circle = Circle(center=Point(x=50, y=50), radius=25)
        line = Line(start_point=Point(x=0, y=0), end_point=Point(x=100, y=100))
        text = Text(position=Point(x=10, y=10), content="Hello")
        group = Group(name="Grouped")
        
        layer.add_object(circle)
        layer.add_object(line)
        layer.add_object(text)
        layer.add_object(group)
        
        assert len(layer.objects) == 4
        assert isinstance(layer.objects[0], Circle)
        assert isinstance(layer.objects[1], Line)
        assert isinstance(layer.objects[2], Text)
        assert isinstance(layer.objects[3], Group)


class TestDrawing:
    def test_drawing_creation(self):
        drawing = Drawing()
        assert drawing.name == "Untitled Drawing"
        assert drawing.width == 800
        assert drawing.height == 600
        assert drawing.background_color.r == 255
        assert drawing.background_color.g == 255
        assert drawing.background_color.b == 255
        assert len(drawing.layers) == 0
    
    def test_drawing_with_properties(self):
        drawing = Drawing(
            name="My Drawing",
            width=1920,
            height=1080,
            background_color=Color(r=200, g=200, b=200),
            metadata={"author": "Test User", "version": "1.0"}
        )
        assert drawing.name == "My Drawing"
        assert drawing.width == 1920
        assert drawing.height == 1080
        assert drawing.background_color.r == 200
        assert drawing.metadata["author"] == "Test User"
    
    def test_drawing_add_layer(self):
        drawing = Drawing()
        layer1 = Layer(name="Layer 1")
        layer2 = Layer(name="Layer 2")
        
        drawing.add_layer(layer1)
        drawing.add_layer(layer2)
        
        assert len(drawing.layers) == 2
        assert drawing.layers[0] == layer1
        assert drawing.layers[1] == layer2
    
    def test_drawing_remove_layer(self):
        drawing = Drawing()
        layer1 = Layer(name="Layer 1")
        layer2 = Layer(name="Layer 2")
        
        drawing.add_layer(layer1)
        drawing.add_layer(layer2)
        
        removed = drawing.remove_layer(layer1.id)
        assert removed is True
        assert len(drawing.layers) == 1
        assert drawing.layers[0] == layer2
        
        removed = drawing.remove_layer(layer1.id)  # Try again
        assert removed is False
    
    def test_drawing_get_layer_by_id(self):
        drawing = Drawing()
        layer = Layer(name="Test Layer")
        drawing.add_layer(layer)
        
        found = drawing.get_layer_by_id(layer.id)
        assert found == layer
        
        not_found = drawing.get_layer_by_id(UUID('00000000-0000-0000-0000-000000000000'))
        assert not_found is None
    
    def test_drawing_get_all_objects(self):
        drawing = Drawing()
        
        layer1 = Layer(name="Layer 1")
        circle = Circle(center=Point(x=50, y=50), radius=25)
        rect = Rectangle(top_left=Point(x=0, y=0), width=100, height=100)
        layer1.add_object(circle)
        layer1.add_object(rect)
        
        layer2 = Layer(name="Layer 2")
        line = Line(start_point=Point(x=0, y=0), end_point=Point(x=100, y=100))
        layer2.add_object(line)
        
        drawing.add_layer(layer1)
        drawing.add_layer(layer2)
        
        all_objects = drawing.get_all_objects()
        assert len(all_objects) == 3
        assert circle in all_objects
        assert rect in all_objects
        assert line in all_objects
    
    def test_drawing_updated_timestamp(self):
        drawing = Drawing()
        initial_updated = drawing.updated_at
        
        layer = Layer(name="New Layer")
        drawing.add_layer(layer)
        
        assert drawing.updated_at > initial_updated
    
    def test_drawing_to_svg_basic(self):
        drawing = Drawing(width=200, height=100)
        svg = drawing.to_svg()
        
        assert 'width="200' in svg and 'height="100' in svg
        assert 'xmlns="http://www.w3.org/2000/svg"' in svg
        assert 'fill="#ffffff"' in svg
        assert '</svg>' in svg
    
    def test_drawing_validation(self):
        with pytest.raises(ValueError):
            Drawing(width=0, height=100)  # width must be > 0
        
        with pytest.raises(ValueError):
            Drawing(width=100, height=-10)  # height must be > 0
    
    def test_drawing_serialization(self):
        drawing = Drawing(name="Test")
        layer = Layer(name="Layer 1")
        circle = Circle(
            center=Point(x=50, y=50),
            radius=25,
            fill=FillStyle(color=Color(r=255, g=0, b=0))
        )
        layer.add_object(circle)
        drawing.add_layer(layer)
        
        # Convert to dict (JSON-serializable)
        drawing_dict = drawing.model_dump()
        
        assert drawing_dict["name"] == "Test"
        assert len(drawing_dict["layers"]) == 1
        assert drawing_dict["layers"][0]["name"] == "Layer 1"
        assert len(drawing_dict["layers"][0]["objects"]) == 1
        
        # Recreate from dict
        drawing2 = Drawing(**drawing_dict)
        assert drawing2.name == drawing.name
        assert len(drawing2.layers) == len(drawing.layers)


class TestIntegration:
    def test_complex_drawing(self):
        # Create a complex drawing with multiple layers and objects
        drawing = Drawing(name="Complex Drawing", width=1000, height=800)
        
        # Background layer
        bg_layer = Layer(name="Background", z_index=0)
        bg = Rectangle(
            top_left=Point(x=0, y=0),
            width=1000,
            height=800,
            fill=FillStyle(color=Color(r=240, g=240, b=240))
        )
        bg_layer.add_object(bg)
        
        # Shapes layer
        shapes_layer = Layer(name="Shapes", z_index=1)
        
        # Create a group of shapes
        shape_group = Group(name="Shape Group")
        for i in range(5):
            circle = Circle(
                center=Point(x=100 + i * 150, y=200),
                radius=40,
                fill=FillStyle(color=Color(r=255 - i * 50, g=i * 50, b=128))
            )
            shape_group.add_object(circle)
        
        shapes_layer.add_object(shape_group)
        
        # Text layer
        text_layer = Layer(name="Text", z_index=2)
        title = Text(
            position=Point(x=500, y=100),
            content="Complex Drawing Test",
            font_size=32,
            font_weight="bold"
        )
        text_layer.add_object(title)
        
        # Add all layers
        drawing.add_layer(bg_layer)
        drawing.add_layer(shapes_layer)
        drawing.add_layer(text_layer)
        
        # Verify structure
        assert len(drawing.layers) == 3
        assert len(drawing.get_all_objects()) == 3  # bg, group, title
        assert len(shape_group.objects) == 5
        
        # Test layer ordering
        sorted_layers = sorted(drawing.layers, key=lambda l: l.z_index)
        assert sorted_layers[0].name == "Background"
        assert sorted_layers[1].name == "Shapes"
        assert sorted_layers[2].name == "Text"