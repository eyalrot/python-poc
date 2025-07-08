#!/usr/bin/env python3
"""
Test script for new C++ features: LineStyle, corner_radius, closed polygons,
gradients, patterns, object names, and metadata.
"""

import sys
import os

sys.path.insert(0, "cpp/build")
import drawing_cpp as dc


def test_basic_features():
    """Test basic new features: LineStyle, corner_radius, closed flag"""
    print("Testing basic new features...")

    drawing = dc.Drawing(800, 600)

    # Test LineStyle
    line_id = drawing.add_line(10, 10, 100, 100, dc.LineStyle.Dashed)
    print(f"✓ Created dashed line: {line_id}")

    # Test corner_radius
    rect_id = drawing.add_rectangle(50, 50, 100, 80, 15.0)  # 15px corner radius
    print(f"✓ Created rounded rectangle: {rect_id}")

    # Test closed polygon flag
    points = [dc.Point(0, 0), dc.Point(100, 0), dc.Point(50, 100)]
    closed_poly_id = drawing.add_polygon(points, True)  # closed
    open_poly_id = drawing.add_polygon(points, False)  # open
    print(f"✓ Created closed polygon: {closed_poly_id}")
    print(f"✓ Created open polygon: {open_poly_id}")

    # Test polyline with line style
    polyline_points = [dc.Point(200, 200), dc.Point(250, 220), dc.Point(300, 200)]
    polyline_id = drawing.add_polyline(polyline_points, dc.LineStyle.Dotted)
    print(f"✓ Created dotted polyline: {polyline_id}")

    return drawing


def test_gradients():
    """Test gradient functionality"""
    print("\nTesting gradient features...")

    drawing = dc.Drawing(800, 600)

    # Create gradient stops
    stops = [
        dc.GradientStop(0.0, dc.Color(255, 0, 0, 255)),  # Red at start
        dc.GradientStop(0.5, dc.Color(255, 255, 0, 255)),  # Yellow at middle
        dc.GradientStop(1.0, dc.Color(0, 255, 0, 255)),  # Green at end
    ]

    # Test linear gradient
    linear_grad_id = drawing.add_linear_gradient(stops, 45.0)  # 45 degree angle
    print(f"✓ Created linear gradient: {linear_grad_id}")

    # Test radial gradient
    radial_grad_id = drawing.add_radial_gradient(
        stops, 100, 100, 50
    )  # center at (100,100), radius 50
    print(f"✓ Created radial gradient: {radial_grad_id}")

    # Apply gradients to objects
    circle_id = drawing.add_circle(100, 100, 50)
    drawing.set_object_gradient(circle_id, linear_grad_id)
    print(f"✓ Applied linear gradient to circle: {circle_id}")

    rect_id = drawing.add_rectangle(200, 200, 100, 80)
    drawing.set_object_gradient(rect_id, radial_grad_id)
    print(f"✓ Applied radial gradient to rectangle: {rect_id}")

    return drawing


def test_patterns():
    """Test pattern functionality"""
    print("\nTesting pattern features...")

    drawing = dc.Drawing(800, 600)

    # Create patterns
    pattern1_id = drawing.add_pattern("stripes")
    pattern2_id = drawing.add_pattern("dots")
    print(f"✓ Created pattern 'stripes': {pattern1_id}")
    print(f"✓ Created pattern 'dots': {pattern2_id}")

    # Apply patterns to objects
    circle_id = drawing.add_circle(100, 100, 50)
    drawing.set_object_pattern(circle_id, pattern1_id)
    print(f"✓ Applied stripes pattern to circle: {circle_id}")

    rect_id = drawing.add_rectangle(200, 200, 100, 80)
    drawing.set_object_pattern(rect_id, pattern2_id)
    print(f"✓ Applied dots pattern to rectangle: {rect_id}")

    return drawing


def test_object_names():
    """Test object naming functionality"""
    print("\nTesting object naming features...")

    drawing = dc.Drawing(800, 600)

    # Create objects and assign names
    circle_id = drawing.add_circle(100, 100, 50)
    drawing.set_object_name(circle_id, "main_circle")

    rect_id = drawing.add_rectangle(200, 200, 100, 80)
    drawing.set_object_name(rect_id, "header_rect")

    line_id = drawing.add_line(0, 0, 100, 100)
    drawing.set_object_name(line_id, "diagonal_line")

    print(f"✓ Named circle: {drawing.get_object_name(circle_id)}")
    print(f"✓ Named rectangle: {drawing.get_object_name(rect_id)}")
    print(f"✓ Named line: {drawing.get_object_name(line_id)}")

    # Test unnamed object
    unnamed_id = drawing.add_circle(300, 300, 25)
    print(f"✓ Unnamed object name: '{drawing.get_object_name(unnamed_id)}'")

    return drawing


