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
                    
                case ObjectType::Polygon:
                    // Polygon handling would go here
                    // For now, skip as it requires friend access or public polygon array
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