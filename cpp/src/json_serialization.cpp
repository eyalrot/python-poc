#include "drawing/serialization.hpp"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <ctime>

namespace drawing {

// Simple JSON writer (minimal implementation)
class JsonWriter {
private:
    std::ostream& stream;
    int indent_level = 0;
    bool first_in_object = true;
    bool first_in_array = true;
    
public:
    void write_indent() {
        for (int i = 0; i < indent_level; ++i) {
            stream << "  ";
        }
    }
    
    void write_comma() {
        if (!first_in_object && !first_in_array) {
            stream << ",";
        }
        stream << "\n";
        first_in_object = false;
        first_in_array = false;
    }
    
public:
    explicit JsonWriter(std::ostream& s) : stream(s) {}
    
    void begin_object() {
        stream << "{";
        indent_level++;
        first_in_object = true;
    }
    
    void end_object() {
        indent_level--;
        stream << "\n";
        write_indent();
        stream << "}";
        first_in_object = false;
    }
    
    void begin_array() {
        stream << "[";
        indent_level++;
        first_in_array = true;
    }
    
    void end_array() {
        indent_level--;
        stream << "\n";
        write_indent();
        stream << "]";
        first_in_array = false;
    }
    
    void write_key(const std::string& key) {
        write_comma();
        write_indent();
        stream << "\"" << key << "\": ";
    }
    
    void write_string(const std::string& value) {
        stream << "\"" << value << "\"";
    }
    
    void write_number(float value) {
        stream << value;
    }
    
    void write_number(int value) {
        stream << value;
    }
    
    void write_bool(bool value) {
        stream << (value ? "true" : "false");
    }
    
    void write_null() {
        stream << "null";
    }
    
    void write_color(const Color& c) {
        begin_object();
        write_key("r"); write_number(static_cast<int>(c.r));
        write_key("g"); write_number(static_cast<int>(c.g));
        write_key("b"); write_number(static_cast<int>(c.b));
        write_key("a"); write_number(static_cast<int>(c.a));
        end_object();
    }
    
    void write_point(const Point& p) {
        begin_object();
        write_key("x"); write_number(p.x);
        write_key("y"); write_number(p.y);
        end_object();
    }
};

// Generate UUID-like string for compatibility
std::string generate_id_string(ObjectStorage::ObjectID id) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << id << "-0000-0000-0000-";
    ss << std::setw(12) << id;
    return ss.str();
}

// Write object base properties
void write_object_base(JsonWriter& writer, const CompactObject& obj, ObjectStorage::ObjectID id) {
    writer.write_key("id"); writer.write_string(generate_id_string(id));
    writer.write_key("type"); writer.write_string("object");
    
    // Stroke
    if (obj.flags.has_stroke()) {
        writer.write_key("stroke");
        writer.begin_object();
        writer.write_key("color"); writer.write_color(obj.stroke_color);
        writer.write_key("width"); writer.write_number(obj.stroke_width);
        writer.write_key("style"); writer.write_string("SOLID");
        writer.end_object();
    } else {
        writer.write_key("stroke"); writer.write_null();
    }
    
    // Fill
    if (obj.flags.has_fill()) {
        writer.write_key("fill");
        writer.begin_object();
        writer.write_key("color"); writer.write_color(obj.fill_color);
        writer.end_object();
    } else {
        writer.write_key("fill"); writer.write_null();
    }
    
    writer.write_key("opacity"); writer.write_number(obj.opacity);
    
    // Transform (identity for now)
    writer.write_key("transform");
    writer.begin_object();
    writer.write_key("m11"); writer.write_number(1.0f);
    writer.write_key("m12"); writer.write_number(0.0f);
    writer.write_key("m13"); writer.write_number(0.0f);
    writer.write_key("m21"); writer.write_number(0.0f);
    writer.write_key("m22"); writer.write_number(1.0f);
    writer.write_key("m23"); writer.write_number(0.0f);
    writer.end_object();
    
    // Metadata
    writer.write_key("metadata");
    writer.begin_object();
    writer.end_object();
    
    // Timestamps (current time)
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", gmtime(&time_t));
    std::string timestamp = std::string(buffer) + "Z";
    
    writer.write_key("created_at"); writer.write_string(timestamp);
    writer.write_key("updated_at"); writer.write_string(timestamp);
}

