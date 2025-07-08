#include "drawing/batch_operations.hpp"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <immintrin.h>  // For SIMD intrinsics

namespace drawing {

BatchOperations::PerformanceStats BatchOperations::last_operation_stats = {0, 0.0, 0.0};

// Helper implementations
Point BatchOperations::get_object_center(const ObjectStorage& storage, ObjectStorage::ObjectID id) {
    switch (ObjectStorage::get_type(id)) {
        case ObjectType::Circle: {
            auto* circle = storage.get_circle(id);
            return circle ? Point(circle->x, circle->y) : Point();
        }
        case ObjectType::Rectangle: {
            auto* rect = storage.get_rectangle(id);
            return rect ? Point(rect->x + rect->width/2, rect->y + rect->height/2) : Point();
        }
        case ObjectType::Line: {
            auto* line = storage.get_line(id);
            return line ? Point((line->x1 + line->x2)/2, (line->y1 + line->y2)/2) : Point();
        }
        default:
            return Point();
    }
}

void BatchOperations::set_object_position(ObjectStorage& storage, ObjectStorage::ObjectID id, 
                                         float x, float y) {
    switch (ObjectStorage::get_type(id)) {
        case ObjectType::Circle: {
            auto* circle = storage.get_circle(id);
            if (circle) {
                circle->x = x;
                circle->y = y;
            }
            break;
        }
        case ObjectType::Rectangle: {
            auto* rect = storage.get_rectangle(id);
            if (rect) {
                rect->x = x - rect->width/2;
                rect->y = y - rect->height/2;
            }
            break;
        }
        case ObjectType::Line: {
            auto* line = storage.get_line(id);
            if (line) {
                float cx = (line->x1 + line->x2) / 2;
                float cy = (line->y1 + line->y2) / 2;
                float dx = x - cx;
                float dy = y - cy;
                line->x1 += dx;
                line->y1 += dy;
                line->x2 += dx;
                line->y2 += dy;
            }
            break;
        }
    }
}

BoundingBox BatchOperations::get_object_bbox(const ObjectStorage& storage, ObjectStorage::ObjectID id) {
    switch (ObjectStorage::get_type(id)) {
        case ObjectType::Circle: {
            auto* circle = storage.get_circle(id);
            return circle ? circle->get_bounding_box() : BoundingBox();
        }
        case ObjectType::Rectangle: {
            auto* rect = storage.get_rectangle(id);
            return rect ? rect->get_bounding_box() : BoundingBox();
        }
        case ObjectType::Line: {
            auto* line = storage.get_line(id);
            return line ? line->get_bounding_box() : BoundingBox();
        }
        default:
            return BoundingBox();
    }
}

// Batch transform operations
void BatchOperations::translate_objects(ObjectStorage& storage, 
                                       const std::vector<ObjectStorage::ObjectID>& ids,
                                       float dx, float dy) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Group objects by type for better cache performance
    std::vector<size_t> circle_indices, rect_indices, line_indices;
    
    for (auto id : ids) {
        switch (ObjectStorage::get_type(id)) {
            case ObjectType::Circle:
                circle_indices.push_back(ObjectStorage::get_index(id));
                break;
            case ObjectType::Rectangle:
                rect_indices.push_back(ObjectStorage::get_index(id));
                break;
            case ObjectType::Line:
                line_indices.push_back(ObjectStorage::get_index(id));
                break;
            default:
                break;
        }
    }
    
    // Process circles with SIMD if available
    #ifdef __SSE2__
    if (!circle_indices.empty()) {
        simd::translate_circles_simd(storage.circles.data(), circle_indices.size(), dx, dy);
    }
    #else
    for (size_t idx : circle_indices) {
        if (idx < storage.circles.size()) {
            storage.circles[idx].x += dx;
            storage.circles[idx].y += dy;
        }
    }
    #endif
    
    // Process rectangles
    #ifdef __SSE2__
    if (!rect_indices.empty()) {
        simd::translate_rectangles_simd(storage.rectangles.data(), rect_indices.size(), dx, dy);
    }
    #else
    for (size_t idx : rect_indices) {
        if (idx < storage.rectangles.size()) {
            storage.rectangles[idx].x += dx;
            storage.rectangles[idx].y += dy;
        }
    }
    #endif
    
