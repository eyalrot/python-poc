#!/usr/bin/env python3
"""Test the C++ Python bindings."""

import sys
sys.path.insert(0, 'cpp')  # Add cpp directory to path

import drawing_cpp
from python.drawing_cpp_wrapper import DrawingCpp, compare_performance

def test_basic_functionality():
    """Test basic C++ drawing functionality."""
    print("Testing C++ Drawing Library")
    print("===========================")
    
    # Create drawing
    drawing = DrawingCpp(800, 600)
    print(f"Created: {drawing}")
    
    # Add objects
    circle_id = drawing.add_circle(100, 100, 50, fill_color=(255, 0, 0))
    rect_id = drawing.add_rectangle(200, 200, 100, 80, fill_color=(0, 255, 0))
    line_id = drawing.add_line(50, 50, 350, 350, stroke_color=(0, 0, 255))
    
    print(f"Added 3 objects. Total: {drawing.total_objects}")
    print(f"Memory usage: {drawing.memory_usage} bytes")
    
    # Test spatial queries
    objects_at_point = drawing.find_objects_at_point(100, 100, tolerance=5)
    print(f"Objects at (100, 100): {objects_at_point}")
    
    objects_in_rect = drawing.find_objects_in_rect(0, 0, 200, 200)
    print(f"Objects in rect (0,0,200,200): {objects_in_rect}")
    
    # Test batch operations
    drawing.set_opacity([circle_id, rect_id], 0.5)
    print("Set opacity to 0.5 for circle and rectangle")
    
    # Get bounding box
    bbox = drawing.get_bounding_box()
    print(f"Drawing bounding box: {bbox}")
    
    # Test serialization
    print("\nTesting Serialization:")
    drawing.save_binary("test_cpp.bin")
    drawing.save_json("test_cpp.json")
    
    import os
    bin_size = os.path.getsize("test_cpp.bin")
    json_size = os.path.getsize("test_cpp.json")
    
    print(f"Binary file size: {bin_size} bytes")
    print(f"JSON file size: {json_size} bytes")
    print(f"JSON is {json_size/bin_size:.1f}x larger than binary")
    
    # Test loading
    loaded = DrawingCpp.load_binary("test_cpp.bin")
    if loaded:
        print(f"Successfully loaded: {loaded}")
    
    # Cleanup
    os.remove("test_cpp.bin")
    os.remove("test_cpp.json")
    
    print("\nSize information:")
    sizes = drawing_cpp.get_compact_sizes()
    for name, size in sizes.items():
        print(f"  {name}: {size} bytes")


if __name__ == "__main__":
    test_basic_functionality()
    print("\n" + "="*50 + "\n")
    compare_performance()