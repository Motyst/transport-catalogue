#include "../src/map_renderer.h"

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                    double max_height, double padding) : padding_(padding) {
    if (points_begin == points_end) {
        return;
    }

    const auto [left_it, right_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lng < rhs.lng;
        });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    const auto [bottom_it, top_it]
        = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lat < rhs.lat;
        });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    std::optional<double> width_zoom;
    if (!(std::abs(max_lon - min_lon_) < EPSILON)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!(std::abs(max_lat_ - min_lat) < EPSILON)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        zoom_coeff_ = *height_zoom;
    }
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

namespace map{
using namespace json;
using namespace svg;
using namespace std::string_literals;

void MapRender::RenderObjects(std::ostream& out) const {
    objects_.Render(out);
}

// ---------- Adding Objects ------------------
void MapRender::AddRenderData(const std::vector<entities::BusRouteRenderInfo>& bus_routes){
    SetSphereProjector(bus_routes);

    std::map<std::string_view, svg::Point> sorted_stops;

    // Line - route
    for(size_t i = 0; i < bus_routes.size(); ++i){
        int color_id = i % render_settings_.color_palette.size();
        svg::Color color = render_settings_.color_palette[color_id];

        AddBusRoute(bus_routes[i].stops, color);

        for(const auto& stop : bus_routes[i].stops){
            sorted_stops.emplace(stop->name, CalculateLocation(stop->location));
        }
    }

    // Text - Bus name
    for(size_t i = 0; i < bus_routes.size(); ++i){
        int color_id = i % render_settings_.color_palette.size();
        svg::Color color = render_settings_.color_palette[color_id];

        geo::Coordinates first_stop = bus_routes[i].stops.front()->location;

        if(bus_routes[i].route_cirular){
            AddBusRouteName(bus_routes[i].name, first_stop, color);
        }else{
            AddBusRouteName(bus_routes[i].name, first_stop, color);

            int end_stop = bus_routes[i].stops.size() / 2;
            geo::Coordinates last_stop = bus_routes[i].stops[end_stop]->location;

            if(first_stop.lat != last_stop.lat || first_stop.lng != last_stop.lng){
                AddBusRouteName(bus_routes[i].name, last_stop, color);
            }
        }
    }

    // Circle - Stop location
    for(const auto& [name, location] : sorted_stops){
        AddStopCircle(location);
    }

    // Text - Stop name
    for(const auto& [name, location] : sorted_stops){
        AddStopName(std::string(name), location);
    }
}

// --------------Route line---------------------
void MapRender::AddBusRoute(const std::vector<entities::StopPtr>& stops, svg::Color fill_color){
    Polyline route;
    for(const auto& stop : stops){
        route.AddPoint(CalculateLocation(stop->location));
    }
    route.SetFillColor(NoneColor)
         .SetStrokeColor(fill_color)
         .SetStrokeWidth(render_settings_.line_width)
         .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
         .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    objects_.Add(route);
}

// --------------Bus Name---------------------
void MapRender::AddBusRouteName(const std::string& name, const geo::Coordinates location, svg::Color fill_color){
    objects_.Add(Text()
                    .SetFontFamily("Verdana"s)
                    .SetPosition(CalculateLocation(location))
                    .SetOffset(render_settings_.bus_label_offset)
                    .SetFontSize(render_settings_.bus_label_font_size)
                    .SetFontWeight("bold"s)
                    .SetData(name)
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetStrokeColor(render_settings_.underlayer_color)
                    .SetStrokeWidth(render_settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
    objects_.Add(Text()
                    .SetFontFamily("Verdana"s)
                    .SetPosition(CalculateLocation(location))
                    .SetOffset(render_settings_.bus_label_offset)
                    .SetFontSize(render_settings_.bus_label_font_size)
                    .SetFontWeight("bold"s)
                    .SetData(name)
                    .SetFillColor(fill_color));
}

// --------------Stop Circle---------------------
void MapRender::AddStopCircle(const svg::Point location){
    objects_.Add(Circle()
                    .SetCenter(location)
                    .SetRadius(render_settings_.stop_radius)
                    .SetFillColor("white"));
}

// --------------Stop Name---------------------
void MapRender::AddStopName(const std::string& name, const svg::Point location){
    objects_.Add(Text()
                    .SetFontFamily("Verdana"s)
                    .SetPosition(location)
                    .SetOffset(render_settings_.stop_label_offset)
                    .SetFontSize(render_settings_.stop_label_font_size)
                    .SetData(name)
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetStrokeColor(render_settings_.underlayer_color)
                    .SetStrokeWidth(render_settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
    objects_.Add(Text()
                    .SetFontFamily("Verdana"s)
                    .SetPosition(location)
                    .SetOffset(render_settings_.stop_label_offset)
                    .SetFontSize(render_settings_.stop_label_font_size)
                    .SetData(name)
                    .SetFillColor("black"));
}

// ---------- Render Setting------------------
void MapRender::SetRenderSettings(const json::Dict& node){
    render_settings_.height = node.at("height").AsDouble();
    render_settings_.width = node.at("width").AsDouble();

    render_settings_.padding = node.at("padding").AsDouble();
    render_settings_.line_width = node.at("line_width").AsDouble();
    render_settings_.stop_radius = node.at("stop_radius").AsDouble();

    render_settings_.bus_label_font_size = node.at("bus_label_font_size").AsInt();
    render_settings_.bus_label_offset = {node.at("bus_label_offset").AsArray()[0].AsDouble(),
            node.at("bus_label_offset").AsArray()[1].AsDouble()};

    render_settings_.stop_label_font_size = node.at("stop_label_font_size").AsInt();
    render_settings_.stop_label_offset = {node.at("stop_label_offset").AsArray()[0].AsDouble(),
            node.at("stop_label_offset").AsArray()[1].AsDouble()};

    render_settings_.underlayer_color = CheckColorType(node.at("underlayer_color"));
    render_settings_.underlayer_width = node.at("underlayer_width").AsDouble();

    for(const auto& color : node.at("color_palette").AsArray()){
        render_settings_.color_palette.push_back(CheckColorType(color));
    }
}

svg::Color MapRender::CheckColorType(const json::Node& node)const{
    if(node.IsString()){
        return node.AsString();
    }else{
        // Checks for RGB or RGBA
        if(node.AsArray().size() == 3){
            return svg::Rgb(node.AsArray()[0].AsDouble(),
                            node.AsArray()[1].AsDouble(),
                            node.AsArray()[2].AsDouble());
        }else{
            return svg::Rgba(node.AsArray()[0].AsDouble(),
                             node.AsArray()[1].AsDouble(),
                             node.AsArray()[2].AsDouble(),
                             node.AsArray()[3].AsDouble());
        }
    }
}

void MapRender::SetSphereProjector(const std::vector<entities::BusRouteRenderInfo>& bus_routes){
    std::vector<geo::Coordinates> all_coordinates;

    for(const auto& route : bus_routes){
        for(const auto& stop : route.stops){
            all_coordinates.push_back(stop->location);
        }
    }
    SphereProjector sphere(all_coordinates.begin(), all_coordinates.end(), render_settings_.width,
                          render_settings_.height, render_settings_.padding);

    sphere_ = sphere;
}

svg::Point MapRender::CalculateLocation(const geo::Coordinates& location)const {
    return sphere_(location);
}
}
