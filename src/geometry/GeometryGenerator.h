#pragma once

#include "Mesh.h"
#include <memory>

class GeometryGenerator {
public:
	static std::shared_ptr<Mesh> createCube(float size = 1.0f);
	static std::shared_ptr<Mesh> createSphere(float radius = 1.0f, int segments = 32);
	static std::shared_ptr<Mesh> createDodecahedron(float radius = 1.0f);
	static std::shared_ptr<Mesh> createPlane(float width = 1.0f, float height = 1.0f);
	static std::shared_ptr<Mesh> createCylinder(float radius = 1.0f, float height = 2.0f, int segments = 32);

private:
	static void calculateNormals(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	static Vector3 calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
};