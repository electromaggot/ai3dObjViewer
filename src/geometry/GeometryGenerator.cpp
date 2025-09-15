#include "GeometryGenerator.h"
#include <cmath>

const float PI = 3.14159265359f;
const float GOLDEN_RATIO = (1.0f + std::sqrt(5.0f)) / 2.0f;

std::shared_ptr<Mesh> GeometryGenerator::createCube(float size) {
    auto mesh = std::make_shared<Mesh>();
    
    float halfSize = size * 0.5f;
    
    std::vector<Vertex> vertices = {
        // Front face (looking at -Z)
        {{-halfSize, -halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{ halfSize, -halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{ halfSize,  halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-halfSize,  halfSize,  halfSize}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
        
        // Back face
        {{ halfSize, -halfSize, -halfSize}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},
        {{-halfSize, -halfSize, -halfSize}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 1.0f}},
        {{-halfSize,  halfSize, -halfSize}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},
        {{ halfSize,  halfSize, -halfSize}, {0.0f, 0.0f, -1.0f}, {0.5f, 0.5f, 0.5f}},
        
        // Left face
        {{-halfSize, -halfSize, -halfSize}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.0f}},
        {{-halfSize, -halfSize,  halfSize}, {-1.0f, 0.0f, 0.0f}, {0.5f, 1.0f, 0.0f}},
        {{-halfSize,  halfSize,  halfSize}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.5f, 1.0f}},
        {{-halfSize,  halfSize, -halfSize}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.5f}},
        
        // Right face
        {{ halfSize, -halfSize,  halfSize}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 1.0f}},
        {{ halfSize, -halfSize, -halfSize}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.5f}},
        {{ halfSize,  halfSize, -halfSize}, {1.0f, 0.0f, 0.0f}, {0.5f, 1.0f, 0.5f}},
        {{ halfSize,  halfSize,  halfSize}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 1.0f}},
        
        // Top face
        {{-halfSize,  halfSize,  halfSize}, {0.0f, 1.0f, 0.0f}, {0.8f, 0.2f, 0.8f}},
        {{ halfSize,  halfSize,  halfSize}, {0.0f, 1.0f, 0.0f}, {0.2f, 0.8f, 0.8f}},
        {{ halfSize,  halfSize, -halfSize}, {0.0f, 1.0f, 0.0f}, {0.8f, 0.8f, 0.2f}},
        {{-halfSize,  halfSize, -halfSize}, {0.0f, 1.0f, 0.0f}, {0.8f, 0.8f, 0.8f}},
        
        // Bottom face
        {{-halfSize, -halfSize, -halfSize}, {0.0f, -1.0f, 0.0f}, {0.6f, 0.3f, 0.3f}},
        {{ halfSize, -halfSize, -halfSize}, {0.0f, -1.0f, 0.0f}, {0.3f, 0.6f, 0.3f}},
        {{ halfSize, -halfSize,  halfSize}, {0.0f, -1.0f, 0.0f}, {0.3f, 0.3f, 0.6f}},
        {{-halfSize, -halfSize,  halfSize}, {0.0f, -1.0f, 0.0f}, {0.6f, 0.6f, 0.3f}}
    };
    
    // FIXED: Counter-clockwise winding for Vulkan
    std::vector<uint32_t> indices = {
        // Front face
        0, 1, 2,    0, 2, 3,
        // Back face
        4, 5, 6,    4, 6, 7,
        // Left face
        8, 9, 10,   8, 10, 11,
        // Right face
        12, 13, 14, 12, 14, 15,
        // Top face
        16, 17, 18, 16, 18, 19,
        // Bottom face
        20, 21, 22, 20, 22, 23
    };
    
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    
    return mesh;
}

std::shared_ptr<Mesh> GeometryGenerator::createSphere(float radius, int segments) {
    auto mesh = std::make_shared<Mesh>();
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Generate vertices
    for (int i = 0; i <= segments; ++i) {
        float phi = PI * float(i) / float(segments);
        
        for (int j = 0; j <= segments; ++j) {
            float theta = 2.0f * PI * float(j) / float(segments);
            
            Vector3 position(
                radius * std::sin(phi) * std::cos(theta),
                radius * std::cos(phi),
                radius * std::sin(phi) * std::sin(theta)
            );
            
            Vector3 normal = position.normalized();
            
            // Generate a color based on position
            Vector3 color(
                (normal.x + 1.0f) * 0.5f,
                (normal.y + 1.0f) * 0.5f,
                (normal.z + 1.0f) * 0.5f
            );
            
            vertices.push_back({position, normal, color});
        }
    }
    
    // Generate indices - FIXED: Counter-clockwise winding
    for (int i = 0; i < segments; ++i) {
        for (int j = 0; j < segments; ++j) {
            int first = i * (segments + 1) + j;
            int second = first + segments + 1;
            
            // First triangle (counter-clockwise)
            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second);
            
            // Second triangle (counter-clockwise)
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second + 1);
        }
    }
    
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    
    return mesh;
}

