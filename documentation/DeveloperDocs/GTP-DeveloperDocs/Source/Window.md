Owned by a [[WindowManager]].

Re-renders when a change is detected. Does not clear all but instead draws over existing image where needed.

The window contains Window-specific Vulkan stuff, and also coordinates with the [[VulkanHandler]] where needed.

Window contains (depending on type):
A [[WindowHeader]], a [[WindowFooter]], a [[WindowToolbar]] and a set of [[WinTile|WindowTiles]].
Some windows, eg tooltips do not contain any of this.
Therefore it may be worth separating Window into a BaseWindow and other window types.

BaseWindow:
⨽ TiledWindow
     Holds a window with a header, footer, toolbar, and a full array of scalable tiles that can hold any panel.
⨽ Tooltip
     Holds only an info box, no Minimise/Maximise/Close buttons, just a box with text (possibly scrollable, and sometimes with icons and buttons.)
     Tooltip appears when hovering mouse on something that spawns it and disappears when mouse is cleared or when dismissed with hotkey.
⨽ ContextWindow
     Displays a list of clickable context options, which themselves can have nested options.
⨽ DedicatedWindow
     Holds a single UiPanel that renders things as normal, no tiling and no toolbar. Headers and footers are included as normal.
⨽ TestWindow
     Handles its own rendering and input management independent of other services to test features ahead of full implementation.