#include "json_builder.h"
#include <exception>
#include <variant>
#include <utility>

using namespace std::literals;

namespace json {

    Builder::Builder()
        : root_()
        , nodes_stack_{ &root_ }
    {
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Attempt to build JSON which isn't finalized"s);
        }
        return std::move(root_);
    }

    KeyContext Builder::Key(std::string key) {
        Node::Value& host_value = GetCurrentValue();

        if (!std::holds_alternative<Dict>(host_value)) {
            throw std::logic_error("Key() outside a dict"s);
        }

        nodes_stack_.push_back(
            &std::get<Dict>(host_value)[std::move(key)]
        );
        return *this;
    }

     Builder& Builder::Value(Node::Value value) {
        AddObject(std::move(value), /* one_shot */ true);
        return *this;
    }

    StartDictContext Builder::StartDict() {
        AddObject(Dict{}, /* one_shot */ false);
        return StartDictContext(*this);
    }

    StartArrayContext Builder::StartArray() {
        AddObject(Array{}, /* one_shot */ false);
        return *this;
    }

    Builder& Builder::EndDict() {
        if (!std::holds_alternative<Dict>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside a dict"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (!std::holds_alternative<Array>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside an array"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    // Current value can be:
    // * Dict, when .Key().Value() or EndDict() is expected
    // * Array, when .Value() or EndArray() is expected
    // * nullptr (default), when first call or dict Value() is expected

    Node::Value& Builder::GetCurrentValue() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Attempt to change finalized JSON"s);
        }
        return nodes_stack_.back()->GetValue();
    }

    // Tell about this trick
    const Node::Value& Builder::GetCurrentValue() const {
        return const_cast<Builder*>(this)->GetCurrentValue();
    }

    void Builder::AssertNewObjectContext() const {
        if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
            throw std::logic_error("New object in wrong context"s);
        }
    }

    void Builder::AddObject(Node::Value value, bool one_shot) {
        Node::Value& host_value = GetCurrentValue();
        if (std::holds_alternative<Array>(host_value)) {
            // Tell about emplace_back
            Node& node
                = std::get<Array>(host_value).emplace_back(std::move(value));
            if (!one_shot) {
                nodes_stack_.push_back(&node);
            }
        }
        else {
            AssertNewObjectContext();
            host_value = std::move(value);
            if (one_shot) {
                nodes_stack_.pop_back();
            }
        }
    }

   ValueKeyContext KeyContext::Value(Node::Value value)
    {
        return builder_.Value(std::move(value));
    }

    StartDictContext KeyContext::StartDict()
    {
        return builder_.StartDict();
    }

    StartArrayContext KeyContext::StartArray()
    {
        return builder_.StartArray();

    }
    ValueArrayContext StartArrayContext::Value(Node::Value value)
    {
        return builder_.Value(std::move(value));
    }
    StartDictContext StartArrayContext::StartDict()
    {
        return builder_.StartDict();
    }
    StartArrayContext StartArrayContext::StartArray()
    {
        return builder_.StartArray();
    }
    Builder& StartArrayContext::EndArray()
    {
        return builder_.EndArray();
    }
    
    KeyContext ValueKeyContext::Key(std::string key)
    {
        return builder_.Key(std::move(key));
    }

    Builder& ValueKeyContext::EndDict()
    {
        return builder_.EndDict();
    }

    ValueArrayContext ValueArrayContext::Value(Node::Value value)
    {
        builder_.Value(std::move(value));
        return *this ;
    }

    StartDictContext ValueArrayContext::StartDict()
    {
        return builder_.StartDict();
    }

    StartArrayContext ValueArrayContext::StartArray()
    {
        return builder_.StartArray();
    }

    Builder& ValueArrayContext::EndArray()
    {
        return builder_.EndArray();
    }

}