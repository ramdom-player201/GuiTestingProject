#include "UiFrame.h"

#include <algorithm>

bool UiFrame::ProcessInput(UiPassParams& params) {
	return false; // Frame does not take input
}

bool UiFrame::HandleBindings(UiPassParams& params) {
	return false; // Frame does not have bindings
}

bool UiFrame::RecalculateLayout(UiPassParams& params) {
	// Recalculate layout and return true for redraw if updated
	float resolvedX = params.parentContentRect.x
		+ (params.parentContentRect.width * position.scaleX)
		+ (position.offsetX * params.guiScaleFactor);

	float resolvedY = params.parentContentRect.y
		+ (params.parentContentRect.height * position.scaleY)
		+ (position.offsetY * params.guiScaleFactor);

	float resolvedW = (params.parentContentRect.width * size.scaleX)
		+ (size.offsetX * params.guiScaleFactor);

	float resolvedH = (params.parentContentRect.height * size.scaleY)
		+ (size.offsetY * params.guiScaleFactor);

	DrawRect newElementRect{ resolvedX,resolvedY,resolvedW,resolvedH };
	bool layoutChanged{ newElementRect != absoluteElementRect };
	absoluteElementRect = newElementRect;
	return layoutChanged;
}

void UiFrame::DrawElement(UiPassParams& params, bool needsRedraw) {
	return; // Frame has nothing to draw
}

// ==================================================

void UiFrame::ProcessElement(UiPassParams& params) {
	if (!visible) { return; } // Do not process if hidden

	// Layout
	bool layoutChangedThisFrame{ RecalculateLayout(params) };
	params.layoutChanged |= layoutChangedThisFrame;

	// Push clip if element clips
	bool isClipping{ false };
	if (clipChildren) { // Element has clipping enabled
		auto clip = GetClipRect();
		if (clip) { // Element has clipable bounds
			params.clipStack.rects.push_back(*clip);
			isClipping = true;
		}
	}

	// Process Children
	UiPassParams childParams = params;
	childParams.parentContentRect = GetContentRect();

	uint32_t currentDepth = params.clipStack.Count();
	childParams.currentZBucket = currentDepth;

	for (auto& child : children) {
		child->ProcessElement(childParams);
	}

	// Pop clip from stack
	if (isClipping) {
		params.clipStack.rects.pop_back();
	}

	// Input (process inputs from leaf nodes back up the tree)
	params.layoutChanged |= ProcessInput(params);

	// Bindings
	params.layoutChanged |= HandleBindings(params);

	// Draw
	DrawElement(params, params.layoutChanged);

	// Propagate results
	params.result = childParams.result; // May or may not be deprecated
	params.activeTarget = childParams.activeTarget; // Persists the full update loop across all BaseWindows
	params.layoutChanged |= childParams.layoutChanged; // Determines whether to redraw the gui root
}

// ==================================================

void UiFrame::AddChild(std::unique_ptr<UiFrame> child) {
	child->parent = this;
	children.push_back(std::move(child));
}

void UiFrame::RemoveChild(UiFrame* child) {
	children.erase(
		std::remove_if(children.begin(), children.end(),
			[child](const std::unique_ptr<UiFrame>& c) { return c.get() == child; }
		),
		children.end()
	);
}

void UiFrame::ClearChildren() {
	children.clear();
}

// TODO: update ProcessElement such that the children iteration loop comes before processing.
// ^ We might need certain elements such as UiScrollingFrame to take into account children in its drawing.
// ^ Eg, a UiScrollingFrame might override the render batches such that children write their draw data to UiScrollingFrame's batch instead of the default batch.
// ^ That way, UiScrollingFrame can generate its own VkImage texture using its children.
// ^ Should ProcessElement be made virtual so that UiScrollingFrame can access the batches, or should we add a new function or property for it?
// ^ params.layoutChanged needs to propogate upwards, so the root can decide whether to redraw the gui or keep the last frame.
// ^ UiScrollingFrame could intercept that to decide whether to redraw its contents, in addition to its own needsRedraw.
// ^ needsRedraw does not need to propogate downwards, it is only used for regenerating cache of heavy elements, as all elements must redraw to some extent.
// ^ Input must be processed top to bottom, as top-level elements can sink input. Since inputs can be persistent, we cannot process them bottom-up with overwriting.
// ^ ScrollingFrame might need custom iteration logic, to handle when it has more children than are visible.
// ^ Might we have UiConstraints (eg AspectRatio) which override the properties of their parent? This also requires the order to be updated as above.

