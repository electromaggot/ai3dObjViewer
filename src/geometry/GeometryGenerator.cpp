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

	// Counter-clockwise winding for OpenGL compatibility
	std::vector<uint32_t> indices = {
		// Front face
		0, 2, 1,	0, 3, 2,
		// Back face
		4, 6, 5,	4, 7, 6,
		// Left face
		8, 10, 9,	8, 11, 10,
		// Right face
		12, 14, 13, 12, 15, 14,
		// Top face
		16, 18, 17, 16, 19, 18,
		// Bottom face
		20, 22, 21, 20, 23, 22
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

	// Generate indices - Counter-clockwise winding
	for (int i = 0; i < segments; ++i) {
		for (int j = 0; j < segments; ++j) {
			int first = i * (segments + 1) + j;
			int second = first + segments + 1;

			// First triangle (counter-clockwise)
			indices.push_back(first);
			indices.push_back(second);
			indices.push_back(first + 1);

			// Second triangle (counter-clockwise)
			indices.push_back(second);
			indices.push_back(second + 1);
			indices.push_back(first + 1);
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

	const float phi = GOLDEN_RATIO;
	const float invPhi = 1.0f / phi;

	// 20 canonical dodecahedron vertex positions
	std::vector<Vector3> basePositions = {
		// Cube corners
		{ 1,  1,  1}, { 1,  1, -1}, { 1, -1,  1}, { 1, -1, -1},
		{-1,  1,  1}, {-1,  1, -1}, {-1, -1,  1}, {-1, -1, -1},
		// Rectangle in YZ plane
		{0,  invPhi,  phi}, {0, -invPhi,  phi}, {0,  invPhi, -phi}, {0, -invPhi, -phi},
		// Rectangle in XZ plane
		{ invPhi,  phi, 0}, {-invPhi,  phi, 0}, { invPhi, -phi, 0}, {-invPhi, -phi, 0},
		// Rectangle in XY plane
		{ phi, 0,  invPhi}, { phi, 0, -invPhi}, {-phi, 0,  invPhi}, {-phi, 0, -invPhi}
	};

	// Normalize and scale all positions
	for (auto& pos : basePositions) {
		pos = pos.normalized() * radius;
	}

	// Dodecahedron face connectivity (12 pentagonal faces)
	// Each vertex should appear in exactly 3 faces
	std::vector<std::vector<uint32_t>> faces = { // (around vertex:)
		{0, 16,  2,  9,  8},	// Face 1 - Red		 (0)
		{0,  8,  4, 13, 12},	// Face 2 - Green	 (0)
		{0, 12,  1, 17, 16},	// Face 3 - Blue	 (0)
		{1, 12, 13,  5, 10},	// Face 4 - Yellow	 (1)
		{1, 10, 11,  3, 17},	// Face 5 - Magenta	 (1)
		{2, 16, 17,  3, 14},	// Face 6 - Cyan	 (2)
		{2, 14, 15,  6,  9},	// Face 7 - Orange	 (2)
		{3, 11,  7, 15, 14},	// Face 8 - Purple	 (3)
		{4,  8,  9,  6, 18},	// Face 9 - Teal	 (4)
		{4, 18, 19,  5, 13},	// Face 10 - Pink	 (4)
		{5, 19,  7, 11, 10},	// Face 11 - Lime	 (5)
		{6, 15,  7, 19, 18}		// Face 12 - Sky Blue(6)
	};

	// 12 distinct bright colors for sharp face separation
	std::vector<Vector3> faceColors = {
		{1.0f, 0.3f, 0.3f}, // Bright red
		{0.3f, 1.0f, 0.3f}, // Bright green
		{0.3f, 0.3f, 1.0f}, // Bright blue
		{1.0f, 1.0f, 0.3f}, // Bright yellow
		{1.0f, 0.3f, 1.0f}, // Bright magenta
		{0.3f, 1.0f, 1.0f}, // Bright cyan
		{1.0f, 0.7f, 0.3f}, // Bright orange
		{0.7f, 0.3f, 1.0f}, // Bright purple
		{0.3f, 0.9f, 0.7f}, // Bright teal
		{1.0f, 0.6f, 0.8f}, // Bright pink
		{0.6f, 1.0f, 0.4f}, // Bright lime
		{0.5f, 0.7f, 1.0f}  // Bright sky blue
	};

	// Create separate geometry for each pentagonal face
	for (size_t faceIndex = 0; faceIndex < faces.size(); ++faceIndex) {
		const auto& face = faces[faceIndex];

		// Calculate face normal from first 3 vertices (counter-clockwise)
		Vector3 v0 = basePositions[face[0]];
		Vector3 v1 = basePositions[face[1]];
		Vector3 v2 = basePositions[face[2]];

		Vector3 edge1 = v1 - v0;
		Vector3 edge2 = v2 - v0;
		Vector3 faceNormal = edge1.cross(edge2).normalized();

		// Calculate face centroid to verify normal direction
		Vector3 centroid = Vector3::zero();
		for (uint32_t vertIndex : face) {
			centroid += basePositions[vertIndex];
		}
		centroid = centroid * (1.0f / static_cast<float>(face.size()));

		// Ensure normal points outward from origin
		if (faceNormal.dot(centroid) < 0.0f) {
			faceNormal = faceNormal * -1.0f;
		}

		// Get this face's color
		Vector3 faceColor = faceColors[faceIndex];

		// Add 5 unique vertices for this face (no sharing between faces)
		uint32_t baseVertexIndex = static_cast<uint32_t>(vertices.size());
		for (uint32_t vertIndex : face) {
			vertices.push_back({
				basePositions[vertIndex], // Position
				faceNormal,				  // Same normal for entire face
				faceColor				  // Same color for entire face
			});
		}

		// Triangulate pentagon using fan method: (0,1,2), (0,2,3), (0,3,4)
		for (uint32_t i = 1; i < face.size() - 1; ++i) {
			indices.push_back(baseVertexIndex + 0);		// Center vertex
			indices.push_back(baseVertexIndex + i);		// Current vertex
			indices.push_back(baseVertexIndex + i + 1);	// Next vertex
		}
	}

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

	// Counter-clockwise winding for OpenGL compatibility
	std::vector<uint32_t> indices = {
		0, 1, 2,	0, 2, 3
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

	// Counter-clockwise winding for all faces
	// Create top cap triangles (viewed from above)
	for (int i = 0; i < segments; ++i) {
		indices.push_back(0); // Top center
		indices.push_back(2 + i * 2); // Current top vertex
		indices.push_back(2 + ((i + 1) % segments) * 2); // Next top vertex
	}

	// Create bottom cap triangles (viewed from below)
	for (int i = 0; i < segments; ++i) {
		indices.push_back(1); // Bottom center
		indices.push_back(3 + ((i + 1) % segments) * 2); // Next bottom vertex
		indices.push_back(3 + i * 2); // Current bottom vertex
	}

	// Create side triangles
	for (int i = 0; i < segments; ++i) {
		int topCurrent = 2 + i * 2;
		int bottomCurrent = 3 + i * 2;
		int topNext = 2 + ((i + 1) % segments) * 2;
		int bottomNext = 3 + ((i + 1) % segments) * 2;

		// First triangle (counter-clockwise when viewed from outside)
		indices.push_back(topCurrent);
		indices.push_back(bottomCurrent);
		indices.push_back(topNext);

		// Second triangle (counter-clockwise when viewed from outside)
		indices.push_back(topNext);
		indices.push_back(bottomCurrent);
		indices.push_back(bottomNext);
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
