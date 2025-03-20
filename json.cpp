#include "json.h"
#include <iomanip>
#include <sstream>

using namespace std;

namespace json {

    // Реализация методов Node

    bool Node::IsInt() const {
        return holds_alternative<int>(as_variant_);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(as_variant_) || holds_alternative<int>(as_variant_);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(as_variant_);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(as_variant_);
    }

    bool Node::IsString() const {
        return holds_alternative<string>(as_variant_);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(as_variant_);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(as_variant_);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(as_variant_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("Not a bool");
        }
        return get<bool>(as_variant_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("Not an int");
        }
        return get<int>(as_variant_);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw logic_error("Not a double");
        }
        if (IsInt()) {
            return static_cast<double>(get<int>(as_variant_));
        }
        return get<double>(as_variant_);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("Not a string");
        }
        return get<string>(as_variant_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("Not an array");
        }
        return get<Array>(as_variant_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw logic_error("Not a map");
        }
        return get<Dict>(as_variant_);
    }

    bool Node::operator==(const Node& right) const {
        return as_variant_ == right.as_variant_;
    }

    bool Node::operator!=(const Node& right) const {
        return as_variant_ != right.as_variant_;
    }

    const Variant& Node::GetValue() const {
        return as_variant_;
    }

    // Реализация Document

    Document::Document(Node root) : root_(std::move(root)) {}

    const Node& Document::GetRoot() const {
        return root_;
    }

    // Вспомогательные функции для парсинга

    namespace {

        Node LoadNode(istream& input);
        Node LoadNull(istream& input);
        Node LoadBool(istream& input);  // Новая функция
        Node LoadString(istream& input);
        Node LoadNumber(istream& input);
        Node LoadArray(istream& input);
        Node LoadDict(istream& input);

        Node LoadNull(istream& input) {
            string temp;
            char c = static_cast<char>(input.get());
            while (isalpha(c))
            {
                temp += c;
                c = static_cast<char>(input.get());
            }
            input.putback(c);
            if (temp != "null") {
                throw ParsingError("Expected 'null'");
            }
            return Node(nullptr);
        }

        Node LoadBool(istream& input) {
            string temp;
            char c;
            c = static_cast<char>(input.get());
            while (isalpha(c))
            {
                temp += c;
                c = static_cast<char>(input.get());
            }
            input.putback(c);

            if (temp == "true") {
                return Node(true);
            }
            else if (temp == "false") {
                return Node(false);
            }
            throw ParsingError("Expected 'true' or 'false'");
        }
        std::string LoadStr(std::istream& input) {
            using namespace std::literals;

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
                }
                else if (ch == '\\') {
                   
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
                }
                else if (ch == '\n' || ch == '\r') {
                    
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                  
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }
        Node LoadString(istream& input) {
            try {
                return Node(LoadStr(input));
            }
            catch (const ParsingError&) {
                throw ParsingError("String parsing error");
            }
        }

        Node LoadNumber(istream& input) {
            string parsed_num;
            char c;
            while (input.get(c) && (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-')) {
                parsed_num += c;
            }
            input.putback(c);

            try {
                if (parsed_num.find('.') != string::npos || parsed_num.find('e') != string::npos || parsed_num.find('E') != string::npos) {
                    return Node(stod(parsed_num));
                }
                else {
                    return Node(stoi(parsed_num));
                }
            }
            catch (...) {
                throw ParsingError("Failed to parse number");
            }
        }

        Node LoadArray(istream& input) {
            Array result;
            char c;
            input >> c;
            if (input.eof()) {
                
                throw ParsingError("Unexpected end of input: array not closed");
            }
            if (c == ']') {
                return Node(move(result));
            }
            while (true) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
                input >> c;
                if (c == ']') {
                    break;
                }
                if (c != ',') {
                    throw ParsingError("Expected ',' or ']' in array");
                }
                if (input.eof()) {
                    throw ParsingError("Unexpected end of input: array not closed");
                }
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;

            while (input >> c && c != '}') {
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input).AsString();
                input >> c; 
                result.insert({ move(key), LoadNode(input) });
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (input.eof()) {
                throw ParsingError("Unexpected end of input");
            }

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                if (input.peek() == EOF) {
                    throw ParsingError("Unexpected end of input: dictionary not closed");
                }
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

    // Реализация функций Load и Print

    Document Load(istream& input) {

        return Document(LoadNode(input));
    }

    void Print(const Document& doc, ostream& output) {
        const Node& root = doc.GetRoot();
        if (root.IsNull()) {
            output << "null";
        }
        else if (root.IsBool()) {
            output << (root.AsBool() ? "true" : "false");
        }
        else if (root.IsInt()) {
            output << root.AsInt();
        }
        else if (root.IsDouble()) {
            output << root.AsDouble();
        }
        else if (root.IsString()) {
            output << "\"";
            for (char ch : root.AsString()) {
                switch (ch) {
                case '"': output << "\\\""; break;  
                case '\\': output << "\\\\"; break;  
                case '\n': output << "\\n"; break;  
                case '\t': output << "\\t"; break;  
                case '\r': output << "\\r"; break;  
                case '\b': output << "\\b"; break;  
                case '\f': output << "\\f"; break;  
                default:
                    if (static_cast<unsigned char>(ch) < 0x20) {
                        output << "\\u" << hex << setw(4) << setfill('0') << static_cast<int>(ch);
                    }
                    else {
                        output << ch;  
                    }
                    break;
                }
            }
            output << "\"";
        }
        else if (root.IsArray()) {
            output << "[";
            const Array& array = root.AsArray();
            for (size_t i = 0; i < array.size(); ++i) {
                if (i > 0) {
                    output << ", ";
                }
                Print(Document(array[i]), output);
            }
            output << "]";
        }
        else if (root.IsMap()) {
            output << "{";
            const Dict& dict = root.AsMap();
            bool first = true;
            for (const auto& [key, value] : dict) {
                if (!first) {
                    output << ", ";
                }
                output << "\"" << key << "\": ";
                Print(Document(value), output);
                first = false;
            }
            output << "}";
        }
    }
    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() != rhs.GetRoot();
    }
}  // namespace json