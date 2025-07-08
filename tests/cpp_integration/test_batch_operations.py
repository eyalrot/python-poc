#!/usr/bin/env python3
"""Test batch operations with C++ drawing library."""

import sys

sys.path.insert(0, "cpp")

import drawing_cpp
from python.drawing_cpp_wrapper import DrawingCpp
import time
import math


def test_batch_operations():
    """Test batch geometric operations."""
    print("Testing Batch Operations")
    print("========================")

    # Create drawing with grid of circles
    drawing = DrawingCpp(1000, 1000)
    storage = drawing._drawing.get_storage()

    # Create a 10x10 grid using batch operation
    grid_ids = drawing_cpp.BatchOperations.create_grid(
        storage,
        drawing_cpp.ObjectType.Circle,
        10,
        10,  # rows, cols
        50,
        50,  # cell width/height
        100,
        100,  # offset
    )

    print(f"Created {len(grid_ids)} circles in a grid")

    # Test batch translate
    print("\nTranslating all objects by (50, 50)...")
    start = time.time()
    drawing_cpp.BatchOperations.translate_objects(storage, grid_ids, 50, 50)
    elapsed = (time.time() - start) * 1000

    stats = drawing_cpp.get_last_operation_stats()
    print(f"  Time: {stats.time_ms:.3f} ms")
    print(f"  Objects/second: {stats.objects_per_second:,.0f}")

    # Test batch scale
    print("\nScaling all objects by 1.5x...")
    center = drawing_cpp.Point(350, 350)
    drawing_cpp.BatchOperations.scale_objects(storage, grid_ids, 1.5, 1.5, center)

    # Test batch rotate
    print("\nRotating all objects by 45 degrees...")
    drawing_cpp.BatchOperations.rotate_objects(storage, grid_ids, math.radians(45), center)

    # Test batch alignment
    print("\nAligning objects to left edge...")
    drawing_cpp.BatchOperations.align_objects_left(storage, grid_ids[:10])

    # Calculate bounding box
    bbox = drawing_cpp.BatchOperations.calculate_bounding_box(storage, grid_ids)
    print(f"\nBounding box of all objects:")
    print(f"  Min: ({bbox.min_x:.1f}, {bbox.min_y:.1f})")
    print(f"  Max: ({bbox.max_x:.1f}, {bbox.max_y:.1f})")
    print(f"  Size: {bbox.width():.1f} x {bbox.height():.1f}")

    # Save result
    drawing.save_json("batch_operations_result.json")
    print("\nSaved result to batch_operations_result.json")


def benchmark_batch_vs_individual():
    """Compare batch operations vs individual operations."""
    print("\n\nBenchmark: Batch vs Individual Operations")
    print("==========================================")

    num_objects = 50000

    # Create two identical drawings
    drawing1 = DrawingCpp(5000, 5000)
    drawing2 = DrawingCpp(5000, 5000)

    # Add many circles
    print(f"Creating {num_objects} circles...")
    ids1 = []
    ids2 = []

    for i in range(num_objects):
        x = (i % 1000) * 5
        y = (i // 1000) * 5
        ids1.append(drawing1.add_circle(x, y, 2))
        ids2.append(drawing2.add_circle(x, y, 2))

    # Time batch operation
    storage1 = drawing1._drawing.get_storage()
    start = time.time()
    drawing_cpp.BatchOperations.translate_objects(storage1, ids1, 100, 100)
    batch_time = time.time() - start

    # Time individual operations
    start = time.time()
    for obj_id in ids2:
        storage2 = drawing2._drawing.get_storage()
        # Note: In real use, we'd have a better API for this
        # For now, we use batch operation with single item
        drawing_cpp.BatchOperations.translate_objects(storage2, [obj_id], 100, 100)
    individual_time = time.time() - start

    print(f"\nResults for {num_objects} objects:")
    print(f"  Batch operation: {batch_time*1000:.1f} ms")
    print(f"  Individual operations: {individual_time*1000:.1f} ms")
    print(f"  Speedup: {individual_time/batch_time:.1f}x")

    # Performance with different operations
    print("\nTesting different batch operations:")

    operations = [
        ("Translate", lambda s, ids: drawing_cpp.BatchOperations.translate_objects(s, ids, 10, 10)),
        ("Scale", lambda s, ids: drawing_cpp.BatchOperations.scale_objects(s, ids, 1.1, 1.1)),
        (
            "Calculate bbox",
            lambda s, ids: drawing_cpp.BatchOperations.calculate_bounding_box(s, ids),
        ),
    ]

    for op_name, op_func in operations:
        start = time.time()
        op_func(storage1, ids1)
        elapsed = (time.time() - start) * 1000

        stats = drawing_cpp.get_last_operation_stats()
        if stats.objects_processed > 0:
            print(
                f"  {op_name}: {elapsed:.2f} ms ({stats.objects_per_second/1e6:.1f}M objects/sec)"
            )


if __name__ == "__main__":
    test_batch_operations()
    benchmark_batch_vs_individual()

