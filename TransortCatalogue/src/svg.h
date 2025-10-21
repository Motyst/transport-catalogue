#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace svg {

struct Rgb{
    Rgb() = default;

    Rgb(uint8_t red_color, uint8_t green_color, uint8_t blue_color) :
        red(red_color),
        green(green_color),
        blue(blue_color)
    {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba{
    Rgba() = default;

    Rgba(uint8_t red_color, uint8_t green_color, uint8_t blue_color, double color_opacity) :
        red(red_color),
        green(green_color),
        blue(blue_color),
        opacity(color_opacity)
    {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
inline const Color NoneColor{"none"};

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

struct ColorPrinter {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << "none";
    }
    void operator()(const std::string& color) const {
        out << color;
    }
    void operator()(svg::Rgb color) const {
        out << "rgb(" << unsigned(color.red)  << "," << unsigned(color.green) << "," << unsigned(color.blue) << ")";
    }
    void operator()(svg::Rgba color) const {
        out
        << "rgba("<< unsigned(color.red)  << ","
        << unsigned(color.green) << ","
        << unsigned(color.blue) << ","
        << color.opacity << ")";
    }
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = std::move(line_cap);
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = std::move(line_join);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (!(std::holds_alternative<std::monostate>(fill_color_))) {
            out << " fill=\""sv;
            std::visit(ColorPrinter{ out }, fill_color_);
            out << "\""sv;
        }

        if (!std::holds_alternative<std::monostate>(stroke_color_)) {
            out << " stroke=\""sv;
            std::visit(ColorPrinter{ out }, stroke_color_);
            out << "\""sv;
        }

        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    Color fill_color_;
    Color stroke_color_;
    std::optional<double> stroke_width_;

    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class Circle : public Object, public PathProps<Circle> {
public:
    Circle() = default;

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_ = {0.0, 0.0};
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline() = default;
    Polyline& AddPoint(Point point);
private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;
    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t size_ = 1;

    std::string font_family_;
    std::string font_weight_;
    std::string data_ = "";
};

class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) {
        objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    virtual ~ObjectContainer() = default;

protected:
    std::vector<std::unique_ptr<Object>> objects_;
};

class Document : public ObjectContainer{
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    void Render(std::ostream& out) const;
};

class Drawable{
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

}  // namespace svg
