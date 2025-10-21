#pragma once

#include <algorithm>
#include <iostream>
#include <deque>
#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "../src/domain.h"
#include "../src/geo.h"

namespace transport_catalogue{
using namespace entities;

class TransportCatalogue{
public:
    void AddBus(const std::string& bus, std::vector<std::string> stops, bool roundtrip);
    void AddStop(const std::string& stop);
    void AddStop(const std::string& stop, const geo::Coordinates& coordinates);

    void SetDistanceBetweenStops(const std::string& stop,
                                 const std::vector<std::pair<std::string, int>>& distance_to_stops);

public:
    std::vector<StopPtr> FindBusRoute(const std::string& bus) const;
    StopPtr FindStop(const std::string& bus_stop) const;
    StopBusList StopInformation(const std::string& stop) const;
    BusRoute RouteInformation(const std::string& bus) const;
    std::vector<BusRouteRenderInfo> GetRenderData()const;

private:
    std::deque<Bus> buses_;
    std::deque<Stop> stops_;

    std::unordered_map<std::string_view, BusPtr> bus_access_;
    std::unordered_map<std::string_view, Stop*> stop_access_;

    std::unordered_map<StopPtr, std::unordered_set<BusPtr>> bus_by_stop_;
    std::unordered_map<BusPtr, std::unordered_set<StopPtr>> stop_by_bus_;

    std::unordered_map<std::pair<StopPtr, StopPtr>, int, hashers::StopDistanceHasher> distance_between_stops_;
};

}
