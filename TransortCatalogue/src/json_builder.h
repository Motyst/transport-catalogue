#pragma once

#include <iostream>
#include <map>
#include <variant>

#include "../src/json.h"

namespace json{
using namespace json;

enum class JsonBuilderOperations{
    CONSTRUCTOR,
    KEY,
    VALUE,
    START_ARRAY,
    END_ARRAY,
    START_DICT,
    END_DICT
};

class Builder;
class ArrayItemContext;
class DictItemContext;
class DictValueContext;

class Builder{
public:
    Builder& Value(Node::Value value);
    DictValueContext Key(std::string key);

    DictItemContext StartDict();
    Builder& EndDict();

    ArrayItemContext StartArray();
    Builder& EndArray();

    json::Node Build();

private:
    template <typename Container>
    void StartChild(Container* container, Node::Value& value){
        if(container->IsArray()){
            Array* array_node = const_cast<Array*>(&container->AsArray());
            array_node->push_back(std::move(Node(value)));
        }else if(container->IsDict()){
            Dict* dict_node = const_cast<Dict*>(&container->AsDict());
            dict_node->emplace(keys_.back(), Node(value));
            keys_.pop_back();
        }
    }

    void CheckFunctionSequenceCall();

private:
    json::Node root_;
    std::vector<Node*> nodes_stack_;

    std::vector<std::string> keys_;
    JsonBuilderOperations last_operation_ = JsonBuilderOperations::CONSTRUCTOR;
};

class ArrayItemContext : public Builder {
public:
    ArrayItemContext(Builder base) : Builder(base) {}

    ArrayItemContext Value(Node::Value value) {
        return Builder::Value(std::move(value));
    }

    Node Build() = delete;
    DictValueContext Key(std::string key) = delete;
    Builder EndDict() = delete;
};

class DictItemContext : public Builder {
public:
    DictItemContext(Builder base) : Builder(base) {}

    Builder& Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    json::Node Build() = delete;
};

class DictValueContext : public Builder {
public:
    DictValueContext(Builder base) : Builder(base) {}

    DictItemContext Value(Node::Value value){
        return Builder::Value(value);
    }

    DictValueContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
    json::Node Build() = delete;
};

}
