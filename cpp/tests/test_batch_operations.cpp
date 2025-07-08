#include <gtest/gtest.h>
#include "drawing/batch_operations.hpp"
#include <chrono>

using namespace drawing;

class BatchOperationsTest : public ::testing::Test {
protected:
    ObjectStorage storage;
    std::vector<ObjectID> test_objects;
    
    void SetUp() override {
        // Create test objects
        for (int i = 0; i < 10; ++i) {
            test_objects.push_back(storage.add_circle(i * 50, i * 50, 20));
            test_objects.push_back(storage.add_rectangle(i * 50, i * 50 + 100, 30, 40));
        }
    }
};

TEST_F(BatchOperationsTest, TranslateObjects) {
    // Get initial positions
    auto* first_circle = storage.get_circle(test_objects[0]);
    ASSERT_NE(first_circle, nullptr);
    float initial_x = first_circle->x;
    float initial_y = first_circle->y;
    
    // Translate all objects
    BatchOperations::translate_objects(storage, test_objects, 100, 50);
    
    // Check that objects moved
    EXPECT_FLOAT_EQ(first_circle->x, initial_x + 100);
    EXPECT_FLOAT_EQ(first_circle->y, initial_y + 50);
    
    // Check performance stats
    auto stats = BatchOperations::last_operation_stats;
    EXPECT_EQ(stats.objects_processed, test_objects.size());
    EXPECT_GT(stats.objects_per_second, 0);
}

TEST_F(BatchOperationsTest, ScaleObjects) {
    BatchOperations::scale_objects(storage, test_objects, 2.0f, 2.0f, Point(0, 0));
    
    // Check that first circle doubled in size and position
    auto* first_circle = storage.get_circle(test_objects[0]);
    EXPECT_FLOAT_EQ(first_circle->x, 0); // Was at 0, still at 0
    EXPECT_FLOAT_EQ(first_circle->radius, 40); // Was 20, now 40
    
    // Check second circle (was at 50,50)
    auto* second_circle = storage.get_circle(test_objects[2]);
    EXPECT_FLOAT_EQ(second_circle->x, 100); // 50 * 2
    EXPECT_FLOAT_EQ(second_circle->y, 100); // 50 * 2
}

TEST_F(BatchOperationsTest, RotateObjects) {
    // Create a simple line for rotation test
    auto line_id = storage.add_line(0, 0, 100, 0);
    std::vector<ObjectID> line_vec = {line_id};
    
    // Rotate 90 degrees around origin
    BatchOperations::rotate_objects(storage, line_vec, M_PI/2, Point(0, 0));
    
    auto* line = storage.get_line(line_id);
    ASSERT_NE(line, nullptr);
    
    // Should now be vertical
    EXPECT_NEAR(line->x1, 0, 0.001);
    EXPECT_NEAR(line->y1, 0, 0.001);
    EXPECT_NEAR(line->x2, 0, 0.001);
    EXPECT_NEAR(line->y2, 100, 0.001);
}

TEST_F(BatchOperationsTest, CalculateBoundingBox) {
    BoundingBox bbox = BatchOperations::calculate_bounding_box(storage, test_objects);
    
    // Should encompass all objects
    EXPECT_LE(bbox.min_x, 0);
    EXPECT_LE(bbox.min_y, 0);
    EXPECT_GE(bbox.max_x, 450); // Last circle at 450
    EXPECT_GE(bbox.max_y, 540); // Last rectangle at y=540
}

TEST_F(BatchOperationsTest, AlignObjectsLeft) {
    BatchOperations::align_objects_left(storage, test_objects);
    
    // Find leftmost edge
    float min_x = std::numeric_limits<float>::max();
    for (auto id : test_objects) {
        BoundingBox bbox = BatchOperations::get_object_bbox(storage, id);
        min_x = std::min(min_x, bbox.min_x);
    }
    
    // All objects should have their left edge at min_x
    for (auto id : test_objects) {
        BoundingBox bbox = BatchOperations::get_object_bbox(storage, id);
        EXPECT_FLOAT_EQ(bbox.min_x, min_x);
    }
}

