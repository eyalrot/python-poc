#pragma once

#include "types.hpp"
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

namespace drawing {

// Forward declarations
class ObjectStorage;

// Object ID is encoded as: [type:8bits][index:24bits]
using ObjectID = uint32_t;

// Base compact object header (28 bytes)
struct CompactObject {
    ObjectType type;        // 1 byte
    uint8_t layer_id;       // 1 byte  
    ObjectFlags flags;      // 2 bytes
    Color fill_color;       // 4 bytes
    Color stroke_color;     // 4 bytes
    float stroke_width;     // 4 bytes
    float opacity;          // 4 bytes
    uint16_t gradient_id;   // 2 bytes - index into gradients array (0xFFFF = none)
    uint16_t pattern_id;    // 2 bytes - index into patterns array (0xFFFF = none)
    uint32_t name_id;       // 4 bytes - index into object names array (0xFFFFFFFF = none)
    
    CompactObject(ObjectType t = ObjectType::None) 
        : type(t), layer_id(0), flags(), 
          fill_color(Color::BLACK), stroke_color(Color::BLACK),
          stroke_width(1.0f), opacity(1.0f), gradient_id(0xFFFF), pattern_id(0xFFFF), 
          name_id(0xFFFFFFFF) {}
    
    BoundingBox get_bounding_box() const;
};

// Compact Circle (40 bytes total)
struct CompactCircle {
    CompactObject base;     // 28 bytes
    float x, y, radius;     // 12 bytes
    
    CompactCircle() : base(ObjectType::Circle), x(0), y(0), radius(0) {}
    CompactCircle(float x, float y, float r) 
        : base(ObjectType::Circle), x(x), y(y), radius(r) {}
    
    BoundingBox get_bounding_box() const {
        return BoundingBox(x - radius, y - radius, x + radius, y + radius);
    }
};

// Compact Rectangle (48 bytes total)
struct CompactRectangle {
    CompactObject base;     // 28 bytes
    float x, y, width, height; // 16 bytes
    float corner_radius;    // 4 bytes
    
    CompactRectangle() : base(ObjectType::Rectangle), x(0), y(0), width(0), height(0), corner_radius(0) {}
    CompactRectangle(float x, float y, float w, float h, float corner_radius = 0)
        : base(ObjectType::Rectangle), x(x), y(y), width(w), height(h), corner_radius(corner_radius) {}
    
    BoundingBox get_bounding_box() const {
        return BoundingBox(x, y, x + width, y + height);
    }
};

// Compact Line (48 bytes total)
struct CompactLine {
    CompactObject base;     // 28 bytes
    float x1, y1, x2, y2;   // 16 bytes
    LineStyle line_style;   // 1 byte
    uint8_t padding[3];     // 3 bytes padding for alignment
    
    CompactLine() : base(ObjectType::Line), x1(0), y1(0), x2(0), y2(0), 
                    line_style(LineStyle::Solid) {
        padding[0] = padding[1] = padding[2] = 0;
    }
    CompactLine(float x1, float y1, float x2, float y2, LineStyle style = LineStyle::Solid)
        : base(ObjectType::Line), x1(x1), y1(y1), x2(x2), y2(y2), line_style(style) {
        padding[0] = padding[1] = padding[2] = 0;
    }
    
    BoundingBox get_bounding_box() const {
        return BoundingBox(
            std::min(x1, x2), std::min(y1, y2),
            std::max(x1, x2), std::max(y1, y2)
        );
    }
};

// Compact Ellipse (48 bytes total)
struct CompactEllipse {
    CompactObject base;     // 28 bytes
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
    CompactObject base;     // 28 bytes
    uint32_t point_offset;  // 4 bytes - offset into point array
    uint32_t point_count;   // 4 bytes
    bool closed;            // 1 byte - whether polygon is closed
    uint8_t padding[3];     // 3 bytes padding for alignment
    
