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
    std::vector<ObjectStorage::ObjectID> object_ids;
    
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
    
    void add_object(ObjectStorage::ObjectID id) {
        object_ids.push_back(id);
    }
    
    void remove_object(ObjectStorage::ObjectID id) {
        object_ids.erase(
            std::remove(object_ids.begin(), object_ids.end(), id), 
            object_ids.end()
        );
    }
    
    const std::vector<ObjectStorage::ObjectID>& get_objects() const { 
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
    ObjectStorage::ObjectID add_circle(float x, float y, float radius, uint8_t layer_id = 0) {
        auto id = storage.add_circle(x, y, radius);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* circle = storage.get_circle(id)) {
                circle->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectStorage::ObjectID add_rectangle(float x, float y, float w, float h, uint8_t layer_id = 0) {
        auto id = storage.add_rectangle(x, y, w, h);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* rect = storage.get_rectangle(id)) {
                rect->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectStorage::ObjectID add_line(float x1, float y1, float x2, float y2, uint8_t layer_id = 0) {
        auto id = storage.add_line(x1, y1, x2, y2);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* line = storage.get_line(id)) {
                line->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectStorage::ObjectID add_polygon(const std::vector<Point>& points, uint8_t layer_id = 0) {
        auto id = storage.add_polygon(points);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* poly = storage.get_polygon(id)) {
                poly->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectStorage::ObjectID add_ellipse(float x, float y, float rx, float ry, float rotation = 0, uint8_t layer_id = 0) {
        auto id = storage.add_ellipse(x, y, rx, ry, rotation);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* ellipse = storage.get_ellipse(id)) {
                ellipse->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectStorage::ObjectID add_polyline(const std::vector<Point>& points, uint8_t layer_id = 0) {
        auto id = storage.add_polyline(points);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* polyline = storage.get_polyline(id)) {
                polyline->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    ObjectStorage::ObjectID add_arc(float x, float y, float radius, float start_angle, float end_angle, uint8_t layer_id = 0) {
        auto id = storage.add_arc(x, y, radius, start_angle, end_angle);
        if (auto* layer = get_layer(layer_id)) {
            layer->add_object(id);
            if (auto* arc = storage.get_arc(id)) {
                arc->base.layer_id = layer_id;
            }
        }
        return id;
    }
    
    // Access to storage for advanced operations
    ObjectStorage& get_storage() { return storage; }
    const ObjectStorage& get_storage() const { return storage; }
    
    // Drawing-wide operations
    BoundingBox get_bounding_box() const;
    std::vector<ObjectStorage::ObjectID> find_objects_in_rect(const BoundingBox& rect) const {
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