void save_json(const Drawing& drawing, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) return;
    
    JsonWriter writer(file);
    
    writer.begin_object();
    
    // Drawing properties
    writer.write_key("width"); writer.write_number(drawing.get_width());
    writer.write_key("height"); writer.write_number(drawing.get_height());
    writer.write_key("background_color"); writer.write_color(drawing.get_background());
    
    // Metadata
    writer.write_key("metadata");
    writer.begin_object();
    writer.end_object();
    
    // Layers
    writer.write_key("layers");
    writer.begin_array();
    
    for (const auto& layer : drawing.get_layers()) {
        writer.write_comma();
        writer.write_indent();
        writer.begin_object();
        
        writer.write_key("id"); writer.write_string(generate_id_string(layer->get_id()));
        writer.write_key("name"); writer.write_string(layer->get_name());
        writer.write_key("visible"); writer.write_bool(layer->is_visible());
        writer.write_key("locked"); writer.write_bool(layer->is_locked());
        writer.write_key("opacity"); writer.write_number(layer->get_opacity());
        writer.write_key("z_index"); writer.write_number(static_cast<int>(layer->get_id()));
        
        // Objects in layer
        writer.write_key("objects");
        writer.begin_array();
        
        const auto& storage = drawing.get_storage();
        for (auto obj_id : layer->get_objects()) {
            writer.write_comma();
            writer.write_indent();
            writer.begin_object();
            
            switch (ObjectStorage::get_type(obj_id)) {
                case ObjectType::Circle: {
                    auto* circle = storage.get_circle(obj_id);
                    if (circle) {
                        write_object_base(writer, circle->base, obj_id);
                        writer.write_key("center"); 
                        writer.write_point(Point(circle->x, circle->y));
                        writer.write_key("radius"); writer.write_number(circle->radius);
                    }
                    break;
                }
                
                case ObjectType::Rectangle: {
                    auto* rect = storage.get_rectangle(obj_id);
                    if (rect) {
                        write_object_base(writer, rect->base, obj_id);
                        writer.write_key("x"); writer.write_number(rect->x);
                        writer.write_key("y"); writer.write_number(rect->y);
                        writer.write_key("width"); writer.write_number(rect->width);
                        writer.write_key("height"); writer.write_number(rect->height);
                    }
                    break;
                }
                
                case ObjectType::Line: {
                    auto* line = storage.get_line(obj_id);
                    if (line) {
                        write_object_base(writer, line->base, obj_id);
                        writer.write_key("start"); 
                        writer.write_point(Point(line->x1, line->y1));
                        writer.write_key("end"); 
                        writer.write_point(Point(line->x2, line->y2));
                    }
                    break;
                }
                
                case ObjectType::Polygon: {
                    auto* poly = storage.get_polygon(obj_id);
                    if (poly) {
                        write_object_base(writer, poly->base, obj_id);
                        writer.write_key("points");
                        writer.begin_array();
                        
                        auto [points, count] = storage.get_polygon_points(*poly);
                        for (size_t i = 0; i < count; ++i) {
                            writer.write_comma();
                            writer.write_indent();
                            writer.write_point(points[i]);
                        }
                        
                        writer.end_array();
                    }
                    break;
                }
                
                default:
                    break;
            }
            
            writer.end_object();
        }
        
        writer.end_array();
        writer.end_object();
    }
    
    writer.end_array();
    writer.end_object();
    
    file << "\n";
}

std::unique_ptr<Drawing> load_json(const std::string& filename) {
    // JSON loading would require a proper JSON parser
    // For now, return nullptr - would integrate with a library like nlohmann/json
    return nullptr;
}

} // namespace drawing