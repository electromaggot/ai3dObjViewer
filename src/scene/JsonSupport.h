#pragma once

#define STUBBED

#include <string>
#include <vector>
#include <unordered_map>

#ifdef STUBBED
/**
 * Simple JSON wrapper to isolate dependency on nlohmann/json.
 * This allows the scene system to have serialization interfaces
 * without requiring the actual JSON library to be installed.
 * Can be easily replaced with real implementation later.
 */
class JsonValue {
public:
    JsonValue() = default;
    JsonValue(const std::string& str) : stringValue(str), isString(true) {}
    JsonValue(double num) : numberValue(num), isNumber(true) {}
    JsonValue(bool b) : boolValue(b), isBool(true) {}

    // Basic type checking (stub implementation)
    bool contains(const std::string& key) const { return false; }
    bool is_array() const { return false; }
    size_t size() const { return 0; }

    // Assignment operators (stub implementation)
    JsonValue& operator=(const std::string& str) { stringValue = str; isString = true; return *this; }
    JsonValue& operator=(double num) { numberValue = num; isNumber = true; return *this; }
    JsonValue& operator=(bool b) { boolValue = b; isBool = true; return *this; }

    // Access operators (simplified to avoid ambiguity)
    JsonValue& operator[](const std::string&) { return *this; }
    const JsonValue& operator[](const std::string&) const { return *this; }
    JsonValue& operator[](const char*) { return *this; }
    const JsonValue& operator[](const char*) const { return *this; }

    // Assignment from const char* to avoid ambiguity
    JsonValue& operator=(const char* str) { stringValue = str; isString = true; return *this; }

    // Conversion operators (stub implementation)
    operator std::string() const { return isString ? stringValue : ""; }

    // Utility methods
    std::string dump(int indent = 0) const { return "{}"; }  // Stub
    static JsonValue array() { return JsonValue(); }  // Stub
    void push_back(const JsonValue& value) {}  // Stub

    // Public for stream operations (temporary)
    std::string stringValue;

private:
    double numberValue = 0.0;
    bool boolValue = false;
    bool isString = false;
    bool isNumber = false;
    bool isBool = false;
};

// Type alias to match nlohmann::json interface
using json = JsonValue;

#else
// Real nlohmann/json implementation
#include <nlohmann/json.hpp>

/**
 * Wrapper around nlohmann::json to provide the same interface as the stub.
 * This maintains API compatibility while using the real JSON library.
 */
class JsonValue {
private:
    nlohmann::json jsonData;

public:
    JsonValue() = default;
    JsonValue(const std::string& str) : jsonData(str) {}
    JsonValue(double num) : jsonData(num) {}
    JsonValue(bool b) : jsonData(b) {}
    JsonValue(const nlohmann::json& j) : jsonData(j) {}

    // Basic type checking
    bool contains(const std::string& key) const {
        return jsonData.is_object() && jsonData.contains(key);
    }
    bool is_array() const { return jsonData.is_array(); }
    size_t size() const { return jsonData.size(); }

    // Assignment operators
    JsonValue& operator=(const std::string& str) { jsonData = str; return *this; }
    JsonValue& operator=(double num) { jsonData = num; return *this; }
    JsonValue& operator=(bool b) { jsonData = b; return *this; }
    JsonValue& operator=(const char* str) { jsonData = str; return *this; }
    JsonValue& operator=(const nlohmann::json& j) { jsonData = j; return *this; }

    // Access operators
    JsonValue operator[](const std::string& key) const {
        if (jsonData.is_object() && jsonData.contains(key)) {
            return JsonValue(jsonData[key]);
        }
        return JsonValue();
    }
    JsonValue& operator[](const std::string& key) {
        return *reinterpret_cast<JsonValue*>(&jsonData[key]);
    }
    JsonValue operator[](const char* key) const {
        return (*this)[std::string(key)];
    }
    JsonValue& operator[](const char* key) {
        return (*this)[std::string(key)];
    }

    // Conversion operators
    operator std::string() const {
        if (jsonData.is_string()) {
            return jsonData.get<std::string>();
        }
        return jsonData.dump();
    }

    // Utility methods
    std::string dump(int indent = 0) const { return jsonData.dump(indent); }
    static JsonValue array() { return JsonValue(nlohmann::json::array()); }
    void push_back(const JsonValue& value) {
        if (!jsonData.is_array()) {
            jsonData = nlohmann::json::array();
        }
        jsonData.push_back(value.jsonData);
    }

    // Access to underlying nlohmann::json for advanced operations
    const nlohmann::json& get_json() const { return jsonData; }
    nlohmann::json& get_json() { return jsonData; }

    // Public for stream operations compatibility
    std::string stringValue() const {
        if (jsonData.is_string()) {
            return jsonData.get<std::string>();
        }
        return "";
    }
};

// Type alias to match stub interface
using json = JsonValue;

#endif