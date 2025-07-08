#!/usr/bin/env python3
"""
Comprehensive benchmarking suite for comparing Python vs C++ implementations.
"""

import time
import json
import sys
import os
import statistics
from typing import Dict, List, Any, Callable
from dataclasses import dataclass
try:
    import matplotlib.pyplot as plt
    import pandas as pd
    PLOTTING_AVAILABLE = True
except ImportError:
    PLOTTING_AVAILABLE = False
    print("Warning: matplotlib and/or pandas not available. Plotting features disabled.")

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Import both implementations
from python.data.models import (
    Drawing as PyDrawing, 
    Circle as PyCircle, 
    Rectangle as PyRectangle,
    Point as PyPoint, 
    Color as PyColor
)

# Import C++ implementation
sys.path.insert(0, 'cpp')
try:
    import drawing_cpp
    from python.drawing_cpp_wrapper import DrawingCpp
    CPP_AVAILABLE = True
except ImportError:
    print("Error: C++ bindings not available. This benchmark requires C++ bindings.")
    print("Please build them first:")
    print("  cd cpp")
    print("  python setup.py build_ext --inplace")
    sys.exit(1)


@dataclass
class BenchmarkResult:
    name: str
    implementation: str
    operation: str
    num_objects: int
    time_ms: float
    memory_bytes: int
    objects_per_second: float
    

class Benchmark:
    """Base class for benchmarks."""
    
    def __init__(self, name: str):
        self.name = name
        self.results: List[BenchmarkResult] = []
    
    def measure_time(self, func: Callable, *args, **kwargs) -> float:
        """Measure execution time in milliseconds."""
        start = time.perf_counter()
        result = func(*args, **kwargs)
        end = time.perf_counter()
        return (end - start) * 1000, result
    
    def run(self, sizes: List[int]) -> List[BenchmarkResult]:
        """Run benchmark for different object counts."""
        raise NotImplementedError


