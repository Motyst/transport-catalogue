#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../src/json.h"
#include "../src/json_builder.h"
#include "../src/json_reader.h"
#include "../src/svg.h"
#include "../src/transport_catalogue.h"

using namespace std::string_literals;
using namespace json;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    JsonReader json_input;

    json::Document test_node = json::Load(std::cin);
    json_input.ExecuteJsonQuery(test_node.GetRoot(), catalogue);

    return 0;
}