    CompactPolygon() : base(ObjectType::Polygon), point_offset(0), point_count(0), closed(true) {
        padding[0] = padding[1] = padding[2] = 0;
    }
    CompactPolygon(uint32_t offset, uint32_t count, bool closed = true)
        : base(ObjectType::Polygon), point_offset(offset), point_count(count), closed(closed) {
        padding[0] = padding[1] = padding[2] = 0;
    }
};

// Compact Polyline (40 bytes total) - same structure as polygon but open path
struct CompactPolyline {
    CompactObject base;     // 28 bytes
    uint32_t point_offset;  // 4 bytes - offset into point array
    uint32_t point_count;   // 4 bytes
    LineStyle line_style;   // 1 byte
    uint8_t padding[3];     // 3 bytes padding for alignment
    
    CompactPolyline() : base(ObjectType::Polyline), point_offset(0), point_count(0), 
                        line_style(LineStyle::Solid) {
        padding[0] = padding[1] = padding[2] = 0;
    }
    CompactPolyline(uint32_t offset, uint32_t count, LineStyle style = LineStyle::Solid)
        : base(ObjectType::Polyline), point_offset(offset), point_count(count), 
          line_style(style) {
        padding[0] = padding[1] = padding[2] = 0;
    }
};

// Compact Arc (44 bytes total)
struct CompactArc {
    CompactObject base;     // 28 bytes
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

// Text alignment flags
enum class TextAlign : uint8_t {
    Left = 0,
    Center = 1,
    Right = 2
};

enum class TextBaseline : uint8_t {
    Top = 0,
    Middle = 1,
    Bottom = 2,
    Alphabetic = 3
};

// Compact Text (48 bytes total)
struct CompactText {
    CompactObject base;     // 28 bytes
    float x, y;             // 8 bytes - position
    uint32_t text_index;    // 4 bytes - index into text storage
    float font_size;        // 4 bytes
    uint16_t font_index;    // 2 bytes - index into font name storage
    uint8_t align;          // 1 byte - TextAlign
    uint8_t baseline;       // 1 byte - TextBaseline
    
    CompactText() : base(ObjectType::Text), x(0), y(0), text_index(0), 
                    font_size(16), font_index(0), align(0), baseline(0) {}
    CompactText(float x, float y, uint32_t text_idx, float size, uint16_t font_idx,
                TextAlign align = TextAlign::Left, TextBaseline baseline = TextBaseline::Alphabetic)
        : base(ObjectType::Text), x(x), y(y), text_index(text_idx), 
          font_size(size), font_index(font_idx), 
          align(static_cast<uint8_t>(align)), baseline(static_cast<uint8_t>(baseline)) {}
    
    BoundingBox get_bounding_box() const {
        // Approximate bounding box - would need font metrics for exact bounds
        float estimated_width = font_size * 0.6f * 10; // Rough estimate
        float estimated_height = font_size * 1.2f;
        
        float left = x;
        if (align == static_cast<uint8_t>(TextAlign::Center)) {
            left = x - estimated_width / 2;
        } else if (align == static_cast<uint8_t>(TextAlign::Right)) {
            left = x - estimated_width;
        }
        
        float top = y;
        if (baseline == static_cast<uint8_t>(TextBaseline::Middle)) {
            top = y - estimated_height / 2;
        } else if (baseline == static_cast<uint8_t>(TextBaseline::Top)) {
            top = y;
        } else if (baseline == static_cast<uint8_t>(TextBaseline::Bottom)) {
            top = y - estimated_height;
        } else { // Alphabetic
            top = y - estimated_height * 0.8f;
        }
        
        return BoundingBox(left, top, left + estimated_width, top + estimated_height);
    }
};

// Path command types (simplified SVG subset)
enum class PathCommand : uint8_t {
    MoveTo = 0,      // M x y
    LineTo = 1,      // L x y
    CurveTo = 2,     // C x1 y1 x2 y2 x y
    QuadTo = 3,      // Q x1 y1 x y
    ArcTo = 4,       // A rx ry rot large sweep x y
    Close = 5        // Z
};

// Compact path command storage
struct PathSegment {
    PathCommand cmd;
    uint8_t param_count;  // Number of parameters for this command
    uint16_t param_offset; // Offset into path parameters array
    
