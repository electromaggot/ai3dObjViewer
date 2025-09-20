#pragma once

#include "SceneObject.h"

/**
 * Scene object for procedurally generated geometry.
 * Subtypes define specific shapes (cube, sphere, cylinder, etc.)
 */

static const char* Name[] = {
	"Cube",
	"Sphere",
	"Dodecahedron",
	"Cylinder",
	"Plane",
	"Dodecahedron",
	"Torus",
	"Cone",
	"Tetrahedron",
	"unknown"
};


class GeneratedModel : public SceneObject {
public:
	enum class Shape {
		CUBE,
		SPHERE,
		CYLINDER,
		PLANE,
		DODECAHEDRON,
		TORUS,			// Future extension
		CONE,			// Future extension
		TETRAHEDRON		// Future extension
	};

	const char* name() const { return Name[(int) shape]; }


	GeneratedModel(Shape shape, const std::string& name);
	GeneratedModel(Shape shape) : GeneratedModel(shape, std::string(name())) { }
	virtual ~GeneratedModel() = default;

	// SceneObject interface
	ObjectType getType() const override { return ObjectType::PROCEDURAL_MODEL; }
	std::unique_ptr<Model> createModel() const override;
	json serialize() const override;
	void deserialize(const json& jsonData) override;
	std::unique_ptr<SceneObject> clone() const override;

	// Procedural-specific methods
	Shape getShape() const { return shape; }
	void setShape(Shape s) { shape = s; }

	// Shape parameters (meaning depends on shape type)
	float getParameter1() const { return param1; }
	void setParameter1(float p) { param1 = p; }

	float getParameter2() const { return param2; }
	void setParameter2(float p) { param2 = p; }

	int getSegments() const { return segments; }
	void setSegments(int s) { segments = s; }

private:
	Shape shape;
	float param1;	// e.g., size for cube, radius for sphere
	float param2;	// e.g., height for cylinder, unused for cube
	int segments;	// tessellation level for curved surfaces

	void initializeDefaults();
};

