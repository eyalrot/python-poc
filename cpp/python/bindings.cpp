#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "drawing/drawing.hpp"
#include "drawing/serialization.hpp"
#include "drawing/batch_operations.hpp"

namespace py = pybind11;
using namespace drawing;

PYBIND11_MODULE(drawing_cpp, m) {
    m.doc() = "High-performance C++ drawing library with Python bindings";
    
    // Color class
    py::class_<Color>(m, "Color")
        .def(py::init<>())
        .def(py::init<uint8_t, uint8_t, uint8_t>(), py::arg("r"), py::arg("g"), py::arg("b"))
        .def(py::init<uint8_t, uint8_t, uint8_t, uint8_t>(), 
             py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a"))
        .def_readwrite("r", &Color::r)
        .def_readwrite("g", &Color::g)
        .def_readwrite("b", &Color::b)
        .def_readwrite("a", &Color::a)
        .def("to_rgba32", &Color::to_rgba32)
        .def_static("from_rgba32", &Color::from_rgba32)
        .def("__repr__", [](const Color& c) {
            return "Color(r=" + std::to_string(c.r) + 
                   ", g=" + std::to_string(c.g) + 
                   ", b=" + std::to_string(c.b) + 
                   ", a=" + std::to_string(c.a) + ")";
        })
        .def_readonly_static("BLACK", &Color::BLACK)
        .def_readonly_static("WHITE", &Color::WHITE)
        .def_readonly_static("TRANSPARENT", &Color::TRANSPARENT);
    
    // Point class
    py::class_<Point>(m, "Point")
        .def(py::init<>())
        .def(py::init<float, float>(), py::arg("x"), py::arg("y"))
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def("__repr__", [](const Point& p) {
            return "Point(x=" + std::to_string(p.x) + 
                   ", y=" + std::to_string(p.y) + ")";
        });
    
    // BoundingBox class
    py::class_<BoundingBox>(m, "BoundingBox")
        .def(py::init<>())
        .def(py::init<float, float, float, float>(), 
             py::arg("min_x"), py::arg("min_y"), py::arg("max_x"), py::arg("max_y"))
        .def_readwrite("min_x", &BoundingBox::min_x)
        .def_readwrite("min_y", &BoundingBox::min_y)
        .def_readwrite("max_x", &BoundingBox::max_x)
        .def_readwrite("max_y", &BoundingBox::max_y)
        .def("width", &BoundingBox::width)
        .def("height", &BoundingBox::height)
        .def("center", &BoundingBox::center)
        .def("contains", &BoundingBox::contains)
        .def("intersects", &BoundingBox::intersects)
        .def("expand", py::overload_cast<const Point&>(&BoundingBox::expand))
        .def("expand", py::overload_cast<const BoundingBox&>(&BoundingBox::expand));
    
    // LineStyle enum
    py::enum_<LineStyle>(m, "LineStyle")
        .value("Solid", LineStyle::Solid)
        .value("Dashed", LineStyle::Dashed)
        .value("Dotted", LineStyle::Dotted)
        .value("DashDot", LineStyle::DashDot);
    
    // GradientType enum
    py::enum_<GradientType>(m, "GradientType")
        .value("Linear", GradientType::Linear)
        .value("Radial", GradientType::Radial);
    
    // ObjectType enum
    py::enum_<ObjectType>(m, "ObjectType")
        .value("None", ObjectType::None)
        .value("Line", ObjectType::Line)
        .value("Circle", ObjectType::Circle)
        .value("Ellipse", ObjectType::Ellipse)
        .value("Rectangle", ObjectType::Rectangle)
        .value("Polygon", ObjectType::Polygon)
        .value("Polyline", ObjectType::Polyline)
        .value("Arc", ObjectType::Arc)
        .value("Text", ObjectType::Text)
        .value("Path", ObjectType::Path)
        .value("Group", ObjectType::Group);
    
    // Text alignment enums
    py::enum_<TextAlign>(m, "TextAlign")
        .value("Left", TextAlign::Left)
        .value("Center", TextAlign::Center)
        .value("Right", TextAlign::Right);
    
    py::enum_<TextBaseline>(m, "TextBaseline")
        .value("Top", TextBaseline::Top)
        .value("Middle", TextBaseline::Middle)
        .value("Bottom", TextBaseline::Bottom)
        .value("Alphabetic", TextBaseline::Alphabetic);
    
    // GradientStop class
    py::class_<GradientStop>(m, "GradientStop")
        .def(py::init<>())
        .def(py::init<float, Color>(), py::arg("offset"), py::arg("color"))
        .def_readwrite("offset", &GradientStop::offset)
        .def_readwrite("color", &GradientStop::color)
        .def("__repr__", [](const GradientStop& gs) {
            return "GradientStop(offset=" + std::to_string(gs.offset) + 
                   ", color=Color(" + std::to_string(gs.color.r) + "," + 
                   std::to_string(gs.color.g) + "," + std::to_string(gs.color.b) + "," + 
                   std::to_string(gs.color.a) + "))";
        });
    
    // Layer class
    py::class_<Layer>(m, "Layer")
        .def("get_id", &Layer::get_id)
        .def("get_name", &Layer::get_name)
        .def("is_visible", &Layer::is_visible)
        .def("is_locked", &Layer::is_locked)
        .def("get_opacity", &Layer::get_opacity)
        .def("set_visible", &Layer::set_visible)
        .def("set_locked", &Layer::set_locked)
        .def("set_opacity", &Layer::set_opacity)
        .def("object_count", &Layer::object_count)
        .def("get_objects", &Layer::get_objects);
    
    // Drawing class
    py::class_<Drawing> drawing_class(m, "Drawing");
    drawing_class
        .def(py::init<>())
        .def(py::init<float, float>(), py::arg("width")=800, py::arg("height")=600)
        .def("get_width", &Drawing::get_width)
        .def("get_height", &Drawing::get_height)
        .def("get_background", &Drawing::get_background)
        .def("set_width", &Drawing::set_width)
        .def("set_height", &Drawing::set_height)
        .def("set_background", &Drawing::set_background)
        .def("add_layer", &Drawing::add_layer, py::arg("name")="")
        .def("get_layer", &Drawing::get_layer, py::return_value_policy::reference_internal)
        .def("add_circle", &Drawing::add_circle, 
             py::arg("x"), py::arg("y"), py::arg("radius"), py::arg("layer_id")=0)
        .def("add_rectangle", &Drawing::add_rectangle,
             py::arg("x"), py::arg("y"), py::arg("width"), py::arg("height"), 
             py::arg("corner_radius")=0, py::arg("layer_id")=0)
        .def("add_line", &Drawing::add_line,
             py::arg("x1"), py::arg("y1"), py::arg("x2"), py::arg("y2"), 
             py::arg("line_style")=LineStyle::Solid, py::arg("layer_id")=0)
        .def("add_polygon", &Drawing::add_polygon,
             py::arg("points"), py::arg("closed")=true, py::arg("layer_id")=0)
        .def("add_ellipse", &Drawing::add_ellipse,
             py::arg("x"), py::arg("y"), py::arg("rx"), py::arg("ry"), 
             py::arg("rotation")=0, py::arg("layer_id")=0)
        .def("add_polyline", &Drawing::add_polyline,
             py::arg("points"), py::arg("line_style")=LineStyle::Solid, py::arg("layer_id")=0)
        .def("add_arc", &Drawing::add_arc,
             py::arg("x"), py::arg("y"), py::arg("radius"), 
             py::arg("start_angle"), py::arg("end_angle"), py::arg("layer_id")=0)
        .def("add_text", &Drawing::add_text,
             py::arg("x"), py::arg("y"), py::arg("text"), 
             py::arg("font_size")=16.0f, py::arg("font_name")="Arial",
             py::arg("align")=TextAlign::Left, py::arg("baseline")=TextBaseline::Alphabetic,
             py::arg("layer_id")=0)
        .def("add_path", &Drawing::add_path,
             py::arg("path_data"), py::arg("layer_id")=0)
        .def("add_group", py::overload_cast<uint8_t>(&Drawing::add_group),
             py::arg("layer_id")=0)
        .def("add_group", py::overload_cast<const std::vector<ObjectID>&, uint8_t>(&Drawing::add_group),
             py::arg("children"), py::arg("layer_id")=0)
        .def("add_to_group", &Drawing::add_to_group,
             py::arg("group_id"), py::arg("child_id"))
        .def("add_linear_gradient", &Drawing::add_linear_gradient,
             py::arg("stops"), py::arg("angle")=0.0f)
        .def("add_radial_gradient", &Drawing::add_radial_gradient,
             py::arg("stops"), py::arg("center_x"), py::arg("center_y"), py::arg("radius"))
        .def("set_object_gradient", &Drawing::set_object_gradient,
             py::arg("object_id"), py::arg("gradient_id"))
        .def("add_pattern", &Drawing::add_pattern,
             py::arg("pattern_name"))
        .def("set_object_pattern", &Drawing::set_object_pattern,
             py::arg("object_id"), py::arg("pattern_id"))
        .def("set_object_name", &Drawing::set_object_name,
             py::arg("object_id"), py::arg("name"))
        .def("get_object_name", &Drawing::get_object_name,
             py::arg("object_id"))
        .def("set_object_metadata", &Drawing::set_object_metadata,
             py::arg("object_id"), py::arg("key"), py::arg("value"))
        .def("get_object_metadata", &Drawing::get_object_metadata,
             py::arg("object_id"), py::arg("key"))
        .def("get_all_object_metadata", &Drawing::get_all_object_metadata,
             py::arg("object_id"))
        .def("get_bounding_box", &Drawing::get_bounding_box)
        .def("find_objects_in_rect", &Drawing::find_objects_in_rect)
        .def("total_objects", &Drawing::total_objects)
        .def("memory_usage", &Drawing::memory_usage)
        .def("get_storage", py::overload_cast<>(&Drawing::get_storage), 
             py::return_value_policy::reference_internal)
        .def("__repr__", [](const Drawing& d) {
            return "Drawing(width=" + std::to_string(d.get_width()) + 
                   ", height=" + std::to_string(d.get_height()) + 
                   ", objects=" + std::to_string(d.total_objects()) + ")";
        });
    
    // ObjectStorage access (for advanced usage)
    py::class_<ObjectStorage>(m, "ObjectStorage")
        .def("total_objects", &ObjectStorage::total_objects)
        .def("memory_usage", &ObjectStorage::memory_usage)
        .def("set_fill_color", &ObjectStorage::set_fill_color)
        .def("set_stroke_color", &ObjectStorage::set_stroke_color)
        .def("set_opacity", &ObjectStorage::set_opacity)
        .def("find_in_rect", &ObjectStorage::find_in_rect)
        .def("find_at_point", &ObjectStorage::find_at_point, 
             py::arg("point"), py::arg("tolerance")=1.0f)
        .def("add_linear_gradient", &ObjectStorage::add_linear_gradient,
             py::arg("stops"), py::arg("angle")=0.0f)
        .def("add_radial_gradient", &ObjectStorage::add_radial_gradient,
             py::arg("stops"), py::arg("center_x"), py::arg("center_y"), py::arg("radius"))
        .def("set_object_gradient", &ObjectStorage::set_object_gradient,
             py::arg("object_id"), py::arg("gradient_id"))
        .def("add_pattern", &ObjectStorage::add_pattern,
             py::arg("pattern_name"))
        .def("set_object_pattern", &ObjectStorage::set_object_pattern,
             py::arg("object_id"), py::arg("pattern_id"))
        .def("set_object_name", &ObjectStorage::set_object_name,
             py::arg("object_id"), py::arg("name"))
        .def("get_object_name", &ObjectStorage::get_object_name,
             py::arg("object_id"))
        .def("set_object_metadata", &ObjectStorage::set_object_metadata,
             py::arg("object_id"), py::arg("key"), py::arg("value"))
        .def("get_object_metadata", &ObjectStorage::get_object_metadata,
             py::arg("object_id"), py::arg("key"))
        .def("get_all_object_metadata", &ObjectStorage::get_all_object_metadata,
             py::arg("object_id"));
    
    // Serialization functions
    m.def("save_binary", &save_binary, "Save drawing to binary format",
          py::arg("drawing"), py::arg("filename"));
    m.def("load_binary", &load_binary, "Load drawing from binary format",
          py::arg("filename"));
    m.def("save_json", &save_json, "Save drawing to JSON format",
          py::arg("drawing"), py::arg("filename"));
    
    // Performance info
    m.def("get_compact_sizes", []() {
        py::dict sizes;
        sizes["CompactCircle"] = sizeof(CompactCircle);
        sizes["CompactRectangle"] = sizeof(CompactRectangle);
        sizes["CompactLine"] = sizeof(CompactLine);
        sizes["CompactEllipse"] = sizeof(CompactEllipse);
        sizes["CompactPolygon"] = sizeof(CompactPolygon);
        sizes["CompactPolyline"] = sizeof(CompactPolyline);
        sizes["CompactArc"] = sizeof(CompactArc);
        sizes["CompactText"] = sizeof(CompactText);
        sizes["CompactPath"] = sizeof(CompactPath);
        sizes["PathSegment"] = sizeof(PathSegment);
        sizes["Color"] = sizeof(Color);
        sizes["Point"] = sizeof(Point);
        return sizes;
    }, "Get size in bytes of compact object types");
    
    // Batch operations
    py::class_<BatchOperations>(m, "BatchOperations")
        .def_static("translate_objects", &BatchOperations::translate_objects,
                    "Translate multiple objects by dx, dy",
                    py::arg("storage"), py::arg("ids"), py::arg("dx"), py::arg("dy"))
        .def_static("scale_objects", &BatchOperations::scale_objects,
                    "Scale multiple objects by sx, sy around center",
                    py::arg("storage"), py::arg("ids"), py::arg("sx"), py::arg("sy"), 
                    py::arg("center")=Point(0, 0))
        .def_static("rotate_objects", &BatchOperations::rotate_objects,
                    "Rotate multiple objects by angle around center",
                    py::arg("storage"), py::arg("ids"), py::arg("angle_radians"), 
                    py::arg("center")=Point(0, 0))
        .def_static("calculate_bounding_box", &BatchOperations::calculate_bounding_box,
                    "Calculate bounding box of multiple objects",
                    py::arg("storage"), py::arg("ids"))
        .def_static("align_objects_left", &BatchOperations::align_objects_left,
                    "Align objects to leftmost edge",
                    py::arg("storage"), py::arg("ids"))
        .def_static("create_grid", &BatchOperations::create_grid,
                    "Create a grid pattern of objects",
                    py::arg("storage"), py::arg("type"), py::arg("rows"), py::arg("cols"),
                    py::arg("cell_width"), py::arg("cell_height"), 
                    py::arg("x_offset")=0, py::arg("y_offset")=0);
    
    // Performance stats
    py::class_<BatchOperations::PerformanceStats>(m, "PerformanceStats")
        .def_readonly("objects_processed", &BatchOperations::PerformanceStats::objects_processed)
        .def_readonly("time_ms", &BatchOperations::PerformanceStats::time_ms)
        .def_readonly("objects_per_second", &BatchOperations::PerformanceStats::objects_per_second);
    
    m.def("get_last_operation_stats", []() {
        return BatchOperations::last_operation_stats;
    }, "Get performance statistics from the last batch operation");
    
    // Version info
    m.attr("__version__") = "0.1.0";
    m.attr("BYTES_PER_CIRCLE") = sizeof(CompactCircle);
    m.attr("BYTES_PER_RECTANGLE") = sizeof(CompactRectangle);
    m.attr("BYTES_PER_LINE") = sizeof(CompactLine);
    m.attr("BYTES_PER_ELLIPSE") = sizeof(CompactEllipse);
    m.attr("BYTES_PER_POLYGON") = sizeof(CompactPolygon);
    m.attr("BYTES_PER_POLYLINE") = sizeof(CompactPolyline);
    m.attr("BYTES_PER_ARC") = sizeof(CompactArc);
    m.attr("BYTES_PER_TEXT") = sizeof(CompactText);
    m.attr("BYTES_PER_PATH") = sizeof(CompactPath);
    m.attr("BYTES_PER_GROUP") = sizeof(CompactGroup);
}