    PathSegment() : cmd(PathCommand::MoveTo), param_count(0), param_offset(0) {}
    PathSegment(PathCommand c, uint8_t count, uint16_t offset) 
        : cmd(c), param_count(count), param_offset(offset) {}
};

// Compact Path (40 bytes total)
struct CompactPath {
    CompactObject base;     // 28 bytes
    uint32_t segment_offset; // 4 bytes - offset into path segments
    uint16_t segment_count;  // 2 bytes - number of segments
    uint16_t param_offset;   // 2 bytes - offset into path parameters
    uint16_t param_count;    // 2 bytes - total parameter count
    uint16_t flags;          // 2 bytes - reserved for future use
    
    CompactPath() : base(ObjectType::Path), segment_offset(0), segment_count(0),
                    param_offset(0), param_count(0), flags(0) {}
    
    CompactPath(uint32_t seg_offset, uint16_t seg_count, 
                uint16_t par_offset, uint16_t par_count)
        : base(ObjectType::Path), segment_offset(seg_offset), segment_count(seg_count),
          param_offset(par_offset), param_count(par_count), flags(0) {}
    
    // Calculate bounding box from path data (requires access to segments and params)
    BoundingBox calculate_bbox(const std::vector<PathSegment>& segments, 
                              const std::vector<float>& params) const {
        BoundingBox bbox;
        float current_x = 0, current_y = 0;
        bool has_points = false;
        
        for (uint16_t i = 0; i < segment_count; ++i) {
            const auto& seg = segments[segment_offset + i];
            const float* p = &params[seg.param_offset];
            
            switch (seg.cmd) {
                case PathCommand::MoveTo:
                    current_x = p[0];
                    current_y = p[1];
                    if (!has_points) {
                        bbox = BoundingBox(current_x, current_y, current_x, current_y);
                        has_points = true;
                    } else {
                        bbox.expand(Point(current_x, current_y));
                    }
                    break;
                    
                case PathCommand::LineTo:
                    current_x = p[0];
                    current_y = p[1];
                    bbox.expand(Point(current_x, current_y));
                    break;
                    
                case PathCommand::CurveTo:
                    // Include control points and end point
                    bbox.expand(Point(p[0], p[1]));
                    bbox.expand(Point(p[2], p[3]));
                    current_x = p[4];
                    current_y = p[5];
                    bbox.expand(Point(current_x, current_y));
                    break;
                    
                case PathCommand::QuadTo:
                    // Include control point and end point
                    bbox.expand(Point(p[0], p[1]));
                    current_x = p[2];
                    current_y = p[3];
                    bbox.expand(Point(current_x, current_y));
                    break;
                    
                case PathCommand::ArcTo:
                    // Simplified - just use the end point
                    current_x = p[5];
                    current_y = p[6];
                    bbox.expand(Point(current_x, current_y));
                    break;
                    
                case PathCommand::Close:
                    // No new points
                    break;
            }
        }
        
        return bbox;
    }
};

// Compact Group (40 bytes total) - container for nested objects
struct CompactGroup {
    CompactObject base;     // 28 bytes
    uint32_t child_offset;  // 4 bytes - offset into group children array
    uint16_t child_count;   // 2 bytes - number of children
    uint16_t parent_id;     // 2 bytes - parent group ID (0xFFFF for no parent)
    float pivot_x, pivot_y; // 8 bytes - transform pivot point
    
    CompactGroup() : base(ObjectType::Group), child_offset(0), child_count(0), 
                     parent_id(0xFFFF), pivot_x(0), pivot_y(0) {}
    
    CompactGroup(uint32_t offset, uint16_t count)
        : base(ObjectType::Group), child_offset(offset), child_count(count), 
          parent_id(0xFFFF), pivot_x(0), pivot_y(0) {}
    
