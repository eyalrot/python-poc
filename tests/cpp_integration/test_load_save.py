"""Test saving and loading drawing from disk"""

import json
from pathlib import Path
from python.data import Drawing, Layer, Circle, Point, Color, FillStyle


def test_save_and_load():
    # Create a simple drawing
    drawing = Drawing(name="Test Drawing", width=400, height=300)
    layer = Layer(name="Test Layer")

    circle = Circle(
        center=Point(x=200, y=150), radius=50, fill=FillStyle(color=Color(r=255, g=0, b=0))
    )

    layer.add_object(circle)
    drawing.add_layer(layer)

    # Save to JSON
    output_dir = Path("test_output")
    output_dir.mkdir(exist_ok=True)

    json_path = output_dir / "test_drawing.json"
    with open(json_path, "w") as f:
        json.dump(drawing.model_dump(), f, indent=2, default=str)

    print(f"Saved drawing to: {json_path}")

    # Load from JSON
    with open(json_path, "r") as f:
        data = json.load(f)

    # Create Drawing object from loaded data
    loaded_drawing = Drawing.model_validate(data)

    print(f"\nLoaded drawing: {loaded_drawing.name}")
    print(f"Size: {loaded_drawing.width}x{loaded_drawing.height}")
    print(f"Layers: {len(loaded_drawing.layers)}")
    print(f"Objects in first layer: {len(loaded_drawing.layers[0].objects)}")

    # Verify the loaded data
    assert loaded_drawing.name == drawing.name
    assert loaded_drawing.width == drawing.width
    assert loaded_drawing.height == drawing.height
    assert len(loaded_drawing.layers) == len(drawing.layers)

    print("\n✓ Save and load test passed!")

    # Clean up
    json_path.unlink()
    output_dir.rmdir()


if __name__ == "__main__":
    test_save_and_load()

