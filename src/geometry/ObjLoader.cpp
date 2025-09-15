#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

ObjLoader::ObjLoader() {
}

ObjLoader::~ObjLoader() {
}

std::shared_ptr<Mesh> ObjLoader::load(const std::string& filename) {
    std::string content = loadFile(filename);
    ObjData objData = parseObj(content);

    auto mesh = std::make_shared<Mesh>();

    // Build the final vertex and index buffers
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // For OBJ files with separate position/normal indices, we need to create unique vertices
    if (!objData.faceVertices.empty()) {
        // Use the face data to create proper vertices
        std::unordered_map<std::string, uint32_t> uniqueVertices;

        for (const auto& faceVert : objData.faceVertices) {
            // Create a unique key for this vertex combination
            std::string key = std::to_string(faceVert.positionIndex) + "/" + 
                             std::to_string(faceVert.normalIndex);

            auto it = uniqueVertices.find(key);
            if (it == uniqueVertices.end()) {
                // Create new vertex
                Vertex vertex;

                // Get position
                if (faceVert.positionIndex >= 0 && faceVert.positionIndex < (int) objData.positions.size()) {
                    vertex.position = objData.positions[faceVert.positionIndex];
                }

                // Get normal
                if (faceVert.normalIndex >= 0 && faceVert.normalIndex < (int) objData.normals.size()) {
                    vertex.normal = objData.normals[faceVert.normalIndex];
                } else if (!objData.generatedNormals.empty() && faceVert.positionIndex < (int) objData.generatedNormals.size()) {
                    vertex.normal = objData.generatedNormals[faceVert.positionIndex];
                } else {
                    vertex.normal = Vector3(0.0f, 1.0f, 0.0f);
                }

                // Generate color based on position
                vertex.color = Vector3(
                    (vertex.position.x + 1.0f) * 0.5f,
                    (vertex.position.y + 1.0f) * 0.5f,
                    (vertex.position.z + 1.0f) * 0.5f
                );

                uint32_t newIndex = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
                uniqueVertices[key] = newIndex;
                indices.push_back(newIndex);
            } else {
                // Reuse existing vertex
                indices.push_back(it->second);
            }
        }
    } else {
        // Simple case: use indices directly
        vertices.reserve(objData.positions.size());

        for (size_t i = 0; i < objData.positions.size(); ++i) {
            Vertex vertex;
            vertex.position = objData.positions[i];

            if (i < objData.normals.size()) {
                vertex.normal = objData.normals[i];
            } else if (!objData.generatedNormals.empty() && i < objData.generatedNormals.size()) {
                vertex.normal = objData.generatedNormals[i];
            } else {
                vertex.normal = Vector3(0.0f, 1.0f, 0.0f);
            }

            vertex.color = Vector3(
                (vertex.position.x + 1.0f) * 0.5f,
                (vertex.position.y + 1.0f) * 0.5f,
                (vertex.position.z + 1.0f) * 0.5f
            );

            vertices.push_back(vertex);
        }

        indices = objData.indices;
    }

    std::cout << "Loaded OBJ with " << vertices.size() << " vertices and " 
              << indices.size() / 3 << " triangles" << std::endl;

    mesh->setVertices(vertices);
    mesh->setIndices(indices);

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

    std::vector<FaceVertex> faceVertices;

    while (stream >> token) {
        FaceVertex fv;
        fv.positionIndex = -1;
        fv.texCoordIndex = -1;
        fv.normalIndex = -1;

        // Parse vertex reference: v, v/vt, v/vt/vn, v//vn
        size_t slash1 = token.find('/');
        if (slash1 == std::string::npos) {
            // Format: v
            fv.positionIndex = std::stoi(token) - 1;  // Convert to 0-based
        } else {
            // Format with slashes
            fv.positionIndex = std::stoi(token.substr(0, slash1)) - 1;

            size_t slash2 = token.find('/', slash1 + 1);
            if (slash2 != std::string::npos) {
                // Format: v/vt/vn or v//vn
                if (slash2 > slash1 + 1) {
                    // Has texture coordinate
                    fv.texCoordIndex = std::stoi(token.substr(slash1 + 1, slash2 - slash1 - 1)) - 1;
                }
                fv.normalIndex = std::stoi(token.substr(slash2 + 1)) - 1;
            } else {
                // Format: v/vt
                fv.texCoordIndex = std::stoi(token.substr(slash1 + 1)) - 1;
            }
        }

        faceVertices.push_back(fv);
    }

    // Triangulate face with CORRECT counter-clockwise winding for Vulkan
    // The OBJ format typically uses counter-clockwise winding when viewed from outside
    // We need to maintain that for Vulkan
    for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
        // Triangle fan triangulation maintaining CCW order
        data.faceVertices.push_back(faceVertices[0]);
        data.faceVertices.push_back(faceVertices[i]);
        data.faceVertices.push_back(faceVertices[i + 1]);

        // Also store simple indices if no separate normal indices
        if (faceVertices[0].normalIndex == -1) {
            data.indices.push_back(faceVertices[0].positionIndex);
            data.indices.push_back(faceVertices[i].positionIndex);
            data.indices.push_back(faceVertices[i + 1].positionIndex);
        }
    }
}

void ObjLoader::generateNormals(ObjData& data) {
    data.generatedNormals.resize(data.positions.size(), Vector3::zero());

    // Calculate face normals and accumulate at vertices
    if (!data.indices.empty()) {
        // Use simple indices
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

                data.generatedNormals[i0] += normal;
                data.generatedNormals[i1] += normal;
                data.generatedNormals[i2] += normal;
            }
        }
    } else if (!data.faceVertices.empty()) {
        // Use face vertices
        for (size_t i = 0; i < data.faceVertices.size(); i += 3) {
            uint32_t i0 = data.faceVertices[i].positionIndex;
            uint32_t i1 = data.faceVertices[i + 1].positionIndex;
            uint32_t i2 = data.faceVertices[i + 2].positionIndex;

            if (i0 < data.positions.size() && i1 < data.positions.size() && i2 < data.positions.size()) {
                Vector3 v0 = data.positions[i0];
                Vector3 v1 = data.positions[i1];
                Vector3 v2 = data.positions[i2];

                Vector3 edge1 = v1 - v0;
                Vector3 edge2 = v2 - v0;
                Vector3 normal = edge1.cross(edge2).normalized();

                data.generatedNormals[i0] += normal;
                data.generatedNormals[i1] += normal;
                data.generatedNormals[i2] += normal;
            }
        }
    }

    // Normalize accumulated normals
    for (auto& normal : data.generatedNormals) {
        if (normal.length() > 0.0001f) {
            normal.normalize();
        } else {
            normal = Vector3(0.0f, 1.0f, 0.0f);  // Default up normal
        }
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