    BoundingBox calculate_bbox(const std::vector<ObjectID>& children,
                              const class ObjectStorage& storage) const;
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
    std::vector<CompactText> texts;
    std::vector<CompactPath> paths;
    std::vector<CompactGroup> groups;
    
    // Variable data storage (public for serialization)
    std::vector<Point> polygon_points;
    std::vector<Point> polyline_points;
    std::vector<std::string> text_strings;
    std::vector<std::string> font_names;
    std::vector<PathSegment> path_segments;
    std::vector<float> path_parameters;
    std::vector<ObjectID> group_children;
    
    // Gradient and pattern storage
    std::vector<CompactGradient> gradients;
    std::vector<GradientStop> gradient_stops;
    std::vector<std::string> patterns;  // Pattern names/references
    std::vector<std::string> object_names;  // Object names
    
    // Metadata storage
    std::vector<MetadataEntry> metadata_entries;
    std::vector<std::string> metadata_keys;
    std::vector<std::string> metadata_values;
    
private:
    std::vector<Transform2D> transforms;
    
    // Spatial index (to be implemented)
    // std::unique_ptr<RTree> spatial_index;
    
public:
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
    
    ObjectID add_rectangle(float x, float y, float width, float height, float corner_radius = 0) {
        rectangles.emplace_back(x, y, width, height, corner_radius);
        return make_id(ObjectType::Rectangle, rectangles.size() - 1);
    }
    
    ObjectID add_line(float x1, float y1, float x2, float y2, LineStyle style = LineStyle::Solid) {
        lines.emplace_back(x1, y1, x2, y2, style);
        return make_id(ObjectType::Line, lines.size() - 1);
    }
    
    ObjectID add_ellipse(float x, float y, float rx, float ry, float rotation = 0) {
        ellipses.emplace_back(x, y, rx, ry, rotation);
        return make_id(ObjectType::Ellipse, ellipses.size() - 1);
    }
    
    ObjectID add_polygon(const std::vector<Point>& points, bool closed = true) {
        CompactPolygon poly(polygon_points.size(), points.size(), closed);
        polygon_points.insert(polygon_points.end(), points.begin(), points.end());
        polygons.push_back(poly);
        return make_id(ObjectType::Polygon, polygons.size() - 1);
    }
    
    ObjectID add_polyline(const std::vector<Point>& points, LineStyle style = LineStyle::Solid) {
        CompactPolyline polyline(polyline_points.size(), points.size(), style);
        polyline_points.insert(polyline_points.end(), points.begin(), points.end());
        polylines.push_back(polyline);
        return make_id(ObjectType::Polyline, polylines.size() - 1);
    }
    
    ObjectID add_arc(float x, float y, float radius, float start_angle, float end_angle) {
        arcs.emplace_back(x, y, radius, start_angle, end_angle);
        return make_id(ObjectType::Arc, arcs.size() - 1);
    }
    
    ObjectID add_text(float x, float y, const std::string& text, float font_size = 16.0f,
                      const std::string& font_name = "Arial", 
                      TextAlign align = TextAlign::Left,
                      TextBaseline baseline = TextBaseline::Alphabetic) {
        // Store text string
        uint32_t text_idx = text_strings.size();
        text_strings.push_back(text);
        
        // Store or find font name
        uint16_t font_idx = 0;
        auto it = std::find(font_names.begin(), font_names.end(), font_name);
        if (it != font_names.end()) {
            font_idx = std::distance(font_names.begin(), it);
        } else {
            font_idx = font_names.size();
            font_names.push_back(font_name);
        }
        
        texts.emplace_back(x, y, text_idx, font_size, font_idx, align, baseline);
        return make_id(ObjectType::Text, texts.size() - 1);
    }
    
