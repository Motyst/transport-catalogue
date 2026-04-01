#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>

#include "../src/domain.h"
#include "../src/geo.h"
#include "../src/json.h"
#include "../src/svg.h"

inline const double EPSILON = 1e-6;

class SphereProjector {
public:
    SphereProjector() = default;

    template <typename PointInputIt>
SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_ = 0;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

namespace map{
using namespace json;
using namespace svg;
using namespace std::string_literals;

struct RenderSetting{
    double width = 0.0;
    double height = 0.0;

    double padding = 0.0;

    double line_width = 1.0;
    double stop_radius = 1.0;

    int bus_label_font_size = 1;
    svg::Point bus_label_offset = {0.0, 0.0};

    int stop_label_font_size = 0;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width = 1.0;

    std::vector<svg::Color> color_palette;
};

class MapRender{
public:
    void AddRenderData(const std::vector<entities::BusRouteRenderInfo>& bus_routes);

    void AddBusRouteName(const std::string& name, const geo::Coordinates location, svg::Color fill_color);
    void AddBusRoute(const std::vector<entities::StopPtr>& stops, svg::Color fill_color);

    void AddStopCircle(const svg::Point location);
    void AddStopName(const std::string& name, const svg::Point location);

    void SetRenderSettings(const json::Dict& node);

public:
    void RenderObjects(std::ostream& out) const;

private:
    void SetSphereProjector(const std::vector<entities::BusRouteRenderInfo>& bus_routes);

private:
    svg::Color CheckColorType(const json::Node& node)const;
    svg::Point CalculateLocation(const geo::Coordinates& location)const;

private:
    RenderSetting render_settings_;
    svg::Document objects_;
    SphereProjector sphere_;
};
}
