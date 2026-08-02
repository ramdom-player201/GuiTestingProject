# See [[Rendering.md]] for how GuiLayout integrates with LayoutCompositor and Viewports

# DATA TYPES

STRUCT [DrawRect]
- float x
- float y
- float width
- float height
# represents absolute pixel coordinates on screen
# floats are used to account for off-by-one errors caused by truncation during calculations
# cast to int32_t only for final VkRect2D usage

STRUCT [Vec2]
- float x
- float y
# generic 2D vector
# used for anchor points

STRUCT [UDim]
- float scale # 0.0 to 1.0 (ratio of parent size)
- float offset # absolute pixels (multiplied by uiscale at runtime)

STRUCT [UDim2]
- UDim x
- UDim y
# 2D position/size
# union of ratio and absolute offset for versatile UI rendering

STRUCT [InputEventResult]
- bool inputConsumed
- uint32_t targetViewportId # UINT32_MAX if none
- float localX
- float localY
# returned from GuiLayout into LayoutCompositor to route inputs to 3D viewpors
# localX/Y are mouse coordinates normalised to the viewport

# UI WIDGET TREE

CLASS [UiWidget]
- UDim2 position
- UDim2 size
- Vec2 anchorPoint
- DrawRec absoluteRect
- UiWidgetFlags flags
- UiWidget* parent
- vector<unique_ptr> children
- string id
# base node for all UI elements
# uses OOP inheritance
# virtual lifecycle:
# ^ 1:CalculateLayout - 2:Render - 3:ProcessInput

ENUM[UiWidgetFlags]
- Visible | Hovered | Pressed | Focused
- ClipChildren
- LayoutDirty # fully recalculate layout
- RenderDirty # only recalculate colour details

# GUI MANAGEMENT

CLASS [GuiLayout]
- VulkanHandler reference
- unique_ptr rootWidget
- unordered_map<uint32_t,DrawRect> viewportLayoutRequests
# owned by LayoutCompositor
# renders gui to texture
# decoupled from 3D rendering, only knows 2D widget tree
# processInput() returns a result, this can be passed to viewports via the LayoutCompositor if required

# PAGE SYSTEM

# ui is split into 3 file types to separate macro-geometry, user layout preference, micro-geometry

ENUM [PageMode]
- DockableEditor # Header, Toolbar, DockingArea, Footer
- DedicatedViewer # Header, Content, Footer
- etc
# defines the type of layout to render for the gui, used to build the intial root tree
# could be an enum or small json file, acts as a factory for the rootWidget structure

FILE [PageLayout]
# saves/loads user spatial preferences
# defines split ratios for window tiles
# defines which panels are in which tiles
# eg TILE_LEFT_LOWER contains ["panel_hierarchy", "panel_properties"]

FILE [PagePanel]
# defines actual widget subtree for a given panel
# users can create custom panels by editing this without touching the raw C++ code

# WIDGET SUBCLASSES

CLASS [UiWidget -> WindowTile]
# spacial container inside a DockingArea
# holds 0 or more tabs
# hidden when 0 tabs
# tab switcher included when 2 or more tabs

CLASS [UiWidget -> ViewportWidget]
# represents the space where an actual viewport would be in the gui
# does not actually do any 3D handling
# special input handling, returns a viewport id for GuiLayout to forward to the real Viewport

CLASS [UiWidget -> ShapeWidget]
# parametric shape definition
# can tune individual corner radii

# //////
# //////
# //////

# MOUSE ROUTING

WindowManager polls -> BaseWindow -> LayoutCompositor -> GuiLayout
                                                    ^ -> Viewport

# KEYBOARD ROUTING

GuiLayout holds a UiWidget* focusedWidget
When clicking an input field, this is set to the field
Keyboard events should bypass hit-tree testing and go to the focused widget
If focus, route keyboard -> focused widget
Else, route keyboard -> input handler

# TOOLTIPS & CONTEXT MENUS

These are added as overlays, rather than separate windows

# DIRTY FLAGS & PERFORMANCE

GuiLayout renders to an offscreen texture.
LayoutCompositor draws that texture as a fullscreen quad.
If an object changes colour, only GuiLayout::RenderDirty is set
If a panel is resized, both LayoutDirty and RenderDirty is set
3D viewports are always dirty by their nature, but GuiLayout does not process their contents, only their bounds