#pragma once

#include "types.hpp"
#include <memory>
#include <vector>

namespace drawing {

// Base compact object header (20 bytes)
struct CompactObject {
    ObjectType type;        // 1 byte
    uint8_t layer_id;       // 1 byte  
    ObjectFlags flags;      // 2 bytes
    Color fill_color;       // 4 bytes
    Color stroke_color;     // 4 bytes
    float stroke_width;     // 4 bytes
    float opacity;          // 4 bytes
    
    CompactObject(ObjectType t = ObjectType::None) 
        : type(t), layer_id(0), flags(), 
          fill_color(Color::BLACK), stroke_color(Color::BLACK),
          stroke_width(1.0f), opacity(1.0f) {}
    
    BoundingBox get_bounding_box() const;
};

// Compact Circle (32 bytes total)
struct CompactCircle {
    CompactObject base;     // 20 bytes
    float x, y, radius;     // 12 bytes
    
    CompactCircle() : base(ObjectType::Circle), x(0), y(0), radius(0) {}
    CompactCircle(float x, float y, float r) 
        : base(ObjectType::Circle), x(x), y(y), radius(r) {}
    
    BoundingBox get_bounding_box() const {
        return BoundingBox(x - radius, y - radius, x + radius, y + radius);
    }
};

// Compact Rectangle (36 bytes total)
struct CompactRectangle {
    CompactObject base;     // 20 bytes
    float x, y, width, height; // 16 bytes
    
    CompactRectangle() : base(ObjectType::Rectangle), x(0), y(0), width(0), height(0) {}
    CompactRectangle(float x, float y, float w, float h)
        : base(ObjectType::Rectangle), x(x), y(y), width(w), height(h) {}
    
    BoundingBox get_bounding_box() const {
        return BoundingBox(x, y, x + width, y + height);
    }
};

// Compact Line (36 bytes total)
struct CompactLine {
    CompactObject base;     // 20 bytes
    float x1, y1, x2, y2;   // 16 bytes
    
    CompactLine() : base(ObjectType::Line), x1(0), y1(0), x2(0), y2(0) {}
    CompactLine(float x1, float y1, float x2, float y2)
        : base(ObjectType::Line), x1(x1), y1(y1), x2(x2), y2(y2) {}
    
    BoundingBox get_bounding_box() const {
        return BoundingBox(
            std::min(x1, x2), std::min(y1, y2),
            std::max(x1, x2), std::max(y1, y2)
        );
    }
};

// Compact Ellipse (40 bytes total)
struct CompactEllipse {
    CompactObject base;     // 20 bytes
    float x, y;             // 8 bytes - center position
    float rx, ry;           // 8 bytes - radii
    float rotation;         // 4 bytes - rotation angle in radians
    
    CompactEllipse() : base(ObjectType::Ellipse), x(0), y(0), rx(0), ry(0), rotation(0) {}
    CompactEllipse(float x, float y, float rx, float ry, float rotation = 0)
        : base(ObjectType::Ellipse), x(x), y(y), rx(rx), ry(ry), rotation(rotation) {}
    
    BoundingBox get_bounding_box() const {
        // For rotated ellipse, we need to calculate the axis-aligned bounding box
        // This is a simplified version - for exact bounds, we'd need trig calculations
        float max_radius = std::max(rx, ry);
        return BoundingBox(x - max_radius, y - max_radius, x + max_radius, y + max_radius);
    }
};

// For variable-size objects like Polygon, we use a different approach
struct CompactPolygon {
    CompactObject base;     // 20 bytes
    uint32_t point_offset;  // 4 bytes - offset into point array
    uint32_t point_count;   // 4 bytes
    // 4 bytes padding for alignment
    
    CompactPolygon() : base(ObjectType::Polygon), point_offset(0), point_count(0) {}
};

// Compact Polyline (28 bytes total) - same structure as polygon but open path
struct CompactPolyline {
    CompactObject base;     // 20 bytes
    uint32_t point_offset;  // 4 bytes - offset into point array
    uint32_t point_count;   // 4 bytes
    
    CompactPolyline() : base(ObjectType::Polyline), point_offset(0), point_count(0) {}
};

// Compact Arc (36 bytes total)
struct CompactArc {
    CompactObject base;     // 20 bytes
    float x, y;             // 8 bytes - center position
    float radius;           // 4 bytes
    float start_angle;      // 4 bytes - in radians
    float end_angle;        // 4 bytes - in radians
    