class CreationBenchmark(Benchmark):
    """Benchmark object creation performance."""
    
    def __init__(self):
        super().__init__("Object Creation")
    
    def create_python_objects(self, n: int) -> PyDrawing:
        drawing = PyDrawing(width=5000, height=5000)
        layer = drawing.add_layer("default")
        
        for i in range(n):
            circle = PyCircle(
                center=PyPoint(x=float(i % 1000), y=float(i // 1000)),
                radius=5.0,
                fill=PyColor(r=255, g=0, b=0)
            )
            layer.add_object(circle)
        
        return drawing
    
    def create_cpp_objects(self, n: int) -> DrawingCpp:
        drawing = DrawingCpp(5000, 5000)
        
        for i in range(n):
            drawing.add_circle(
                x=float(i % 1000),
                y=float(i // 1000),
                radius=5.0,
                fill_color=(255, 0, 0)
            )
        
        return drawing
    
    def run(self, sizes: List[int]) -> List[BenchmarkResult]:
        results = []
        
        for n in sizes:
            # Python implementation
            time_ms, py_drawing = self.measure_time(self.create_python_objects, n)
            
            # Estimate memory (rough approximation)
            py_memory = n * 800  # ~800 bytes per object in Python
            
            results.append(BenchmarkResult(
                name=self.name,
                implementation="Python",
                operation="create",
                num_objects=n,
                time_ms=time_ms,
                memory_bytes=py_memory,
                objects_per_second=n / (time_ms / 1000) if time_ms > 0 else 0
            ))
            
            # C++ implementation
            time_ms, cpp_drawing = self.measure_time(self.create_cpp_objects, n)
            
            results.append(BenchmarkResult(
                name=self.name,
                implementation="C++",
                operation="create",
                num_objects=n,
                time_ms=time_ms,
                memory_bytes=cpp_drawing.memory_usage,
                objects_per_second=n / (time_ms / 1000) if time_ms > 0 else 0
            ))
        
        return results


class SerializationBenchmark(Benchmark):
    """Benchmark serialization performance."""
    
    def __init__(self):
        super().__init__("Serialization")
    
    def run(self, sizes: List[int]) -> List[BenchmarkResult]:
        results = []
        
        for n in sizes:
            # Create test data
            cpp_drawing = DrawingCpp(5000, 5000)
            for i in range(n):
                cpp_drawing.add_circle(i % 1000, i // 1000, 5)
            
            # Binary save
            time_ms, _ = self.measure_time(cpp_drawing.save_binary, "temp_bench.bin")
            file_size = os.path.getsize("temp_bench.bin")
            
            results.append(BenchmarkResult(
                name=self.name,
                implementation="C++",
                operation="save_binary",
                num_objects=n,
                time_ms=time_ms,
                memory_bytes=file_size,
                objects_per_second=n / (time_ms / 1000) if time_ms > 0 else 0
            ))
            
            # Binary load
            time_ms, _ = self.measure_time(DrawingCpp.load_binary, "temp_bench.bin")
            
            results.append(BenchmarkResult(
                name=self.name,
                implementation="C++",
                operation="load_binary",
                num_objects=n,
                time_ms=time_ms,
                memory_bytes=file_size,
                objects_per_second=n / (time_ms / 1000) if time_ms > 0 else 0
            ))
            
            # JSON save (smaller dataset due to size)
            if n <= 10000:
                time_ms, _ = self.measure_time(cpp_drawing.save_json, "temp_bench.json")
                json_size = os.path.getsize("temp_bench.json")
                
                results.append(BenchmarkResult(
                    name=self.name,
                    implementation="C++",
                    operation="save_json",
                    num_objects=n,
                    time_ms=time_ms,
                    memory_bytes=json_size,
                    objects_per_second=n / (time_ms / 1000) if time_ms > 0 else 0
                ))
                
                os.remove("temp_bench.json")
            
            os.remove("temp_bench.bin")
        
        return results


class BatchOperationsBenchmark(Benchmark):
    """Benchmark batch operations performance."""
    
    def __init__(self):
        super().__init__("Batch Operations")
    
    def run(self, sizes: List[int]) -> List[BenchmarkResult]:
        results = []
        
        for n in sizes:
            # Create test data
            drawing = DrawingCpp(5000, 5000)
            ids = []
            for i in range(n):
                ids.append(drawing.add_circle(i % 1000, i // 1000, 5))
            
            storage = drawing._drawing.get_storage()
            
            # Translate
            time_ms, _ = self.measure_time(
                drawing_cpp.BatchOperations.translate_objects,
                storage, ids, 10.0, 20.0
            )
            
            results.append(BenchmarkResult(
                name=self.name,
                implementation="C++",
                operation="translate",
                num_objects=n,
                time_ms=time_ms,
                memory_bytes=0,
                objects_per_second=n / (time_ms / 1000) if time_ms > 0 else 0
            ))
            
            # Scale
            time_ms, _ = self.measure_time(
                drawing_cpp.BatchOperations.scale_objects,
                storage, ids, 1.5, 1.5
            )
            
            results.append(BenchmarkResult(
                name=self.name,
                implementation="C++",
                operation="scale",
                num_objects=n,
                time_ms=time_ms,
                memory_bytes=0,
                objects_per_second=n / (time_ms / 1000) if time_ms > 0 else 0
            ))
            
            # Calculate bounding box
            time_ms, _ = self.measure_time(
                drawing_cpp.BatchOperations.calculate_bounding_box,
                storage, ids
            )
            
            results.append(BenchmarkResult(
                name=self.name,
                implementation="C++",
                operation="calc_bbox",
                num_objects=n,
                time_ms=time_ms,
                memory_bytes=0,
                objects_per_second=n / (time_ms / 1000) if time_ms > 0 else 0
            ))
        
        return results


class BenchmarkRunner:
    """Run all benchmarks and generate reports."""
    
    def __init__(self):
        self.benchmarks = [
            CreationBenchmark(),
            SerializationBenchmark(),
            BatchOperationsBenchmark()
        ]
        self.results: List[BenchmarkResult] = []
    
    def run_all(self, sizes: List[int] = None):
        """Run all benchmarks."""
        if sizes is None:
            sizes = [100, 1000, 10000, 100000]
        
        print("Running benchmarks...")
        print("=" * 60)
        
        for benchmark in self.benchmarks:
            print(f"\n{benchmark.name}:")
            results = benchmark.run(sizes)
            self.results.extend(results)
            
            # Print summary
            for result in results:
                if result.time_ms > 0:
                    print(f"  {result.implementation} {result.operation} "
                          f"({result.num_objects:,} objects): "
                          f"{result.time_ms:.2f}ms, "
                          f"{result.objects_per_second/1e6:.2f}M obj/s")
    
    def save_results(self, filename: str = "benchmark_results.json"):
        """Save results to JSON file."""
        data = [
            {
                "name": r.name,
                "implementation": r.implementation,
                "operation": r.operation,
                "num_objects": r.num_objects,
                "time_ms": r.time_ms,
                "memory_bytes": r.memory_bytes,
                "objects_per_second": r.objects_per_second
            }
            for r in self.results
        ]
        
        with open(filename, 'w') as f:
            json.dump(data, f, indent=2)
        
        print(f"\nResults saved to {filename}")
    
    def generate_plots(self):
        """Generate performance comparison plots."""
        df = pd.DataFrame([r.__dict__ for r in self.results])
        
        # Create figure with subplots
        fig, axes = plt.subplots(2, 2, figsize=(12, 10))
        fig.suptitle('Performance Comparison: Python vs C++', fontsize=16)
        
        # 1. Creation performance
        creation_data = df[df['operation'] == 'create']
        if not creation_data.empty:
            ax = axes[0, 0]
            for impl in creation_data['implementation'].unique():
                data = creation_data[creation_data['implementation'] == impl]
                ax.plot(data['num_objects'], data['objects_per_second']/1e6, 
                       marker='o', label=impl)
            ax.set_xlabel('Number of Objects')
            ax.set_ylabel('Million Objects/Second')
            ax.set_title('Object Creation Performance')
            ax.legend()
            ax.grid(True)
            ax.set_xscale('log')
        
        # 2. Memory usage
        ax = axes[0, 1]
        creation_data = df[df['operation'] == 'create']
        if not creation_data.empty:
            for impl in creation_data['implementation'].unique():
                data = creation_data[creation_data['implementation'] == impl]
                ax.plot(data['num_objects'], data['memory_bytes']/1024/1024, 
                       marker='o', label=impl)
            ax.set_xlabel('Number of Objects')
            ax.set_ylabel('Memory (MB)')
            ax.set_title('Memory Usage')
            ax.legend()
            ax.grid(True)
            ax.set_xscale('log')
        
        # 3. Serialization performance
        ax = axes[1, 0]
        save_data = df[df['operation'].str.contains('save')]
        if not save_data.empty:
            for op in save_data['operation'].unique():
                data = save_data[save_data['operation'] == op]
                ax.plot(data['num_objects'], data['time_ms'], 
                       marker='o', label=op)
            ax.set_xlabel('Number of Objects')
            ax.set_ylabel('Time (ms)')
            ax.set_title('Serialization Performance')
            ax.legend()
            ax.grid(True)
            ax.set_xscale('log')
        
        # 4. Batch operations
        ax = axes[1, 1]
        batch_data = df[df['name'] == 'Batch Operations']
        if not batch_data.empty:
            for op in batch_data['operation'].unique():
                data = batch_data[batch_data['operation'] == op]
                ax.plot(data['num_objects'], data['objects_per_second']/1e6, 
                       marker='o', label=op)
            ax.set_xlabel('Number of Objects')
            ax.set_ylabel('Million Objects/Second')
            ax.set_title('Batch Operations Performance')
            ax.legend()
            ax.grid(True)
            ax.set_xscale('log')
        
        plt.tight_layout()
        plt.savefig('benchmark_results.png', dpi=150)
        print("\nPlots saved to benchmark_results.png")
    
    def generate_report(self):
        """Generate a markdown report."""
        report = ["# Drawing Library Performance Benchmark Report\n"]
        report.append(f"Generated on: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
        
        # Summary statistics
        report.append("## Summary\n")
        
        # Group results by operation
        df = pd.DataFrame([r.__dict__ for r in self.results])
        
        # Memory efficiency
        creation_data = df[df['operation'] == 'create']
        if not creation_data.empty:
            py_mem = creation_data[creation_data['implementation'] == 'Python']['memory_bytes'].max()
            cpp_mem = creation_data[creation_data['implementation'] == 'C++']['memory_bytes'].max()
            if py_mem > 0 and cpp_mem > 0:
                report.append(f"- **Memory Reduction**: {py_mem/cpp_mem:.1f}x "
                            f"(Python: {py_mem/1e6:.1f}MB vs C++: {cpp_mem/1e6:.1f}MB)\n")
        
        # Performance improvements
        report.append("\n## Performance Metrics\n")
        report.append("| Operation | C++ Performance | Notes |\n")
        report.append("|-----------|-----------------|-------|\n")
        
        # Find best performances
        metrics = {
            'create': 'Object Creation',
            'translate': 'Batch Translate',
            'save_binary': 'Binary Save',
            'load_binary': 'Binary Load'
        }
        
        for op, name in metrics.items():
            op_data = df[df['operation'] == op]
            if not op_data.empty:
                cpp_data = op_data[op_data['implementation'] == 'C++']
                if not cpp_data.empty:
                    best = cpp_data.loc[cpp_data['objects_per_second'].idxmax()]
                    report.append(f"| {name} | {best['objects_per_second']/1e6:.1f}M objects/sec | "
                                f"{best['num_objects']:,} objects |\n")
        
        # Detailed results
        report.append("\n## Detailed Results\n")
        
        for benchmark_name in df['name'].unique():
            report.append(f"\n### {benchmark_name}\n")
            benchmark_data = df[df['name'] == benchmark_name]
            
            report.append("| Implementation | Operation | Objects | Time (ms) | Objects/sec | Memory (MB) |\n")
            report.append("|----------------|-----------|---------|-----------|-------------|-------------|\n")
            
            for _, row in benchmark_data.iterrows():
                report.append(f"| {row['implementation']} | {row['operation']} | "
                            f"{row['num_objects']:,} | {row['time_ms']:.2f} | "
                            f"{row['objects_per_second']/1e6:.2f}M | "
                            f"{row['memory_bytes']/1e6:.2f} |\n")
        
        # Write report
        with open('benchmark_report.md', 'w') as f:
            f.write(''.join(report))
        
        print("\nReport saved to benchmark_report.md")


def main():
    """Run all benchmarks and generate reports."""
    runner = BenchmarkRunner()
    
    # Run with different sizes
    sizes = [100, 1000, 10000, 100000, 1000000]
    runner.run_all(sizes)
    
    # Save results
    runner.save_results()
    
    # Generate visualizations
    try:
        runner.generate_plots()
    except ImportError:
        print("Note: Install matplotlib and pandas for plots: pip install matplotlib pandas")
    
    # Generate report
    runner.generate_report()
    
    print("\nBenchmarking complete!")


if __name__ == "__main__":
    main()