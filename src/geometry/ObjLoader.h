#pragma once

#include "../rendering/Mesh.h"
#include "../math/Vector3.h"
#include <memory>
#include <string>
#include <vector>

class ObjLoader {
public:
    ObjLoader();
    ~ObjLoader();

    std::shared_ptr<Mesh> load(const std::string& filename);

private:
    struct FaceVertex {
        int positionIndex;
        int texCoordIndex;
        int normalIndex;
    };

    struct ObjData {
        std::vector<Vector3> positions;
        std::vector<Vector3> normals;
        std::vector<Vector3> generatedNormals;  // For when normals need to be generated
        std::vector<uint32_t> indices;          // Simple indices for basic OBJ files
        std::vector<FaceVertex> faceVertices;   // For complex OBJ files with v/vt/vn format
    };

    ObjData parseObj(const std::string& content);
    Vector3 parseVector3(const std::string& line);
    void parseFace(const std::string& line, ObjData& data);
    void generateNormals(ObjData& data);
    std::string loadFile(const std::string& filename);
};
