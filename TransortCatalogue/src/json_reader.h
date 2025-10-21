#pragma once

#include <algorithm>
#include <deque>
#include <iostream>
#include <iomanip>
#include <string>
#include <variant>

#include "../src/domain.h"
#include "../src/geo.h"
#include "../src/json.h"
#include "../src/map_renderer.h"
#include "../src/svg.h"
#include "../src/transport_catalogue.h"

using namespace std::string_literals;

enum class QueryType {
    NewBusRoute,
    NewStop
};

struct CommandInfo{
    QueryType command_type; // bus or stop
    std::string name; // bus nr or stop name
    std::vector<std::string> data; // further data
    geo::Coordinates stop_location; // further data
    std::vector<std::pair<std::string, int>> stop_distance_data;
    bool is_roundtrip = true;
};

class JsonReader{
public:
    void ExecuteJsonQuery(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue);

private:
    void ReadNode(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue);

    void CheckBaseRequests(const json::Node& node);
    void AddBusRouteToInputList(const json::Node& node);
    void AddStopToInputList(const json::Node& node);

    void CheckStatRequests(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue);
    void GetBusRouteJson(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue);
    void GetStopJson(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue);
    void PrintData();

    void UpdateTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue);
    void SetRenderSettings(const json::Dict& node, map::MapRender renderer);

private:
    void LoadSingleCommand(transport_catalogue::TransportCatalogue& catalogue, const CommandInfo& command)const;
    json::Dict ConvertBusRouteInfoToJson(const entities::BusRoute& route, int request_id)const;
    json::Dict ConvertStopInfoToJson(const entities::StopBusList& stop_info, int request_id)const;

private:
    std::deque<CommandInfo> input_commands_;
    json::Array output_json_;

    map::MapRender renderer_data_;
};