    // Process lines
    for (size_t idx : line_indices) {
        if (idx < storage.lines.size()) {
            storage.lines[idx].x1 += dx;
            storage.lines[idx].y1 += dy;
            storage.lines[idx].x2 += dx;
            storage.lines[idx].y2 += dy;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    last_operation_stats.objects_processed = ids.size();
    last_operation_stats.time_ms = duration.count() / 1000.0;
    last_operation_stats.objects_per_second = ids.size() / (duration.count() / 1000000.0);
}

void BatchOperations::scale_objects(ObjectStorage& storage,
                                   const std::vector<ObjectStorage::ObjectID>& ids,
                                   float sx, float sy,
                                   const Point& center) {
    for (auto id : ids) {
        Point obj_center = get_object_center(storage, id);
        
        // Translate to origin, scale, translate back
        float new_x = center.x + (obj_center.x - center.x) * sx;
        float new_y = center.y + (obj_center.y - center.y) * sy;
        
        switch (ObjectStorage::get_type(id)) {
            case ObjectType::Circle: {
                auto* circle = storage.get_circle(id);
                if (circle) {
                    circle->x = new_x;
                    circle->y = new_y;
                    circle->radius *= sx; // Assuming uniform scale for circles
                }
                break;
            }
            case ObjectType::Rectangle: {
                auto* rect = storage.get_rectangle(id);
                if (rect) {
                    rect->x = center.x + (rect->x - center.x) * sx;
                    rect->y = center.y + (rect->y - center.y) * sy;
                    rect->width *= sx;
                    rect->height *= sy;
                }
                break;
            }
            case ObjectType::Line: {
                auto* line = storage.get_line(id);
                if (line) {
                    line->x1 = center.x + (line->x1 - center.x) * sx;
                    line->y1 = center.y + (line->y1 - center.y) * sy;
                    line->x2 = center.x + (line->x2 - center.x) * sx;
                    line->y2 = center.y + (line->y2 - center.y) * sy;
                }
                break;
            }
        }
    }
}

void BatchOperations::rotate_objects(ObjectStorage& storage,
                                    const std::vector<ObjectStorage::ObjectID>& ids,
                                    float angle_radians,
                                    const Point& center) {
    float cos_a = std::cos(angle_radians);
    float sin_a = std::sin(angle_radians);
    
    for (auto id : ids) {
        switch (ObjectStorage::get_type(id)) {
            case ObjectType::Circle: {
                auto* circle = storage.get_circle(id);
                if (circle) {
                    float dx = circle->x - center.x;
                    float dy = circle->y - center.y;
                    circle->x = center.x + dx * cos_a - dy * sin_a;
                    circle->y = center.y + dx * sin_a + dy * cos_a;
                }
                break;
            }
            case ObjectType::Line: {
                auto* line = storage.get_line(id);
                if (line) {
                    // Rotate both endpoints
                    float dx1 = line->x1 - center.x;
                    float dy1 = line->y1 - center.y;
                    line->x1 = center.x + dx1 * cos_a - dy1 * sin_a;
                    line->y1 = center.y + dx1 * sin_a + dy1 * cos_a;
                    
                    float dx2 = line->x2 - center.x;
                    float dy2 = line->y2 - center.y;
                    line->x2 = center.x + dx2 * cos_a - dy2 * sin_a;
                    line->y2 = center.y + dx2 * sin_a + dy2 * cos_a;
                }
                break;
            }
            // Rectangle rotation would require transform matrix
        }
    }
}

// Batch bounding box calculation
BoundingBox BatchOperations::calculate_bounding_box(const ObjectStorage& storage,
                                                   const std::vector<ObjectStorage::ObjectID>& ids) {
    if (ids.empty()) return BoundingBox();
    
    BoundingBox result = get_object_bbox(storage, ids[0]);
    
    for (size_t i = 1; i < ids.size(); ++i) {
        result.expand(get_object_bbox(storage, ids[i]));
    }
    
    return result;
}

// Batch alignment operations
void BatchOperations::align_objects_left(ObjectStorage& storage,
                                        const std::vector<ObjectStorage::ObjectID>& ids) {
    if (ids.empty()) return;
    
    // Find leftmost edge
    float min_x = std::numeric_limits<float>::max();
    for (auto id : ids) {
        BoundingBox bbox = get_object_bbox(storage, id);
        min_x = std::min(min_x, bbox.min_x);
    }
    
    // Align all objects to this edge
    for (auto id : ids) {
        BoundingBox bbox = get_object_bbox(storage, id);
        float dx = min_x - bbox.min_x;
        
        switch (ObjectStorage::get_type(id)) {
            case ObjectType::Circle: {
                auto* circle = storage.get_circle(id);
                if (circle) circle->x += dx;
                break;
            }
            case ObjectType::Rectangle: {
                auto* rect = storage.get_rectangle(id);
                if (rect) rect->x += dx;
                break;
            }
            case ObjectType::Line: {
                auto* line = storage.get_line(id);
                if (line) {
                    line->x1 += dx;
                    line->x2 += dx;
                }
                break;
            }
        }
    }
}

// Pattern generation
std::vector<ObjectStorage::ObjectID> BatchOperations::create_grid(
    ObjectStorage& storage,
    ObjectType type,
    int rows, int cols,
    float cell_width, float cell_height,
    float x_offset, float y_offset) {
    
    std::vector<ObjectStorage::ObjectID> result;
    result.reserve(rows * cols);
    
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float x = x_offset + col * cell_width + cell_width / 2;
            float y = y_offset + row * cell_height + cell_height / 2;
            
            ObjectStorage::ObjectID id = 0;
            
            switch (type) {
                case ObjectType::Circle:
                    id = storage.add_circle(x, y, std::min(cell_width, cell_height) * 0.4f);
                    break;
                case ObjectType::Rectangle:
                    id = storage.add_rectangle(
                        x - cell_width * 0.4f, 
                        y - cell_height * 0.4f,
                        cell_width * 0.8f,
                        cell_height * 0.8f
                    );
                    break;
                default:
                    break;
            }
            
            if (id != 0) {
                result.push_back(id);
            }
        }
    }
    
