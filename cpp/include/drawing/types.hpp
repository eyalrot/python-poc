#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <cmath>
#include <algorithm>

namespace drawing {

// Forward declarations
struct Point;
struct Color;
struct BoundingBox;

// Compact color representation (4 bytes)
struct Color {
    uint8_t r, g, b, a;
    
    constexpr Color() : r(0), g(0), b(0), a(255) {}
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) 
        : r(r), g(g), b(b), a(a) {}
    
    static constexpr Color from_rgba32(uint32_t rgba) {
        return Color(
            (rgba >> 24) & 0xFF,
            (rgba >> 16) & 0xFF,
            (rgba >> 8) & 0xFF,
            rgba & 0xFF
        );
    }
    
    constexpr uint32_t to_rgba32() const {
        return (uint32_t(r) << 24) | (uint32_t(g) << 16) | 
               (uint32_t(b) << 8) | uint32_t(a);
    }
    
    static const Color BLACK;
    static const Color WHITE;
    static const Color TRANSPARENT;
};

inline constexpr Color Color::BLACK{0, 0, 0, 255};
inline constexpr Color Color::WHITE{255, 255, 255, 255};
inline constexpr Color Color::TRANSPARENT{0, 0, 0, 0};

// Compact point representation (8 bytes)
struct Point {
    float x, y;
    
    constexpr Point() : x(0), y(0) {}
    constexpr Point(float x, float y) : x(x), y(y) {}
};

// Bounding box (16 bytes)
struct BoundingBox {
    float min_x, min_y, max_x, max_y;
    
    constexpr BoundingBox() 
        : min_x(0), min_y(0), max_x(0), max_y(0) {}
    
    constexpr BoundingBox(float min_x, float min_y, float max_x, float max_y)
        : min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y) {}
    
    constexpr float width() const { return max_x - min_x; }
    constexpr float height() const { return max_y - min_y; }
    constexpr Point center() const { 
        return Point((min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f); 
    }
    
    constexpr bool contains(const Point& p) const {
        return p.x >= min_x && p.x <= max_x && p.y >= min_y && p.y <= max_y;
    }
    
    constexpr bool intersects(const BoundingBox& other) const {
        return !(max_x < other.min_x || min_x > other.max_x ||
                 max_y < other.min_y || min_y > other.max_y);
    }
    
    void expand(const Point& p) {
        if (p.x < min_x) min_x = p.x;
        if (p.x > max_x) max_x = p.x;
        if (p.y < min_y) min_y = p.y;
        if (p.y > max_y) max_y = p.y;
    }
    
    void expand(const BoundingBox& other) {
        if (other.min_x < min_x) min_x = other.min_x;
        if (other.max_x > max_x) max_x = other.max_x;
        if (other.min_y < min_y) min_y = other.min_y;
        if (other.max_y > max_y) max_y = other.max_y;
    }
};

// Object types enum (1 byte)
enum class ObjectType : uint8_t {
    None = 0,
    Line = 1,
    Circle = 2,
    Ellipse = 3,
    Rectangle = 4,
    Polygon = 5,
    Polyline = 6,
    Arc = 7,
    Text = 8,
    Path = 9,
    Group = 10
};

// Object flags (2 bytes)
struct ObjectFlags {
    uint16_t value;
    
    static constexpr uint16_t VISIBLE = 1 << 0;
    static constexpr uint16_t LOCKED = 1 << 1;
    static constexpr uint16_t SELECTED = 1 << 2;
    static constexpr uint16_t HAS_FILL = 1 << 3;
    static constexpr uint16_t HAS_STROKE = 1 << 4;
    static constexpr uint16_t HAS_TRANSFORM = 1 << 5;
    
    constexpr ObjectFlags() : value(VISIBLE | HAS_FILL) {}
    
    constexpr bool is_visible() const { return value & VISIBLE; }
    constexpr bool is_locked() const { return value & LOCKED; }
    constexpr bool is_selected() const { return value & SELECTED; }
    constexpr bool has_fill() const { return value & HAS_FILL; }
    constexpr bool has_stroke() const { return value & HAS_STROKE; }
    constexpr bool has_transform() const { return value & HAS_TRANSFORM; }
    
    void set_visible(bool v) { 
        if (v) value |= VISIBLE; 
        else value &= ~VISIBLE; 
    }
    void set_locked(bool v) { 
        if (v) value |= LOCKED; 
        else value &= ~LOCKED; 
    }
    void set_selected(bool v) { 
        if (v) value |= SELECTED; 
        else value &= ~SELECTED; 
    }
};

// Compact transform (when needed) - 24 bytes
struct Transform2D {
    float m11, m12, m13;  // First row: scale_x, shear_x, translate_x
    float m21, m22, m23;  // Second row: shear_y, scale_y, translate_y
    
    constexpr Transform2D() 
        : m11(1), m12(0), m13(0), 
          m21(0), m22(1), m23(0) {}
    
    Point transform(const Point& p) const {
        return Point(
            m11 * p.x + m12 * p.y + m13,
            m21 * p.x + m22 * p.y + m23
        );
    }
    
    static Transform2D translate(float tx, float ty) {
        Transform2D t;
        t.m13 = tx;
        t.m23 = ty;
        return t;
    }
    
    static Transform2D scale(float sx, float sy) {
        Transform2D t;
        t.m11 = sx;
        t.m22 = sy;
        return t;
    }
    
    static Transform2D rotate(float angle_radians) {
        Transform2D t;
        float c = std::cos(angle_radians);
        float s = std::sin(angle_radians);
        t.m11 = c;  t.m12 = -s;
        t.m21 = s;  t.m22 = c;
        return t;
    }
};

} // namespace drawing