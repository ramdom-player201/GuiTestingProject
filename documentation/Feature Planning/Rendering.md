Description of program structure in relation to rendering.

CLASS [Program]
- VulkanHandler
- WindowManager
# holds run context of the program
# VulkanHandler is declared before WindowManager, so WindowManager is destroyed before VulkanHandler
# Reverse destruction order (VH,WM -> WM,VH)
# lifecycle of majority of program is tied to this class

CLASS [WindowManager]
- map<size_t, unique_ptr<BaseWindow>>>
- VulkanHandler reference
# manages and stores BaseWindows
# lifetime managed by Program

CLASS [BaseWindow]
- VulkanHandler reference
- LayoutCompositor
- GLFWwindow pointer
- VkSurfaceKHR
- SwapChainData
# responsible for rendering program
# holds rendering objects unique per window
# lifetime managed by WindowManager

CLASS [VulkanHandler]
- VkInstance
- VkDevice
- VkPhysicalDevice
- VkQueue # graphics
- VkQueue # present
# holds vulkan rendering content shared across program
# lifetime managed by Program

CLASS [LayoutCompositor]
- VulkanHandler reference
- GuiLayout
- vector<Viewport>
# holds the gui image, multiple viewport images, and overlay image; composites them together and renders them to swapchain


# //////
# //////
# //////

# About multi-threading
# Windows are rendered one at a time via their Compositors
# Each Viewport and GuiLayout gives the Compositor their latest finished VkImage
# Viewports may do their actual rendering in separate corountines, but only wheir their target scenes differ