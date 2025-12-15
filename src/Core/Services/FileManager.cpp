#include "FileManager.h"

#include "LogService.h"

// define the static member
std::filesystem::path FileManager::programDirectory{ std::filesystem::current_path() / "ProgramFiles" };

// FUNCTIONS //

std::expected<std::filesystem::path, std::string> FileManager::ValidatePath(const std::string& relativePath)
{
	LogService::Log(
		LogType::TRACE,
		"FileManager",
		"ValidatePath",
		"Attempt access: [" + relativePath + "]"
	);

	// get target directory
	std::filesystem::path targetPath = programDirectory / relativePath;

	// normalise path
	std::filesystem::path normalised = std::filesystem::weakly_canonical(targetPath);

	// check path is in valid location
	LogService::Log(
		LogType::TRACE,
		"FileManager",
		"ValidatePath",
		"Normalised path: [" + normalised.string() + "]"
	);
	if (normalised.string().rfind(programDirectory.string(), 0) != 0) {
		LogService::Log(LogType::SECURITY, "FileManager", "ValidatePath", "SECURITY: ACCESS DENIED");
		return std::unexpected("Security Error, failed to access file because {Out of Bounds}");
	} // code exits here if trying to access out of bounds

	// Check if directory or file path
	if (normalised.has_extension()) {
		// Treat as a file
		if (std::filesystem::exists(normalised.parent_path())) {
			LogService::Log(
				LogType::TRACE,
				"FileManager",
				"ValidatePath",
				"File parent directory is valid"
			);
		}
		else {
			LogService::Log(
				LogType::MED,
				"FileManager",
				"ValidatePath",
				"File parent directory is missing, creating folders"
			);
			std::filesystem::create_directories(normalised.parent_path());
		}
		if (std::filesystem::exists(normalised)) {
			LogService::Log(
				LogType::TRACE,
				"FileManager",
				"ValidatePath",
				"File found"
			);
		}
		else {
			LogService::Log(
				LogType::MED,
				"FileManager",
				"ValidatePath",
				"File does not exist, creating new"
			);
			std::ofstream ofs(normalised);
			if (!ofs) {
				LogService::Log(
					LogType::CRITICAL,
					"FileManager",
					"ValidatePath",
					"Failed to create file"
				);
				return std::unexpected("Failed to create file: " + normalised.string());
			}
		}
	}
	else {
		// Create as a directory
		if (std::filesystem::exists(normalised)) {
			LogService::Log(
				LogType::TRACE,
				"FileManager",
				"ValidatePath",
				"Directory is valid"
			);
		}
		else {
			LogService::Log(
				LogType::MED,
				"FileManager",
				"ValidatePath",
				"Directory is missing, creating folders"
			);
			std::filesystem::create_directories(normalised);
		}
	}
	return normalised;
}

void FileManager::Initialise() {
	//programDirectory = std::filesystem::current_path() / "ProgramFiles";
}