#include "drawing/objects.hpp"
#include <algorithm>
#include <cmath>

namespace drawing {

BoundingBox CompactObject::get_bounding_box() const {
    // This is a dispatch function - actual implementation depends on type
    // In a real implementation, this would be handled by the ObjectStorage
    return BoundingBox();
}

void ObjectStorage::set_fill_color(const std::vector<ObjectID>& ids, Color color) {
    for (ObjectID id : ids) {
        switch (get_type(id)) {
            case ObjectType::Circle:
                if (auto* obj = get_circle(id)) obj->base.fill_color = color;
                break;
            case ObjectType::Rectangle:
                if (auto* obj = get_rectangle(id)) obj->base.fill_color = color;
                break;
            case ObjectType::Line:
                if (auto* obj = get_line(id)) obj->base.fill_color = color;
                break;
            case ObjectType::Polygon:
                if (uint32_t idx = get_index(id); idx < polygons.size()) {
                    polygons[idx].base.fill_color = color;
                }
                break;
            default:
                break;
        }
    }
}

void ObjectStorage::set_stroke_color(const std::vector<ObjectID>& ids, Color color) {
    for (ObjectID id : ids) {
        switch (get_type(id)) {
            case ObjectType::Circle:
                if (auto* obj = get_circle(id)) obj->base.stroke_color = color;
                break;
            case ObjectType::Rectangle:
                if (auto* obj = get_rectangle(id)) obj->base.stroke_color = color;
                break;
            case ObjectType::Line:
                if (auto* obj = get_line(id)) obj->base.stroke_color = color;
                break;
            case ObjectType::Polygon:
                if (uint32_t idx = get_index(id); idx < polygons.size()) {
                    polygons[idx].base.stroke_color = color;
                }
                break;
            default:
                break;
        }
    }
}

void ObjectStorage::set_opacity(const std::vector<ObjectID>& ids, float opacity) {
    opacity = std::clamp(opacity, 0.0f, 1.0f);
    
    for (ObjectID id : ids) {
        switch (get_type(id)) {
            case ObjectType::Circle:
                if (auto* obj = get_circle(id)) obj->base.opacity = opacity;
                break;
            case ObjectType::Rectangle:
                if (auto* obj = get_rectangle(id)) obj->base.opacity = opacity;
                break;
            case ObjectType::Line:
                if (auto* obj = get_line(id)) obj->base.opacity = opacity;
                break;
            case ObjectType::Polygon:
                if (uint32_t idx = get_index(id); idx < polygons.size()) {
                    polygons[idx].base.opacity = opacity;
                }
                break;
            default:
                break;
        }
    }
}

std::vector<ObjectStorage::ObjectID> ObjectStorage::find_in_rect(const BoundingBox& rect) const {
    std::vector<ObjectID> result;
    
    // Check circles
    for (size_t i = 0; i < circles.size(); ++i) {
        if (rect.intersects(circles[i].get_bounding_box())) {
            result.push_back(make_id(ObjectType::Circle, i));
        }
    }
    
    // Check rectangles
    for (size_t i = 0; i < rectangles.size(); ++i) {
        if (rect.intersects(rectangles[i].get_bounding_box())) {
            result.push_back(make_id(ObjectType::Rectangle, i));
        }
    }
    
    // Check lines
    for (size_t i = 0; i < lines.size(); ++i) {
        if (rect.intersects(lines[i].get_bounding_box())) {
            result.push_back(make_id(ObjectType::Line, i));
        }
    }
    
    // Check polygons
    for (size_t i = 0; i < polygons.size(); ++i) {
        auto [points, count] = get_polygon_points(polygons[i]);
        if (points && count > 0) {
            BoundingBox poly_bbox(points[0].x, points[0].y, points[0].x, points[0].y);
            for (size_t j = 1; j < count; ++j) {
                poly_bbox.expand(points[j]);
            }
            if (rect.intersects(poly_bbox)) {
                result.push_back(make_id(ObjectType::Polygon, i));
            }
        }
    }
    
    return result;
}

std::vector<ObjectStorage::ObjectID> ObjectStorage::find_at_point(const Point& point, float tolerance) const {
    std::vector<ObjectID> result;
    float tol_sq = tolerance * tolerance;
    
    // Check circles
    for (size_t i = 0; i < circles.size(); ++i) {
        const auto& circle = circles[i];
        float dx = point.x - circle.x;
        float dy = point.y - circle.y;
        float dist_sq = dx * dx + dy * dy;
        float radius_outer = circle.radius + tolerance;
        float radius_inner = std::max(0.0f, circle.radius - tolerance);
        
        if (dist_sq <= radius_outer * radius_outer && 
            dist_sq >= radius_inner * radius_inner) {
            result.push_back(make_id(ObjectType::Circle, i));
        }
    }
    
    // Check rectangles
    for (size_t i = 0; i < rectangles.size(); ++i) {
        const auto& rect = rectangles[i];
        BoundingBox expanded(
            rect.x - tolerance, rect.y - tolerance,
            rect.x + rect.width + tolerance, rect.y + rect.height + tolerance
        );
        
        if (expanded.contains(point)) {
            // Check if point is near edge
            bool near_edge = 
                point.x <= rect.x + tolerance || 
                point.x >= rect.x + rect.width - tolerance ||
                point.y <= rect.y + tolerance || 
                point.y >= rect.y + rect.height - tolerance;
                
            if (near_edge || (point.x >= rect.x && point.x <= rect.x + rect.width &&
                              point.y >= rect.y && point.y <= rect.y + rect.height)) {
                result.push_back(make_id(ObjectType::Rectangle, i));
            }
        }
    }
    
    // Check lines
    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        
        // Point-to-line distance calculation
        float dx = line.x2 - line.x1;
        float dy = line.y2 - line.y1;
        float len_sq = dx * dx + dy * dy;
        
        if (len_sq > 0) {
            float t = std::clamp(
                ((point.x - line.x1) * dx + (point.y - line.y1) * dy) / len_sq,
                0.0f, 1.0f
            );
            float px = line.x1 + t * dx;
            float py = line.y1 + t * dy;
            float dist_sq = (point.x - px) * (point.x - px) + (point.y - py) * (point.y - py);
            
            if (dist_sq <= tol_sq) {
                result.push_back(make_id(ObjectType::Line, i));
            }
        }
    }
    
    return result;
}

} // namespace drawing