    ObjectID add_path(const std::string& path_data) {
        // Parse SVG path string and create path
        uint32_t seg_offset = path_segments.size();
        uint16_t param_offset = path_parameters.size();
        
        // Simple SVG path parser
        size_t i = 0;
        PathCommand current_cmd = PathCommand::MoveTo;
        
        while (i < path_data.length()) {
            // Skip whitespace
            while (i < path_data.length() && std::isspace(path_data[i])) i++;
            if (i >= path_data.length()) break;
            
            // Check for command letter
            char c = path_data[i];
            if (std::isalpha(c)) {
                switch (std::toupper(c)) {
                    case 'M': current_cmd = PathCommand::MoveTo; break;
                    case 'L': current_cmd = PathCommand::LineTo; break;
                    case 'C': current_cmd = PathCommand::CurveTo; break;
                    case 'Q': current_cmd = PathCommand::QuadTo; break;
                    case 'A': current_cmd = PathCommand::ArcTo; break;
                    case 'Z': 
                        path_segments.emplace_back(PathCommand::Close, 0, path_parameters.size());
                        i++;
                        continue;
                    default:
                        i++; // Skip unknown commands
                        continue;
                }
                i++;
            }
            
            // Parse parameters based on command
            std::vector<float> params;
            int expected_params = 0;
            switch (current_cmd) {
                case PathCommand::MoveTo:
                case PathCommand::LineTo:
                    expected_params = 2; break;
                case PathCommand::QuadTo:
                    expected_params = 4; break;
                case PathCommand::CurveTo:
                    expected_params = 6; break;
                case PathCommand::ArcTo:
                    expected_params = 7; break;
                default:
                    break;
            }
            
            // Parse numbers
            for (int p = 0; p < expected_params; ++p) {
                // Skip whitespace and commas
                while (i < path_data.length() && 
                       (std::isspace(path_data[i]) || path_data[i] == ',')) i++;
                
                if (i >= path_data.length()) break;
                
                // Parse number
                size_t end;
                float value = std::stof(path_data.substr(i), &end);
                params.push_back(value);
                i += end;
            }
            
            if (params.size() == static_cast<size_t>(expected_params)) {
                uint16_t param_idx = path_parameters.size();
                path_segments.emplace_back(current_cmd, params.size(), param_idx);
                path_parameters.insert(path_parameters.end(), params.begin(), params.end());
            }
        }
        
        uint16_t seg_count = path_segments.size() - seg_offset;
        uint16_t param_count = path_parameters.size() - param_offset;
        
        paths.emplace_back(seg_offset, seg_count, param_offset, param_count);
        return make_id(ObjectType::Path, paths.size() - 1);
    }
    
    ObjectID add_group() {
        uint32_t child_offset = group_children.size();
        groups.emplace_back(child_offset, 0);
        return make_id(ObjectType::Group, groups.size() - 1);
    }
    
    ObjectID add_group(const std::vector<ObjectID>& children) {
        uint32_t child_offset = group_children.size();
        uint16_t child_count = children.size();
        
        // Add children to the children array
        group_children.insert(group_children.end(), children.begin(), children.end());
        
        groups.emplace_back(child_offset, child_count);
        return make_id(ObjectType::Group, groups.size() - 1);
    }
    
    void add_to_group(ObjectID group_id, ObjectID child_id) {
        if (get_type(group_id) != ObjectType::Group) return;
        uint32_t idx = get_index(group_id);
        if (idx >= groups.size()) return;
        
        auto& group = groups[idx];
        
        // If this is the first child or we need to expand
        if (group.child_count == 0) {
            group.child_offset = group_children.size();
            group_children.push_back(child_id);
            group.child_count = 1;
        } else {
            // Insert at the end of this group's children
            // This is simplified - in production we'd handle reallocation better
            uint32_t insert_pos = group.child_offset + group.child_count;
            if (insert_pos <= group_children.size()) {
                group_children.insert(group_children.begin() + insert_pos, child_id);
                group.child_count++;
                
                // Update offsets for subsequent groups
                for (size_t i = idx + 1; i < groups.size(); ++i) {
                    if (groups[i].child_offset >= insert_pos) {
                        groups[i].child_offset++;
                    }
                }
            }
        }
    }
    
