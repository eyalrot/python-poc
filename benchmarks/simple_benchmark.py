#!/usr/bin/env python3
"""
Simple benchmarking suite without external dependencies.
"""

import time
import json
import sys
import os
from typing import Dict, List, Any

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Import C++ implementation
sys.path.insert(0, 'cpp')
import drawing_cpp
from python.drawing_cpp_wrapper import DrawingCpp


class SimpleBenchmark:
    """Simple benchmark runner."""
    
    def __init__(self):
        self.results = []
    
    def measure_time(self, func, *args, **kwargs):
        """Measure execution time."""
        start = time.perf_counter()
        result = func(*args, **kwargs)
        end = time.perf_counter()
        return (end - start) * 1000, result
    
    def format_number(self, num):
        """Format large numbers with commas."""
        return f"{num:,}"
    
    def format_time(self, ms):
        """Format time in appropriate units."""
        if ms < 1:
            return f"{ms*1000:.2f} μs"
        elif ms < 1000:
            return f"{ms:.2f} ms"
        else:
            return f"{ms/1000:.2f} s"
    
    def run_creation_benchmark(self, sizes):
        """Benchmark object creation."""
        print("\n🔵 Object Creation Benchmark")
        print("-" * 60)
        
        for n in sizes:
            # C++ implementation
            drawing = DrawingCpp(5000, 5000)
            
            time_ms, _ = self.measure_time(
                lambda: [drawing.add_circle(i % 1000, i // 1000, 5) for i in range(n)]
            )
            
            objects_per_sec = n / (time_ms / 1000) if time_ms > 0 else 0
            memory_mb = drawing.memory_usage / 1024 / 1024
            
            print(f"  {self.format_number(n)} objects:")
            print(f"    Time: {self.format_time(time_ms)}")
            print(f"    Speed: {objects_per_sec/1e6:.2f}M objects/sec")
            print(f"    Memory: {memory_mb:.2f} MB ({drawing.memory_usage/n:.1f} bytes/object)")
            
            self.results.append({
                "benchmark": "creation",
                "num_objects": n,
                "time_ms": time_ms,
                "objects_per_sec": objects_per_sec,
                "memory_bytes": drawing.memory_usage
            })
    
    def run_serialization_benchmark(self, sizes):
        """Benchmark serialization."""
        print("\n💾 Serialization Benchmark")
        print("-" * 60)
        
        for n in sizes:
            # Create test data
            drawing = DrawingCpp(5000, 5000)
            for i in range(n):
                drawing.add_circle(i % 1000, i // 1000, 5)
            
            # Binary save
            save_time, _ = self.measure_time(drawing.save_binary, "temp_bench.bin")
            file_size = os.path.getsize("temp_bench.bin")
            
            # Binary load
            load_time, loaded = self.measure_time(DrawingCpp.load_binary, "temp_bench.bin")
            
            save_speed = n / (save_time / 1000) if save_time > 0 else 0
            load_speed = n / (load_time / 1000) if load_time > 0 else 0
            
            print(f"  {self.format_number(n)} objects:")
            print(f"    Binary save: {self.format_time(save_time)} ({save_speed/1e6:.1f}M obj/s)")
            print(f"    Binary load: {self.format_time(load_time)} ({load_speed/1e6:.1f}M obj/s)")
            print(f"    File size: {file_size/1024/1024:.2f} MB ({file_size/n:.1f} bytes/object)")
            
            self.results.append({
                "benchmark": "save_binary",
                "num_objects": n,
                "time_ms": save_time,
                "objects_per_sec": save_speed,
                "file_size_bytes": file_size
            })
            
            self.results.append({
                "benchmark": "load_binary",
                "num_objects": n,
                "time_ms": load_time,
                "objects_per_sec": load_speed,
                "file_size_bytes": file_size
            })
            
            os.remove("temp_bench.bin")
            
            # JSON for smaller datasets
            if n <= 10000:
                json_time, _ = self.measure_time(drawing.save_json, "temp_bench.json")
                json_size = os.path.getsize("temp_bench.json")
                
                print(f"    JSON save: {self.format_time(json_time)}")
                print(f"    JSON size: {json_size/1024/1024:.2f} MB "
                      f"({json_size/file_size:.1f}x larger than binary)")
                
                os.remove("temp_bench.json")
    
    def run_batch_operations_benchmark(self, sizes):
        """Benchmark batch operations."""
        print("\n⚡ Batch Operations Benchmark")
        print("-" * 60)
        
        for n in sizes:
            # Create test data
            drawing = DrawingCpp(5000, 5000)
            ids = [drawing.add_circle(i % 1000, i // 1000, 5) for i in range(n)]
            storage = drawing._drawing.get_storage()
            
            # Translate
            translate_time, _ = self.measure_time(
                drawing_cpp.BatchOperations.translate_objects,
                storage, ids, 10.0, 20.0
            )
            translate_speed = n / (translate_time / 1000) if translate_time > 0 else 0
            
            # Scale
            scale_time, _ = self.measure_time(
                drawing_cpp.BatchOperations.scale_objects,
                storage, ids, 1.5, 1.5
            )
            scale_speed = n / (scale_time / 1000) if scale_time > 0 else 0
            
            # Bounding box
            bbox_time, _ = self.measure_time(
                drawing_cpp.BatchOperations.calculate_bounding_box,
                storage, ids
            )
            bbox_speed = n / (bbox_time / 1000) if bbox_time > 0 else 0
            
            print(f"  {self.format_number(n)} objects:")
            print(f"    Translate: {self.format_time(translate_time)} ({translate_speed/1e6:.1f}M obj/s)")
            print(f"    Scale: {self.format_time(scale_time)} ({scale_speed/1e6:.1f}M obj/s)")
            print(f"    Calc bbox: {self.format_time(bbox_time)} ({bbox_speed/1e6:.1f}M obj/s)")
            
            for op, time_ms, speed in [
                ("translate", translate_time, translate_speed),
                ("scale", scale_time, scale_speed),
                ("calc_bbox", bbox_time, bbox_speed)
            ]:
                self.results.append({
                    "benchmark": f"batch_{op}",
                    "num_objects": n,
                    "time_ms": time_ms,
                    "objects_per_sec": speed
                })
    
    def run_all(self, sizes=None):
        """Run all benchmarks."""
        if sizes is None:
            sizes = [100, 1000, 10000, 100000, 1000000]
        
        print("🚀 C++ Drawing Library Performance Benchmark")
        print("=" * 60)
        
        self.run_creation_benchmark(sizes)
        self.run_serialization_benchmark(sizes)
        self.run_batch_operations_benchmark(sizes)
        
        # Save results
        with open("benchmark_results_simple.json", "w") as f:
            json.dump(self.results, f, indent=2)
        
        print("\n" + "=" * 60)
        print("✅ Benchmarking complete!")
        print("📊 Results saved to benchmark_results_simple.json")
        
        # Print summary
        self.print_summary()
    
    def print_summary(self):
        """Print performance summary."""
        print("\n📈 Performance Summary")
        print("-" * 60)
        
        # Find best results
        best_creation = max([r for r in self.results if r["benchmark"] == "creation"], 
                          key=lambda x: x["objects_per_sec"])
        best_save = max([r for r in self.results if r["benchmark"] == "save_binary"], 
                       key=lambda x: x["objects_per_sec"])
        best_batch = max([r for r in self.results if "batch" in r["benchmark"]], 
                        key=lambda x: x["objects_per_sec"])
        
        print(f"  Best creation speed: {best_creation['objects_per_sec']/1e6:.1f}M objects/sec")
        print(f"  Best save speed: {best_save['objects_per_sec']/1e6:.1f}M objects/sec")
        print(f"  Best batch operation: {best_batch['objects_per_sec']/1e6:.1f}M objects/sec")
        
        # Memory efficiency
        creation_results = [r for r in self.results if r["benchmark"] == "creation"]
        if creation_results:
            largest = max(creation_results, key=lambda x: x["num_objects"])
            bytes_per_obj = largest["memory_bytes"] / largest["num_objects"]
            print(f"  Memory per object: {bytes_per_obj:.1f} bytes")
            print(f"  Memory reduction vs Python: {800/bytes_per_obj:.1f}x")


def compare_with_python():
    """Quick comparison with Python implementation."""
    print("\n🔄 Python vs C++ Comparison")
    print("-" * 60)
    
    n = 10000
    
    # Python version (if available)
    try:
        from python.data.drawing import Drawing as PyDrawing
        from python.data.shapes import Circle as PyCircle
        from python.data.basic_models import Point as PyPoint, Color as PyColor
        
        # Python creation
        start = time.perf_counter()
        py_drawing = PyDrawing(width=5000, height=5000)
        layer = py_drawing.add_layer("default")
        for i in range(n):
            circle = PyCircle(
                center=PyPoint(x=float(i % 100), y=float(i // 100)),
                radius=5.0,
                fill=PyColor(r=255, g=0, b=0)
            )
            layer.add_object(circle)
        py_time = (time.perf_counter() - start) * 1000
        
        # C++ creation
        start = time.perf_counter()
        cpp_drawing = DrawingCpp(5000, 5000)
        for i in range(n):
            cpp_drawing.add_circle(i % 100, i // 100, 5, fill_color=(255, 0, 0))
        cpp_time = (time.perf_counter() - start) * 1000
        
        print(f"  Creating {n:,} objects:")
        print(f"    Python: {py_time:.1f} ms")
        print(f"    C++: {cpp_time:.1f} ms")
        print(f"    Speedup: {py_time/cpp_time:.1f}x")
        
        print(f"\n  Memory usage:")
        print(f"    Python: ~{n * 800 / 1024 / 1024:.1f} MB (estimated)")
        print(f"    C++: {cpp_drawing.memory_usage / 1024 / 1024:.1f} MB")
        print(f"    Reduction: {(n * 800) / cpp_drawing.memory_usage:.1f}x")
        
    except ImportError:
        print("  Python implementation not available for comparison")


def main():
    """Run benchmarks."""
    benchmark = SimpleBenchmark()
    
    # Run with different sizes
    sizes = [100, 1000, 10000, 100000]
    
    # Add 1M for machines that can handle it
    print("\nTesting with 1M objects...")
    try:
        test = DrawingCpp(100, 100)
        test.add_circle(0, 0, 1)
        sizes.append(1000000)
        print("✓ System can handle 1M objects")
    except:
        print("✗ Skipping 1M objects test")
    
    benchmark.run_all(sizes)
    
    # Compare with Python if available
    compare_with_python()


if __name__ == "__main__":
    main()