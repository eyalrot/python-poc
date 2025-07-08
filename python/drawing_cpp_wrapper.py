"""
Python wrapper for the C++ drawing library.
Provides compatibility layer with the existing Pydantic models.
"""

import drawing_cpp
from typing import Optional, Tuple, List


class DrawingCpp:
    """High-performance drawing using C++ backend."""
    
    def __init__(self, width: float = 800, height: float = 600):
        self._drawing = drawing_cpp.Drawing(width, height)
        self._default_layer_id = 0
    
    @property
    def width(self) -> float:
        return self._drawing.get_width()
    
    @width.setter
    def width(self, value: float):
        self._drawing.set_width(value)
    
    @property
    def height(self) -> float:
        return self._drawing.get_height()
    
    @height.setter  
    def height(self, value: float):
        self._drawing.set_height(value)
    
    @property
    def background_color(self) -> Tuple[int, int, int, int]:
        c = self._drawing.get_background()
        return (c.r, c.g, c.b, c.a)
    
    @background_color.setter
    def background_color(self, color: Tuple[int, int, int, int]):
        if len(color) == 3:
            self._drawing.set_background(drawing_cpp.Color(*color))
        else:
            self._drawing.set_background(drawing_cpp.Color(*color))
    
    def add_layer(self, name: str = "") -> int:
        """Add a new layer and return its ID."""
        return self._drawing.add_layer(name)
    
    def add_circle(self, x: float, y: float, radius: float, 
                   fill_color: Optional[Tuple[int, int, int]] = None,
                   stroke_color: Optional[Tuple[int, int, int]] = None,
                   stroke_width: float = 1.0,
                   layer_id: Optional[int] = None) -> int:
        """Add a circle to the drawing."""
        if layer_id is None:
            layer_id = self._default_layer_id
            
        obj_id = self._drawing.add_circle(x, y, radius, layer_id)
        
        # Set colors if provided
        if fill_color or stroke_color:
            storage = self._drawing.get_storage()
            if fill_color:
                color = drawing_cpp.Color(*fill_color)
                storage.set_fill_color([obj_id], color)
            if stroke_color:
                color = drawing_cpp.Color(*stroke_color)
                storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def add_rectangle(self, x: float, y: float, width: float, height: float,
                      fill_color: Optional[Tuple[int, int, int]] = None,
                      stroke_color: Optional[Tuple[int, int, int]] = None,
                      stroke_width: float = 1.0,
                      layer_id: Optional[int] = None) -> int:
        """Add a rectangle to the drawing."""
        if layer_id is None:
            layer_id = self._default_layer_id
            
        obj_id = self._drawing.add_rectangle(x, y, width, height, layer_id)
        
        # Set colors if provided
        if fill_color or stroke_color:
            storage = self._drawing.get_storage()
            if fill_color:
                color = drawing_cpp.Color(*fill_color)
                storage.set_fill_color([obj_id], color)
            if stroke_color:
                color = drawing_cpp.Color(*stroke_color)
                storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def add_line(self, x1: float, y1: float, x2: float, y2: float,
                 stroke_color: Optional[Tuple[int, int, int]] = None,
                 stroke_width: float = 1.0,
                 layer_id: Optional[int] = None) -> int:
        """Add a line to the drawing."""
        if layer_id is None:
            layer_id = self._default_layer_id
            
        obj_id = self._drawing.add_line(x1, y1, x2, y2, layer_id)
        
        # Set stroke color if provided
        if stroke_color:
            storage = self._drawing.get_storage()
            color = drawing_cpp.Color(*stroke_color)
            storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def add_polygon(self, points: List[Tuple[float, float]],
                    fill_color: Optional[Tuple[int, int, int]] = None,
                    stroke_color: Optional[Tuple[int, int, int]] = None,
                    stroke_width: float = 1.0,
                    layer_id: Optional[int] = None) -> int:
        """Add a polygon to the drawing."""
        if layer_id is None:
            layer_id = self._default_layer_id
            
        # Convert points to drawing_cpp.Point objects
        cpp_points = [drawing_cpp.Point(x, y) for x, y in points]
        obj_id = self._drawing.add_polygon(cpp_points, layer_id)
        
        # Set colors if provided
        if fill_color or stroke_color:
            storage = self._drawing.get_storage()
            if fill_color:
                color = drawing_cpp.Color(*fill_color)
                storage.set_fill_color([obj_id], color)
            if stroke_color:
                color = drawing_cpp.Color(*stroke_color)
                storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def add_ellipse(self, x: float, y: float, rx: float, ry: float,
                    rotation: float = 0,
                    fill_color: Optional[Tuple[int, int, int]] = None,
                    stroke_color: Optional[Tuple[int, int, int]] = None,
                    stroke_width: float = 1.0,
                    layer_id: Optional[int] = None) -> int:
        """Add an ellipse to the drawing.
        
        Args:
            x, y: Center position
            rx, ry: Radii in x and y directions
            rotation: Rotation angle in radians
            fill_color: Optional fill color as (r, g, b)
            stroke_color: Optional stroke color as (r, g, b)
            stroke_width: Stroke width (not used yet)
            layer_id: Optional layer ID
        """
        if layer_id is None:
            layer_id = self._default_layer_id
            
        obj_id = self._drawing.add_ellipse(x, y, rx, ry, rotation, layer_id)
        
        # Set colors if provided
        if fill_color or stroke_color:
            storage = self._drawing.get_storage()
            if fill_color:
                color = drawing_cpp.Color(*fill_color)
                storage.set_fill_color([obj_id], color)
            if stroke_color:
                color = drawing_cpp.Color(*stroke_color)
                storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def add_polyline(self, points: List[Tuple[float, float]],
                     stroke_color: Optional[Tuple[int, int, int]] = None,
                     stroke_width: float = 1.0,
                     layer_id: Optional[int] = None) -> int:
        """Add a polyline (open path) to the drawing.
        
        Args:
            points: List of (x, y) coordinate tuples
            stroke_color: Optional stroke color as (r, g, b)
            stroke_width: Stroke width (not used yet)
            layer_id: Optional layer ID
        """
        if layer_id is None:
            layer_id = self._default_layer_id
            
        # Convert points to drawing_cpp.Point objects
        cpp_points = [drawing_cpp.Point(x, y) for x, y in points]
        obj_id = self._drawing.add_polyline(cpp_points, layer_id)
        
        # Set stroke color if provided (polylines typically don't have fill)
        if stroke_color:
            storage = self._drawing.get_storage()
            color = drawing_cpp.Color(*stroke_color)
            storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def add_arc(self, x: float, y: float, radius: float,
                start_angle: float, end_angle: float,
                fill_color: Optional[Tuple[int, int, int]] = None,
                stroke_color: Optional[Tuple[int, int, int]] = None,
                stroke_width: float = 1.0,
                layer_id: Optional[int] = None) -> int:
        """Add an arc to the drawing.
        
        Args:
            x, y: Center position
            radius: Arc radius
            start_angle: Starting angle in radians
            end_angle: Ending angle in radians
            fill_color: Optional fill color as (r, g, b)
            stroke_color: Optional stroke color as (r, g, b)
            stroke_width: Stroke width (not used yet)
            layer_id: Optional layer ID
        """
        if layer_id is None:
            layer_id = self._default_layer_id
            
        obj_id = self._drawing.add_arc(x, y, radius, start_angle, end_angle, layer_id)
        
        # Set colors if provided
        if fill_color or stroke_color:
            storage = self._drawing.get_storage()
            if fill_color:
                color = drawing_cpp.Color(*fill_color)
                storage.set_fill_color([obj_id], color)
            if stroke_color:
                color = drawing_cpp.Color(*stroke_color)
                storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def add_text(self, x: float, y: float, text: str,
                 font_size: float = 16.0,
                 font_family: str = "Arial",
                 text_align: str = "left",
                 text_baseline: str = "alphabetic",
                 fill_color: Optional[Tuple[int, int, int]] = None,
                 stroke_color: Optional[Tuple[int, int, int]] = None,
                 stroke_width: float = 1.0,
                 layer_id: Optional[int] = None) -> int:
        """Add text to the drawing.
        
        Args:
            x, y: Text position
            text: Text string to display
            font_size: Font size in pixels
            font_family: Font family name
            text_align: Horizontal alignment ("left", "center", "right")
            text_baseline: Vertical alignment ("top", "middle", "bottom", "alphabetic")
            fill_color: Optional fill color as (r, g, b)
            stroke_color: Optional stroke color as (r, g, b)
            stroke_width: Stroke width (not used yet)
            layer_id: Optional layer ID
        """
        if layer_id is None:
            layer_id = self._default_layer_id
        
        # Convert string alignment to enum
        align_map = {
            "left": drawing_cpp.TextAlign.Left,
            "center": drawing_cpp.TextAlign.Center,
            "right": drawing_cpp.TextAlign.Right
        }
        align = align_map.get(text_align.lower(), drawing_cpp.TextAlign.Left)
        
        baseline_map = {
            "top": drawing_cpp.TextBaseline.Top,
            "middle": drawing_cpp.TextBaseline.Middle,
            "bottom": drawing_cpp.TextBaseline.Bottom,
            "alphabetic": drawing_cpp.TextBaseline.Alphabetic
        }
        baseline = baseline_map.get(text_baseline.lower(), drawing_cpp.TextBaseline.Alphabetic)
        
        obj_id = self._drawing.add_text(x, y, text, font_size, font_family, 
                                        align, baseline, layer_id)
        
        # Set colors if provided
        if fill_color or stroke_color:
            storage = self._drawing.get_storage()
            if fill_color:
                color = drawing_cpp.Color(*fill_color)
                storage.set_fill_color([obj_id], color)
            if stroke_color:
                color = drawing_cpp.Color(*stroke_color)
                storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def add_path(self, path_data: str,
                 fill_color: Optional[Tuple[int, int, int]] = None,
                 stroke_color: Optional[Tuple[int, int, int]] = None,
                 stroke_width: float = 1.0,
                 layer_id: Optional[int] = None) -> int:
        """Add a path to the drawing using SVG path syntax.
        
        Args:
            path_data: SVG path string (e.g., "M 10 10 L 20 20 C 30 30 40 40 50 50 Z")
            fill_color: Optional fill color as (r, g, b)
            stroke_color: Optional stroke color as (r, g, b)
            stroke_width: Stroke width (not used yet)
            layer_id: Optional layer ID
            
        Supported commands:
            M/m: MoveTo
            L/l: LineTo
            C/c: CurveTo (cubic Bezier)
            Q/q: QuadTo (quadratic Bezier)
            A/a: ArcTo
            Z/z: Close path
        """
        if layer_id is None:
            layer_id = self._default_layer_id
            
        obj_id = self._drawing.add_path(path_data, layer_id)
        
        # Set colors if provided
        if fill_color or stroke_color:
            storage = self._drawing.get_storage()
            if fill_color:
                color = drawing_cpp.Color(*fill_color)
                storage.set_fill_color([obj_id], color)
            if stroke_color:
                color = drawing_cpp.Color(*stroke_color)
                storage.set_stroke_color([obj_id], color)
                
        return obj_id
    
    def find_objects_at_point(self, x: float, y: float, tolerance: float = 1.0) -> List[int]:
        """Find objects at the given point."""
        storage = self._drawing.get_storage()
        point = drawing_cpp.Point(x, y)
        return storage.find_at_point(point, tolerance)
    
    def find_objects_in_rect(self, x1: float, y1: float, x2: float, y2: float) -> List[int]:
        """Find objects within the given rectangle."""
        bbox = drawing_cpp.BoundingBox(x1, y1, x2, y2)
        return self._drawing.find_objects_in_rect(bbox)
    
    def set_fill_color(self, object_ids: List[int], color: Tuple[int, int, int]):
        """Set fill color for multiple objects."""
        storage = self._drawing.get_storage()
        cpp_color = drawing_cpp.Color(*color)
        storage.set_fill_color(object_ids, cpp_color)
    
    def set_stroke_color(self, object_ids: List[int], color: Tuple[int, int, int]):
        """Set stroke color for multiple objects."""
        storage = self._drawing.get_storage()
        cpp_color = drawing_cpp.Color(*color)
        storage.set_stroke_color(object_ids, cpp_color)
    
    def set_opacity(self, object_ids: List[int], opacity: float):
        """Set opacity for multiple objects."""
        storage = self._drawing.get_storage()
        storage.set_opacity(object_ids, opacity)
    
    @property
    def total_objects(self) -> int:
        """Get total number of objects."""
        return self._drawing.total_objects()
    
    @property
    def memory_usage(self) -> int:
        """Get memory usage in bytes."""
        return self._drawing.memory_usage()
    
    def save_binary(self, filename: str) -> bool:
        """Save drawing in compact binary format."""
        return drawing_cpp.save_binary(self._drawing, filename)
    
    def save_json(self, filename: str):
        """Save drawing in JSON format (compatible with Python version)."""
        drawing_cpp.save_json(self._drawing, filename)
    
    @classmethod
    def load_binary(cls, filename: str) -> Optional['DrawingCpp']:
        """Load drawing from binary format."""
        cpp_drawing = drawing_cpp.load_binary(filename)
        if cpp_drawing:
            wrapper = cls()
            wrapper._drawing = cpp_drawing
            return wrapper
        return None
    
    def get_bounding_box(self) -> Tuple[float, float, float, float]:
        """Get bounding box of all objects."""
        bbox = self._drawing.get_bounding_box()
        return (bbox.min_x, bbox.min_y, bbox.max_x, bbox.max_y)
    
    def __repr__(self):
        return f"DrawingCpp(width={self.width}, height={self.height}, objects={self.total_objects})"