    // Gradient management
    uint16_t add_linear_gradient(const std::vector<GradientStop>& stops, float angle = 0.0f) {
        uint16_t stop_offset = gradient_stops.size();
        uint8_t stop_count = std::min(stops.size(), static_cast<size_t>(255));
        
        gradient_stops.insert(gradient_stops.end(), stops.begin(), stops.begin() + stop_count);
        
        CompactGradient gradient(GradientType::Linear, stop_count, stop_offset, angle);
        gradients.push_back(gradient);
        
        return gradients.size() - 1;
    }
    
    uint16_t add_radial_gradient(const std::vector<GradientStop>& stops, 
                                float center_x, float center_y, float radius) {
        uint16_t stop_offset = gradient_stops.size();
        uint8_t stop_count = std::min(stops.size(), static_cast<size_t>(255));
        
        gradient_stops.insert(gradient_stops.end(), stops.begin(), stops.begin() + stop_count);
        
        CompactGradient gradient(GradientType::Radial, stop_count, stop_offset, 
                               0.0f, center_x, center_y, radius);
        gradients.push_back(gradient);
        
        return gradients.size() - 1;
    }
    
    uint16_t add_pattern(const std::string& pattern_name) {
        patterns.push_back(pattern_name);
        return patterns.size() - 1;
    }
    
    void set_object_gradient(ObjectID id, uint16_t gradient_id) {
        CompactObject* obj = get_object_base(id);
        if (obj && gradient_id < gradients.size()) {
            obj->gradient_id = gradient_id;
            obj->flags.set_gradient(true);
        }
    }
    
    void set_object_pattern(ObjectID id, uint16_t pattern_id) {
        CompactObject* obj = get_object_base(id);
        if (obj && pattern_id < patterns.size()) {
            obj->pattern_id = pattern_id;
            obj->flags.set_pattern(true);
        }
    }
    
    uint32_t add_object_name(const std::string& name) {
        // Check if name already exists to avoid duplicates
        for (size_t i = 0; i < object_names.size(); ++i) {
            if (object_names[i] == name) {
                return static_cast<uint32_t>(i);
            }
        }
        object_names.push_back(name);
        return object_names.size() - 1;
    }
    
    void set_object_name(ObjectID id, const std::string& name) {
        CompactObject* obj = get_object_base(id);
        if (obj) {
            obj->name_id = add_object_name(name);
        }
    }
    
    const std::string& get_object_name(ObjectID id) const {
        static const std::string empty_name;
        const CompactObject* obj = get_object_base_const(id);
        if (obj && obj->name_id != 0xFFFFFFFF && obj->name_id < object_names.size()) {
            return object_names[obj->name_id];
        }
        return empty_name;
    }
    
    // Metadata management
    uint32_t find_or_add_key(const std::string& key) {
        for (size_t i = 0; i < metadata_keys.size(); ++i) {
            if (metadata_keys[i] == key) {
                return static_cast<uint32_t>(i);
            }
        }
        metadata_keys.push_back(key);
        return metadata_keys.size() - 1;
    }
    
    uint32_t find_or_add_value(const std::string& value) {
        for (size_t i = 0; i < metadata_values.size(); ++i) {
            if (metadata_values[i] == value) {
                return static_cast<uint32_t>(i);
            }
        }
        metadata_values.push_back(value);
        return metadata_values.size() - 1;
    }
    
    void set_object_metadata(ObjectID id, const std::string& key, const std::string& value) {
        CompactObject* obj = get_object_base(id);
        if (!obj) return;
        
        uint32_t key_idx = find_or_add_key(key);
        uint32_t val_idx = find_or_add_value(value);
        
        // Check if metadata entry already exists for this object and key
        for (auto& entry : metadata_entries) {
            if (entry.object_id == id && entry.key_index == key_idx) {
                entry.value_index = val_idx;  // Update existing value
                return;
            }
        }
        
        // Add new metadata entry
        metadata_entries.emplace_back(key_idx, val_idx, id);
        obj->flags.set_metadata(true);
    }
    
