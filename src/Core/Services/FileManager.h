#pragma once

#include <filesystem>
#include <fstream>
#include <expected>
#include <string>

class FileManager {
private:

	// Directory for the program, file edits may only occur in here
	static std::filesystem::path programDirectory;

public: // temporary access

	// Helper function to sanitise relativePaths to ensure they don't escape the directory
	static std::expected<std::filesystem::path,std::string> ValidatePath(const std::string& relativePath);

public:

	// File Operations
	static bool QueryFile(const std::string& relativePath);			// check if file exists
	static size_t QueryFileSize(const std::string& relativePath);	// check size of file, return 0 if it doesn't exist
	static std::fstream OpenFile(const std::string& relativePath);	// open file, and maintain ownership
	static std::ifstream ReadFile(const std::string& relativePath);	// read file, no edits changed

	// consider using a "File" class to automatically manage file opening/closing via constructor/destructor

	// ClassName
	static constexpr std::string_view className{"FileManager"};
};

// used to create/edit/delete files and folders
// works only in the ProgramFiles folder and below, no program access external to this location
// upon initialisation, attempt to find ProgramFiles or create if missing