    return result;
}

// SIMD implementations
namespace simd {

#ifdef __SSE2__
void translate_circles_simd(CompactCircle* circles, size_t count, float dx, float dy) {
    __m128 delta_x = _mm_set1_ps(dx);
    __m128 delta_y = _mm_set1_ps(dy);
    
    // Process 4 circles at a time
    size_t simd_count = count & ~3;
    for (size_t i = 0; i < simd_count; i += 4) {
        // Load x coordinates
        __m128 x = _mm_set_ps(circles[i+3].x, circles[i+2].x, circles[i+1].x, circles[i].x);
        __m128 y = _mm_set_ps(circles[i+3].y, circles[i+2].y, circles[i+1].y, circles[i].y);
        
        // Add deltas
        x = _mm_add_ps(x, delta_x);
        y = _mm_add_ps(y, delta_y);
        
        // Store back
        float x_arr[4], y_arr[4];
        _mm_store_ps(x_arr, x);
        _mm_store_ps(y_arr, y);
        
        for (int j = 0; j < 4; ++j) {
            circles[i+j].x = x_arr[j];
            circles[i+j].y = y_arr[j];
        }
    }
    
    // Handle remaining circles
    for (size_t i = simd_count; i < count; ++i) {
        circles[i].x += dx;
        circles[i].y += dy;
    }
}

void translate_rectangles_simd(CompactRectangle* rects, size_t count, float dx, float dy) {
    // Similar implementation for rectangles
    __m128 delta_x = _mm_set1_ps(dx);
    __m128 delta_y = _mm_set1_ps(dy);
    
    size_t simd_count = count & ~3;
    for (size_t i = 0; i < simd_count; i += 4) {
        __m128 x = _mm_set_ps(rects[i+3].x, rects[i+2].x, rects[i+1].x, rects[i].x);
        __m128 y = _mm_set_ps(rects[i+3].y, rects[i+2].y, rects[i+1].y, rects[i].y);
        
        x = _mm_add_ps(x, delta_x);
        y = _mm_add_ps(y, delta_y);
        
        float x_arr[4], y_arr[4];
        _mm_store_ps(x_arr, x);
        _mm_store_ps(y_arr, y);
        
        for (int j = 0; j < 4; ++j) {
            rects[i+j].x = x_arr[j];
            rects[i+j].y = y_arr[j];
        }
    }
    
    for (size_t i = simd_count; i < count; ++i) {
        rects[i].x += dx;
        rects[i].y += dy;
    }
}
#else
// Fallback implementations without SIMD
void translate_circles_simd(CompactCircle* circles, size_t count, float dx, float dy) {
    for (size_t i = 0; i < count; ++i) {
        circles[i].x += dx;
        circles[i].y += dy;
    }
}

void translate_rectangles_simd(CompactRectangle* rects, size_t count, float dx, float dy) {
    for (size_t i = 0; i < count; ++i) {
        rects[i].x += dx;
        rects[i].y += dy;
    }
}
#endif

} // namespace simd

} // namespace drawing