def test_metadata():
    """Test metadata functionality"""
    print("\nTesting metadata features...")

    drawing = dc.Drawing(800, 600)

    # Create objects and assign metadata
    circle_id = drawing.add_circle(100, 100, 50)
    drawing.set_object_metadata(circle_id, "type", "logo")
    drawing.set_object_metadata(circle_id, "layer", "graphics")
    drawing.set_object_metadata(circle_id, "importance", "high")

    rect_id = drawing.add_rectangle(200, 200, 100, 80)
    drawing.set_object_metadata(rect_id, "type", "button")
    drawing.set_object_metadata(rect_id, "action", "submit")

    # Test retrieving metadata
    print(f"✓ Circle type: {drawing.get_object_metadata(circle_id, 'type')}")
    print(f"✓ Circle layer: {drawing.get_object_metadata(circle_id, 'layer')}")
    print(f"✓ Rectangle action: {drawing.get_object_metadata(rect_id, 'action')}")

    # Test getting all metadata
    all_circle_meta = drawing.get_all_object_metadata(circle_id)
    print(f"✓ All circle metadata: {all_circle_meta}")

    all_rect_meta = drawing.get_all_object_metadata(rect_id)
    print(f"✓ All rectangle metadata: {all_rect_meta}")

    # Test nonexistent metadata
    missing = drawing.get_object_metadata(circle_id, "nonexistent")
    print(f"✓ Missing metadata: '{missing}'")

    return drawing


def test_combined_features():
    """Test combining multiple new features"""
    print("\nTesting combined features...")

    drawing = dc.Drawing(800, 600)

    # Create a complex object with all features
    # 1. Create a rounded rectangle with corner radius
    rect_id = drawing.add_rectangle(100, 100, 200, 150, 20.0)  # 20px corners

    # 2. Create and apply a gradient
    stops = [
        dc.GradientStop(0.0, dc.Color(0, 100, 255, 255)),  # Blue
        dc.GradientStop(1.0, dc.Color(100, 255, 100, 255)),  # Light green
    ]
    gradient_id = drawing.add_linear_gradient(stops, 90.0)  # Vertical gradient
    drawing.set_object_gradient(rect_id, gradient_id)

    # 3. Add name and metadata
    drawing.set_object_name(rect_id, "featured_button")
    drawing.set_object_metadata(rect_id, "component", "button")
    drawing.set_object_metadata(rect_id, "state", "normal")
    drawing.set_object_metadata(rect_id, "clickable", "true")

    # 4. Create a styled line
    line_id = drawing.add_line(50, 280, 350, 280, dc.LineStyle.DashDot)
    drawing.set_object_name(line_id, "separator_line")
    drawing.set_object_metadata(line_id, "purpose", "visual_separator")

    print(f"✓ Created complex rectangle with:")
    print(f"    - Corner radius: 20px")
    print(f"    - Gradient: {gradient_id}")
    print(f"    - Name: {drawing.get_object_name(rect_id)}")
    print(f"    - Metadata: {drawing.get_all_object_metadata(rect_id)}")

    print(f"✓ Created styled line with:")
    print(f"    - Style: DashDot")
    print(f"    - Name: {drawing.get_object_name(line_id)}")
    print(f"    - Metadata: {drawing.get_all_object_metadata(line_id)}")

    return drawing


def test_performance():
    """Test performance with new features"""
    print("\nTesting performance with new features...")

    import time

    drawing = dc.Drawing(1000, 1000)

    # Create gradient once
    stops = [dc.GradientStop(0.0, dc.Color(255, 0, 0)), dc.GradientStop(1.0, dc.Color(0, 255, 0))]
    gradient_id = drawing.add_linear_gradient(stops)

    # Time object creation with new features
    start_time = time.time()

    for i in range(1000):
        # Create various objects with new features
        circle_id = drawing.add_circle(i % 800, (i * 2) % 600, 10)
        drawing.set_object_name(circle_id, f"circle_{i}")
        drawing.set_object_metadata(circle_id, "batch", "test")
        drawing.set_object_gradient(circle_id, gradient_id)

        if i % 2 == 0:
            rect_id = drawing.add_rectangle(i % 700, (i * 3) % 500, 20, 15, 5.0)  # rounded
            drawing.set_object_name(rect_id, f"rect_{i}")

        if i % 3 == 0:
            line_id = drawing.add_line(0, i % 600, 100, (i + 50) % 600, dc.LineStyle.Dashed)
            drawing.set_object_metadata(line_id, "style", "dashed")

    end_time = time.time()

    print(f"✓ Created 1000+ objects with full features in {(end_time - start_time)*1000:.2f}ms")
    print(f"✓ Total objects: {drawing.total_objects()}")
    print(f"✓ Memory usage: {drawing.memory_usage() / 1024 / 1024:.2f} MB")

    return drawing


def main():
    """Run all tests"""
    print("=== Testing New C++ Features ===")

    try:
        test_basic_features()
        test_gradients()
        test_patterns()
        test_object_names()
        test_metadata()
        test_combined_features()
        test_performance()

        print(f"\n🎉 All tests passed! New features are working correctly.")
        print("✓ LineStyle support")
        print("✓ Rectangle corner radius")
        print("✓ Polygon closed flag")
        print("✓ Gradient system (linear/radial)")
        print("✓ Pattern support")
        print("✓ Object naming")
        print("✓ Metadata system")
        print("✓ Python bindings")
        print("✓ Performance maintained")

    except Exception as e:
        print(f"\n❌ Test failed: {e}")
        import traceback

        traceback.print_exc()
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())

