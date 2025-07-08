#!/usr/bin/env python3
"""Test the Path object functionality."""

import drawing_cpp
from python.drawing_cpp_wrapper import DrawingCpp
import json


def test_path_basic():
    """Test basic path creation and commands."""
    print("Testing Path functionality...")
    
    # Create drawing
    drawing = DrawingCpp(800, 600)
    
    # Test simple path with MoveTo and LineTo
    path1 = drawing.add_path("M 10 20 L 30 40", fill_color=(255, 0, 0))
    print(f"Created simple path: ID={path1}")
    
    # Test path with multiple commands
    path2 = drawing.add_path(
        "M 100 100 L 200 100 L 200 200 L 100 200 Z",
        fill_color=(0, 255, 0),
        stroke_color=(0, 0, 255)
    )
    print(f"Created square path: ID={path2}")
    
    # Test path with curves
    path3 = drawing.add_path(
        "M 300 300 C 350 250 450 250 500 300",
        stroke_color=(255, 0, 255),
        stroke_width=3
    )
    print(f"Created curved path: ID={path3}")
    
    # Test path with quadratic Bezier
    path4 = drawing.add_path(
        "M 50 400 Q 150 350 250 400",
        stroke_color=(255, 128, 0)
    )
    print(f"Created quadratic path: ID={path4}")
    
    # Test complex path with multiple segments
    complex_path = drawing.add_path(
        "M 400 50 L 450 50 C 500 50 550 100 550 150 L 550 200 Q 500 250 450 200 L 400 200 Z",
        fill_color=(128, 128, 255),
        stroke_color=(0, 0, 128)
    )
    print(f"Created complex path: ID={complex_path}")
    
    print(f"\nTotal objects: {drawing.total_objects}")
    print(f"Memory usage: {drawing.memory_usage} bytes")
    print(f"Bytes per path: {drawing_cpp.BYTES_PER_PATH}")
    
    # Test spatial queries
    objects_at_center = drawing.find_objects_at_point(200, 150, tolerance=60)
    print(f"\nObjects near (200, 150): {objects_at_center}")
    
    # Test serialization
    print("\nTesting serialization...")
    drawing.save_binary("test_path.bin")
    drawing.save_json("test_path.json")
    
    # Load and verify
    loaded = DrawingCpp.load_binary("test_path.bin")
    if loaded:
        print(f"Loaded drawing: {loaded.total_objects} objects")
        
        # Check JSON output
        with open("test_path.json", "r") as f:
            json_data = json.load(f)
            print("\nFirst path in JSON:")
            if json_data["layers"] and json_data["layers"][0]["objects"]:
                first_obj = json_data["layers"][0]["objects"][0]
                print(f"  Path data: {first_obj.get('d', 'N/A')}")
    
    # Test arc paths
    print("\nTesting arc paths...")
    arc_path = drawing.add_path(
        "M 100 500 A 50 50 0 0 1 200 500",
        stroke_color=(0, 255, 255)
    )
    print(f"Created arc path: ID={arc_path}")
    
    # Cleanup
    import os
    os.remove("test_path.bin")
    os.remove("test_path.json")
    
    print("\nPath tests completed successfully!")


def test_path_memory():
    """Test memory efficiency with many paths."""
    print("\nTesting path memory efficiency...")
    
    drawing = DrawingCpp(1000, 1000)
    
    # Create many simple paths
    num_paths = 10000
    for i in range(num_paths):
        x = (i % 100) * 10
        y = (i // 100) * 10
        drawing.add_path(f"M {x} {y} L {x+5} {y+5}", fill_color=(255, 0, 0))
    
    total_memory = drawing.memory_usage
    avg_memory = total_memory / num_paths
    
    print(f"Created {num_paths} paths")
    print(f"Total memory: {total_memory:,} bytes")
    print(f"Average per path: {avg_memory:.1f} bytes")
    print(f"Expected per path: {drawing_cpp.BYTES_PER_PATH} bytes (compact struct only)")


if __name__ == "__main__":
    test_path_basic()
    test_path_memory()