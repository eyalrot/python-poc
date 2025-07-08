#include <gtest/gtest.h>
#include "drawing/types.hpp"

using namespace drawing;

TEST(TypesTest, ColorConstruction) {
    Color c1;
    EXPECT_EQ(c1.r, 0);
    EXPECT_EQ(c1.g, 0);
    EXPECT_EQ(c1.b, 0);
    EXPECT_EQ(c1.a, 255);
    
    Color c2(255, 128, 64, 200);
    EXPECT_EQ(c2.r, 255);
    EXPECT_EQ(c2.g, 128);
    EXPECT_EQ(c2.b, 64);
    EXPECT_EQ(c2.a, 200);
}

TEST(TypesTest, ColorConversion) {
    Color c(255, 128, 64, 200);
    uint32_t rgba = c.to_rgba32();
    Color c2 = Color::from_rgba32(rgba);
    
    EXPECT_EQ(c.r, c2.r);
    EXPECT_EQ(c.g, c2.g);
    EXPECT_EQ(c.b, c2.b);
    EXPECT_EQ(c.a, c2.a);
}

TEST(TypesTest, ColorConstants) {
    EXPECT_EQ(Color::BLACK.r, 0);
    EXPECT_EQ(Color::BLACK.g, 0);
    EXPECT_EQ(Color::BLACK.b, 0);
    EXPECT_EQ(Color::BLACK.a, 255);
    
    EXPECT_EQ(Color::WHITE.r, 255);
    EXPECT_EQ(Color::WHITE.g, 255);
    EXPECT_EQ(Color::WHITE.b, 255);
    EXPECT_EQ(Color::WHITE.a, 255);
    
    EXPECT_EQ(Color::TRANSPARENT.a, 0);
}

TEST(TypesTest, PointConstruction) {
    Point p1;
    EXPECT_EQ(p1.x, 0.0f);
    EXPECT_EQ(p1.y, 0.0f);
    
    Point p2(10.5f, 20.5f);
    EXPECT_EQ(p2.x, 10.5f);
    EXPECT_EQ(p2.y, 20.5f);
}

TEST(TypesTest, BoundingBoxOperations) {
    BoundingBox bbox(10, 20, 30, 40);
    
    EXPECT_EQ(bbox.width(), 20.0f);
    EXPECT_EQ(bbox.height(), 20.0f);
    
    Point center = bbox.center();
    EXPECT_EQ(center.x, 20.0f);
    EXPECT_EQ(center.y, 30.0f);
    
    EXPECT_TRUE(bbox.contains(Point(20, 30)));
    EXPECT_FALSE(bbox.contains(Point(5, 30)));
    
    BoundingBox bbox2(25, 35, 45, 55);
    EXPECT_TRUE(bbox.intersects(bbox2));
    
    BoundingBox bbox3(40, 50, 60, 70);
    EXPECT_FALSE(bbox.intersects(bbox3));
}

TEST(TypesTest, BoundingBoxExpansion) {
    BoundingBox bbox(10, 10, 20, 20);
    
    bbox.expand(Point(5, 15));
    EXPECT_EQ(bbox.min_x, 5.0f);
    EXPECT_EQ(bbox.min_y, 10.0f);
    
    bbox.expand(Point(25, 25));
    EXPECT_EQ(bbox.max_x, 25.0f);
    EXPECT_EQ(bbox.max_y, 25.0f);
    
    BoundingBox bbox2(0, 0, 30, 30);
    bbox.expand(bbox2);
    EXPECT_EQ(bbox.min_x, 0.0f);
    EXPECT_EQ(bbox.min_y, 0.0f);
    EXPECT_EQ(bbox.max_x, 30.0f);
    EXPECT_EQ(bbox.max_y, 30.0f);
}

TEST(TypesTest, ObjectFlags) {
    ObjectFlags flags;
    
    EXPECT_TRUE(flags.is_visible());
    EXPECT_FALSE(flags.is_locked());
    EXPECT_FALSE(flags.is_selected());
    EXPECT_TRUE(flags.has_fill());
    EXPECT_FALSE(flags.has_stroke());
    
    flags.set_locked(true);
    EXPECT_TRUE(flags.is_locked());
    
    flags.set_visible(false);
    EXPECT_FALSE(flags.is_visible());
}

TEST(TypesTest, Transform2D) {
    Transform2D identity;
    Point p(10, 20);
    Point p2 = identity.transform(p);
    EXPECT_EQ(p2.x, 10.0f);
    EXPECT_EQ(p2.y, 20.0f);
    
    Transform2D translate = Transform2D::translate(5, 10);
    p2 = translate.transform(p);
    EXPECT_EQ(p2.x, 15.0f);
    EXPECT_EQ(p2.y, 30.0f);
    
    Transform2D scale = Transform2D::scale(2, 3);
    p2 = scale.transform(p);
    EXPECT_EQ(p2.x, 20.0f);
    EXPECT_EQ(p2.y, 60.0f);
}