#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {
    class StartDictCond;
    class KeyCond;
    class StartArrayCond;
    class ValueKeyCond;
    class ValueArrayCond;

    class Builder {
    public:
        Builder();
        Node Build();
        KeyCond Key(std::string key);
        Builder& Value(Node::Value value);
        StartDictCond StartDict();
        StartArrayCond StartArray();
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

    class KeyCond {
        Builder& builder_;
    public:
        KeyCond(Builder& builder) :builder_(builder) {};
        ValueKeyCond Value(Node::Value value); 
        StartDictCond StartDict();
        StartArrayCond StartArray();
        Node Build() = delete;
        KeyCond& Key(std::string key) = delete;
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;
    };
    class StartDictCond  {
        Builder& builder_;
    public:
        StartDictCond(Builder& builder) :builder_(builder) {};

        KeyCond Key(std::string key) { return builder_.Key(std::move(key)); }
        Builder& EndDict() { return builder_.EndDict(); }

        Node Build()=delete;
        Builder& Value(Node::Value value)=delete;
        Builder& StartDict()=delete;
        Builder StartArray()=delete;
        Builder& EndArray()=delete;
    };

    class StartArrayCond {
        Builder& builder_;
    public:
        StartArrayCond(Builder& builder) :builder_(builder) {};

        Node Build()=delete;
        KeyCond Key(std::string key)=delete;
        ValueArrayCond Value(Node::Value value);
        StartDictCond StartDict();
        StartArrayCond StartArray();
        Builder& EndDict()=delete;
        Builder& EndArray();


    };

    class ValueKeyCond {
        Builder& builder_;
    public:
        ValueKeyCond(Builder& builder) :builder_(builder) {};
        Node Build()=delete;
        KeyCond Key(std::string key);
        Builder& Value(Node::Value value)=delete;
        StartDictCond StartDict()=delete;
        StartArrayCond StartArray()=delete;
        Builder& EndDict();
        Builder& EndArray()=delete;
    };

    class ValueArrayCond {
        Builder& builder_;
    public:
        ValueArrayCond(Builder& builder) :builder_(builder) {};
        Node Build()=delete;
        KeyCond Key(std::string key)=delete;
        ValueArrayCond Value(Node::Value value);
        StartDictCond StartDict();
        StartArrayCond StartArray();
        Builder& EndDict()=delete;
        Builder& EndArray();
    };

}  // namespace 