    CompactArc() : base(ObjectType::Arc), x(0), y(0), radius(0), start_angle(0), end_angle(0) {}
    CompactArc(float x, float y, float radius, float start_angle, float end_angle)
        : base(ObjectType::Arc), x(x), y(y), radius(radius), 
          start_angle(start_angle), end_angle(end_angle) {}
    
    BoundingBox get_bounding_box() const {
        // Simple approximation - use full circle bounds
        // For exact bounds, we'd need to check which quadrants the arc spans
        return BoundingBox(x - radius, y - radius, x + radius, y + radius);
    }
};

// Object storage using Structure-of-Arrays for better cache performance
class ObjectStorage {
public:
    // Fixed-size object arrays (public for now, will add proper accessors later)
    std::vector<CompactCircle> circles;
    std::vector<CompactRectangle> rectangles;
    std::vector<CompactLine> lines;
    std::vector<CompactEllipse> ellipses;
    std::vector<CompactPolygon> polygons;
    std::vector<CompactPolyline> polylines;
    std::vector<CompactArc> arcs;
    
    // Variable data storage (public for serialization)
    std::vector<Point> polygon_points;
    std::vector<Point> polyline_points;
    
private:
    std::vector<Transform2D> transforms;
    
    // Spatial index (to be implemented)
    // std::unique_ptr<RTree> spatial_index;
    
public:
    // Object ID is encoded as: [type:8bits][index:24bits]
    using ObjectID = uint32_t;
    
    static constexpr ObjectID make_id(ObjectType type, uint32_t index) {
        return (static_cast<uint32_t>(type) << 24) | (index & 0xFFFFFF);
    }
    
    static constexpr ObjectType get_type(ObjectID id) {
        return static_cast<ObjectType>(id >> 24);
    }
    
    static constexpr uint32_t get_index(ObjectID id) {
        return id & 0xFFFFFF;
    }
    
    // Add objects
    ObjectID add_circle(float x, float y, float radius) {
        circles.emplace_back(x, y, radius);
        return make_id(ObjectType::Circle, circles.size() - 1);
    }
    
    ObjectID add_rectangle(float x, float y, float width, float height) {
        rectangles.emplace_back(x, y, width, height);
        return make_id(ObjectType::Rectangle, rectangles.size() - 1);
    }
    
    ObjectID add_line(float x1, float y1, float x2, float y2) {
        lines.emplace_back(x1, y1, x2, y2);
        return make_id(ObjectType::Line, lines.size() - 1);
    }
    
    ObjectID add_ellipse(float x, float y, float rx, float ry, float rotation = 0) {
        ellipses.emplace_back(x, y, rx, ry, rotation);
        return make_id(ObjectType::Ellipse, ellipses.size() - 1);
    }
    
    ObjectID add_polygon(const std::vector<Point>& points) {
        CompactPolygon poly;
        poly.point_offset = polygon_points.size();
        poly.point_count = points.size();
        polygon_points.insert(polygon_points.end(), points.begin(), points.end());
        polygons.push_back(poly);
        return make_id(ObjectType::Polygon, polygons.size() - 1);
    }
    
    ObjectID add_polyline(const std::vector<Point>& points) {
        CompactPolyline polyline;
        polyline.point_offset = polyline_points.size();
        polyline.point_count = points.size();
        polyline_points.insert(polyline_points.end(), points.begin(), points.end());
        polylines.push_back(polyline);
        return make_id(ObjectType::Polyline, polylines.size() - 1);
    }
    
    ObjectID add_arc(float x, float y, float radius, float start_angle, float end_angle) {
        arcs.emplace_back(x, y, radius, start_angle, end_angle);
        return make_id(ObjectType::Arc, arcs.size() - 1);
    }
    
    // Get objects
    CompactCircle* get_circle(ObjectID id) {
        if (get_type(id) != ObjectType::Circle) return nullptr;
        uint32_t idx = get_index(id);
        return idx < circles.size() ? &circles[idx] : nullptr;
    }
    
    const CompactCircle* get_circle(ObjectID id) const {
        if (get_type(id) != ObjectType::Circle) return nullptr;
        uint32_t idx = get_index(id);
        return idx < circles.size() ? &circles[idx] : nullptr;
    }
    
    CompactRectangle* get_rectangle(ObjectID id) {
        if (get_type(id) != ObjectType::Rectangle) return nullptr;
        uint32_t idx = get_index(id);
        return idx < rectangles.size() ? &rectangles[idx] : nullptr;
    }
    
    const CompactRectangle* get_rectangle(ObjectID id) const {
        if (get_type(id) != ObjectType::Rectangle) return nullptr;
        uint32_t idx = get_index(id);
        return idx < rectangles.size() ? &rectangles[idx] : nullptr;
    }
    
