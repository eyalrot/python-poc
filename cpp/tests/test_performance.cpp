#include <gtest/gtest.h>
#include <chrono>
#include "drawing/drawing.hpp"

using namespace drawing;
using namespace std::chrono;

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Warm up
    }
    
    template<typename F>
    double measure_time_ms(F&& func) {
        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();
        return duration_cast<microseconds>(end - start).count() / 1000.0;
    }
};

TEST_F(PerformanceTest, CreateManyObjects) {
    Drawing drawing;
    const int num_objects = 100000;
    
    double time_ms = measure_time_ms([&]() {
        for (int i = 0; i < num_objects; ++i) {
            drawing.add_circle(i % 1000, i / 1000, 5);
        }
    });
    
    std::cout << "Created " << num_objects << " circles in " << time_ms << " ms\n";
    std::cout << "Average: " << (time_ms * 1000 / num_objects) << " us per object\n";
    
    // Should be able to create 100k objects in under 100ms
    EXPECT_LT(time_ms, 100.0);
}

TEST_F(PerformanceTest, SpatialQuery) {
    Drawing drawing;
    
    // Create grid of objects
    for (int x = 0; x < 100; ++x) {
        for (int y = 0; y < 100; ++y) {
            drawing.add_circle(x * 10, y * 10, 4);
        }
    }
    
    EXPECT_EQ(drawing.total_objects(), 10000);
    
    BoundingBox query_rect(450, 450, 550, 550);
    
    double time_ms = measure_time_ms([&]() {
        for (int i = 0; i < 1000; ++i) {
            auto found = drawing.find_objects_in_rect(query_rect);
        }
    });
    
    std::cout << "1000 spatial queries on 10k objects took " << time_ms << " ms\n";
    std::cout << "Average: " << (time_ms / 1000) << " ms per query\n";
    
    // Should be able to do 1000 queries in under 100ms (0.1ms per query)
    EXPECT_LT(time_ms, 100.0);
}

TEST_F(PerformanceTest, BatchColorUpdate) {
    Drawing drawing;
    std::vector<ObjectID> ids;
    
    // Create many objects
    for (int i = 0; i < 10000; ++i) {
        ids.push_back(drawing.add_circle(i % 100, i / 100, 5));
    }
    
    Color new_color(255, 0, 0);
    
    double time_ms = measure_time_ms([&]() {
        drawing.get_storage().set_fill_color(ids, new_color);
    });
    
    std::cout << "Updated color of " << ids.size() << " objects in " << time_ms << " ms\n";
    
    // Should be very fast - under 10ms for 10k objects
    EXPECT_LT(time_ms, 10.0);
}

TEST_F(PerformanceTest, MemoryEfficiency) {
    Drawing drawing;
    const int num_objects = 1000000; // 1 million objects
    
    size_t initial_mem = drawing.memory_usage();
    
    // Add 1 million circles
    for (int i = 0; i < num_objects; ++i) {
        drawing.add_circle(i % 1000, i / 1000, 5);
    }
    
    size_t final_mem = drawing.memory_usage();
    size_t used_mem = final_mem - initial_mem;
    double bytes_per_object = static_cast<double>(used_mem) / num_objects;
    
    std::cout << "Memory for " << num_objects << " objects: " << used_mem / 1024.0 / 1024.0 << " MB\n";
    std::cout << "Bytes per object: " << bytes_per_object << "\n";
    
    // Should average less than 100 bytes per object
    EXPECT_LT(bytes_per_object, 100.0);
    
    // Target: 32 bytes per circle + some overhead
    EXPECT_LT(bytes_per_object, 50.0); // Allow some overhead
}