#include "../src/svg.h"

namespace svg {
using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    RenderObject(context);
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap){
    using namespace std::literals;
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            return out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            return out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            return out << "square"sv;
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join){
    using namespace std::literals;
    switch (line_join) {
        case StrokeLineJoin::ARCS:
            return out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            return out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            return out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            return out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            return out << "round"sv;
            break;
    }
    return out;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>\\n"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline"sv;

    if (points_.empty()) {
        out << " points=\"\""sv;
    }
    else{
        out << " points=\""sv << points_[0].x << ","sv << points_[0].y;

        for (size_t i = 1; i < points_.size(); ++i) {
            out << " "sv << points_[i].x << ","sv << points_[i].y;
        }
        out << "\""sv;
    }
    RenderAttrs(out);

    out << "/>\\n"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data){
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(out);

    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" " << "dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << size_ << "\""sv;

    if(!font_family_.empty()){
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if(!font_weight_.empty()){
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }

    out << ">" << data_ << "</text>\\n"sv;
}

// ---------- Document ------------------
void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\\n"sv;
    RenderContext ctx(out, 2, 2);

    for(const auto& object : objects_){
        object->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg
