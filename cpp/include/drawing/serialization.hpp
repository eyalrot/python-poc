#pragma once

#include "drawing.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

namespace drawing {

// Simple binary format constants
namespace BinaryFormat {
    constexpr uint32_t MAGIC = 0x44525747; // "DRWG"
    constexpr uint32_t VERSION = 1;
    
    enum class ChunkType : uint16_t {
        Header = 1,
        Layer = 2,
        Circles = 3,
        Rectangles = 4,
        Lines = 5,
        Polygons = 6,
        PolygonPoints = 7,
        Ellipses = 8,
        Polylines = 9,
        PolylinePoints = 10,
        Arcs = 11,
        Texts = 12,
        TextStrings = 13,
        FontNames = 14,
        Paths = 15,
        PathSegments = 16,
        PathParameters = 17,
        Groups = 18,
        GroupChildren = 19,
        End = 999
    };
}

// Binary serializer
class BinarySerializer {
private:
    std::ostream& stream;
    
    template<typename T>
    void write_pod(const T& value) {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }
    
    void write_string(const std::string& str) {
        uint32_t len = str.length();
        write_pod(len);
        stream.write(str.data(), len);
    }
    
    template<typename T>
    void write_vector(const std::vector<T>& vec) {
        uint32_t count = vec.size();
        write_pod(count);
        if (count > 0) {
            stream.write(reinterpret_cast<const char*>(vec.data()), 
                        sizeof(T) * count);
        }
    }
    
public:
    explicit BinarySerializer(std::ostream& s) : stream(s) {}
    
    void serialize(const Drawing& drawing) {
        // Write magic and version
        write_pod(BinaryFormat::MAGIC);
        write_pod(BinaryFormat::VERSION);
        
        // Write header chunk
        write_pod(BinaryFormat::ChunkType::Header);
        write_pod(drawing.get_width());
        write_pod(drawing.get_height());
        write_pod(drawing.get_background());
        
        // Write layers
        for (const auto& layer : drawing.get_layers()) {
            write_pod(BinaryFormat::ChunkType::Layer);
            write_pod(layer->get_id());
            write_string(layer->get_name());
            write_pod(layer->is_visible());
            write_pod(layer->is_locked());
            write_pod(layer->get_opacity());
            
            // Write object IDs for this layer
            const auto& objects = layer->get_objects();
            write_pod(static_cast<uint32_t>(objects.size()));
            stream.write(reinterpret_cast<const char*>(objects.data()), 
                        sizeof(ObjectID) * objects.size());
        }
        
        // Write object arrays
        const auto& storage = drawing.get_storage();
        
        // Circles
        if (!storage.circles.empty()) {
            write_pod(BinaryFormat::ChunkType::Circles);
            write_vector(storage.circles);
        }
        
        // Rectangles
        if (!storage.rectangles.empty()) {
            write_pod(BinaryFormat::ChunkType::Rectangles);
            write_vector(storage.rectangles);
        }
        
        // Lines
        if (!storage.lines.empty()) {
            write_pod(BinaryFormat::ChunkType::Lines);
            write_vector(storage.lines);
        }
        
        // Polygons
        if (!storage.polygons.empty()) {
            write_pod(BinaryFormat::ChunkType::Polygons);
            write_vector(storage.polygons);
            
            // Polygon points
            write_pod(BinaryFormat::ChunkType::PolygonPoints);
            write_vector(storage.polygon_points);
        }
        
        // Ellipses
        if (!storage.ellipses.empty()) {
            write_pod(BinaryFormat::ChunkType::Ellipses);
            write_vector(storage.ellipses);
        }
        
        // Polylines
        if (!storage.polylines.empty()) {
            write_pod(BinaryFormat::ChunkType::Polylines);
            write_vector(storage.polylines);
            
            // Polyline points
            write_pod(BinaryFormat::ChunkType::PolylinePoints);
            write_vector(storage.polyline_points);
        }
        
        // Arcs
        if (!storage.arcs.empty()) {
            write_pod(BinaryFormat::ChunkType::Arcs);
            write_vector(storage.arcs);
        }
        
        // Texts
        if (!storage.texts.empty()) {
            write_pod(BinaryFormat::ChunkType::Texts);
            write_vector(storage.texts);
            
            // Text strings
            write_pod(BinaryFormat::ChunkType::TextStrings);
            write_pod(static_cast<uint32_t>(storage.text_strings.size()));
            for (const auto& str : storage.text_strings) {
                write_string(str);
            }
            
            // Font names
            write_pod(BinaryFormat::ChunkType::FontNames);
            write_pod(static_cast<uint32_t>(storage.font_names.size()));
            for (const auto& font : storage.font_names) {
                write_string(font);
            }
        }
        
        // Paths
        if (!storage.paths.empty()) {
            write_pod(BinaryFormat::ChunkType::Paths);
            write_vector(storage.paths);
            
            // Path segments
            write_pod(BinaryFormat::ChunkType::PathSegments);
            write_vector(storage.path_segments);
            
            // Path parameters
            write_pod(BinaryFormat::ChunkType::PathParameters);
            write_vector(storage.path_parameters);
        }
        
        // Groups
        if (!storage.groups.empty()) {
            write_pod(BinaryFormat::ChunkType::Groups);
            write_vector(storage.groups);
            
            // Group children
            write_pod(BinaryFormat::ChunkType::GroupChildren);
            write_vector(storage.group_children);
        }
        
        // End marker
        write_pod(BinaryFormat::ChunkType::End);
    }
};

// Binary deserializer
class BinaryDeserializer {
private:
    std::istream& stream;
    std::vector<Point> polygon_points;
    
