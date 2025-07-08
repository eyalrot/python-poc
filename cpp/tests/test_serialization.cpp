#include <gtest/gtest.h>
#include "drawing/serialization.hpp"
#include <sstream>
#include <filesystem>

using namespace drawing;

class SerializationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test drawing
        test_drawing = std::make_unique<Drawing>(1024, 768);
        test_drawing->set_background(Color(128, 128, 128));
        
        // Add some objects
        auto circle_id = test_drawing->add_circle(100, 100, 50);
        auto rect_id = test_drawing->add_rectangle(200, 200, 100, 80);
        auto line_id = test_drawing->add_line(0, 0, 300, 300);
        
        // Modify some properties
        auto& storage = test_drawing->get_storage();
        if (auto* circle = storage.get_circle(circle_id)) {
            circle->base.fill_color = Color(255, 0, 0);
            circle->base.opacity = 0.8f;
        }
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("test_drawing.bin");
        std::filesystem::remove("test_drawing.json");
    }
    
    std::unique_ptr<Drawing> test_drawing;
};

TEST_F(SerializationTest, BinarySerializeToStream) {
    std::stringstream ss;
    BinarySerializer serializer(ss);
    
    serializer.serialize(*test_drawing);
    
    std::string data = ss.str();
    EXPECT_GT(data.size(), 100); // Should have some data
    
    // Check magic number
    uint32_t magic;
    ss.seekg(0);
    ss.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    EXPECT_EQ(magic, BinaryFormat::MAGIC);
}

TEST_F(SerializationTest, BinaryRoundTrip) {
    // Save
    ASSERT_TRUE(save_binary(*test_drawing, "test_drawing.bin"));
    
    // Load
    auto loaded = load_binary("test_drawing.bin");
    ASSERT_NE(loaded, nullptr);
    
    // Verify properties
    EXPECT_EQ(loaded->get_width(), 1024);
    EXPECT_EQ(loaded->get_height(), 768);
    EXPECT_EQ(loaded->get_background().r, 128);
    EXPECT_EQ(loaded->get_background().g, 128);
    EXPECT_EQ(loaded->get_background().b, 128);
    
    // Verify objects
    EXPECT_EQ(loaded->total_objects(), 3);
    
    // Check first circle
    const auto& storage = loaded->get_storage();
    ASSERT_GT(storage.circles.size(), 0);
    const auto& circle = storage.circles[0];
    EXPECT_EQ(circle.x, 100);
    EXPECT_EQ(circle.y, 100);
    EXPECT_EQ(circle.radius, 50);
    EXPECT_EQ(circle.base.fill_color.r, 255);
    EXPECT_EQ(circle.base.fill_color.g, 0);
    EXPECT_FLOAT_EQ(circle.base.opacity, 0.8f);
}

TEST_F(SerializationTest, BinaryFileSize) {
    // Create drawing with many objects
    Drawing big_drawing;
    for (int i = 0; i < 10000; ++i) {
        big_drawing.add_circle(i % 100, i / 100, 5);
    }
    
    ASSERT_TRUE(save_binary(big_drawing, "test_drawing.bin"));
    
    auto file_size = std::filesystem::file_size("test_drawing.bin");
    
    std::cout << "Binary file size for 10k circles: " << file_size << " bytes\n";
    std::cout << "Bytes per object: " << file_size / 10000.0 << "\n";
    
    // Should be very compact - 36 bytes per object is excellent!
    EXPECT_LT(file_size / 10000.0, 40); // Less than 40 bytes per object with overhead
}

TEST_F(SerializationTest, JsonExport) {
    save_json(*test_drawing, "test_drawing.json");
    
    // Verify file exists and has content
    ASSERT_TRUE(std::filesystem::exists("test_drawing.json"));
    auto file_size = std::filesystem::file_size("test_drawing.json");
    EXPECT_GT(file_size, 100);
    
    // Read and check basic structure
    std::ifstream file("test_drawing.json");
    std::string json((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    
    // Should contain expected fields
    EXPECT_NE(json.find("\"width\": 1024"), std::string::npos);
    EXPECT_NE(json.find("\"height\": 768"), std::string::npos);
    EXPECT_NE(json.find("\"layers\""), std::string::npos);
    EXPECT_NE(json.find("\"objects\""), std::string::npos);
}

TEST_F(SerializationTest, PerformanceBinarySave) {
    Drawing big_drawing;
    const int num_objects = 100000;
    
    for (int i = 0; i < num_objects; ++i) {
        big_drawing.add_circle(i % 1000, i / 1000, 5);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    ASSERT_TRUE(save_binary(big_drawing, "test_drawing.bin"));
    auto end = std::chrono::high_resolution_clock::now();
    
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Saved " << num_objects << " objects in " << ms << " ms\n";
    
    // Should save 100k objects in under 50ms
    EXPECT_LT(ms, 50);
}

TEST_F(SerializationTest, PerformanceBinaryLoad) {
    // First create a file
    Drawing big_drawing;
    const int num_objects = 100000;
    
    for (int i = 0; i < num_objects; ++i) {
        big_drawing.add_circle(i % 1000, i / 1000, 5);
    }
    
    ASSERT_TRUE(save_binary(big_drawing, "test_drawing.bin"));
    
    // Now time the load
    auto start = std::chrono::high_resolution_clock::now();
    auto loaded = load_binary("test_drawing.bin");
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->total_objects(), num_objects);
    
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Loaded " << num_objects << " objects in " << ms << " ms\n";
    
    // Should load 100k objects in under 100ms
    EXPECT_LT(ms, 100);
}