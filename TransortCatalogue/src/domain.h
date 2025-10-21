#pragma once

#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "../src/geo.h"

namespace entities{

struct Stop{
    std::string name;
    geo::Coordinates location;
};

using StopPtr = const Stop*;

struct Bus{
    std::string name;
    std::vector<StopPtr> stops;
    bool is_circular = false;
};

using BusPtr = const Bus*;

struct StopBusList{
    std::string stop;
    bool buses_exist = false;
    std::set<std::string_view> bus_list;
};

struct BusRoute {
    std::string bus;
    int stop_count = 0;
    int unique_stops = 0;
    int route_lenght = 0;
    double route_curvature = 0;
};

struct BusRouteRenderInfo{
    std::string name;
    std::vector<StopPtr> stops;
    bool route_cirular = false;
};

}

namespace hashers{
using namespace entities;

struct StopDistanceHasher {
    size_t operator()(const std::pair<StopPtr, StopPtr> stop_pair) const {
        std::string stop1 = stop_pair.first->name;
        std::string stop2 = stop_pair.second->name;

        return hasher((stop1 + stop2));
    }

private:
    std::hash<std::string> hasher;
};

struct PtrHasher {
    size_t operator()(const StopPtr stop) const {
        return ((size_t)stop) >> 3;
    }
};
}