std::shared_ptr<Mesh> GeometryGenerator::createDodecahedron(float radius) {
    auto mesh = std::make_shared<Mesh>();
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Dodecahedron vertices (based on golden ratio)
    float a = 1.0f / std::sqrt(3.0f);
    float b = a / GOLDEN_RATIO;
    float c = a * GOLDEN_RATIO;
    
    // Scale by radius
    a *= radius;
    b *= radius;
    c *= radius;
    
    std::vector<Vector3> positions = {
        // Cube vertices
        { a,  a,  a}, { a,  a, -a}, { a, -a,  a}, { a, -a, -a},
        {-a,  a,  a}, {-a,  a, -a}, {-a, -a,  a}, {-a, -a, -a},
        
        // Rectangle 1
        { b,  c,  0}, {-b,  c,  0}, { b, -c,  0}, {-b, -c,  0},
        
        // Rectangle 2
        { c,  0,  b}, { c,  0, -b}, {-c,  0,  b}, {-c,  0, -b},
        
        // Rectangle 3
        { 0,  b,  c}, { 0, -b,  c}, { 0,  b, -c}, { 0, -b, -c}
    };
    
    // Create vertices with colors
    for (size_t i = 0; i < positions.size(); ++i) {
        Vector3 normal = positions[i].normalized();
        Vector3 color(
            std::abs(normal.x),
            std::abs(normal.y),
            std::abs(normal.z)
        );
        vertices.push_back({positions[i], normal, color});
    }
    
    // Dodecahedron faces (12 pentagonal faces) - FIXED winding order
    std::vector<std::vector<uint32_t>> faces = {
        {0, 16, 4, 9, 8},  {0, 12, 2, 17, 16}, {12, 13, 3, 10, 2},
        {9, 4, 14, 15, 5}, {3, 13, 1, 18, 19}, {7, 15, 14, 6, 11},
        {0, 8, 1, 13, 12}, {8, 9, 5, 18, 1},   {16, 17, 6, 14, 4},
        {6, 17, 2, 10, 11}, {7, 19, 18, 5, 15}, {7, 11, 10, 3, 19}
    };
    
    // Convert pentagonal faces to triangles with correct winding
    for (const auto& face : faces) {
        // Fan triangulation from first vertex
        for (size_t i = 1; i < face.size() - 1; ++i) {
            indices.push_back(face[0]);
            indices.push_back(face[i]);
            indices.push_back(face[i + 1]);
        }
    }
    
    calculateNormals(vertices, indices);
    
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    
    return mesh;
}

std::shared_ptr<Mesh> GeometryGenerator::createPlane(float width, float height) {
    auto mesh = std::make_shared<Mesh>();
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    std::vector<Vertex> vertices = {
        {{-halfWidth, 0.0f, -halfHeight}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ halfWidth, 0.0f, -halfHeight}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ halfWidth, 0.0f,  halfHeight}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-halfWidth, 0.0f,  halfHeight}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}}
    };
    
    // FIXED: Counter-clockwise winding
    std::vector<uint32_t> indices = {
        0, 1, 2,    0, 2, 3
    };
    
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    
    return mesh;
}

std::shared_ptr<Mesh> GeometryGenerator::createCylinder(float radius, float height, int segments) {
    auto mesh = std::make_shared<Mesh>();
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    float halfHeight = height * 0.5f;
    
    // Create top and bottom center vertices
    vertices.push_back({{0.0f, halfHeight, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}});  // Top center
    vertices.push_back({{0.0f, -halfHeight, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}); // Bottom center
    
    // Create side vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI * float(i) / float(segments);
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        
        Vector3 normal(x / radius, 0.0f, z / radius);
        Vector3 color((std::cos(angle) + 1.0f) * 0.5f, 0.5f, (std::sin(angle) + 1.0f) * 0.5f);
        
        // Top ring
        vertices.push_back({{x, halfHeight, z}, normal, color});
        // Bottom ring  
        vertices.push_back({{x, -halfHeight, z}, normal, color});
    }
    
    // FIXED: Counter-clockwise winding for all faces
    // Create top cap triangles
    for (int i = 0; i < segments; ++i) {
        indices.push_back(0); // Top center
        indices.push_back(2 + ((i + 1) % segments) * 2); // Next top vertex
        indices.push_back(2 + i * 2); // Current top vertex
    }
    
    // Create bottom cap triangles
    for (int i = 0; i < segments; ++i) {
        indices.push_back(1); // Bottom center
        indices.push_back(3 + i * 2); // Current bottom vertex
        indices.push_back(3 + ((i + 1) % segments) * 2); // Next bottom vertex
    }
    
    // Create side triangles
    for (int i = 0; i < segments; ++i) {
        int topCurrent = 2 + i * 2;
        int bottomCurrent = 3 + i * 2;
        int topNext = 2 + ((i + 1) % segments) * 2;
        int bottomNext = 3 + ((i + 1) % segments) * 2;
        
        // First triangle
        indices.push_back(topCurrent);
        indices.push_back(topNext);
        indices.push_back(bottomCurrent);
        
        // Second triangle
        indices.push_back(topNext);
        indices.push_back(bottomNext);
        indices.push_back(bottomCurrent);
    }
    
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    
    return mesh;
}

void GeometryGenerator::calculateNormals(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    // Reset all normals
    for (auto& vertex : vertices) {
        vertex.normal = Vector3::zero();
    }
    
    // Calculate face normals and accumulate vertex normals
    for (size_t i = 0; i < indices.size(); i += 3) {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];
        
        Vector3 faceNormal = calculateFaceNormal(
            vertices[i0].position,
            vertices[i1].position,
            vertices[i2].position
        );
        
        vertices[i0].normal += faceNormal;
        vertices[i1].normal += faceNormal;
        vertices[i2].normal += faceNormal;
    }
    
    // Normalize all vertex normals
    for (auto& vertex : vertices) {
        vertex.normal.normalize();
    }
}

Vector3 GeometryGenerator::calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3) {
    Vector3 edge1 = v2 - v1;
    Vector3 edge2 = v3 - v1;
    return edge1.cross(edge2).normalized();
}