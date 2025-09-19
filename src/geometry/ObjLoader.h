#pragma once

#include "../rendering/Mesh.h"
#include "../math/Vector3.h"
#include "../math/Vector2.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class ObjLoader {
public:
    struct Material {
        std::string name;
        std::string diffuseTexture;					// map_Kd
        Vector3 diffuseColor{1.0f, 1.0f, 1.0f};		// Kd
        Vector3 ambientColor{0.1f, 0.1f, 0.1f};		// Ka
        Vector3 specularColor{1.0f, 1.0f, 1.0f};	// Ks
        float shininess{32.0f};						// Ns

        bool hasTexture() const { return !diffuseTexture.empty(); }
    };

    struct ObjResult {
        std::shared_ptr<Mesh> mesh;
        Material material;
    };

    ObjLoader(bool flipTextureY = true);  // Default to Vulkan coordinate system
    ~ObjLoader();

    std::shared_ptr<Mesh> load(const std::string& filename);
    ObjResult loadWithMaterial(const std::string& filename);

private:
    struct FaceVertex {
        int positionIndex;
        int texCoordIndex;
        int normalIndex;
    };

    struct ObjData {
        std::vector<Vector3> positions;
        std::vector<Vector2> texCoords;         // UV texture coordinates
        std::vector<Vector3> normals;
        std::vector<Vector3> generatedNormals;  // For when normals need to be generated
        std::vector<uint32_t> indices;          // Simple indices for basic OBJ files
        std::vector<FaceVertex> faceVertices;   // For complex OBJ files with v/vt/vn format

        // Material info
        std::string materialLibrary;            // mtllib
        std::string currentMaterial;            // usemtl
    };

    ObjData parseObj(const std::string& content);
    std::unordered_map<std::string, Material> parseMtl(const std::string& filename);
    Vector3 parseVector3(const std::string& line);
    Vector2 parseVector2(const std::string& line);
    void parseFace(const std::string& line, ObjData& data);
    void generateNormals(ObjData& data);
    std::string loadFile(const std::string& filename);
    std::string getDirectoryPath(const std::string& filepath);

    bool flipTextureY;  // Whether to flip Y coordinate for texture coordinates
};
