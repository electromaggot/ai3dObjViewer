#pragma once

#include "rendering/Mesh.h"
#include <string>
#include <memory>

class ObjLoader {
public:
    ObjLoader();
    ~ObjLoader();
    
    std::shared_ptr<Mesh> load(const std::string& filename);
    
private:
    struct ObjData {
        std::vector<Vector3> positions;
        std::vector<Vector3> normals;
        std::vector<uint32_t> indices;
    };
    
    ObjData parseObj(const std::string& content);
    Vector3 parseVector3(const std::string& line);
    void parseFace(const std::string& line, ObjData& data);
    void generateNormals(ObjData& data);
    std::string loadFile(const std::string& filename);
};