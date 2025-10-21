#include "../src/json_reader.h"

void JsonReader::ExecuteJsonQuery(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue){
    ReadNode(node, catalogue);
}

void JsonReader::ReadNode(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue) {
    for(const auto& request : node.AsDict()){
        if(request.first == "base_requests"){
            CheckBaseRequests(request.second);
            UpdateTransportCatalogue(catalogue);
        }
        else if(request.first == "render_settings"){
            renderer_data_.SetRenderSettings(request.second.AsDict());
        }else if(request.first == "stat_requests"){
            CheckStatRequests(request.second, catalogue);
            PrintData();
        }
    }
}

// ---------------- STAT REQUESTS --------------------------
void JsonReader::CheckStatRequests(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue){
    output_json_.reserve(node.AsArray().size());

    for(const auto& request : node.AsArray()){
        if(request.AsDict().at("type").AsString() == "Bus"){
            GetBusRouteJson(request, catalogue);
        }else if(request.AsDict().at("type").AsString() == "Stop"){
            GetStopJson(request, catalogue);
        }
        else if(request.AsDict().at("type").AsString() == "Map"){
            std::vector<entities::BusRouteRenderInfo> bus_routes = catalogue.GetRenderData();
            renderer_data_.AddRenderData(bus_routes);

            // Put all svg render data into Json
            std::stringstream map_string;
            renderer_data_.RenderObjects(map_string);

            output_json_.push_back(json::Dict{
                {"map", map_string.str()},
                {"request_id", request.AsDict().at("id").AsInt()}
            });
        }
    }
}

void JsonReader::GetBusRouteJson(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue) {
    std::string bus_name = node.AsDict().at("name").AsString();

    entities::BusRoute route = catalogue.RouteInformation(bus_name);
    output_json_.push_back(ConvertBusRouteInfoToJson(route, node.AsDict().at("id").AsInt()));
}

json::Dict JsonReader::ConvertBusRouteInfoToJson(const entities::BusRoute& route, int request_id)const{
    json::Dict node;
    if(!route.stop_count){
        return json::Dict{
            {"request_id", request_id},
            {"error_message", "not found"}
        };
    }else{
        return json::Dict{
            {"curvature", route.route_curvature},
            {"request_id", request_id},
            {"route_length", route.route_lenght * 1.0},
            {"stop_count", route.stop_count},
            {"unique_stop_count", route.unique_stops}
        };
    }
    return node;
}

void JsonReader::GetStopJson(const json::Node& node, transport_catalogue::TransportCatalogue& catalogue) {
    std::string stop_name = node.AsDict().at("name").AsString();
    output_json_.push_back(ConvertStopInfoToJson(catalogue.StopInformation(stop_name), node.AsDict().at("id").AsInt()));
}

json::Dict JsonReader::ConvertStopInfoToJson(const entities::StopBusList& stop_info, int request_id)const{
    if(!stop_info.buses_exist){
        return json::Dict{
            {"request_id", request_id},
            {"error_message", "not found"}
        };
    }else{
        json::Array bus_list;
        bus_list.reserve(stop_info.bus_list.size());

        for(const auto& name : stop_info.bus_list){
            bus_list.push_back(json::Node{std::string(name)});
        }

        return json::Dict{
            {"buses", bus_list},
            {"request_id", request_id}
        };
    }
}

void JsonReader::PrintData(){
    json::Print(json::Document{output_json_}, std::cout);
    output_json_.clear(); // Maybe resize to 0
}

// ---------------- BASE REQUESTS --------------------------

void JsonReader::CheckBaseRequests(const json::Node& node){
    for(const auto& request : node.AsArray()){
        if(request.AsDict().at("type").AsString() == "Stop"){
            AddStopToInputList(request);
        }else
        if(request.AsDict().at("type").AsString() == "Bus"){
            AddBusRouteToInputList(request);
        }
    }
}

void JsonReader::AddBusRouteToInputList(const json::Node& node){
    CommandInfo new_command;
    new_command.command_type = QueryType::NewBusRoute;
    new_command.name = node.AsDict().at("name").AsString();

    new_command.data.reserve(node.AsDict().at("stops").AsArray().size() * 2);
    for(const auto& stop : node.AsDict().at("stops").AsArray()){
        new_command.data.push_back(stop.AsString());
    }

    if(!node.AsDict().at("is_roundtrip").AsBool()){
       new_command.data.insert(new_command.data.end(), new_command.data.rbegin() + 1, new_command.data.rend());
       new_command.is_roundtrip = false;
    }
    input_commands_.push_back(new_command);
}


void JsonReader::AddStopToInputList(const json::Node& node){
    CommandInfo new_command;

    new_command.command_type = QueryType::NewStop;
    new_command.name = node.AsDict().at("name").AsString();
    new_command.stop_location.lat = node.AsDict().at("latitude").AsDouble();
    new_command.stop_location.lng = node.AsDict().at("longitude").AsDouble();

    for(const auto& [stop, distance] : node.AsDict().at("road_distances").AsDict()){
        new_command.stop_distance_data.push_back(std::make_pair(stop, distance.AsInt()));
    }
    input_commands_.push_back(new_command);
}

void JsonReader::UpdateTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue){
    std::partition(input_commands_.begin(), input_commands_.end(),
                   [](auto& command){return command.command_type == QueryType::NewStop;});

    for(const auto& command : input_commands_){
        JsonReader::LoadSingleCommand(catalogue, command);
    }
    input_commands_.clear();
}

void JsonReader::LoadSingleCommand(transport_catalogue::TransportCatalogue& catalogue, const CommandInfo& command)const{
    switch(command.command_type){
        case QueryType::NewBusRoute:
            catalogue.AddBus(std::move(command.name), std::move(command.data), command.is_roundtrip);
            break;
        case QueryType::NewStop:
            catalogue.AddStop(command.name, command.stop_location);
            if(!command.stop_distance_data.empty()){
                catalogue.SetDistanceBetweenStops(command.name, std::move(command.stop_distance_data));
            }
            break;
    }
}
