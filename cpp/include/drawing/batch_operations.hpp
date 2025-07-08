#pragma once

#include "drawing.hpp"
#include <vector>
#include <algorithm>
#include <execution>
#include <numeric>

namespace drawing {

// Batch geometric operations for high performance
class BatchOperations {
public:
    // Batch transform operations
    static void translate_objects(ObjectStorage& storage, 
                                 const std::vector<ObjectID>& ids,
                                 float dx, float dy);
    
    static void scale_objects(ObjectStorage& storage,
                             const std::vector<ObjectID>& ids,
                             float sx, float sy,
                             const Point& center = Point(0, 0));
    
    static void rotate_objects(ObjectStorage& storage,
                              const std::vector<ObjectID>& ids,
                              float angle_radians,
                              const Point& center = Point(0, 0));
    
    // Batch bounding box calculations
    static BoundingBox calculate_bounding_box(const ObjectStorage& storage,
                                            const std::vector<ObjectID>& ids);
    
    // Batch distance calculations
    static std::vector<float> calculate_distances_to_point(const ObjectStorage& storage,
                                                          const std::vector<ObjectID>& ids,
                                                          const Point& point);
    
    // Batch collision detection
    struct CollisionPair {
        ObjectID id1;
        ObjectID id2;
    };
    
    static std::vector<CollisionPair> find_collisions(const ObjectStorage& storage,
                                                      const std::vector<ObjectID>& ids);
    
    // Batch alignment operations
    static void align_objects_left(ObjectStorage& storage,
                                  const std::vector<ObjectID>& ids);
    
    static void align_objects_right(ObjectStorage& storage,
                                   const std::vector<ObjectID>& ids);
    
    static void align_objects_top(ObjectStorage& storage,
                                 const std::vector<ObjectID>& ids);
    
    static void align_objects_bottom(ObjectStorage& storage,
                                    const std::vector<ObjectID>& ids);
    
    static void align_objects_center_h(ObjectStorage& storage,
                                      const std::vector<ObjectID>& ids);
    
    static void align_objects_center_v(ObjectStorage& storage,
                                      const std::vector<ObjectID>& ids);
    
    // Batch distribution operations
    static void distribute_horizontally(ObjectStorage& storage,
                                       const std::vector<ObjectID>& ids,
                                       float spacing = -1.0f); // -1 means equal spacing
    
    static void distribute_vertically(ObjectStorage& storage,
                                     const std::vector<ObjectID>& ids,
                                     float spacing = -1.0f);
    
    // Pattern generation
    static std::vector<ObjectID> create_grid(ObjectStorage& storage,
                                                           ObjectType type,
                                                           int rows, int cols,
                                                           float cell_width, float cell_height,
                                                           float x_offset = 0, float y_offset = 0);
    
    static std::vector<ObjectID> create_circular_pattern(ObjectStorage& storage,
                                                                      ObjectType type,
                                                                      int count,
                                                                      float radius,
                                                                      float center_x, float center_y);
    
    // Performance statistics
    struct PerformanceStats {
        size_t objects_processed;
        double time_ms;
        double objects_per_second;
    };
    
    static PerformanceStats last_operation_stats;
    
    // Helper to get object bounding box (public for testing)
    static BoundingBox get_object_bbox(const ObjectStorage& storage, ObjectID id);
    
private:
    // Helper to get object center
    static Point get_object_center(const ObjectStorage& storage, ObjectID id);
    
    // Helper to set object position
    static void set_object_position(ObjectStorage& storage, ObjectID id, 
                                   float x, float y);
};

// SIMD-optimized batch operations (when available)
namespace simd {
    
    // Process 4 circles at once using SSE/AVX
    void translate_circles_simd(CompactCircle* circles, size_t count, float dx, float dy);
    void scale_circles_simd(CompactCircle* circles, size_t count, float scale, 
                           float center_x, float center_y);
    
    // Process 4 rectangles at once
    void translate_rectangles_simd(CompactRectangle* rects, size_t count, float dx, float dy);
    void scale_rectangles_simd(CompactRectangle* rects, size_t count, 
                              float sx, float sy, float center_x, float center_y);
    
    // Batch color operations using SIMD
    void set_colors_simd(Color* colors, size_t count, const Color& new_color);
    void blend_colors_simd(Color* colors, size_t count, const Color& blend_color, float alpha);
    
} // namespace simd

} // namespace drawing