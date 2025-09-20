//
// logToFile.h - Minimal stub for ai3dObjViewer project
//
// This file is temporary, allowing us to bolt-in Logging
//	from the other Vulkan-related projects.
// Yes logging should be simple, so why overcomplicate
//	with dependencies (?) but here's a couple reasons why:
//	 - config file - used to enable logging to file.
//	 - where goes file? more complex on mobile device.
// So this works for now, till we move those things over.
//
#ifndef logToFile_h
#define logToFile_h

#include <string>
#include <fstream>

using std::ofstream;


struct AppSettings {
	bool isDebugLogToFile = false;
	std::string filePath = "";
};

struct AppConstantsStruct {
	AppSettings Settings;
	const char* DebugLogFileName = "debug.log";

	const char* getExePath() const {
		return "ai3dObjViewer";
	}
};

//extern AppConstantsStruct AppConstants;
// (only included once)
AppConstantsStruct AppConstants;

class FileSystem {
public:
	static std::string AppLocalStorageDirectory() {
		return "./";
	}
};

#endif // logToFile_h
