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
            case ObjectType::Ellipse:
                if (auto* obj = get_ellipse(id)) obj->base.fill_color = color;
                break;
            case ObjectType::Polygon:
                if (uint32_t idx = get_index(id); idx < polygons.size()) {
                    polygons[idx].base.fill_color = color;
                }
                break;
            case ObjectType::Polyline:
                if (uint32_t idx = get_index(id); idx < polylines.size()) {
                    polylines[idx].base.fill_color = color;
                }
                break;
            case ObjectType::Arc:
                if (auto* obj = get_arc(id)) obj->base.fill_color = color;
                break;
            case ObjectType::Text:
                if (auto* obj = get_text(id)) obj->base.fill_color = color;
                break;
            case ObjectType::Path:
                if (auto* obj = get_path(id)) obj->base.fill_color = color;
                break;
            case ObjectType::Group:
                if (auto* obj = get_group(id)) obj->base.fill_color = color;
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
            case ObjectType::Ellipse:
                if (auto* obj = get_ellipse(id)) obj->base.stroke_color = color;
                break;
            case ObjectType::Polygon:
                if (uint32_t idx = get_index(id); idx < polygons.size()) {
                    polygons[idx].base.stroke_color = color;
                }
                break;
            case ObjectType::Polyline:
                if (uint32_t idx = get_index(id); idx < polylines.size()) {
                    polylines[idx].base.stroke_color = color;
                }
                break;
            case ObjectType::Arc:
                if (auto* obj = get_arc(id)) obj->base.stroke_color = color;
                break;
            case ObjectType::Text:
                if (auto* obj = get_text(id)) obj->base.stroke_color = color;
                break;
            case ObjectType::Path:
                if (auto* obj = get_path(id)) obj->base.stroke_color = color;
                break;
            case ObjectType::Group:
                if (auto* obj = get_group(id)) obj->base.stroke_color = color;
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
            case ObjectType::Ellipse:
                if (auto* obj = get_ellipse(id)) obj->base.opacity = opacity;
                break;
            case ObjectType::Polygon:
                if (uint32_t idx = get_index(id); idx < polygons.size()) {
                    polygons[idx].base.opacity = opacity;
                }
                break;
            case ObjectType::Polyline:
                if (uint32_t idx = get_index(id); idx < polylines.size()) {
                    polylines[idx].base.opacity = opacity;
                }
                break;
            case ObjectType::Arc:
                if (auto* obj = get_arc(id)) obj->base.opacity = opacity;
                break;
            case ObjectType::Text:
                if (auto* obj = get_text(id)) obj->base.opacity = opacity;
                break;
            case ObjectType::Path:
                if (auto* obj = get_path(id)) obj->base.opacity = opacity;
                break;
            case ObjectType::Group:
                if (auto* obj = get_group(id)) obj->base.opacity = opacity;
                break;
            default:
                break;
        }
    }
}

