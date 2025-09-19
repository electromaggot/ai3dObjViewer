#pragma once

#define JSON_LITE

#include <string>
#include <vector>
#include <unordered_map>
#include <initializer_list>

#ifdef JSON_LITE	// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
/**
 * Enhanced JSON wrapper that provides basic JSON parsing for scene loading.
 * This allows the scene system to work without requiring nlohmann/json dependency.
 */
class JsonValue {
public:
    JsonValue() = default;
    JsonValue(const std::string& str) : stringValue(str), isString(true) {}
    JsonValue(double num) : numberValue(num), isNumber(true) {}
    JsonValue(int num) : numberValue(static_cast<double>(num)), isNumber(true) {}
    JsonValue(bool b) : boolValue(b), isBool(true) {}

    // Initializer list constructors for nlohmann::json compatibility:
    JsonValue(std::initializer_list<std::pair<std::string, JsonValue>> init) {
        for (const auto& pair : init) {
            objectData[pair.first] = pair.second;
        }
    }

    JsonValue(std::initializer_list<JsonValue> init) {
        isArrayType = true;
        for (const auto& value : init) {
            arrayData.push_back(value);
        }
    }

    // Basic type checking
    bool contains(const std::string& key) const { return objectData.find(key) != objectData.end(); }
    bool is_array() const { return isArrayType; }
    size_t size() const { return isArrayType ? arrayData.size() : objectData.size(); }

    // Assignment operators
    JsonValue& operator=(const std::string& str) {
        clear(); stringValue = str; isString = true; return *this;
    }
    JsonValue& operator=(double num) {
        clear(); numberValue = num; isNumber = true; return *this;
    }
    JsonValue& operator=(int num) {
        clear(); numberValue = static_cast<double>(num); isNumber = true; return *this;
    }
    JsonValue& operator=(bool b) {
        clear(); boolValue = b; isBool = true; return *this;
    }
    JsonValue& operator=(const char* str) {
        clear(); stringValue = str; isString = true; return *this;
    }

    // Initializer list assignment operators:
    JsonValue& operator=(std::initializer_list<std::pair<std::string, JsonValue>> init) {
        clear();
        for (const auto& pair : init) {
            objectData[pair.first] = pair.second;
        }
        return *this;
    }

    JsonValue& operator=(std::initializer_list<JsonValue> init) {
        clear();
        isArrayType = true;
        for (const auto& value : init) {
            arrayData.push_back(value);
        }
        return *this;
    }

    // Access operators
    JsonValue& operator[](const std::string& key) {
        if (!isArrayType) objectData[key] = JsonValue();
        return objectData[key];
    }
    const JsonValue& operator[](const std::string& key) const {
        static JsonValue empty;
        auto it = objectData.find(key);
        return it != objectData.end() ? it->second : empty;
    }
    JsonValue& operator[](const char* key) { return (*this)[std::string(key)]; }
    const JsonValue& operator[](const char* key) const { return (*this)[std::string(key)]; }
    JsonValue& operator[](size_t index) {
        if (isArrayType && index < arrayData.size()) return arrayData[index];
        static JsonValue empty; return empty;
    }
    const JsonValue& operator[](size_t index) const {
        if (isArrayType && index < arrayData.size()) return arrayData[index];
        static JsonValue empty; return empty;
    }

    // Type conversion methods for nlohmann::json compatibility:
    template<typename T> T get() const;

    // Conversion operators
    operator std::string() const { return isString ? stringValue : ""; }

    // Utility methods
    std::string dump(int indent = 0) const;
    static JsonValue array() { JsonValue j; j.isArrayType = true; return j; }
    static JsonValue object(std::initializer_list<std::pair<std::string, JsonValue>> init) {
        JsonValue j;
        for (const auto& pair : init) {
            j.objectData[pair.first] = pair.second;
        }
        return j;
    }
    void push_back(const JsonValue& value) { if (isArrayType) arrayData.push_back(value); }

    // Basic JSON parsing (simplified)
    static JsonValue parse(const std::string& jsonStr);

    // Public members for compatibility
    std::string stringValue;

private:
    double numberValue = 0.0;
    bool boolValue = false;
    bool isString = false;
    bool isNumber = false;
    bool isBool = false;
    bool isArrayType = false;

    std::unordered_map<std::string, JsonValue> objectData;
    std::vector<JsonValue> arrayData;

    void parseValue(const std::string& str, size_t& pos);
    void skipWhitespace(const std::string& str, size_t& pos);
    std::string parseString(const std::string& str, size_t& pos);

    void clear() {
        stringValue.clear();
        numberValue = 0.0;
        boolValue = false;
        isString = false;
        isNumber = false;
        isBool = false;
        isArrayType = false;
        objectData.clear();
        arrayData.clear();
    }
};

// Type alias to match nlohmann::json interface:
using json = JsonValue;

#else	// NON-"LITE" JSON SUPPORT = = = = = = = = = = = = = = = = = = = = = = = =

// Real nlohmann/json implementation.
#include <nlohmann/json.hpp>

// Just use nlohmann::json directly for now.
using JsonValue = nlohmann::json;

// Type alias to match "lite" interface.
using json = JsonValue;

#endif	// EOF = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
