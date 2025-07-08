#include "drawing/drawing.hpp"

namespace drawing {

BoundingBox Drawing::get_bounding_box() const {
    BoundingBox bbox;
    bool first = true;
    
    for (const auto& layer : layers) {
        if (!layer->is_visible()) continue;
        
        for (auto obj_id : layer->get_objects()) {
            BoundingBox obj_bbox;
            bool has_bbox = false;
            
            switch (ObjectStorage::get_type(obj_id)) {
                case ObjectType::Circle:
                    if (auto* circle = storage.get_circle(obj_id)) {
                        obj_bbox = circle->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Rectangle:
                    if (auto* rect = storage.get_rectangle(obj_id)) {
                        obj_bbox = rect->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Line:
                    if (auto* line = storage.get_line(obj_id)) {
                        obj_bbox = line->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Ellipse:
                    if (auto* ellipse = storage.get_ellipse(obj_id)) {
                        obj_bbox = ellipse->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                case ObjectType::Polygon: {
                    if (auto* poly = storage.get_polygon(obj_id)) {
                        auto [points, count] = storage.get_polygon_points(*poly);
                        if (points && count > 0) {
                            BoundingBox poly_bbox(points[0].x, points[0].y, points[0].x, points[0].y);
                            for (size_t i = 1; i < count; ++i) {
                                poly_bbox.expand(points[i]);
                            }
                            obj_bbox = poly_bbox;
                            has_bbox = true;
                        }
                    }
                    break;
                }
                    
                case ObjectType::Polyline: {
                    if (auto* polyline = storage.get_polyline(obj_id)) {
                        auto [points, count] = storage.get_polyline_points(*polyline);
                        if (points && count > 0) {
                            BoundingBox polyline_bbox(points[0].x, points[0].y, points[0].x, points[0].y);
                            for (size_t i = 1; i < count; ++i) {
                                polyline_bbox.expand(points[i]);
                            }
                            obj_bbox = polyline_bbox;
                            has_bbox = true;
                        }
                    }
                    break;
                }
                
                case ObjectType::Arc:
                    if (auto* arc = storage.get_arc(obj_id)) {
                        obj_bbox = arc->get_bounding_box();
                        has_bbox = true;
                    }
                    break;
                    
                default:
                    break;
            }
            
            if (has_bbox) {
                if (first) {
                    bbox = obj_bbox;
                    first = false;
                } else {
                    bbox.expand(obj_bbox);
                }
            }
        }
    }
    
    return bbox;
}

} // namespace drawing