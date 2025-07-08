#include <gtest/gtest.h>
#include "drawing/drawing.hpp"

using namespace drawing;

TEST(DrawingTest, Construction) {
    Drawing drawing(1024, 768);
    
    EXPECT_EQ(drawing.get_width(), 1024.0f);
    EXPECT_EQ(drawing.get_height(), 768.0f);
    EXPECT_EQ(drawing.get_background().r, 255);
    EXPECT_EQ(drawing.get_background().g, 255);
    EXPECT_EQ(drawing.get_background().b, 255);
    
    // Should have one default layer
    EXPECT_EQ(drawing.get_layers().size(), 1);
}

TEST(DrawingTest, LayerManagement) {
    Drawing drawing;
    
    auto layer_id = drawing.add_layer("Background");
    auto* layer = drawing.get_layer(layer_id);
    
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->get_name(), "Background");
    EXPECT_TRUE(layer->is_visible());
    EXPECT_FALSE(layer->is_locked());
}

TEST(DrawingTest, AddObjects) {
    Drawing drawing;
    
    auto circle_id = drawing.add_circle(100, 100, 50);
    auto rect_id = drawing.add_rectangle(200, 200, 100, 80);
    auto line_id = drawing.add_line(0, 0, 300, 300);
    
    EXPECT_EQ(drawing.total_objects(), 3);
    
    // Check objects were added to default layer
    auto* layer = drawing.get_layer(0);
    EXPECT_EQ(layer->object_count(), 3);
}

TEST(DrawingTest, MultipleLayersWithObjects) {
    Drawing drawing;
    
    auto bg_layer = drawing.add_layer("Background");
    auto fg_layer = drawing.add_layer("Foreground");
    
    // Add objects to different layers
    drawing.add_circle(100, 100, 50, bg_layer);
    drawing.add_rectangle(200, 200, 100, 80, 0, bg_layer);
    drawing.add_line(0, 0, 300, 300, LineStyle::Solid, fg_layer);
    
    EXPECT_EQ(drawing.total_objects(), 3);
    EXPECT_EQ(drawing.get_layer(bg_layer)->object_count(), 2);
    EXPECT_EQ(drawing.get_layer(fg_layer)->object_count(), 1);
}

TEST(DrawingTest, DrawingBoundingBox) {
    Drawing drawing;
    
    drawing.add_circle(100, 100, 50);    // bbox: 50,50 to 150,150
    drawing.add_rectangle(200, 200, 100, 80); // bbox: 200,200 to 300,280
    
    BoundingBox bbox = drawing.get_bounding_box();
    EXPECT_EQ(bbox.min_x, 50.0f);
    EXPECT_EQ(bbox.min_y, 50.0f);
    EXPECT_EQ(bbox.max_x, 300.0f);
    EXPECT_EQ(bbox.max_y, 280.0f);
}

TEST(DrawingTest, FindObjectsInRect) {
    Drawing drawing;
    
    auto c1 = drawing.add_circle(50, 50, 25);
    auto c2 = drawing.add_circle(250, 250, 25);
    auto r1 = drawing.add_rectangle(100, 100, 50, 50);
    
    BoundingBox search_rect(0, 0, 200, 200);
    auto found = drawing.find_objects_in_rect(search_rect);
    
    // Should find circle 1 and rectangle, but not circle 2
    EXPECT_EQ(found.size(), 2);
    EXPECT_TRUE(std::find(found.begin(), found.end(), c1) != found.end());
    EXPECT_TRUE(std::find(found.begin(), found.end(), r1) != found.end());
    EXPECT_FALSE(std::find(found.begin(), found.end(), c2) != found.end());
}

TEST(DrawingTest, MemoryUsage) {
    Drawing drawing;
    
    // Add many objects
    for (int i = 0; i < 1000; ++i) {
        drawing.add_circle(i * 10, i * 10, 5);
    }
    
    size_t mem = drawing.memory_usage();
    size_t expected_max = 1000 * 32 + 10000; // ~32 bytes per circle + overhead
    
    EXPECT_GT(mem, 0);
    EXPECT_LT(mem, expected_max);
    
    // Average memory per object
    size_t avg_mem = mem / drawing.total_objects();
    EXPECT_LT(avg_mem, 100); // Should be well under 100 bytes per object
}