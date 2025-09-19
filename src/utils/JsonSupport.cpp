#include "JsonSupport.h"
#include <sstream>
#include <cctype>

#ifdef JSON_LITE

// Template specializations for type conversion
template<> std::string JsonValue::get<std::string>() const {
    return isString ? stringValue : "";
}

template<> float JsonValue::get<float>() const {
    return isNumber ? static_cast<float>(numberValue) : 0.0f;
}

template<> double JsonValue::get<double>() const {
    return isNumber ? numberValue : 0.0;
}

template<> int JsonValue::get<int>() const {
    return isNumber ? static_cast<int>(numberValue) : 0;
}

template<> bool JsonValue::get<bool>() const {
    return isBool ? boolValue : false;
}

std::string JsonValue::dump(int indent) const {
    if (isString) {
        return "\"" + stringValue + "\"";
    } else if (isNumber) {
        return std::to_string(numberValue);
    } else if (isBool) {
        return boolValue ? "true" : "false";
    } else if (isArrayType) {
        std::string result = "[";
        for (size_t i = 0; i < arrayData.size(); ++i) {
            if (i > 0) result += ",";
            result += arrayData[i].dump(indent);
        }
        result += "]";
        return result;
    } else {
        std::string result = "{";
        bool first = true;
        for (const auto& pair : objectData) {
            if (!first) result += ",";
            result += "\"" + pair.first + "\":" + pair.second.dump(indent);
            first = false;
        }
        result += "}";
        return result;
    }
}

JsonValue JsonValue::parse(const std::string& jsonStr) {
    JsonValue result;
    size_t pos = 0;
    result.parseValue(jsonStr, pos);
    return result;
}

void JsonValue::parseValue(const std::string& str, size_t& pos) {
    skipWhitespace(str, pos);

    if (pos >= str.length()) return;

    char c = str[pos];

    if (c == '{') {
        // Parse object
        ++pos;
        skipWhitespace(str, pos);

        while (pos < str.length() && str[pos] != '}') {
            skipWhitespace(str, pos);
            if (str[pos] == '}') break;

            // Parse key
            std::string key = parseString(str, pos);
            skipWhitespace(str, pos);

            if (pos < str.length() && str[pos] == ':') {
                ++pos;
                skipWhitespace(str, pos);

                // Parse value
                JsonValue value;
                value.parseValue(str, pos);
                objectData[key] = value;

                skipWhitespace(str, pos);
                if (pos < str.length() && str[pos] == ',') {
                    ++pos;
                }
            }
        }
        if (pos < str.length()) ++pos; // skip '}'

    } else if (c == '[') {
        // Parse array
        isArrayType = true;
        ++pos;
        skipWhitespace(str, pos);

        while (pos < str.length() && str[pos] != ']') {
            skipWhitespace(str, pos);
            if (str[pos] == ']') break;

            JsonValue value;
            value.parseValue(str, pos);
            arrayData.push_back(value);

            skipWhitespace(str, pos);
            if (pos < str.length() && str[pos] == ',') {
                ++pos;
            }
        }
        if (pos < str.length()) ++pos; // skip ']'

    } else if (c == '"') {
        // Parse string
        stringValue = parseString(str, pos);
        isString = true;

    } else if (c == 't' || c == 'f') {
        // Parse boolean
        if (str.substr(pos, 4) == "true") {
            boolValue = true;
            pos += 4;
        } else if (str.substr(pos, 5) == "false") {
            boolValue = false;
            pos += 5;
        }
        isBool = true;

    } else if (std::isdigit(c) || c == '-') {
        // Parse number
        std::string numStr;
        if (c == '-') {
            numStr += c;
            ++pos;
        }
        while (pos < str.length() && (std::isdigit(str[pos]) || str[pos] == '.')) {
            numStr += str[pos];
            ++pos;
        }
        numberValue = std::stod(numStr);
        isNumber = true;
    }
}

void JsonValue::skipWhitespace(const std::string& str, size_t& pos) {
    while (pos < str.length() && std::isspace(str[pos])) {
        ++pos;
    }
}

std::string JsonValue::parseString(const std::string& str, size_t& pos) {
    std::string result;
    if (pos < str.length() && str[pos] == '"') {
        ++pos;
        while (pos < str.length() && str[pos] != '"') {
            result += str[pos];
            ++pos;
        }
        if (pos < str.length()) ++pos; // skip closing '"'
    }
    return result;
}

#endif