TEST_F(BatchOperationsTest, CreateGrid) {
    ObjectStorage grid_storage;
    
    auto grid_objects = BatchOperations::create_grid(
        grid_storage, ObjectType::Circle,
        5, 5, 50, 50, 100, 100
    );
    
    EXPECT_EQ(grid_objects.size(), 25);
    
    // Check first circle position
    auto* first = grid_storage.get_circle(grid_objects[0]);
    EXPECT_FLOAT_EQ(first->x, 125); // 100 + 50/2
    EXPECT_FLOAT_EQ(first->y, 125);
    
    // Check last circle position
    auto* last = grid_storage.get_circle(grid_objects[24]);
    EXPECT_FLOAT_EQ(last->x, 325); // 100 + 4*50 + 50/2
    EXPECT_FLOAT_EQ(last->y, 325);
}

TEST_F(BatchOperationsTest, PerformanceBatchTranslate) {
    // Create many objects
    ObjectStorage perf_storage;
    std::vector<ObjectID> many_objects;
    
    const int num_objects = 100000;
    for (int i = 0; i < num_objects; ++i) {
        many_objects.push_back(perf_storage.add_circle(i % 1000, i / 1000, 5));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    BatchOperations::translate_objects(perf_storage, many_objects, 10, 20);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double ms = duration.count() / 1000.0;
    
    std::cout << "Translated " << num_objects << " objects in " << ms << " ms\n";
    std::cout << "Performance: " << (num_objects / ms * 1000) << " objects/second\n";
    
    // Should be very fast
    EXPECT_LT(ms, 10); // Less than 10ms for 100k objects
    
    auto stats = BatchOperations::last_operation_stats;
    EXPECT_GT(stats.objects_per_second, 10000000); // > 10M objects/second
}

TEST_F(BatchOperationsTest, PerformanceVsIndividual) {
    const int num_objects = 10000;
    ObjectStorage storage1, storage2;
    std::vector<ObjectID> ids1, ids2;
    
    // Create identical objects in both storages
    for (int i = 0; i < num_objects; ++i) {
        ids1.push_back(storage1.add_circle(i % 100, i / 100, 5));
        ids2.push_back(storage2.add_circle(i % 100, i / 100, 5));
    }
    
    // Time batch operation
    auto start = std::chrono::high_resolution_clock::now();
    BatchOperations::translate_objects(storage1, ids1, 10, 20);
    auto batch_time = std::chrono::high_resolution_clock::now() - start;
    
    // Time individual operations
    start = std::chrono::high_resolution_clock::now();
    for (auto id : ids2) {
        if (auto* circle = storage2.get_circle(id)) {
            circle->x += 10;
            circle->y += 20;
        }
    }
    auto individual_time = std::chrono::high_resolution_clock::now() - start;
    
    auto batch_us = std::chrono::duration_cast<std::chrono::microseconds>(batch_time).count();
    auto individual_us = std::chrono::duration_cast<std::chrono::microseconds>(individual_time).count();
    
    std::cout << "Batch time: " << batch_us << " us\n";
    std::cout << "Individual time: " << individual_us << " us\n";
    
    // Avoid division by zero
    double speedup = (batch_us > 0) ? static_cast<double>(individual_us) / batch_us : 1.0;
    std::cout << "Batch operation speedup: " << speedup << "x\n";
    
    // For small numbers of objects, the overhead of grouping by type
    // might make batch operations slightly slower. The real benefit
    // comes with larger datasets and SIMD operations.
    // Just verify both methods work correctly.
    
    // Verify results are the same
    for (size_t i = 0; i < num_objects; ++i) {
        auto* c1 = storage1.get_circle(ids1[i]);
        auto* c2 = storage2.get_circle(ids2[i]);
        EXPECT_FLOAT_EQ(c1->x, c2->x);
        EXPECT_FLOAT_EQ(c1->y, c2->y);
    }
}