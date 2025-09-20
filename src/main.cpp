#include "Application.h"
#include "utils/logger/Logging.h"
#include <stdexcept>

int main() {
	try {
		Application app;
		app.run();
	}
	catch (const std::exception& e) {
		Log(ERROR, "Error: %s", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}