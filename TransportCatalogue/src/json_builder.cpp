#include "../src/json_builder.h"

namespace json{
using namespace json;

Builder& Builder::Value(Node::Value value){
    CheckFunctionSequenceCall();
    last_operation_ = JsonBuilderOperations::VALUE;

    if(std::holds_alternative<Array>(value) || std::holds_alternative<Dict>(value)){
        nodes_stack_.push_back(new Node(value));
    }else{
        if(!nodes_stack_.empty() && (nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsDict())){
            StartChild(nodes_stack_.back(), value);
        }else{
            nodes_stack_.push_back(new Node(value));
        }
    }
    return *this;
}

DictValueContext Builder::Key(std::string key){
    if(!nodes_stack_.back()->IsDict() || last_operation_ == JsonBuilderOperations::KEY){
        throw std::logic_error("Key function called outside Dictionary or after another Key()!");
    }

    last_operation_ = JsonBuilderOperations::KEY;
    keys_.push_back(std::move(key));

    return DictValueContext(*this);
}

ArrayItemContext Builder::StartArray(){
    CheckFunctionSequenceCall();
    last_operation_ = JsonBuilderOperations::START_ARRAY;

    nodes_stack_.push_back(new Node(Array{}));
    return ArrayItemContext(*this);
}

Builder& Builder::EndArray(){
    if(!nodes_stack_.back()->IsArray()){
        throw std::logic_error("Wrong container ending! Must be Dict!");
    }
    last_operation_ = JsonBuilderOperations::END_ARRAY;

    Node* array_node = nodes_stack_.back();
    nodes_stack_.pop_back();

    // If there is a container before this one
    if(!nodes_stack_.empty() && nodes_stack_.back()->IsArray()){
        Array* last_node = const_cast<Array*>(&nodes_stack_.back()->AsArray());
        last_node->push_back(std::move(*array_node));
    }else if(!nodes_stack_.empty() && nodes_stack_.back()->IsDict()){
        Dict* last_node = const_cast<Dict*>(&nodes_stack_.back()->AsDict());
        last_node->emplace(keys_.back(), *array_node);
        keys_.pop_back();
    }else{
        root_ = std::move(*array_node);
    }
    return *this;
}

DictItemContext Builder::StartDict(){
    CheckFunctionSequenceCall();
    last_operation_ = JsonBuilderOperations::START_DICT;

    nodes_stack_.push_back(new Node(Dict{}));
    return DictItemContext(*this);
}

Builder& Builder::EndDict(){
    if(!nodes_stack_.back()->IsDict()){
        throw std::logic_error("Wrong container ending! Must be Array!");
    }
    last_operation_ = JsonBuilderOperations::END_DICT;

    Node* dict_node = nodes_stack_.back();
    nodes_stack_.pop_back();

    if(!nodes_stack_.empty() && nodes_stack_.back()->IsArray()){
        Array* last_node = const_cast<Array*>(&nodes_stack_.back()->AsArray());
        last_node->push_back(std::move(*dict_node));
    }else if(!nodes_stack_.empty() && nodes_stack_.back()->IsDict()){
        Dict* last_node = const_cast<Dict*>(&nodes_stack_.back()->AsDict());
        last_node->emplace(keys_.back(), *dict_node);
        keys_.pop_back();
    }else{
        root_ = std::move(*dict_node);
    }
    return *this;
}

json::Node Builder::Build(){
    if(nodes_stack_.size() == 1){
        root_ = std::move(*nodes_stack_.back());
        nodes_stack_.pop_back();
    }
    if(!nodes_stack_.empty() || last_operation_ == JsonBuilderOperations::CONSTRUCTOR){
        throw std::logic_error("Object not complete!");
    }
    return root_;
}

void Builder::CheckFunctionSequenceCall(){
     if(last_operation_ == JsonBuilderOperations::START_DICT){
         throw std::logic_error("Function must be called after constructor, Key() or container element!");
     }
     if((last_operation_ == JsonBuilderOperations::END_DICT ||
             last_operation_ == JsonBuilderOperations::END_ARRAY) && nodes_stack_.empty()){
         throw std::logic_error("Value node is being created outside the container!");
     }
     if(!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && last_operation_ != JsonBuilderOperations::KEY){
          throw std::logic_error("Key must be set 1st before value in dictionary!");
     }
}

}
