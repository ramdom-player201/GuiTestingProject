#include "FileManager.h"

#include "LogService.h"

// define the static member
std::filesystem::path FileManager::programDirectory{ std::filesystem::current_path() / "ProgramFiles" };

// FUNCTIONS //

std::expected<std::filesystem::path, std::string> FileManager::ValidatePath(const std::string& relativePath)
{
	constexpr std::string_view functionName{ "ValidatePath" };

	LogService::Log(
		LogType::TRACE,
		className,
		functionName,
		"Attempt access: [" + relativePath + "]"
	);

	// get target directory
	std::filesystem::path targetPath = programDirectory / relativePath;

	// normalise path
	std::filesystem::path normalised = std::filesystem::weakly_canonical(targetPath);
	// file may not exist yet, but missing directories are generated after security check, hence `weakly_canonical` over standard `canonical`

	// check path is in valid location
	LogService::Log(
		LogType::TRACE,
		className,
		functionName,
		"Normalised path: [" + normalised.string() + "]"
	);
	if (normalised.string().rfind(programDirectory.string(), 0) != 0) {
		LogService::Log(
			LogType::SECURITY,
			className,
			functionName,
			"SECURITY: ACCESS DENIED"
		);
		return std::unexpected("Security Error, failed to access file because {Out of Bounds}");
	} // code exits here if trying to access out of bounds

	// Check if directory or file path
	if (normalised.has_extension()) {
		// Treat as a file
		if (std::filesystem::exists(normalised.parent_path())) {
			LogService::Log(
				LogType::TRACE,
				className,
				functionName,
				"File parent directory is valid"
			);
		}
		else {
			LogService::Log(
				LogType::MED,
				className,
				functionName,
				"File parent directory is missing, creating folders"
			);
			std::filesystem::create_directories(normalised.parent_path());
		}
		if (std::filesystem::exists(normalised)) {
			LogService::Log(
				LogType::TRACE,
				className,
				functionName,
				"File found"
			);
		}
		else {
			LogService::Log(
				LogType::MED,
				className,
				functionName,
				"File does not exist, creating new"
			);
			std::ofstream ofs(normalised);
			if (!ofs) {
				LogService::Log(
					LogType::CRITICAL,
					className,
					functionName,
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
				className,
				functionName,
				"Directory is valid"
			);
		}
		else {
			LogService::Log(
				LogType::MED,
				className,
				functionName,
				"Directory is missing, creating folders"
			);
			std::filesystem::create_directories(normalised);
		}
	}
	return normalised;
}