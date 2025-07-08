#include <benchmark/benchmark.h>
#include "drawing/drawing.hpp"
#include "drawing/batch_operations.hpp"
#include "drawing/serialization.hpp"
#include <random>
#include <fstream>

using namespace drawing;

// Helper to create test data
static Drawing* CreateTestDrawing(int num_objects) {
    auto* drawing = new Drawing(5000, 5000);
    
    for (int i = 0; i < num_objects; ++i) {
        drawing->add_circle(i % 1000, i / 1000, 5.0f);
    }
    
    return drawing;
}

// Benchmark object creation
static void BM_CreateCircles(benchmark::State& state) {
    const int num_objects = state.range(0);
    
    for (auto _ : state) {
        Drawing drawing(5000, 5000);
        for (int i = 0; i < num_objects; ++i) {
            drawing.add_circle(i % 1000, i / 1000, 5.0f);
        }
        benchmark::DoNotOptimize(drawing.total_objects());
    }
    
    state.SetItemsProcessed(state.iterations() * num_objects);
    state.SetBytesProcessed(state.iterations() * num_objects * sizeof(CompactCircle));
}
BENCHMARK(BM_CreateCircles)->Range(100, 1000000);

// Benchmark batch translation
static void BM_BatchTranslate(benchmark::State& state) {
    const int num_objects = state.range(0);
    Drawing drawing(5000, 5000);
    
    std::vector<ObjectStorage::ObjectID> ids;
    for (int i = 0; i < num_objects; ++i) {
        ids.push_back(drawing.add_circle(i % 1000, i / 1000, 5.0f));
    }
    
    auto& storage = drawing.get_storage();
    
    for (auto _ : state) {
        BatchOperations::translate_objects(storage, ids, 10.0f, 20.0f);
    }
    
    state.SetItemsProcessed(state.iterations() * num_objects);
}
BENCHMARK(BM_BatchTranslate)->Range(100, 1000000);

// Benchmark spatial queries
static void BM_SpatialQueryRect(benchmark::State& state) {
    const int num_objects = state.range(0);
    Drawing drawing(5000, 5000);
    
    // Create random distribution
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(0, 5000);
    
    for (int i = 0; i < num_objects; ++i) {
        drawing.add_circle(dist(gen), dist(gen), 20.0f);
    }
    
    BoundingBox query_rect(1000, 1000, 2000, 2000);
    
    for (auto _ : state) {
        auto found = drawing.find_objects_in_rect(query_rect);
        benchmark::DoNotOptimize(found.size());
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_SpatialQueryRect)->Range(1000, 100000);

// Benchmark serialization
static void BM_BinarySave(benchmark::State& state) {
    const int num_objects = state.range(0);
    auto* drawing = CreateTestDrawing(num_objects);
    
    for (auto _ : state) {
        std::ofstream file("benchmark_temp.bin", std::ios::binary);
        BinarySerializer serializer(file);
        serializer.serialize(*drawing);
    }
    
    delete drawing;
    std::remove("benchmark_temp.bin");
    
    state.SetItemsProcessed(state.iterations() * num_objects);
}
BENCHMARK(BM_BinarySave)->Range(1000, 100000);

// Benchmark deserialization
static void BM_BinaryLoad(benchmark::State& state) {
    const int num_objects = state.range(0);
    
    // Create test file
    {
        auto* drawing = CreateTestDrawing(num_objects);
        save_binary(*drawing, "benchmark_temp.bin");
        delete drawing;
    }
    
    for (auto _ : state) {
        auto loaded = load_binary("benchmark_temp.bin");
        benchmark::DoNotOptimize(loaded->total_objects());
    }
    
    std::remove("benchmark_temp.bin");
    
    state.SetItemsProcessed(state.iterations() * num_objects);
}
BENCHMARK(BM_BinaryLoad)->Range(1000, 100000);

// Benchmark memory usage
static void BM_MemoryUsage(benchmark::State& state) {
    const int num_objects = state.range(0);
    
    for (auto _ : state) {
        Drawing drawing(5000, 5000);
        
        for (int i = 0; i < num_objects; ++i) {
            drawing.add_circle(i % 1000, i / 1000, 5.0f);
        }
        
        benchmark::DoNotOptimize(drawing.memory_usage());
    }
    
    // Report memory per object
    state.counters["bytes_per_object"] = sizeof(CompactCircle);
}
BENCHMARK(BM_MemoryUsage)->Range(1000, 1000000);

// Benchmark bounding box calculation
static void BM_CalculateBoundingBox(benchmark::State& state) {
    const int num_objects = state.range(0);
    Drawing drawing(5000, 5000);
    
    std::vector<ObjectStorage::ObjectID> ids;
    for (int i = 0; i < num_objects; ++i) {
        ids.push_back(drawing.add_circle(
            (i % 100) * 50, (i / 100) * 50, 20.0f
        ));
    }
    
    auto& storage = drawing.get_storage();
    
    for (auto _ : state) {
        auto bbox = BatchOperations::calculate_bounding_box(storage, ids);
        benchmark::DoNotOptimize(bbox.width());
    }
    
    state.SetItemsProcessed(state.iterations() * num_objects);
}
BENCHMARK(BM_CalculateBoundingBox)->Range(100, 100000);

BENCHMARK_MAIN();