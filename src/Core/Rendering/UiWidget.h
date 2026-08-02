#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

#include "LayoutTypes.h"

// Forward declarations
struct InputEvent;
class UiSettings; // global UI scale, fonts, etc

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// WIDGET FLAGS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class UiWidgetFlags : uint32_t {
    None = 0,
    Visible = 1 << 0,
    Hovered = 1 << 1,
    Pressed = 1 << 2,
    Focused = 1 << 3,
    ClipChildren = 1 << 4, // If true, use VkRect2D scissor before drawing children
    LayoutDirty = 1 << 5, // Size/position needs recalculating
    RenderDirty = 1 << 6  // Visuals need redrawing to the texture
};

// Allow bitwise operations on the flags
inline UiWidgetFlags operator|(UiWidgetFlags a, UiWidgetFlags b) {
    return static_cast<UiWidgetFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline UiWidgetFlags operator&(UiWidgetFlags a, UiWidgetFlags b) {
    return static_cast<UiWidgetFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline bool HasFlag(UiWidgetFlags flags, UiWidgetFlags flagToCheck) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flagToCheck)) != 0;
}
inline void SetFlag(UiWidgetFlags& flags, UiWidgetFlags flagToSet, bool value) {
    if (value) { 
        flags = flags | flagToSet; 
    }
    else {
        flags = flags & static_cast<UiWidgetFlags>(~static_cast<uint32_t>(flagToSet));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// UI WIDGET BASE CLASS
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class UiWidget {
public:
    virtual ~UiWidget() = default;

    // Tree management (Used by parsers)
    void AddChild(std::unique_ptr<UiWidget> child);
    void RemoveChild(UiWidget* child);
    UiWidget* GetParent()const { return parent; }
    const std::vector<std::unique_ptr<UiWidget>>& GetChildren() const { return children; }

    // Core lifecycle (called from GuiLayout every frame)

    virtual void CalculateLayout(DrawRect parentAbsoluteRect, float globalUiScale);
    virtual void Render() const;
    virtual InputEventResult ProcessInput(const InputEvent& event, DrawRect parentAbsoluteRect);

    // Utilities
    UiWidget* FindWidgetById(const std::string& searchId);

    // State
    UiWidgetFlags flags = UiWidgetFlags::Visible | UiWidgetFlags::LayoutDirty | UiWidgetFlags::RenderDirty;
    std::string id;
protected:
    // Dimensions
    UDim2 position;
    UDim2 size;
    Vec2 anchorPoint{ 0.0f,0.0f };

    // Calulated runtime pixels
    DrawRect absoluteRect;

    // Hierarchy
    UiWidget* parent{ nullptr };
    std::vector<std::unique_ptr<UiWidget>> children;
};