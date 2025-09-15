// Add this test code to your main.cpp or wherever you initialize your renderer
// This will help verify that the matrix operations are now correct

#include <iostream>
#include "rendering/Camera.h"
#include "math/Matrix4.h"
#include "math/Vector3.h"

void testMatrixFixes() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "     TESTING MATRIX FIXES" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Test the static test function
    Camera::testMatrixOperations();
    
    // Additional specific tests for the issues we fixed
    std::cout << "\n=== Testing Fixed Issues ===" << std::endl;
    
    // Test 1: Matrix multiplication order
    std::cout << "\n1. Testing matrix multiplication order:" << std::endl;
    Matrix4 a = Matrix4::translation(Vector3(1, 0, 0));
    Matrix4 b = Matrix4::translation(Vector3(0, 1, 0));
    Matrix4 c = a * b;  // Should translate by (1, 1, 0)
    Vector3 origin(0, 0, 0);
    Vector3 result = c * origin;
    std::cout << "   Translation(1,0,0) * Translation(0,1,0) * origin = (" 
              << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    std::cout << "   Expected: (1, 1, 0)" << std::endl;
    bool test1 = (std::abs(result.x - 1.0f) < 0.001f && 
                  std::abs(result.y - 1.0f) < 0.001f && 
                  std::abs(result.z - 0.0f) < 0.001f);
    std::cout << "   Result: " << (test1 ? "PASS" : "FAIL") << std::endl;
    
    // Test 2: Perspective projection
    std::cout << "\n2. Testing perspective projection:" << std::endl;
    Matrix4 proj = Matrix4::perspective(90.0f, 1.0f, 1.0f, 100.0f);
    Vector3 nearPoint(0, 0, -1);  // Point on near plane
    Vector3 farPoint(0, 0, -100);  // Point on far plane
    Vector3 nearResult = proj * nearPoint;
    Vector3 farResult = proj * farPoint;
    std::cout << "   Near plane point (0,0,-1) -> (" 
              << nearResult.x << ", " << nearResult.y << ", " << nearResult.z << ")" << std::endl;
    std::cout << "   Expected Z near 0 for Vulkan" << std::endl;
    std::cout << "   Far plane point (0,0,-100) -> (" 
              << farResult.x << ", " << farResult.y << ", " << farResult.z << ")" << std::endl;
    std::cout << "   Expected Z near 1 for Vulkan" << std::endl;
    bool test2 = (nearResult.z >= -0.1f && nearResult.z <= 0.1f);
    std::cout << "   Result: " << (test2 ? "PASS" : "FAIL") << std::endl;
    
    // Test 3: View matrix (lookAt)
    std::cout << "\n3. Testing view matrix (lookAt):" << std::endl;
    Matrix4 view = Matrix4::lookAt(
        Vector3(0, 0, 10),  // Eye at (0, 0, 10)
        Vector3(0, 0, 0),   // Looking at origin
        Vector3(0, 1, 0)    // Up is Y
    );
    Vector3 worldOrigin(0, 0, 0);
    Vector3 viewOrigin = view * worldOrigin;
    std::cout << "   World origin in view space: (" 
              << viewOrigin.x << ", " << viewOrigin.y << ", " << viewOrigin.z << ")" << std::endl;
    std::cout << "   Expected: (0, 0, -10)" << std::endl;
    bool test3 = (std::abs(viewOrigin.x - 0.0f) < 0.001f && 
                  std::abs(viewOrigin.y - 0.0f) < 0.001f && 
                  std::abs(viewOrigin.z + 10.0f) < 0.001f);
    std::cout << "   Result: " << (test3 ? "PASS" : "FAIL") << std::endl;
    
    // Test 4: Combined view-projection
    std::cout << "\n4. Testing combined view-projection:" << std::endl;
    Camera camera;
    camera.setPosition(Vector3(0, 0, 5));
    camera.lookAt(Vector3(0, 0, 0));
    camera.setPerspective(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
    
    Matrix4 vp = camera.getViewProjectionMatrix();
    Vector3 testPoint(0, 0, 0);  // Origin
    Vector3 projected = vp * testPoint;
    std::cout << "   Origin through camera VP: (" 
              << projected.x << ", " << projected.y << ", " << projected.z << ")" << std::endl;
    std::cout << "   Should be (0, 0, positive value < 1)" << std::endl;
    bool test4 = (std::abs(projected.x) < 0.001f && 
                  std::abs(projected.y) < 0.001f && 
                  projected.z > 0 && projected.z < 1);
    std::cout << "   Result: " << (test4 ? "PASS" : "FAIL") << std::endl;
    
    // Test 5: Orthographic projection
    std::cout << "\n5. Testing orthographic projection:" << std::endl;
    Matrix4 ortho = Matrix4::orthographic(-10, 10, -10, 10, 0.1f, 100.0f);
    Vector3 orthoTest(5, 5, -50);
    Vector3 orthoResult = ortho * orthoTest;
    std::cout << "   Point (5,5,-50) through ortho: (" 
              << orthoResult.x << ", " << orthoResult.y << ", " << orthoResult.z << ")" << std::endl;
    std::cout << "   X,Y should be in [-1,1], Z in [0,1]" << std::endl;
    bool test5 = (orthoResult.x >= -1 && orthoResult.x <= 1 &&
                  orthoResult.y >= -1 && orthoResult.y <= 1 &&
                  orthoResult.z >= 0 && orthoResult.z <= 1);
    std::cout << "   Result: " << (test5 ? "PASS" : "FAIL") << std::endl;
    
    // Summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "SUMMARY:" << std::endl;
    int passed = test1 + test2 + test3 + test4 + test5;
    std::cout << "Tests passed: " << passed << "/5" << std::endl;
    if (passed == 5) {
        std::cout << "✓ All matrix operations are working correctly!" << std::endl;
    } else {
        std::cout << "✗ Some tests failed - check the implementation" << std::endl;
    }
    std::cout << "========================================\n" << std::endl;
    
    // Print camera debug info
    std::cout << "\nCamera state for debugging:" << std::endl;
    camera.debugPrintMatrices();
}

// Call this in your main() or initialization:
// testMatrixFixes();