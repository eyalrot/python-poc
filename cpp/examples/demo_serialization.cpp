#include "drawing/serialization.hpp"
#include <iostream>
#include <chrono>

using namespace drawing;

int main() {
    // Create a drawing similar to the Python example
    Drawing drawing(800, 600);
    drawing.set_background(Color(240, 240, 240));
    
    // Add a circle
    auto circle_id = drawing.add_circle(200, 200, 50);
    auto& storage = drawing.get_storage();
    if (auto* circle = storage.get_circle(circle_id)) {
        circle->base.fill_color = Color(255, 100, 100);
        circle->base.stroke_color = Color(200, 0, 0);
        circle->base.stroke_width = 2.0f;
        circle->base.flags.value |= ObjectFlags::HAS_STROKE;
    }
    
    // Add a rectangle
    auto rect_id = drawing.add_rectangle(300, 150, 120, 80);
    if (auto* rect = storage.get_rectangle(rect_id)) {
        rect->base.fill_color = Color(100, 100, 255);
        rect->base.stroke_color = Color(0, 0, 200);
        rect->base.stroke_width = 3.0f;
        rect->base.flags.value |= ObjectFlags::HAS_STROKE;
    }
    
    // Add a line
    auto line_id = drawing.add_line(100, 100, 500, 400);
    if (auto* line = storage.get_line(line_id)) {
        line->base.stroke_color = Color(0, 200, 0);
        line->base.stroke_width = 5.0f;
        line->base.flags.value |= ObjectFlags::HAS_STROKE;
        line->base.flags.value &= ~ObjectFlags::HAS_FILL;
    }
    
    std::cout << "Created drawing with " << drawing.total_objects() << " objects\n";
    std::cout << "Memory usage: " << drawing.memory_usage() << " bytes\n\n";
    
    // Save in binary format
    auto start = std::chrono::high_resolution_clock::now();
    if (save_binary(drawing, "demo_drawing.bin")) {
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "Saved binary file in " << ms << " microseconds\n";
        
        // Get file size
        std::ifstream file("demo_drawing.bin", std::ios::binary | std::ios::ate);
        auto size = file.tellg();
        std::cout << "Binary file size: " << size << " bytes\n";
    }
    
    // Save in JSON format for Python compatibility
    start = std::chrono::high_resolution_clock::now();
    save_json(drawing, "demo_drawing.json");
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Saved JSON file in " << ms << " microseconds\n";
    
    // Get JSON file size
    std::ifstream jfile("demo_drawing.json", std::ios::ate);
    auto jsize = jfile.tellg();
    std::cout << "JSON file size: " << jsize << " bytes\n";
    
    // Get binary file size for comparison
    std::ifstream bfile("demo_drawing.bin", std::ios::binary | std::ios::ate);
    auto bsize = bfile.tellg();
    std::cout << "JSON is " << (jsize / static_cast<float>(bsize)) << "x larger than binary\n\n";
    
    // Test loading binary
    start = std::chrono::high_resolution_clock::now();
    auto loaded = load_binary("demo_drawing.bin");
    end = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    if (loaded) {
        std::cout << "Loaded binary file in " << ms << " microseconds\n";
        std::cout << "Loaded drawing has " << loaded->total_objects() << " objects\n";
    }
    
    return 0;
}