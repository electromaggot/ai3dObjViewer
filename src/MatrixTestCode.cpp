// Add this test code to your main.cpp or wherever you initialize your renderer
// This will help verify that the matrix operations are now correct

#include "utils/logger/Logging.h"
#include "rendering/Camera.h"
#include "math/Matrix4.h"
#include "math/Vector3.h"

void testMatrixFixes() {
	Log(LOW, "\n========================================");
	Log(LOW, "     TESTING MATRIX FIXES");
	Log(LOW, "========================================\n");

	// Test the static test function
	Camera::testMatrixOperations();

	// Additional specific tests for the issues we fixed
	Log(LOW, "\n=== Testing Fixed Issues ===");

	// Test 1: Matrix multiplication order
	Log(LOW, "\n1. Testing matrix multiplication order:");
	Matrix4 a = Matrix4::translation(Vector3(1, 0, 0));
	Matrix4 b = Matrix4::translation(Vector3(0, 1, 0));
	Matrix4 c = a * b;  // Should translate by (1, 1, 0)
	Vector3 origin(0, 0, 0);
	Vector3 result = c * origin;
	Log(LOW, "   Translation(1,0,0) * Translation(0,1,0) * origin = (%f, %f, %f)", result.x, result.y, result.z);
	Log(LOW, "   Expected: (1, 1, 0)");
	bool test1 = (std::abs(result.x - 1.0f) < 0.001f &&
				  std::abs(result.y - 1.0f) < 0.001f &&
				  std::abs(result.z - 0.0f) < 0.001f);
	Log(NOTE, "   Result: %s", (test1 ? "PASS" : "FAIL"));

	// Test 2: Perspective projection
	Log(LOW, "\n2. Testing perspective projection:");
	Matrix4 proj = Matrix4::perspective(90.0f, 1.0f, 1.0f, 100.0f);
	Vector3 nearPoint(0, 0, -1);  // Point on near plane
	Vector3 farPoint(0, 0, -100);  // Point on far plane
	Vector3 nearResult = proj * nearPoint;
	Vector3 farResult = proj * farPoint;
	Log(LOW, "   Near plane point (0,0,-1) -> (%f, %f, %f)", nearResult.x, nearResult.y, nearResult.z);
	Log(LOW, "   Expected Z near 0 for Vulkan");
	Log(LOW, "   Far plane point (0,0,-100) -> (%f, %f, %f)", farResult.x, farResult.y, farResult.z);
	Log(LOW, "   Expected Z near 1 for Vulkan");
	bool test2 = (nearResult.z >= -0.1f && nearResult.z <= 0.1f);
	Log(NOTE, "   Result: %s", (test2 ? "PASS" : "FAIL"));

	// Test 3: View matrix (lookAt)
	Log(LOW, "\n3. Testing view matrix (lookAt):");
	Matrix4 view = Matrix4::lookAt(
		Vector3(0, 0, 10),	// Eye at (0, 0, 10)
		Vector3(0, 0, 0),	// Looking at origin
		Vector3(0, 1, 0)	// Up is Y
	);
	Vector3 worldOrigin(0, 0, 0);
	Vector3 viewOrigin = view * worldOrigin;
	Log(LOW, "   World origin in view space: (%f, %f, %f)", viewOrigin.x, viewOrigin.y, viewOrigin.z);
	Log(LOW, "   Expected: (0, 0, -10)");
	bool test3 = (std::abs(viewOrigin.x - 0.0f) < 0.001f &&
				  std::abs(viewOrigin.y - 0.0f) < 0.001f &&
				  std::abs(viewOrigin.z + 10.0f) < 0.001f);
	Log(NOTE, "   Result: %s", (test3 ? "PASS" : "FAIL"));

	// Test 4: Combined view-projection
	Log(LOW, "\n4. Testing combined view-projection:");
	Camera camera;
	camera.setPosition(Vector3(0, 0, 5));
	camera.lookAt(Vector3(0, 0, 0));
	camera.setPerspective(45.0f, 16.0f/9.0f, 0.1f, 100.0f);

	Matrix4 vp = camera.getViewProjectionMatrix();
	Vector3 testPoint(0, 0, 0);  // Origin
	Vector3 projected = vp * testPoint;
	Log(LOW, "   Origin through camera VP: (%f, %f, %f)", projected.x, projected.y, projected.z);
	Log(LOW, "   Should be (0, 0, positive value < 1)");
	bool test4 = (std::abs(projected.x) < 0.001f &&
				  std::abs(projected.y) < 0.001f &&
				  projected.z > 0 && projected.z < 1);
	Log(NOTE, "   Result: %s", (test4 ? "PASS" : "FAIL"));

	// Test 5: Orthographic projection
	Log(LOW, "\n5. Testing orthographic projection:");
	Matrix4 ortho = Matrix4::orthographic(-10, 10, -10, 10, 0.1f, 100.0f);
	Vector3 orthoTest(5, 5, -50);
	Vector3 orthoResult = ortho * orthoTest;
	Log(LOW, "   Point (5,5,-50) through ortho: (%f, %f, %f)", orthoResult.x, orthoResult.y, orthoResult.z);
	Log(LOW, "   X,Y should be in [-1,1], Z in [0,1]");
	bool test5 = (orthoResult.x >= -1 && orthoResult.x <= 1 &&
				  orthoResult.y >= -1 && orthoResult.y <= 1 &&
				  orthoResult.z >= 0 && orthoResult.z <= 1);
	Log(NOTE, "   Result: %s", (test5 ? "PASS" : "FAIL"));

	// Summary
	Log(LOW, "\n========================================");
	Log(LOW, "SUMMARY:");
	int passed = test1 + test2 + test3 + test4 + test5;
	Log(NOTE, "Tests passed: %d/5", passed);
	if (passed == 5) {
		Log(LOW, "✓ All matrix operations are working correctly!");
	} else {
		Log(LOW, "✗ Some tests failed - check the implementation");
	}
	Log(LOW, "========================================\n");

	// Print camera debug info
	Log(LOW, "\nCamera state for debugging:");
	camera.debugPrintMatrices();
}

// Call this in your main() or initialization:
// testMatrixFixes();
