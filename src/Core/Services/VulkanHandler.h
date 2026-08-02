#pragma once

#include "LogService.h"

#include <vulkan/vulkan.h>
#include <optional>
#include <string>
#include <vector>

class VulkanHandler {
private:
	// Structs
	struct QueueFamilyIndices {
		// Struct containing the indexes for each supported queue family.
		// Optional is used to indicate whether each is supported or not.
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool TheyAllExist() const {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	// Member variables (Global Vulkan State)
	VkInstance vulkanInstance{ VK_NULL_HANDLE };
	VkDevice logicalDevice{ VK_NULL_HANDLE };
	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	VkQueue graphicsQueue{ VK_NULL_HANDLE };
	VkQueue presentQueue{ VK_NULL_HANDLE };
	uint32_t graphicsQueueFamilyIndex{ 0 };

	// Utility functions
	bool IsDeviceSuitable(VkPhysicalDevice deviceToCheck, const VkSurfaceKHR& surface) const;
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice deviceToCheck, const VkSurfaceKHR& surface) const;
	void PickPhysicalDevice(const VkSurfaceKHR& surface); // Requires a surface to check present support
	void CreateLogicalDevice(const VkSurfaceKHR& surface); // Pass surface to ensure queue families are correct
public:
	// Safety locks to prevent copy/move/reassignment
	VulkanHandler() = default;
	VulkanHandler(const VulkanHandler&) = delete;
	VulkanHandler& operator=(const VulkanHandler&) = delete;
	VulkanHandler(VulkanHandler&&) = delete;
	VulkanHandler& operator=(VulkanHandler&&) = delete;

	// Lifecycle
	void CreateVulkanInstance(const std::vector<const char*>& requiredExtensions);
	void InitialiseDevices(const VkSurfaceKHR& surface);
	~VulkanHandler();

	// Getters
	VkInstance GetInstance() const {
		if (vulkanInstance == VK_NULL_HANDLE) {
			LogService::Log(
				LogType::CRITICAL,
				className,
				"GetInstance",
				"VkInstance was not initialised"
			);
		}
		return vulkanInstance;
	}

	VkDevice GetLogicalDevice()const {
		if (logicalDevice == VK_NULL_HANDLE) {
			LogService::Log(
				LogType::CRITICAL,
				className,
				"GetLogicalDevice",
				"Logical device not initialised"
			);
		}
		return logicalDevice;
	}

	const VkPhysicalDevice& GetPhysicalDevice() const {
		if (physicalDevice == VK_NULL_HANDLE) {
			LogService::Log(
				LogType::CRITICAL,
				className,
				"GetPhysicalDevice",
				"PhysicalDevice not selected"
			);
		}
		return physicalDevice;
	}

	VkQueue GetGraphicsQueue() const {
		if (graphicsQueue == VK_NULL_HANDLE) {
			LogService::Log(
				LogType::CRITICAL,
				className,
				"GetGraphicsQueue",
				"Graphics queue not initialised"
			);
		}
		return graphicsQueue;
	}

	VkQueue GetPresentQueue() const {
		if (presentQueue == VK_NULL_HANDLE) {
			LogService::Log(
				LogType::CRITICAL,
				className,
				"GetPresentQueue",
				"Present queue not initialised"
			);
		}
		return presentQueue;
	}

	// Hardware queries
	uint32_t GetGraphicsQueueFamilyIndex() const;
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	// ClassName
	static constexpr std::string_view className{ "VulkanHandler" };
};