// TODO: consider how to handle descendants clipping.
// ^ UiScrollingFrame clips the children which are scrolled outside its bounds.
// ^ UiShape has rounded corners, which may need special clipping logic; how does this affect the processing of nested children?
// ^ Are there cases where we wouldn't want clipping?
// ^ A UiSlider element's notch would be wider than its slide bar; but it is also one element, so it would likely have larger bounds, with a transparent background.
// ^ UiSlider is a single element, so it handles the drawing of both the bar and notch together, rather than as a child-parent in the tree
// ^ Do we want to support element rotation?
// ^ How does input handle clipping?  Eg: a button is partially obscurred inside a UiScrollingFrame
// ^ Should the UI elements modify UiPassParams to block input handling if outside the parent bounds? This is likely better than applying the mask to the input internally.
// ^ Non-clicking input should still be let through if the activeTarget is still set to that element.

// TODO: evaluate how versatile to make the gui
// ^ Do we want this UI to be highly scriptable to support custom logic that spreads across the tree?
// ^ Or do we want the UI to be rigid, with logic isolated to dedicated classes.
// ^ Data bindings allow us to make elements dynamic, based on the status of another object.
// ^ How do we define data bindings?
// ^ A properties UiPanel's UiScrollingFrame needs to be dynamically populated with the explicit properties of the selected object.
// ^ Do we have a factory class that generates a Layout json file for the gui for the given object from an external source?
// ^ Or do we allow the properties panel to have scripted logic to load based on the selected object?
// ^ I think a factory class that takes control over the panel by reloading its contents manually through AddChild/RemoveChild/ClearChildren might be best here.

// TODO: consider batching and rendering order.
// ^ Opaque and transparent objects might be in different batches.
// ^ We might need additional batches for textures and text.
// ^ These elements are heavy and should not be regenerated in their owning elements (UiTexture and UiTextLabel), but do they need to be batched separately?
// ^ They are cached in their elements, and the cache is drawn every frame.
// ^ If an element (eg UiScrollingFrame) takes over rendering of its children, it might override their batches and draw to a texture with internal draw commands.
// ^ If opaque and transparent objects are drawn over each other in a nested hierarchy, we would either need them in the same batch, or with z-index ordering.
// ^ We cannot rely on batching everything in just a single call if textures and text are not compatible.
// ^ Certain renderable elements (eg Viewports) have deferred rendering. The GuiLayout just stores a bounding box used for input catching, but passes this outside the class for external handling.

// TODO: how to handle transparent elements?
// ^ The main program gui doesn't need to handle transparency in most cases.
// ^ UiTile may apply transparent gradients over edges when dragged, but this is not expected to be occluded, except by overlays which are already drawn separately.
// ^ If GuiLayout is used elsewhere, such as used inside a Viewport, or projected onto a mesh; transparency may still be important.
// ^ Should elements inherit transparency?
// ^ If ObjectA contains ObjectB, A has partial transparency and B is opaque. Should B be rendered onto A and then have transparency applied to both? Or should B just be opaque.
// ^ We might want to consdier a UiCanvas object, which would hijack rendering of its children into a texture that transparency can be applied onto in post

// TODO: consider the nature of GuiLayout.
// ^ Should GuiLayout natively have both overlay (tooltips) and base layer? Overlays may not be needed outside of the main gui case.
// ^ We could store 2 GuiLayouts inside LayoutCompositor, one for base and one for overlay.
// ^ However, LayoutCompositor would need to handle input state management from Overlay layer to Base layer.
// ^ LayoutCompositor already handles the passing of inputs from GuiLayout into the Viewports.
// ^ How do we get bounding boxes for deferred elements (Viewports) out of the gui?
// ^ Overlays don't need to contain Viewports. While UiViewportRect could theoretically be injected into an Overlay, it should just render a blank square if the linked Viewport doesn't exist.
// ^ Do we want to be able to support more deferred elements than just Viewports? (Eg large text editors -> scripting editors)
// ^ Are there any cases where an overlay layer might require a deferred element?
// ^ How would LayoutCompositor handle it if the overlay did support deferred elements? Should we just ignore the getter for deferred panel rects so they render as blank?

// TODO: consider batches and pipelines
// ^ Does each batch needs its own pipeline?
// ^ If some elements (eg UiScrollingFrame, UiCanvas) overload rendering for their children, would this lead to duplicated rendering logic from the GuiLayout's main body?
// ^ Should we separate the rendering logic for each batch into its own Pipeline/BatchRenderer class? This would allow us to reuse rendering code within overloading elements.
// ^ It would also help keep the GuiLayout compact and easy to navigate.