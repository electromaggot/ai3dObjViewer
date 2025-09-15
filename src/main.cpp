#include "Application.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        Application app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}