def compare_performance():
    """Compare performance between Python and C++ implementations."""
    import time
    import sys
    
    print("Performance Comparison: Python vs C++")
    print("=====================================")
    print(f"Size of Python Circle object: ~800 bytes (estimated)")
    print(f"Size of C++ Circle object: {drawing_cpp.BYTES_PER_CIRCLE} bytes")
    print(f"Memory reduction: {800 / drawing_cpp.BYTES_PER_CIRCLE:.1f}x\n")
    
    # Test object creation
    num_objects = 100000
    
    # C++ version
    start = time.time()
    cpp_drawing = DrawingCpp(1000, 1000)
    for i in range(num_objects):
        cpp_drawing.add_circle(i % 1000, i // 1000, 5)
    cpp_time = time.time() - start
    cpp_memory = cpp_drawing.memory_usage
    
    print(f"C++ Performance ({num_objects} circles):")
    print(f"  Creation time: {cpp_time*1000:.1f} ms")
    print(f"  Memory usage: {cpp_memory / 1024 / 1024:.1f} MB")
    print(f"  Per object: {cpp_memory / num_objects:.1f} bytes")
    print(f"  Objects/second: {num_objects / cpp_time:,.0f}")
    
    # Test serialization
    start = time.time()
    cpp_drawing.save_binary("test_cpp.bin")
    save_time = time.time() - start
    
    import os
    file_size = os.path.getsize("test_cpp.bin")
    
    print(f"\nSerialization:")
    print(f"  Save time: {save_time*1000:.1f} ms")
    print(f"  File size: {file_size / 1024 / 1024:.1f} MB")
    print(f"  Bytes/object: {file_size / num_objects:.1f}")
    
    # Test loading
    start = time.time()
    loaded = DrawingCpp.load_binary("test_cpp.bin")
    load_time = time.time() - start
    
    print(f"  Load time: {load_time*1000:.1f} ms")
    print(f"  Throughput: {num_objects / (save_time + load_time):,.0f} objects/second")
    
    # Cleanup
    os.remove("test_cpp.bin")


if __name__ == "__main__":
    compare_performance()