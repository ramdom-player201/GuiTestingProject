#pragma once

#include "../Services/VulkanHandler.h"

class GuiLayout {
public:
	GuiLayout(VulkanHandler& vulkanHandler);

	VkImage getLatestImage();
	void setSize(uint32_t width, uint32_t height);
private:
	uint32_t width{ 0 };
	uint32_t height{ 0 };
};