std::vector<ObjectID> ObjectStorage::find_in_rect(const BoundingBox& rect) const {
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
    
    // Check ellipses
    for (size_t i = 0; i < ellipses.size(); ++i) {
        if (rect.intersects(ellipses[i].get_bounding_box())) {
            result.push_back(make_id(ObjectType::Ellipse, i));
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
    
    // Check polylines
    for (size_t i = 0; i < polylines.size(); ++i) {
        auto [points, count] = get_polyline_points(polylines[i]);
        if (points && count > 0) {
            BoundingBox polyline_bbox(points[0].x, points[0].y, points[0].x, points[0].y);
            for (size_t j = 1; j < count; ++j) {
                polyline_bbox.expand(points[j]);
            }
            if (rect.intersects(polyline_bbox)) {
                result.push_back(make_id(ObjectType::Polyline, i));
            }
        }
    }
    
    // Check arcs
    for (size_t i = 0; i < arcs.size(); ++i) {
        if (rect.intersects(arcs[i].get_bounding_box())) {
            result.push_back(make_id(ObjectType::Arc, i));
        }
    }
    
    // Check texts
    for (size_t i = 0; i < texts.size(); ++i) {
        if (rect.intersects(texts[i].get_bounding_box())) {
            result.push_back(make_id(ObjectType::Text, i));
        }
    }
    
    // Check paths
    for (size_t i = 0; i < paths.size(); ++i) {
        BoundingBox path_bbox = paths[i].calculate_bbox(path_segments, path_parameters);
        if (rect.intersects(path_bbox)) {
            result.push_back(make_id(ObjectType::Path, i));
        }
    }
    
    // Check groups
    for (size_t i = 0; i < groups.size(); ++i) {
        BoundingBox group_bbox = groups[i].calculate_bbox(group_children, *this);
        if (rect.intersects(group_bbox)) {
            result.push_back(make_id(ObjectType::Group, i));
        }
    }
    
    return result;
}

std::vector<ObjectID> ObjectStorage::find_at_point(const Point& point, float tolerance) const {
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
    
    // Check ellipses
    for (size_t i = 0; i < ellipses.size(); ++i) {
        const auto& ellipse = ellipses[i];
        
        // Transform point to ellipse coordinate system (account for rotation)
        float cos_rot = std::cos(-ellipse.rotation);
        float sin_rot = std::sin(-ellipse.rotation);
        float dx = point.x - ellipse.x;
        float dy = point.y - ellipse.y;
        float local_x = dx * cos_rot - dy * sin_rot;
        float local_y = dx * sin_rot + dy * cos_rot;
        
        // Check if point is on ellipse edge (within tolerance)
        float rx_outer = ellipse.rx + tolerance;
        float ry_outer = ellipse.ry + tolerance;
        float rx_inner = std::max(0.0f, ellipse.rx - tolerance);
        float ry_inner = std::max(0.0f, ellipse.ry - tolerance);
        
        float outer_check = (local_x * local_x) / (rx_outer * rx_outer) + 
                           (local_y * local_y) / (ry_outer * ry_outer);
        float inner_check = (local_x * local_x) / (rx_inner * rx_inner) + 
                           (local_y * local_y) / (ry_inner * ry_inner);
        
        if (outer_check <= 1.0f && (rx_inner == 0 || ry_inner == 0 || inner_check >= 1.0f)) {
            result.push_back(make_id(ObjectType::Ellipse, i));
        }
    }
    
    // Check polylines (point near any segment)
    for (size_t i = 0; i < polylines.size(); ++i) {
        auto [points, count] = get_polyline_points(polylines[i]);
        if (points && count >= 2) {
            // Check each segment
            for (size_t j = 0; j < count - 1; ++j) {
                float dx = points[j + 1].x - points[j].x;
                float dy = points[j + 1].y - points[j].y;
                float len_sq = dx * dx + dy * dy;
                
                if (len_sq > 0) {
                    float t = std::clamp(
                        ((point.x - points[j].x) * dx + (point.y - points[j].y) * dy) / len_sq,
                        0.0f, 1.0f
                    );
                    float px = points[j].x + t * dx;
                    float py = points[j].y + t * dy;
                    float dist_sq = (point.x - px) * (point.x - px) + (point.y - py) * (point.y - py);
                    
                    if (dist_sq <= tol_sq) {
                        result.push_back(make_id(ObjectType::Polyline, i));
                        break; // Found a hit, no need to check other segments
                    }
                }
            }
        }
    }
    
    // Check arcs
    for (size_t i = 0; i < arcs.size(); ++i) {
        const auto& arc = arcs[i];
        
        // Calculate distance from center
        float dx = point.x - arc.x;
        float dy = point.y - arc.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        // Check if point is near the arc radius
        if (std::abs(dist - arc.radius) <= tolerance) {
            // Check if point is within the arc's angle range
            float angle = std::atan2(dy, dx);
            
            // Normalize angles to [0, 2π]
            auto normalize_angle = [](float a) {
                while (a < 0) a += 2 * M_PI;
                while (a >= 2 * M_PI) a -= 2 * M_PI;
                return a;
            };
            
            angle = normalize_angle(angle);
            float start = normalize_angle(arc.start_angle);
            float end = normalize_angle(arc.end_angle);
            
            bool in_range = false;
            if (start <= end) {
                in_range = (angle >= start && angle <= end);
            } else {
                // Arc crosses 0 degrees
                in_range = (angle >= start || angle <= end);
            }
            
            if (in_range) {
                result.push_back(make_id(ObjectType::Arc, i));
            }
        }
    }
    
    // Check texts
    for (size_t i = 0; i < texts.size(); ++i) {
        const auto& text = texts[i];
        BoundingBox bbox = text.get_bounding_box();
        BoundingBox expanded(
            bbox.min_x - tolerance, bbox.min_y - tolerance,
            bbox.max_x + tolerance, bbox.max_y + tolerance
        );
        
        if (expanded.contains(point)) {
            result.push_back(make_id(ObjectType::Text, i));
        }
    }
    
    // Check paths (simplified - just check bounding box)
    for (size_t i = 0; i < paths.size(); ++i) {
        BoundingBox path_bbox = paths[i].calculate_bbox(path_segments, path_parameters);
        BoundingBox expanded(
            path_bbox.min_x - tolerance, path_bbox.min_y - tolerance,
            path_bbox.max_x + tolerance, path_bbox.max_y + tolerance
        );
        
        if (expanded.contains(point)) {
            result.push_back(make_id(ObjectType::Path, i));
        }
    }
    
    // Check groups (simplified - just check bounding box)
    for (size_t i = 0; i < groups.size(); ++i) {
        BoundingBox group_bbox = groups[i].calculate_bbox(group_children, *this);
        BoundingBox expanded(
            group_bbox.min_x - tolerance, group_bbox.min_y - tolerance,
            group_bbox.max_x + tolerance, group_bbox.max_y + tolerance
        );
        
        if (expanded.contains(point)) {
            result.push_back(make_id(ObjectType::Group, i));
        }
    }
    
    return result;
}

BoundingBox CompactGroup::calculate_bbox(const std::vector<ObjectID>& children,
                                        const ObjectStorage& storage) const {
    BoundingBox bbox;
    bool first = true;
    
    // Get children for this group
    if (child_count > 0 && child_offset + child_count <= children.size()) {
        for (size_t i = 0; i < child_count; ++i) {
            ObjectID child_id = children[child_offset + i];
            BoundingBox child_bbox;
            bool has_bbox = false;
            
            switch (ObjectStorage::get_type(child_id)) {
                case ObjectType::Circle:
                    if (auto* circle = storage.get_circle(child_id)) {
                        child_bbox = circle->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Rectangle:
                    if (auto* rect = storage.get_rectangle(child_id)) {
                        child_bbox = rect->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Line:
                    if (auto* line = storage.get_line(child_id)) {
                        child_bbox = line->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Ellipse:
                    if (auto* ellipse = storage.get_ellipse(child_id)) {
                        child_bbox = ellipse->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Polygon:
                    if (auto* poly = storage.get_polygon(child_id)) {
                        auto [points, count] = storage.get_polygon_points(*poly);
                        if (points && count > 0) {
                            BoundingBox poly_bbox(points[0].x, points[0].y, points[0].x, points[0].y);
                            for (size_t j = 1; j < count; ++j) {
                                poly_bbox.expand(points[j]);
                            }
                            child_bbox = poly_bbox;
                            has_bbox = true;
                        }
                    }
                    break;
                    
                case ObjectType::Polyline:
                    if (auto* polyline = storage.get_polyline(child_id)) {
                        auto [points, count] = storage.get_polyline_points(*polyline);
                        if (points && count > 0) {
                            BoundingBox polyline_bbox(points[0].x, points[0].y, points[0].x, points[0].y);
                            for (size_t j = 1; j < count; ++j) {
                                polyline_bbox.expand(points[j]);
                            }
                            child_bbox = polyline_bbox;
                            has_bbox = true;
                        }
                    }
                    break;
                    
                case ObjectType::Arc:
                    if (auto* arc = storage.get_arc(child_id)) {
                        child_bbox = arc->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Text:
                    if (auto* text = storage.get_text(child_id)) {
                        child_bbox = text->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Path:
                    if (auto* path = storage.get_path(child_id)) {
                        child_bbox = path->calculate_bbox(storage.path_segments, 
                                                         storage.path_parameters);
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Group:
                    if (auto* group = storage.get_group(child_id)) {
                        child_bbox = group->calculate_bbox(children, storage);
                        has_bbox = true;
                    }
                    break;
                    
                default:
                    break;
            }
            
            if (has_bbox) {
                if (first) {
                    bbox = child_bbox;
                    first = false;
                } else {
                    bbox.expand(child_bbox);
                }
            }
        }
    }
    
    return bbox;
}

} // namespace drawing