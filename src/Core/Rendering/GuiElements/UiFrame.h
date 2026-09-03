#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "../LayoutTypes.h"

class UiFrame; // forward declared for UiPassParams lel

struct UiPassParams {
	// Input
	InputEvent event;					// The current input data passed to the window
	UiFrame* activeTarget{ nullptr };	// Whether the input is already tied to a specific element (carried from previous)
	// Layout
	DrawRect parentContentRect;			// The screen rect to which child elements are relative to
	float guiScaleFactor{ 1.0f };		// Scale factor for gui elements, defaults to global, but individual elements can override downstream
	// Output
	InputEventResult result;			// The result of the current action, generally used to pass inputs to Viewports
	GuiBatches* batches{ nullptr };		// For batch rendering of UI elements
	bool layoutChanged{ false };		// Do not redraw gui texture unless something changes
	// Clipping
	ClipStack clipStack;
	uint32_t currentZBucket{ 0 };		// Integer component of Z bucket index
};

// UiFrame is an empty container and base class for gui elements

class UiFrame {
protected:
	// Placement
	ScaleOffset2D position;
	ScaleOffset2D size;
	Vec2 anchorPoint;

	// Tree hierarchy
	UiFrame* parent;
	std::vector<std::unique_ptr<UiFrame>> children;

	// Calculated during runtime, denotes the resolved bounding rect of the UI element
	DrawRect absoluteElementRect;

	// State
	bool visible{ true }; // Skip processing if hidden
	bool visualsChanged{ false }; // Force redraw if the visuals change
	bool clipChildren{ false }; // Defaults to off

	uint32_t zOrder{ 0 };

	virtual std::optional<ClipRect> GetClipRect() const { return std::nullopt; }
	virtual DrawRect GetContentRect() const { return absoluteElementRect; }

	// Lifecycle functions
	virtual bool ProcessInput(UiPassParams& params);
	virtual bool HandleBindings(UiPassParams& params);
	virtual bool RecalculateLayout(UiPassParams& params);
	virtual void DrawElement(UiPassParams& params, bool needsRedraw);

	UiFrame() = default;
public:
	void ProcessElement(UiPassParams& params);

	// Accessors
	const DrawRect& GetElementRect() const { return absoluteElementRect; }
	bool IsVisible() const { return visible; }

	// Tree manipulation
	void AddChild(std::unique_ptr<UiFrame> child);
	void RemoveChild(UiFrame* child);
	void ClearChildren();

	virtual ~UiFrame() = default;

	// ClassName
	static constexpr std::string_view className{ "UiFrame" };
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Consider deprecating UiInsets from frame:
// ^ ContentRect is used by UiTile to allow for border
// ^ Will we have UiPadding constraints?
// ^ The padding of UiTile is *not* editable, it is used internally.

// Consider replacing UDim2 with flattened struct ScaleOffset2D

// Consider how size clamping would work
// ^ No ContentRect to deal with
// ^ Visible is implicitly false if size too small
// ^ Don't render children if too small
// ^ Do we clamp negative absolute size, or do we allow reversed/flipped content?

// UiFrame:
// ^ Absolute minimum required to function, only consider ElementRect, no ContentRect
// ^ No input handling
// ^ No data binding
// ^ Basic layout calculation
// ^ No rendering

// UiShape:
// ^ Needs to draw parametrically defined rectangles with rounded corners
// ^ No input handling
// ^ No data binding
// ^ Has corners (each can be defined separately)
// ^ Has borders
// ^ Has fill
// ^ Has transparency
// ^ Has colours
// ^ Clipping mask, corners are considered, how would this function with children and nested masks?
// ^ Consider how batch rendering could work?

// UiTile:
// ^ Has ContentRect and UiEdgeInsets
// ^ Renders draggable borders for resizing
// ^ Renders scrollable tab list if contents is greater than 1
// ^ How should minimum size be handled?
// ^