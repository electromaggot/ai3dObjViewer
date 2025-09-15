#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

ObjLoader::ObjLoader() {
}

ObjLoader::~ObjLoader() {
}

std::shared_ptr<Mesh> ObjLoader::load(const std::string& filename) {
    std::string content = loadFile(filename);
    ObjData objData = parseObj(content);
    
    auto mesh = std::make_shared<Mesh>();
    
    // Create vertices from parsed data
    std::vector<Vertex> vertices;
    vertices.reserve(objData.positions.size());
    
    for (size_t i = 0; i < objData.positions.size(); ++i) {
        Vertex vertex;
        vertex.position = objData.positions[i];
        
        // Use normal if available, otherwise use generated normal
        if (i < objData.normals.size()) {
            vertex.normal = objData.normals[i];
        } else {
            vertex.normal = Vector3(0.0f, 1.0f, 0.0f); // Default normal
        }
        
        // Generate a simple color based on position
        vertex.color = Vector3(
            (vertex.position.x + 1.0f) * 0.5f,
            (vertex.position.y + 1.0f) * 0.5f,
            (vertex.position.z + 1.0f) * 0.5f
        );
        
        vertices.push_back(vertex);
    }
    
    mesh->setVertices(vertices);
    mesh->setIndices(objData.indices);
    
    return mesh;
}

ObjLoader::ObjData ObjLoader::parseObj(const std::string& content) {
    ObjData data;
    std::istringstream stream(content);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;
        
        if (token == "v") {
            // Vertex position
            data.positions.push_back(parseVector3(line));
        }
        else if (token == "vn") {
            // Vertex normal
            data.normals.push_back(parseVector3(line));
        }
        else if (token == "f") {
            // Face
            parseFace(line, data);
        }
    }
    
    // Generate normals if none were provided
    if (data.normals.empty() && !data.positions.empty()) {
        generateNormals(data);
    }
    
    return data;
}

Vector3 ObjLoader::parseVector3(const std::string& line) {
    std::istringstream stream(line);
    std::string token;
    float x, y, z;
    
    stream >> token >> x >> y >> z;
    return Vector3(x, y, z);
}

void ObjLoader::parseFace(const std::string& line, ObjData& data) {
    std::istringstream stream(line);
    std::string token;
    stream >> token; // Skip "f"
    
    std::vector<int> vertexIndices;
    
    while (stream >> token) {
        // Handle different face formats: v, v/vt, v/vt/vn, v//vn
        size_t slash1 = token.find('/');
        if (slash1 != std::string::npos) {
            // Extract just the vertex index (first number)
            token = token.substr(0, slash1);
        }
        
        int index = std::stoi(token) - 1; // OBJ indices are 1-based
        vertexIndices.push_back(index);
    }
    
    // Triangulate face (assuming it's a polygon)
    for (size_t i = 1; i < vertexIndices.size() - 1; ++i) {
        data.indices.push_back(vertexIndices[0]);
        data.indices.push_back(vertexIndices[i]);
        data.indices.push_back(vertexIndices[i + 1]);
    }
}

void ObjLoader::generateNormals(ObjData& data) {
    data.normals.resize(data.positions.size(), Vector3::zero());
    
    // Calculate face normals and accumulate at vertices
    for (size_t i = 0; i < data.indices.size(); i += 3) {
        uint32_t i0 = data.indices[i];
        uint32_t i1 = data.indices[i + 1];
        uint32_t i2 = data.indices[i + 2];
        
        if (i0 < data.positions.size() && i1 < data.positions.size() && i2 < data.positions.size()) {
            Vector3 v0 = data.positions[i0];
            Vector3 v1 = data.positions[i1];
            Vector3 v2 = data.positions[i2];
            
            Vector3 edge1 = v1 - v0;
            Vector3 edge2 = v2 - v0;
            Vector3 normal = edge1.cross(edge2).normalized();
            
            data.normals[i0] += normal;
            data.normals[i1] += normal;
            data.normals[i2] += normal;
        }
    }
    
    // Normalize accumulated normals
    for (auto& normal : data.normals) {
        normal.normalize();
    }
}

std::string ObjLoader::loadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}