    CompactLine* get_line(ObjectID id) {
        if (get_type(id) != ObjectType::Line) return nullptr;
        uint32_t idx = get_index(id);
        return idx < lines.size() ? &lines[idx] : nullptr;
    }
    
    const CompactLine* get_line(ObjectID id) const {
        if (get_type(id) != ObjectType::Line) return nullptr;
        uint32_t idx = get_index(id);
        return idx < lines.size() ? &lines[idx] : nullptr;
    }
    
    CompactEllipse* get_ellipse(ObjectID id) {
        if (get_type(id) != ObjectType::Ellipse) return nullptr;
        uint32_t idx = get_index(id);
        return idx < ellipses.size() ? &ellipses[idx] : nullptr;
    }
    
    const CompactEllipse* get_ellipse(ObjectID id) const {
        if (get_type(id) != ObjectType::Ellipse) return nullptr;
        uint32_t idx = get_index(id);
        return idx < ellipses.size() ? &ellipses[idx] : nullptr;
    }
    
    CompactPolygon* get_polygon(ObjectID id) {
        if (get_type(id) != ObjectType::Polygon) return nullptr;
        uint32_t idx = get_index(id);
        return idx < polygons.size() ? &polygons[idx] : nullptr;
    }
    
    const CompactPolygon* get_polygon(ObjectID id) const {
        if (get_type(id) != ObjectType::Polygon) return nullptr;
        uint32_t idx = get_index(id);
        return idx < polygons.size() ? &polygons[idx] : nullptr;
    }
    
    CompactPolyline* get_polyline(ObjectID id) {
        if (get_type(id) != ObjectType::Polyline) return nullptr;
        uint32_t idx = get_index(id);
        return idx < polylines.size() ? &polylines[idx] : nullptr;
    }
    
    const CompactPolyline* get_polyline(ObjectID id) const {
        if (get_type(id) != ObjectType::Polyline) return nullptr;
        uint32_t idx = get_index(id);
        return idx < polylines.size() ? &polylines[idx] : nullptr;
    }
    
    CompactArc* get_arc(ObjectID id) {
        if (get_type(id) != ObjectType::Arc) return nullptr;
        uint32_t idx = get_index(id);
        return idx < arcs.size() ? &arcs[idx] : nullptr;
    }
    
    const CompactArc* get_arc(ObjectID id) const {
        if (get_type(id) != ObjectType::Arc) return nullptr;
        uint32_t idx = get_index(id);
        return idx < arcs.size() ? &arcs[idx] : nullptr;
    }
    
    // Get polygon points
    std::pair<const Point*, size_t> get_polygon_points(const CompactPolygon& poly) const {
        if (poly.point_offset + poly.point_count > polygon_points.size()) {
            return {nullptr, 0};
        }
        return {&polygon_points[poly.point_offset], poly.point_count};
    }
    
    // Get polyline points
    std::pair<const Point*, size_t> get_polyline_points(const CompactPolyline& polyline) const {
        if (polyline.point_offset + polyline.point_count > polyline_points.size()) {
            return {nullptr, 0};
        }
        return {&polyline_points[polyline.point_offset], polyline.point_count};
    }
    
    // Batch operations
    void set_fill_color(const std::vector<ObjectID>& ids, Color color);
    void set_stroke_color(const std::vector<ObjectID>& ids, Color color);
    void set_opacity(const std::vector<ObjectID>& ids, float opacity);
    
    // Spatial queries
    std::vector<ObjectID> find_in_rect(const BoundingBox& rect) const;
    std::vector<ObjectID> find_at_point(const Point& point, float tolerance = 1.0f) const;
    
    // Statistics
    size_t total_objects() const {
        return circles.size() + rectangles.size() + lines.size() + ellipses.size() + 
               polygons.size() + polylines.size() + arcs.size();
    }
    
    size_t memory_usage() const {
        return sizeof(CompactCircle) * circles.size() +
               sizeof(CompactRectangle) * rectangles.size() +
               sizeof(CompactLine) * lines.size() +
               sizeof(CompactEllipse) * ellipses.size() +
               sizeof(CompactPolygon) * polygons.size() +
               sizeof(CompactPolyline) * polylines.size() +
               sizeof(CompactArc) * arcs.size() +
               sizeof(Point) * polygon_points.size() +
               sizeof(Point) * polyline_points.size() +
               sizeof(Transform2D) * transforms.size();
    }
};

} // namespace drawing