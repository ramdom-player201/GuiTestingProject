#include "LayoutCompositor.h"

#include "../WindowClasses/BaseWindow.h"
#include "../Services/VulkanHandler.h"
#include "GuiLayout.h"
#include "Viewport.h"

LayoutCompositor::LayoutCompositor(BaseWindow& parentWindow, VulkanHandler& vulkanHandler)
	:parentWindow(parentWindow),
	vulkanHandler(vulkanHandler),
	guiLayout(vulkanHandler) // initialise gui subsystem
{

}

LayoutCompositor::~LayoutCompositor()
{
}

/*
void LayoutCompositor::init() {
    // 1. BaseWindow is now fully constructed. SwapChain exists.
    // 2. Query the actual starting size.
    uint32_t width = parentWindow.getSwapChainExtent().width;
    uint32_t height = parentWindow.getSwapChainExtent().height;

    // 3. Explicitly push the valid state down to the GUI
    guiLayout.setSize(width, height);

    // 4. Now safe to create compositor resources (pipelines, quad buffer, etc.)
    createCompositeResources();
    // ...
}

void LayoutCompositor::handleWindowResize() {
    // 1. BaseWindow has already recreated its SwapChain.
    // 2. Query the new size.
    uint32_t newWidth = parentWindow.getSwapChainExtent().width;
    uint32_t newHeight = parentWindow.getSwapChainExtent().height;

    // 3. Push the new state to the GUI (GuiLayout handles destroying/recreating its VkImage internally)
    guiLayout.setSize(newWidth, newHeight);

    // 4. Recreate compositor resources that depend on swapchain size (framebuffers, command buffers)
    cleanupCompositeResources();
    createCompositeResources();
}

void LayoutCompositor::drawFrame() {
    // By the time we get here, init() or handleWindowResize() has guaranteed a size exists.
    VkImageView guiTexture = guiLayout.getLatestImage();

    // ... proceed with compositing ...
}
*/