    template<typename T>
    bool read_pod(T& value) {
        return stream.read(reinterpret_cast<char*>(&value), sizeof(T)).good();
    }
    
    bool read_string(std::string& str) {
        uint32_t len;
        if (!read_pod(len)) return false;
        if (len > 1000000) return false; // Sanity check
        
        str.resize(len);
        return stream.read(&str[0], len).good();
    }
    
    template<typename T>
    bool read_vector(std::vector<T>& vec) {
        uint32_t count;
        if (!read_pod(count)) return false;
        if (count > 10000000) return false; // Sanity check
        
        vec.resize(count);
        if (count > 0) {
            return stream.read(reinterpret_cast<char*>(vec.data()), 
                             sizeof(T) * count).good();
        }
        return true;
    }
    
public:
    explicit BinaryDeserializer(std::istream& s) : stream(s) {}
    
    std::unique_ptr<Drawing> deserialize() {
        // Check magic and version
        uint32_t magic, version;
        if (!read_pod(magic) || magic != BinaryFormat::MAGIC) {
            return nullptr;
        }
        if (!read_pod(version) || version != BinaryFormat::VERSION) {
            return nullptr;
        }
        
        auto drawing = std::make_unique<Drawing>();
        
        while (stream.good()) {
            BinaryFormat::ChunkType chunk_type;
            if (!read_pod(chunk_type)) break;
            
            switch (chunk_type) {
                case BinaryFormat::ChunkType::Header: {
                    float width, height;
                    Color bg;
                    if (!read_pod(width) || !read_pod(height) || !read_pod(bg)) {
                        return nullptr;
                    }
                    drawing->set_width(width);
                    drawing->set_height(height);
                    drawing->set_background(bg);
                    break;
                }
                
                case BinaryFormat::ChunkType::Layer: {
                    uint8_t id;
                    std::string name;
                    bool visible, locked;
                    float opacity;
                    uint32_t obj_count;
                    
                    if (!read_pod(id) || !read_string(name) || 
                        !read_pod(visible) || !read_pod(locked) || 
                        !read_pod(opacity) || !read_pod(obj_count)) {
                        return nullptr;
                    }
                    
                    // Skip object IDs for now - will reconstruct later
                    stream.seekg(obj_count * sizeof(ObjectID), 
                               std::ios::cur);
                    break;
                }
                
                case BinaryFormat::ChunkType::Circles: {
                    if (!read_vector(drawing->get_storage().circles)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::Rectangles: {
                    if (!read_vector(drawing->get_storage().rectangles)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::Lines: {
                    if (!read_vector(drawing->get_storage().lines)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::Polygons: {
                    if (!read_vector(drawing->get_storage().polygons)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::PolygonPoints: {
                    if (!read_vector(polygon_points)) {
                        return nullptr;
                    }
                    // TODO: Copy polygon points to storage
                    break;
                }
                
                case BinaryFormat::ChunkType::Ellipses: {
                    if (!read_vector(drawing->get_storage().ellipses)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::Polylines: {
                    if (!read_vector(drawing->get_storage().polylines)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::PolylinePoints: {
                    if (!read_vector(drawing->get_storage().polyline_points)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::Arcs: {
                    if (!read_vector(drawing->get_storage().arcs)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::Texts: {
                    if (!read_vector(drawing->get_storage().texts)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::TextStrings: {
                    uint32_t count;
                    if (!read_pod(count)) return nullptr;
                    drawing->get_storage().text_strings.resize(count);
                    for (uint32_t i = 0; i < count; ++i) {
                        if (!read_string(drawing->get_storage().text_strings[i])) {
                            return nullptr;
                        }
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::FontNames: {
                    uint32_t count;
                    if (!read_pod(count)) return nullptr;
                    drawing->get_storage().font_names.resize(count);
                    for (uint32_t i = 0; i < count; ++i) {
                        if (!read_string(drawing->get_storage().font_names[i])) {
                            return nullptr;
                        }
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::Paths: {
                    if (!read_vector(drawing->get_storage().paths)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::PathSegments: {
                    if (!read_vector(drawing->get_storage().path_segments)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::PathParameters: {
                    if (!read_vector(drawing->get_storage().path_parameters)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::Groups: {
                    if (!read_vector(drawing->get_storage().groups)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::GroupChildren: {
                    if (!read_vector(drawing->get_storage().group_children)) {
                        return nullptr;
                    }
                    break;
                }
                
                case BinaryFormat::ChunkType::End:
                    return drawing;
                    
                default:
                    return nullptr; // Unknown chunk
            }
        }
        
        return drawing;
    }
};

// Convenience functions
inline bool save_binary(const Drawing& drawing, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;
    
    BinarySerializer serializer(file);
    serializer.serialize(drawing);
    return file.good();
}

inline std::unique_ptr<Drawing> load_binary(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return nullptr;
    
    BinaryDeserializer deserializer(file);
    return deserializer.deserialize();
}

// JSON serialization (for compatibility)
void save_json(const Drawing& drawing, const std::string& filename);
std::unique_ptr<Drawing> load_json(const std::string& filename);

} // namespace drawing