    std::string get_object_metadata(ObjectID id, const std::string& key) const {
        uint32_t key_idx = 0xFFFFFFFF;
        for (size_t i = 0; i < metadata_keys.size(); ++i) {
            if (metadata_keys[i] == key) {
                key_idx = static_cast<uint32_t>(i);
                break;
            }
        }
        
        if (key_idx == 0xFFFFFFFF) return "";
        
        for (const auto& entry : metadata_entries) {
            if (entry.object_id == id && entry.key_index == key_idx) {
                if (entry.value_index < metadata_values.size()) {
                    return metadata_values[entry.value_index];
                }
            }
        }
        
        return "";
    }
    
    std::vector<std::pair<std::string, std::string>> get_all_object_metadata(ObjectID id) const {
        std::vector<std::pair<std::string, std::string>> result;
        
        for (const auto& entry : metadata_entries) {
            if (entry.object_id == id) {
                if (entry.key_index < metadata_keys.size() && 
                    entry.value_index < metadata_values.size()) {
                    result.emplace_back(metadata_keys[entry.key_index], 
                                       metadata_values[entry.value_index]);
                }
            }
        }
        
        return result;
    }
    
    CompactObject* get_object_base(ObjectID id) {
        ObjectType type = get_type(id);
        uint32_t idx = get_index(id);
        
        switch (type) {
            case ObjectType::Circle: 
                return idx < circles.size() ? &circles[idx].base : nullptr;
            case ObjectType::Rectangle: 
                return idx < rectangles.size() ? &rectangles[idx].base : nullptr;
            case ObjectType::Line: 
                return idx < lines.size() ? &lines[idx].base : nullptr;
            case ObjectType::Ellipse: 
                return idx < ellipses.size() ? &ellipses[idx].base : nullptr;
            case ObjectType::Polygon: 
                return idx < polygons.size() ? &polygons[idx].base : nullptr;
            case ObjectType::Polyline: 
                return idx < polylines.size() ? &polylines[idx].base : nullptr;
            case ObjectType::Arc: 
                return idx < arcs.size() ? &arcs[idx].base : nullptr;
            case ObjectType::Text: 
                return idx < texts.size() ? &texts[idx].base : nullptr;
            case ObjectType::Path: 
                return idx < paths.size() ? &paths[idx].base : nullptr;
            case ObjectType::Group: 
                return idx < groups.size() ? &groups[idx].base : nullptr;
            default: 
                return nullptr;
        }
    }
    
