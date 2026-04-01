#include "../src/transport_catalogue.h"

namespace transport_catalogue{
using namespace entities;

void TransportCatalogue::AddBus(const std::string& bus, std::vector<std::string> stops, bool roundtrip) {
    std::vector<StopPtr> stop_pointers;

    for(const std::string& stop : stops){
        if(stop_access_.find(stop) == stop_access_.end()){
            AddStop(stop);
        }
        stop_pointers.push_back(stop_access_[stop]);
    }

    // Creates bus in deque
    Bus new_bus = {bus, stop_pointers, roundtrip};

    auto& bus_reference = buses_.emplace_back(new_bus);

    // Adding buses to stops
    for(const auto& stop : stop_pointers){
        bus_by_stop_[stop_access_[stop->name]].emplace(&bus_reference);
        bus_by_stop_.at(stop_access_[stop->name]).size();
    }

    // Creates bus access
    bus_access_.emplace(bus_reference.name, &bus_reference);
}

void TransportCatalogue::AddStop(const std::string& stop){
    Stop new_stop = {stop, {}};
    auto& stop_reference = stops_.emplace_back(new_stop);
    stop_access_.emplace(stop_reference.name, &stop_reference);
}

void TransportCatalogue::AddStop(const std::string& stop, const geo::Coordinates& coordinates){
    // Check if stop is new
    if(stop_access_.find(stop) == stop_access_.end()){
        Stop new_stop = {stop, coordinates};

        auto& stop_reference = stops_.emplace_back(std::move(new_stop));
        stop_access_.emplace(stop_reference.name, &stop_reference);
    }else{
        stop_access_[stop]->location = coordinates;
    }
}

void TransportCatalogue::SetDistanceBetweenStops(const std::string& stop,
                                                 const std::vector<std::pair<std::string, int>>& distance_to_stops){
    Stop* main_stop = stop_access_.at(stop);

    for(const auto& [stop_name, distance] : distance_to_stops){
        if(stop_access_.find(stop_name) == stop_access_.end()){
            AddStop(stop_name);
        }
        Stop* stop_from_list = stop_access_.at(stop_name);

        distance_between_stops_[std::make_pair(main_stop, stop_from_list)] = distance;
        distance_between_stops_.emplace(std::make_pair(stop_from_list, main_stop), distance);

    }
}

std::vector<BusRouteRenderInfo> TransportCatalogue::GetRenderData()const{
    std::vector<BusRouteRenderInfo> route_info;
    for(const auto& bus : buses_){
        if(!bus.stops.empty()){
            route_info.push_back({bus.name, bus.stops, bus.is_circular});
        }
    }

    std::sort(route_info.begin(), route_info.end(),[](const auto& left, const auto& right){
        return left.name < right.name;
    });

    return route_info;
}

std::vector<StopPtr> TransportCatalogue::FindBusRoute(const std::string& bus) const {
      return bus_access_.at(bus)->stops;
}

StopPtr TransportCatalogue::FindStop(const std::string& bus_stop) const {
    return stop_access_.at(bus_stop);
}

StopBusList TransportCatalogue::StopInformation(const std::string& stop) const {
    StopBusList result = {stop, {}, {}};
    // Stop doesn't exist
    if(stop_access_.find(stop) == stop_access_.end()){
        return result;
    }
    result.buses_exist = true;

    // Stop exist's without a bus
    const auto stop_ptr = stop_access_.at(stop);
    if(bus_by_stop_.find(stop_ptr) == bus_by_stop_.end()){
        return result;
    }

    for(const auto& bus : bus_by_stop_.at(stop_access_.at(stop))){
        result.bus_list.emplace(bus->name) ;
    }

    return result;
}

BusRoute TransportCatalogue::RouteInformation(const std::string& bus) const {

    if(bus_access_.find(bus) == bus_access_.end()){
        return {bus, 0, 0, 0, 0};
    }
    int stop_count = bus_access_.at(bus)->stops.size();

    std::set<std::string_view> unique_stops;
    for(auto& stop : bus_access_.at(bus)->stops){
        unique_stops.emplace(stop->name);
    }

    int route_length = 0;
    double route_curvature = 0;

    for(int i = 0; i < stop_count - 1; ++i){
        StopPtr stop1 = stop_access_.at(bus_access_.at(bus)->stops[i]->name);
        StopPtr stop2 = stop_access_.at(bus_access_.at(bus)->stops[i + 1]->name);

        route_length += distance_between_stops_.at(std::make_pair(stop1, stop2));

        geo::Coordinates location1 = stop1->location;
        geo::Coordinates location2 = stop2->location;
        route_curvature += geo::ComputeDistance(location1, location2);
    }

    route_curvature = route_length/route_curvature;
    return {bus, stop_count, static_cast<int>(unique_stops.size()), route_length, route_curvature};
}
}
