[[VulkanHandler]]
[[WindowManager]]
[[Window]]

[[VulkanHandler]]
VulkanHandler handles core Vulkan systems shared across Project.

[[WindowManager]]
WindowManager stores multiple windows.

[[Window]]
Window stores a set of [[WinTile|Tiles]], which each contain a set of [[UiPanel|Tabs]], which each contain a set of render objects on which render is called.

The window needs to hold the window-level vulkan objects used in rendering.

While RenderObjects stored inside [[UiPanel|Tabs]] do need to be able to render, both the [[WinTile|Tiles]] and [[UiPanel|Tabs]] need to do some rendering too.

Each [[WinTile|Tile]] defines a quadrant in the [[Window]]. It needs to render its draggable border edges and highlight them on hover.
It also needs to call render on the selected [[UiPanel|Tab]] and render a tab switcher if there are multiple.

The window also needs to render the header and footer, as well as the toolbar if relevant for that window type.

Some windows, eg tooltips won't render with anything other than text.