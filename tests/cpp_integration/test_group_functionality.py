#!/usr/bin/env python3
"""Test the Group object functionality."""

import drawing_cpp
from python.drawing_cpp_wrapper import DrawingCpp
import json


def test_group_basic():
    """Test basic group creation and operations."""
    print("Testing Group functionality...")

    # Create drawing
    drawing = DrawingCpp(800, 600)

    # Create some objects to group
    circle1 = drawing.add_circle(100, 100, 50, fill_color=(255, 0, 0))
    circle2 = drawing.add_circle(200, 100, 50, fill_color=(0, 255, 0))
    rect = drawing.add_rectangle(150, 150, 100, 50, fill_color=(0, 0, 255))

    print(f"Created objects: circle1={circle1}, circle2={circle2}, rect={rect}")

    # Create an empty group
    empty_group = drawing.add_group()
    print(f"Created empty group: ID={empty_group}")

    # Add objects to the group
    drawing.add_to_group(empty_group, circle1)
    drawing.add_to_group(empty_group, rect)
    print(f"Added circle1 and rect to group")

    # Create a group with initial children
    group_with_children = drawing.add_group([circle2])
    print(f"Created group with initial child: ID={group_with_children}")

    # Create nested groups
    nested_group = drawing.add_group()
    drawing.add_to_group(nested_group, empty_group)
    drawing.add_to_group(nested_group, group_with_children)
    print(f"Created nested group containing other groups: ID={nested_group}")

    print(f"\nTotal objects: {drawing.total_objects}")
    print(f"Memory usage: {drawing.memory_usage} bytes")
    print(f"Bytes per group: {drawing_cpp.BYTES_PER_GROUP}")

    # Test spatial queries - groups should have bounding boxes
    objects_in_area = drawing.find_objects_in_rect(50, 50, 250, 200)
    print(f"\nObjects in area (50,50)-(250,200): {objects_in_area}")

    # Test serialization
    print("\nTesting serialization...")
    drawing.save_binary("test_groups.bin")
    drawing.save_json("test_groups.json")

    # Load and verify
    loaded = DrawingCpp.load_binary("test_groups.bin")
    if loaded:
        print(f"Loaded drawing: {loaded.total_objects} objects")

        # Check JSON output
        with open("test_groups.json", "r") as f:
            json_data = json.load(f)
            print("\nJSON structure:")
            for layer in json_data["layers"]:
                for obj in layer["objects"]:
                    if obj.get("children") is not None:
                        print(f"  Group {obj['id']} has {len(obj['children'])} children")

    # Cleanup
    import os

    os.remove("test_groups.bin")
    os.remove("test_groups.json")

    print("\nGroup basic tests completed successfully!")


def test_group_operations():
    """Test group operations like setting colors."""
    print("\nTesting group operations...")

    drawing = DrawingCpp(800, 600)

    # Create objects
    objects = []
    for i in range(5):
        obj = drawing.add_circle(i * 100 + 50, 100, 30, fill_color=(128, 128, 128))
        objects.append(obj)

    # Group them
    group = drawing.add_group(objects)

    # Set group properties (group itself can have fill/stroke)
    drawing.set_fill_color([group], (255, 255, 0))
    drawing.set_opacity([group], 0.7)

    print(f"Created group with 5 circles")
    print(f"Set group fill color and opacity")

    # Create complex hierarchy
    subgroup1 = drawing.add_group(objects[:2])
    subgroup2 = drawing.add_group(objects[2:4])
    main_group = drawing.add_group([subgroup1, subgroup2, objects[4]])

    print(f"\nCreated hierarchy:")
    print(f"  Main group contains: 2 subgroups + 1 object")
    print(f"  Subgroup1 contains: 2 objects")
    print(f"  Subgroup2 contains: 2 objects")


def test_group_memory():
    """Test memory efficiency with many groups."""
    print("\nTesting group memory efficiency...")

    drawing = DrawingCpp(1000, 1000)

    # Create many groups with objects
    num_groups = 1000
    for i in range(num_groups):
        # Create 3 objects per group
        objs = []
        x = (i % 50) * 20
        y = (i // 50) * 20
        objs.append(drawing.add_circle(x, y, 5))
        objs.append(drawing.add_rectangle(x + 5, y, 5, 5))
        objs.append(drawing.add_line(x, y + 10, x + 10, y + 10))

        # Create group
        drawing.add_group(objs)

    total_objects = drawing.total_objects
    total_memory = drawing.memory_usage

    print(f"Created {num_groups} groups with 3 objects each")
    print(f"Total objects: {total_objects:,}")
    print(f"Total memory: {total_memory:,} bytes")
    print(f"Average per object: {total_memory/total_objects:.1f} bytes")
    print(f"Expected group size: {drawing_cpp.BYTES_PER_GROUP} bytes")


if __name__ == "__main__":
    test_group_basic()
    test_group_operations()
    test_group_memory()

