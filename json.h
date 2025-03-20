
#pragma once

#include <variant>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <concepts>
#include <iomanip>
#include <sstream>

namespace json {

    class Node;

    // Псевдонимы для удобства
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Variant = std::variant<std::nullptr_t, Array, Dict, int, std::string, double, bool>;

    // Ошибка парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    // Концепт для проверки допустимых типов
    template<typename T>
    concept is_valid_type = std::is_same_v<T, int>
        || std::is_same_v<T, double>
        || std::is_same_v<T, std::string>
        || std::is_same_v<T, bool>
        || std::is_same_v<T, Array>
        || std::is_same_v<T, Dict>
        || std::is_same_v<T, std::nullptr_t>;

    // Класс Node для представления узла JSON
    class Node {
    public:
        Node() : as_variant_(nullptr) {}
        Node(Variant node) : as_variant_(std::move(node)) {}
        template<is_valid_type T>
        Node(T value) : as_variant_(std::move(value)) {}

        // Методы для проверки типа
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        // Методы для получения значения
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        
        // Операторы сравнения
        bool operator==(const Node& right) const;
        bool operator!=(const Node& right) const;

        // Получение значения
        const Variant& GetValue() const;

    private:
        Variant as_variant_;
    };

    // Класс Document для представления JSON-документа
    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    // Функции для работы с JSON
    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& output);
    bool operator==(const Document& lhs, const Document& rhs);

    bool operator!=(const Document& lhs, const Document& rhs);
    

}  // namespace json