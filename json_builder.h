#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {
    class StartDictContext;
    class KeyContext;
    class StartArrayContext;
    class ValueKeyContext;
    class ValueArrayContext;

    class Builder {
    public:
        Builder();
        Node Build();
        KeyContext Key(std::string key);
        Builder& Value(Node::Value value);
        StartDictContext StartDict();
        StartArrayContext StartArray();
        Builder& EndDict();
        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node::Value& GetCurrentValue();
        const Node::Value& GetCurrentValue() const;

        void AssertNewObjectContext() const;
        void AddObject(Node::Value value, bool one_shot);
    };

    class KeyContext {
        Builder& builder_;
    public:
        KeyContext(Builder& builder) :builder_(builder) {};
        ValueKeyContext Value(Node::Value value); 
        StartDictContext StartDict();
        StartArrayContext StartArray();
        Node Build() = delete;
        KeyContext& Key(std::string key) = delete;
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;
    };
    class StartDictContext  {
        Builder& builder_;
    public:
        StartDictContext(Builder& builder) :builder_(builder) {};

        KeyContext Key(std::string key) { return builder_.Key(std::move(key)); }
        Builder& EndDict() { return builder_.EndDict(); }

        Node Build()=delete;
        Builder& Value(Node::Value value)=delete;
        Builder& StartDict()=delete;
        Builder StartArray()=delete;
        Builder& EndArray()=delete;
    };

    class StartArrayContext {
        Builder& builder_;
    public:
        StartArrayContext(Builder& builder) :builder_(builder) {};

        Node Build()=delete;
        KeyContext Key(std::string key)=delete;
        ValueArrayContext Value(Node::Value value);
        StartDictContext StartDict();
        StartArrayContext StartArray();
        Builder& EndDict()=delete;
        Builder& EndArray();


    };

    class ValueKeyContext {
        Builder& builder_;
    public:
        ValueKeyContext(Builder& builder) :builder_(builder) {};
        Node Build()=delete;
        KeyContext Key(std::string key);
        Builder& Value(Node::Value value)=delete;
        StartDictContext StartDict()=delete;
        StartArrayContext StartArray()=delete;
        Builder& EndDict();
        Builder& EndArray()=delete;
    };

    class ValueArrayContext {
        Builder& builder_;
    public:
        ValueArrayContext(Builder& builder) :builder_(builder) {};
        Node Build()=delete;
        KeyContext Key(std::string key)=delete;
        ValueArrayContext Value(Node::Value value);
        StartDictContext StartDict();
        StartArrayContext StartArray();
        Builder& EndDict()=delete;
        Builder& EndArray();
    };

}  // namespace 