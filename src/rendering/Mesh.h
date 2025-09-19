#pragma once

#include "../math/Vector3.h"
#include "../math/Vector2.h"
#include <vector>
#include <vulkan/vulkan.h>

struct Vertex {
	Vector3 position;
	Vector3 normal;
	Vector3 color;
	Vector2 texCoord;  // UV texture coordinates

	// Constructor for untextured vertices
	Vertex() : position(), normal(), color(), texCoord() {}
	Vertex(const Vector3& pos, const Vector3& norm, const Vector3& col)
		: position(pos), normal(norm), color(col), texCoord() {}

	// Constructor for textured vertices
	Vertex(const Vector3& pos, const Vector3& norm, const Vector3& col, const Vector2& uv)
		: position(pos), normal(norm), color(col), texCoord(uv) {}

	static VkVertexInputBindingDescription getBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

	bool operator==(const Vertex& other) const {
		return position == other.position && normal == other.normal &&
			   color == other.color && texCoord == other.texCoord;
	}
};

class VulkanDevice;

class Mesh {
public:
	Mesh();
	~Mesh();

	void setVertices(const std::vector<Vertex>& vertices);
	void setIndices(const std::vector<uint32_t>& indices);
	void setHasTexture(bool hasTexture) { this->hasTexture = hasTexture; }

	void createBuffers(VulkanDevice& device);
	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

	const std::vector<Vertex>& getVertices() const { return vertices; }
	const std::vector<uint32_t>& getIndices() const { return indices; }

	bool hasIndices() const { return !indices.empty(); }
	bool hasTextureCoordinates() const { return hasTexture; }

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VulkanDevice* device;
	bool buffersCreated;
	bool hasTexture;

	void createVertexBuffer();
	void createIndexBuffer();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
