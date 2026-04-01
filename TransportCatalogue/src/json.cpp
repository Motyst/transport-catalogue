#include "../src/json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadNull(istream& input) {
    std::string line;
    std::getline(input, line, ',');

    if (line == "null" || line == "nullptr") {
        return Node();
    }
    else {
        throw ParsingError("Incorrect null input"s);
    }
}

Node LoadArray(istream& input) {
    Array result;
    char c;

    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']') {
        throw ParsingError("Incorrect array input"s);
    }

    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }

    if (input.peek() == '0') {
        read_char();

    } else {
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line");
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}

Node LoadBool(istream& input) {
    std::string line;
    char c;

    for (; input >> c && c != ',';) {
        if(c == '}' || c == ']'){
            input.putback(c);
            break;
        }
        line.push_back(c);
    }

    if (line == "true") {
        return Node(true);
    }
    else if (line == "false") {
        return Node(false);
    }else{
        throw ParsingError("Incorrect bool input"s);
    }
}

Node LoadDict(istream& input) {
    Dict result;
    char c;

    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if(c != '}'){
        throw ParsingError("Incorrect dictionary input"s);
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (c == '"') {
        return LoadString(input);
    }
    else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    }
    else {
        input.putback(c);
        return LoadNumber(input);
    }
}
}  // namespace

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

// ----------------- Print -------------------------

void PrintString(const Node& node, std::ostream& output){
    output << '"';
    for(char c : node.AsString()){
        if(c == '"'){
            output << "\\";
        }
        output << c;
    }
    output << '"';
}

void PrintArray(const Node& root, std::ostream& output){
    const auto& array_node = root.AsArray();

    bool check_first = true;
    output << "[\n  ";
    for (const auto& node : array_node) {
        if (!check_first) {
            output << ", ";
        }
        Print(Document(node), output);
        check_first = false;
    }
    output << "\n]";
}

void PrintMap(const Node& root, std::ostream& output){
    const auto& map_node = root.AsDict();
    output << "{\n";

    for(const auto& node : map_node){
        output << "  \""<< node.first << "\": ";
        Print(Document(node.second), output);

        if(node == *(--map_node.end())){
            break;
        }
        output << ",\n";
    }

    output << "\n}";
}

void Print(const Document& doc, std::ostream& output) {
    using namespace std::literals;

    const auto& root = doc.GetRoot();

    if (root.IsNull()) {
        output << "null"s;
        return;
    }

    if (root.IsInt()) {
        output << root.AsInt();
        return;
    }

    if (root.IsPureDouble()) {
        output << std::setprecision(6) << root.AsDouble();
        return;
    }

    if (root.IsString()) {
        PrintString(root, output);
        return;
    }

    if (root.IsBool()) {
        output << std::boolalpha << root.AsBool() << std::noboolalpha;
        return;
    }

    if (root.IsArray()) {
        PrintArray(root, output);
        return;
    }

    if (root.IsDict()) {
        PrintMap(root, output);
        return;
    }

}
}  // namespace json