    const CompactObject* get_object_base_const(ObjectID id) const {
        ObjectType type = get_type(id);
        uint32_t idx = get_index(id);
        
        switch (type) {
            case ObjectType::Circle: 
                return idx < circles.size() ? &circles[idx].base : nullptr;
            case ObjectType::Rectangle: 
                return idx < rectangles.size() ? &rectangles[idx].base : nullptr;
            case ObjectType::Line: 
                return idx < lines.size() ? &lines[idx].base : nullptr;
            case ObjectType::Ellipse: 
                return idx < ellipses.size() ? &ellipses[idx].base : nullptr;
            case ObjectType::Polygon: 
                return idx < polygons.size() ? &polygons[idx].base : nullptr;
            case ObjectType::Polyline: 
                return idx < polylines.size() ? &polylines[idx].base : nullptr;
            case ObjectType::Arc: 
                return idx < arcs.size() ? &arcs[idx].base : nullptr;
            case ObjectType::Text: 
                return idx < texts.size() ? &texts[idx].base : nullptr;
            case ObjectType::Path: 
                return idx < paths.size() ? &paths[idx].base : nullptr;
            case ObjectType::Group: 
                return idx < groups.size() ? &groups[idx].base : nullptr;
            default: 
                return nullptr;
        }
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
    
    CompactText* get_text(ObjectID id) {
        if (get_type(id) != ObjectType::Text) return nullptr;
        uint32_t idx = get_index(id);
        return idx < texts.size() ? &texts[idx] : nullptr;
    }
    
    const CompactText* get_text(ObjectID id) const {
        if (get_type(id) != ObjectType::Text) return nullptr;
        uint32_t idx = get_index(id);
        return idx < texts.size() ? &texts[idx] : nullptr;
    }
    
    // Get text string
    const std::string& get_text_string(const CompactText& text) const {
        return text.text_index < text_strings.size() ? 
               text_strings[text.text_index] : text_strings[0];
    }
    
    // Get font name
    const std::string& get_font_name(const CompactText& text) const {
        return text.font_index < font_names.size() ? 
               font_names[text.font_index] : font_names[0];
    }
    
    CompactPath* get_path(ObjectID id) {
        if (get_type(id) != ObjectType::Path) return nullptr;
        uint32_t idx = get_index(id);
        return idx < paths.size() ? &paths[idx] : nullptr;
    }
    
    const CompactPath* get_path(ObjectID id) const {
        if (get_type(id) != ObjectType::Path) return nullptr;
        uint32_t idx = get_index(id);
        return idx < paths.size() ? &paths[idx] : nullptr;
    }
    
    CompactGroup* get_group(ObjectID id) {
        if (get_type(id) != ObjectType::Group) return nullptr;
        uint32_t idx = get_index(id);
        return idx < groups.size() ? &groups[idx] : nullptr;
    }
    
    const CompactGroup* get_group(ObjectID id) const {
        if (get_type(id) != ObjectType::Group) return nullptr;
        uint32_t idx = get_index(id);
        return idx < groups.size() ? &groups[idx] : nullptr;
    }
    
    // Get group children
    std::pair<const ObjectID*, size_t> get_group_children(const CompactGroup& group) const {
        if (group.child_offset + group.child_count > group_children.size()) {
            return {nullptr, 0};
        }
        return {&group_children[group.child_offset], group.child_count};
    }
    
    // Get path segments
    std::pair<const PathSegment*, size_t> get_path_segments(const CompactPath& path) const {
        if (path.segment_offset + path.segment_count > path_segments.size()) {
            return {nullptr, 0};
        }
        return {&path_segments[path.segment_offset], path.segment_count};
    }
    
    // Get path parameters for a segment
    const float* get_segment_params(const PathSegment& segment) const {
        if (segment.param_offset + segment.param_count > path_parameters.size()) {
            return nullptr;
        }
        return &path_parameters[segment.param_offset];
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
               polygons.size() + polylines.size() + arcs.size() + texts.size() + 
               paths.size() + groups.size();
    }
    
    size_t memory_usage() const {
        size_t base_size = sizeof(CompactCircle) * circles.size() +
               sizeof(CompactRectangle) * rectangles.size() +
               sizeof(CompactLine) * lines.size() +
               sizeof(CompactEllipse) * ellipses.size() +
               sizeof(CompactPolygon) * polygons.size() +
               sizeof(CompactPolyline) * polylines.size() +
               sizeof(CompactArc) * arcs.size() +
               sizeof(CompactText) * texts.size() +
               sizeof(CompactPath) * paths.size() +
               sizeof(CompactGroup) * groups.size() +
               sizeof(Point) * polygon_points.size() +
               sizeof(Point) * polyline_points.size() +
               sizeof(PathSegment) * path_segments.size() +
               sizeof(float) * path_parameters.size() +
               sizeof(ObjectID) * group_children.size() +
               sizeof(Transform2D) * transforms.size() +
               sizeof(CompactGradient) * gradients.size() +
               sizeof(GradientStop) * gradient_stops.size() +
               sizeof(MetadataEntry) * metadata_entries.size();
        
        // Add string storage
        size_t string_size = 0;
        for (const auto& s : text_strings) string_size += s.size();
        for (const auto& s : font_names) string_size += s.size();
        for (const auto& s : patterns) string_size += s.size();
        for (const auto& s : object_names) string_size += s.size();
        for (const auto& s : metadata_keys) string_size += s.size();
        for (const auto& s : metadata_values) string_size += s.size();
        
        return base_size + string_size;
    }
};

} // namespace drawing