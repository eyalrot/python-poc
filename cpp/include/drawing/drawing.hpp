#pragma once

#include "types.hpp"
#include "objects.hpp"
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>

namespace drawing {

// Layer class - manages a collection of objects
class Layer {
private:
    uint8_t id;
    std::string name;
    bool visible;
    bool locked;
    float opacity;
    std::vector<ObjectID> object_ids;
    
public:
    Layer(uint8_t id, const std::string& name = "") 
        : id(id), name(name), visible(true), locked(false), opacity(1.0f) {}
    
    uint8_t get_id() const { return id; }
    const std::string& get_name() const { return name; }
    bool is_visible() const { return visible; }
    bool is_locked() const { return locked; }
    float get_opacity() const { return opacity; }
    
    void set_visible(bool v) { visible = v; }
    void set_locked(bool l) { locked = l; }
    void set_opacity(float o) { opacity = std::clamp(o, 0.0f, 1.0f); }
    
    void add_object(ObjectID id) {
        object_ids.push_back(id);
    }
    
    void remove_object(ObjectID id) {
        object_ids.erase(
            std::remove(object_ids.begin(), object_ids.end(), id), 
            object_ids.end()
        );
    }
    
    const std::vector<ObjectID>& get_objects() const { 
        return object_ids; 
    }
    
    size_t object_count() const { return object_ids.size(); }
};

// Main drawing class
class Drawing {
private:
    float width;
    float height;
    Color background_color;
    std::vector<std::unique_ptr<Layer>> layers;
    ObjectStorage storage;
    uint8_t next_layer_id;
    
public:
    Drawing(float width = 800, float height = 600) 
        : width(width), height(height), 
          background_color(Color::WHITE), 
          next_layer_id(0) {
        // Create default layer
        add_layer("Default");
    }
    
    // Drawing properties
    float get_width() const { return width; }
    float get_height() const { return height; }
    Color get_background() const { return background_color; }
    
    void set_width(float w) { width = w; }
    void set_height(float h) { height = h; }
    void set_background(Color c) { background_color = c; }
    
    // Layer management
    uint8_t add_layer(const std::string& name = "") {
        if (layers.size() >= 255) {
            throw std::runtime_error("Maximum number of layers (255) reached");
        }
        layers.push_back(std::make_unique<Layer>(next_layer_id, name));
        return next_layer_id++;
    }
    
    Layer* get_layer(uint8_t id) {
        auto it = std::find_if(layers.begin(), layers.end(),
            [id](const auto& layer) { return layer->get_id() == id; });
        return it != layers.end() ? it->get() : nullptr;
    }
    
    const std::vector<std::unique_ptr<Layer>>& get_layers() const { 
        return layers; 
    }
    
    // Object creation shortcuts (adds to current/first layer)
    ObjectID add_circle(float x, float y, float radius, uint8_t layer_id = 0) {
        auto id = storage.add_circle(x, y, radius);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* circle = storage.get_circle(id)) {
                circle->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_rectangle(float x, float y, float w, float h, float corner_radius = 0, uint8_t layer_id = 0) {
        auto id = storage.add_rectangle(x, y, w, h, corner_radius);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* rect = storage.get_rectangle(id)) {
                rect->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_line(float x1, float y1, float x2, float y2, LineStyle line_style = LineStyle::Solid, uint8_t layer_id = 0) {
        auto id = storage.add_line(x1, y1, x2, y2, line_style);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* line = storage.get_line(id)) {
                line->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_polygon(const std::vector<Point>& points, bool closed = true, uint8_t layer_id = 0) {
        auto id = storage.add_polygon(points, closed);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* poly = storage.get_polygon(id)) {
                poly->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_ellipse(float x, float y, float rx, float ry, float rotation = 0, uint8_t layer_id = 0) {
        auto id = storage.add_ellipse(x, y, rx, ry, rotation);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* ellipse = storage.get_ellipse(id)) {
                ellipse->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_polyline(const std::vector<Point>& points, LineStyle line_style = LineStyle::Solid, uint8_t layer_id = 0) {
        auto id = storage.add_polyline(points, line_style);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* polyline = storage.get_polyline(id)) {
                polyline->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_arc(float x, float y, float radius, float start_angle, float end_angle, uint8_t layer_id = 0) {
        auto id = storage.add_arc(x, y, radius, start_angle, end_angle);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* arc = storage.get_arc(id)) {
                arc->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_text(float x, float y, const std::string& text, 
                                     float font_size = 16.0f, 
                                     const std::string& font_name = "Arial",
                                     TextAlign align = TextAlign::Left,
                                     TextBaseline baseline = TextBaseline::Alphabetic,
                                     uint8_t layer_id = 0) {
        auto id = storage.add_text(x, y, text, font_size, font_name, align, baseline);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* txt = storage.get_text(id)) {
                txt->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_path(const std::string& path_data, uint8_t layer_id = 0) {
        auto id = storage.add_path(path_data);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* path = storage.get_path(id)) {
                path->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_group(uint8_t layer_id = 0) {
        auto id = storage.add_group();
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* group = storage.get_group(id)) {
                group->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectID add_group(const std::vector<ObjectID>& children, 
                                     uint8_t layer_id = 0) {
        auto id = storage.add_group(children);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* group = storage.get_group(id)) {
                group->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    void add_to_group(ObjectID group_id, ObjectID child_id) {
        storage.add_to_group(group_id, child_id);
    }
    
    // Gradient support
    uint16_t add_linear_gradient(const std::vector<GradientStop>& stops, float angle = 0.0f) {
        return storage.add_linear_gradient(stops, angle);
    }
    
    uint16_t add_radial_gradient(const std::vector<GradientStop>& stops, 
                                float center_x, float center_y, float radius) {
        return storage.add_radial_gradient(stops, center_x, center_y, radius);
    }
    
    void set_object_gradient(ObjectID id, uint16_t gradient_id) {
        storage.set_object_gradient(id, gradient_id);
    }
    
    // Pattern support
    uint16_t add_pattern(const std::string& pattern_name) {
        return storage.add_pattern(pattern_name);
    }
    
    void set_object_pattern(ObjectID id, uint16_t pattern_id) {
        storage.set_object_pattern(id, pattern_id);
    }
    
    // Object naming
    void set_object_name(ObjectID id, const std::string& name) {
        storage.set_object_name(id, name);
    }
    
    std::string get_object_name(ObjectID id) const {
        return storage.get_object_name(id);
    }
    
    // Metadata support
    void set_object_metadata(ObjectID id, const std::string& key, const std::string& value) {
        storage.set_object_metadata(id, key, value);
    }
    
    std::string get_object_metadata(ObjectID id, const std::string& key) const {
        return storage.get_object_metadata(id, key);
    }
    
    std::vector<std::pair<std::string, std::string>> get_all_object_metadata(ObjectID id) const {
        return storage.get_all_object_metadata(id);
    }
    
    // Access to storage for advanced operations
    ObjectStorage& get_storage() { return storage; }
    const ObjectStorage& get_storage() const { return storage; }
    
    // Drawing-wide operations
    BoundingBox get_bounding_box() const;
    std::vector<ObjectID> find_objects_in_rect(const BoundingBox& rect) const {
        return storage.find_in_rect(rect);
    }
    
    // Statistics
    size_t total_objects() const { return storage.total_objects(); }
    size_t memory_usage() const { 
        return storage.memory_usage() + 
               sizeof(Drawing) + 
               layers.size() * sizeof(Layer);
    }
};

} // namespace drawing