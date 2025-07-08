#include <gtest/gtest.h>
#include "drawing/objects.hpp"

using namespace drawing;

TEST(ObjectsTest, CompactCircle) {
    CompactCircle circle(100, 200, 50);
    
    EXPECT_EQ(circle.x, 100.0f);
    EXPECT_EQ(circle.y, 200.0f);
    EXPECT_EQ(circle.radius, 50.0f);
    EXPECT_EQ(circle.base.type, ObjectType::Circle);
    
    BoundingBox bbox = circle.get_bounding_box();
    EXPECT_EQ(bbox.min_x, 50.0f);
    EXPECT_EQ(bbox.min_y, 150.0f);
    EXPECT_EQ(bbox.max_x, 150.0f);
    EXPECT_EQ(bbox.max_y, 250.0f);
    
    // Size check (40 bytes with new features: gradient, pattern, name support)
    EXPECT_LE(sizeof(CompactCircle), 40);
}

TEST(ObjectsTest, CompactRectangle) {
    CompactRectangle rect(10, 20, 100, 50);
    
    EXPECT_EQ(rect.x, 10.0f);
    EXPECT_EQ(rect.y, 20.0f);
    EXPECT_EQ(rect.width, 100.0f);
    EXPECT_EQ(rect.height, 50.0f);
    EXPECT_EQ(rect.base.type, ObjectType::Rectangle);
    
    BoundingBox bbox = rect.get_bounding_box();
    EXPECT_EQ(bbox.min_x, 10.0f);
    EXPECT_EQ(bbox.min_y, 20.0f);
    EXPECT_EQ(bbox.max_x, 110.0f);
    EXPECT_EQ(bbox.max_y, 70.0f);
    
    // Size check (48 bytes with new features: gradient, pattern, name + corner_radius)
    EXPECT_LE(sizeof(CompactRectangle), 48);
}

TEST(ObjectsTest, CompactLine) {
    CompactLine line(10, 20, 100, 50);
    
    EXPECT_EQ(line.x1, 10.0f);
    EXPECT_EQ(line.y1, 20.0f);
    EXPECT_EQ(line.x2, 100.0f);
    EXPECT_EQ(line.y2, 50.0f);
    EXPECT_EQ(line.base.type, ObjectType::Line);
    
    BoundingBox bbox = line.get_bounding_box();
    EXPECT_EQ(bbox.min_x, 10.0f);
    EXPECT_EQ(bbox.min_y, 20.0f);
    EXPECT_EQ(bbox.max_x, 100.0f);
    EXPECT_EQ(bbox.max_y, 50.0f);
    
    // Size check (48 bytes with new features: gradient, pattern, name + line_style)
    EXPECT_LE(sizeof(CompactLine), 48);
}

TEST(ObjectsTest, ObjectStorage) {
    ObjectStorage storage;
    
    // Add objects
    auto circle_id = storage.add_circle(100, 100, 50);
    auto rect_id = storage.add_rectangle(200, 200, 100, 100);
    auto line_id = storage.add_line(0, 0, 300, 300);
    
    EXPECT_EQ(storage.total_objects(), 3);
    
    // Retrieve objects
    auto* circle = storage.get_circle(circle_id);
    ASSERT_NE(circle, nullptr);
    EXPECT_EQ(circle->x, 100.0f);
    
    auto* rect = storage.get_rectangle(rect_id);
    ASSERT_NE(rect, nullptr);
    EXPECT_EQ(rect->x, 200.0f);
    
    auto* line = storage.get_line(line_id);
    ASSERT_NE(line, nullptr);
    EXPECT_EQ(line->x1, 0.0f);
    
    // Wrong type access should return nullptr
    EXPECT_EQ(storage.get_circle(rect_id), nullptr);
    EXPECT_EQ(storage.get_rectangle(circle_id), nullptr);
}

TEST(ObjectsTest, ObjectIDs) {
    ObjectStorage storage;
    
    auto id = storage.add_circle(0, 0, 10);
    
    EXPECT_EQ(ObjectStorage::get_type(id), ObjectType::Circle);
    EXPECT_EQ(ObjectStorage::get_index(id), 0);
    
    auto id2 = storage.add_circle(10, 10, 20);
    EXPECT_EQ(ObjectStorage::get_index(id2), 1);
}

TEST(ObjectsTest, BatchOperations) {
    ObjectStorage storage;
    
    auto id1 = storage.add_circle(0, 0, 10);
    auto id2 = storage.add_rectangle(10, 10, 20, 20);
    auto id3 = storage.add_line(0, 0, 30, 30);
    
    std::vector<ObjectID> ids = {id1, id2, id3};
    
    // Set fill color
    Color red(255, 0, 0);
    storage.set_fill_color(ids, red);
    
    auto* circle = storage.get_circle(id1);
    EXPECT_EQ(circle->base.fill_color.r, 255);
    EXPECT_EQ(circle->base.fill_color.g, 0);
    
    // Set opacity
    storage.set_opacity(ids, 0.5f);
    EXPECT_EQ(circle->base.opacity, 0.5f);
}

TEST(ObjectsTest, SpatialQueries) {
    ObjectStorage storage;
    
    storage.add_circle(50, 50, 25);    // Inside query rect
    storage.add_circle(150, 150, 25);  // Outside query rect
    storage.add_rectangle(75, 75, 50, 50); // Partially inside
    
    BoundingBox query_rect(0, 0, 100, 100);
    auto found = storage.find_in_rect(query_rect);
    
    EXPECT_EQ(found.size(), 2); // Circle and rectangle
}

TEST(ObjectsTest, PointQueries) {
    ObjectStorage storage;
    
    auto circle_id = storage.add_circle(50, 50, 25);
    auto rect_id = storage.add_rectangle(100, 100, 50, 50);
    auto line_id = storage.add_line(0, 0, 100, 100);
    
    // Test point on circle edge
    auto found = storage.find_at_point(Point(75, 50), 2.0f);
    EXPECT_TRUE(std::find(found.begin(), found.end(), circle_id) != found.end());
    
    // Test point on rectangle edge
    found = storage.find_at_point(Point(100, 125), 2.0f);
    EXPECT_TRUE(std::find(found.begin(), found.end(), rect_id) != found.end());
    
    // Test point on line
    found = storage.find_at_point(Point(50, 50), 2.0f);
    EXPECT_TRUE(std::find(found.begin(), found.end(), line_id) != found.end());
}

TEST(ObjectsTest, PolygonStorage) {
    ObjectStorage storage;
    
    std::vector<Point> triangle = {
        Point(0, 0),
        Point(100, 0),
        Point(50, 100)
    };
    
    auto poly_id = storage.add_polygon(triangle);
    EXPECT_EQ(ObjectStorage::get_type(poly_id), ObjectType::Polygon);
    
    // Memory usage should be reasonable
    size_t mem = storage.memory_usage();
    EXPECT_GT(mem, 0);
    EXPECT_LT(mem, 1000); // Should be well under 1KB for this simple case
}