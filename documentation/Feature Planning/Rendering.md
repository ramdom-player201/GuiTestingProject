Description of program structure in relation to rendering.

CLASS [Program]
- VulkanHandler
- WindowManager
# holds run context of the program
# VulkanHandler is declared before WindowManager, so WindowManager is destroyed before VulkanHandler
# reverse destruction order (VH,WM -> WM,VH)
# lifecycle of majority of program is tied to this class

CLASS [WindowManager]
- map<size_t, unique_ptr<BaseWindow>>>
- VulkanHandler reference
# manages and stores BaseWindows
# handles inputs and passes events to whichever BaseWindow is in focus (if any)
# there shouldn't be any keybinds that work when everything is minimised, in the context of this application
# lifetime managed by Program

CLASS [BaseWindow]
- VulkanHandler reference
- LayoutCompositor
- GLFWwindow pointer
- VkSurfaceKHR
- SwapChainData
# responsible for managing swapchain and window-specific resources
# passes inputs from WindowManager->LayoutCompositor
# offloads final rendering responsibility to LayoutCompositor
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
- map<uint32_t, shared_ptr<Viewport>>
# requests the gui and viewport output textures; composites them together and renders them to swapchain

CLASS [GuiLayout]
- etc
# holds a gui tree
# processes tree when modified and renders result to texture for compositing
# see [[Gui.md]] for more details on gui rendering
# lifetime managed by LayoutCompositor

CLASS [Viewport]
- etc
# unlike GuiLayout, this class may not be exclusively used by the gui system
# rendering is processed by a separate JobHandler system, allowing multi-threading and decoupling from UI render thread
# holds a reference to a 3D scene and renders it to texture
# for multi-threading reasons, uses triple buffering to allow safe reading of 1 image while rendering in background
# is triple buffering the right choice here?
# are there any scenarios where its texture may be requested from multiple places at once?

# //////
# //////
# //////

# About multi-threading
# Windows are rendered one at a time via their Compositors
# Each Viewport and GuiLayout gives the Compositor their latest finished VkImage
# Viewports may do their actual rendering in separate corountines, but only wheir their target scenes differ