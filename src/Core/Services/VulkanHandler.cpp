#include "VulkanHandler.h"

#include "LogService.h"

void VulkanHandler::Initialise() {
	LogService::Log(
		LogType::TRACE,
		className,
		"Initialise",
		"Initialising Vulkan"
	);
}