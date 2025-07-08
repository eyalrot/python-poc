#!/usr/bin/env python3
"""
Benchmark spatial query operations.
"""

import time
import random
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, 'cpp')

try:
    import drawing_cpp
    from python.drawing_cpp_wrapper import DrawingCpp
    CPP_AVAILABLE = True
except ImportError:
    print("Error: C++ bindings not available. Please build them first:")
    print("  cd cpp")
    print("  python setup.py build_ext --inplace")
    sys.exit(1)


class SpatialBenchmark:
    """Benchmark spatial operations."""
    
    def __init__(self):
        self.results = []
    
    def create_test_scene(self, n_objects, width=5000, height=5000):
        """Create a scene with randomly distributed objects."""
        drawing = DrawingCpp(width, height)
        
        # Mix of different object types
        for i in range(n_objects):
            obj_type = i % 3
            x = random.uniform(0, width)
            y = random.uniform(0, height)
            
            if obj_type == 0:
                drawing.add_circle(x, y, random.uniform(5, 50))
            elif obj_type == 1:
                w = random.uniform(10, 100)
                h = random.uniform(10, 100)
                drawing.add_rectangle(x, y, w, h)
            else:
                x2 = x + random.uniform(-200, 200)
                y2 = y + random.uniform(-200, 200)
                drawing.add_line(x, y, x2, y2)
        
        return drawing
    
    def benchmark_point_queries(self, sizes):
        """Benchmark finding objects at specific points."""
        print("\n📍 Point Query Benchmark")
        print("-" * 60)
        
        for n in sizes:
            drawing = self.create_test_scene(n)
            
            # Generate test points
            test_points = [(random.uniform(0, 5000), random.uniform(0, 5000)) 
                          for _ in range(100)]
            
            # Time queries
            start = time.perf_counter()
            total_found = 0
            for x, y in test_points:
                found = drawing.find_objects_at_point(x, y, tolerance=5.0)
                total_found += len(found)
            elapsed = (time.perf_counter() - start) * 1000
            
            avg_time = elapsed / len(test_points)
            queries_per_sec = len(test_points) / (elapsed / 1000) if elapsed > 0 else 0
            
            print(f"  Scene with {n:,} objects:")
            print(f"    100 point queries: {elapsed:.2f} ms total")
            print(f"    Average per query: {avg_time:.3f} ms")
            print(f"    Queries/second: {queries_per_sec:.0f}")
            print(f"    Objects found: {total_found}")
    
    def benchmark_rect_queries(self, sizes):
        """Benchmark finding objects in rectangles."""
        print("\n📦 Rectangle Query Benchmark")
        print("-" * 60)
        
        for n in sizes:
            drawing = self.create_test_scene(n)
            
            # Generate test rectangles of various sizes
            test_rects = []
            for _ in range(50):
                x1 = random.uniform(0, 4500)
                y1 = random.uniform(0, 4500)
                w = random.uniform(100, 500)
                h = random.uniform(100, 500)
                test_rects.append((x1, y1, x1 + w, y1 + h))
            
            # Time queries
            start = time.perf_counter()
            total_found = 0
            for x1, y1, x2, y2 in test_rects:
                found = drawing.find_objects_in_rect(x1, y1, x2, y2)
                total_found += len(found)
            elapsed = (time.perf_counter() - start) * 1000
            
            avg_time = elapsed / len(test_rects)
            queries_per_sec = len(test_rects) / (elapsed / 1000) if elapsed > 0 else 0
            
            print(f"  Scene with {n:,} objects:")
            print(f"    50 rect queries: {elapsed:.2f} ms total")
            print(f"    Average per query: {avg_time:.3f} ms")
            print(f"    Queries/second: {queries_per_sec:.0f}")
            print(f"    Objects found: {total_found}")
    
    def benchmark_collision_detection(self, sizes):
        """Benchmark collision detection between objects."""
        print("\n💥 Collision Detection Benchmark")
        print("-" * 60)
        
        for n in sizes:
            # Create dense scene for more collisions
            drawing = DrawingCpp(1000, 1000)
            ids = []
            
            # Create grid of circles
            grid_size = int(n ** 0.5)
            spacing = 1000 / grid_size
            
            for i in range(grid_size):
                for j in range(grid_size):
                    if len(ids) < n:
                        x = i * spacing + spacing / 2
                        y = j * spacing + spacing / 2
                        ids.append(drawing.add_circle(x, y, spacing * 0.6))
            
            # Time collision detection (simplified - just bounding box overlap)
            start = time.perf_counter()
            collisions = 0
            storage = drawing._drawing.get_storage()
            
            # Check each object against objects in its vicinity
            for i, id1 in enumerate(ids[:100]):  # Check first 100 objects
                # Get bounding box
                bbox1 = drawing_cpp.BatchOperations.calculate_bounding_box(storage, [id1])
                
                # Expand slightly for search area
                search_rect = drawing_cpp.BoundingBox(
                    bbox1.min_x - 10, bbox1.min_y - 10,
                    bbox1.max_x + 10, bbox1.max_y + 10
                )
                
                # Find nearby objects
                nearby = storage.find_in_rect(search_rect)
                collisions += len(nearby) - 1  # Exclude self
            
            elapsed = (time.perf_counter() - start) * 1000
            
            print(f"  Scene with {n:,} objects (dense grid):")
            print(f"    Checked 100 objects: {elapsed:.2f} ms")
            print(f"    Average per object: {elapsed/100:.3f} ms")
            print(f"    Potential collisions: {collisions}")
    
    def run_all(self):
        """Run all spatial benchmarks."""
        print("🗺️  Spatial Operations Benchmark")
        print("=" * 60)
        
        sizes = [1000, 10000, 100000]
        
        self.benchmark_point_queries(sizes)
        self.benchmark_rect_queries(sizes)
        self.benchmark_collision_detection([100, 1000, 10000])
        
        print("\n" + "=" * 60)
        print("✅ Spatial benchmarking complete!")
        print("\n💡 Note: Performance would improve significantly with R-tree indexing")


def main():
    benchmark = SpatialBenchmark()
    benchmark.run_all()


if __name__ == "__main__":
    main()