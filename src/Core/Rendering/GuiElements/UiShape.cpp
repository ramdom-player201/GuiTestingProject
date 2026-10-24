#include "UiShape.h"

UiShape::UiShape() {
	clipChildren = true;
}

std::optional<ClipRect> UiShape::GetClipRect() const {
	ClipRect clip{};
	clip.x = absoluteElementRect.x + absoluteElementRect.width * 0.5f;
	clip.y = absoluteElementRect.y + absoluteElementRect.height * 0.5f;
	clip.halfW = absoluteElementRect.width * 0.5f;
	clip.halfH = absoluteElementRect.height * 0.5f;
	// Radii for clipping data must be clamped the same way as in DrawElement()
	float maxRadius{ std::min(absoluteElementRect.width, absoluteElementRect.height) * 0.5f };
	Vec4 clampedRadii = cornerRadii.Clamp(0.0f, maxRadius);
	clip.rTL = clampedRadii.x;
	clip.rTR = clampedRadii.y;
	clip.rBR = clampedRadii.z;
	clip.rBL = clampedRadii.w;
	return clip;
}

void UiShape::DrawElement(UiPassParams& params, bool descendantsUpdated) {
	if (params.batches == nullptr) { return; }

	//if (descendantsUpdated) {
		//LogService::Log(LogType::WIP, className, FUNC_NAME, "Consider whether UiShape can cache anything or whether it must redraw even if clean");
		//LogService::Log(LogType::CATCH, className, FUNC_NAME, "REDRAW");

		// NeedsRedraw occurs in the cases of:
		// Element resizes
		// Input indicates required change (null for shapes)
		// Data bindings require change (null for shapes)

		// Currently, a child redrawing does not trigger a redraw in parent
		// But a parent redraw does trigger its children

		// This may need to be inverted

		// Even if a specific element does not change between frames, the entire batch does need to be redrawn
		// As such, we always need to write the information needed to draw the element for a given frame
		// So, we need to push the cachedInstance to the batch, and the local clip stack to the SSBO
		// Because the SSBO's state may differ due to changes elsewhere in the tree, we cannot rely on a static Index+Length
		// As such, we must recalculate the Index+Length for the cachedInstance
		// So there is not much benefit in caching a small object and skipping a couple calculations

		// Redraw should be reserved for render-overloading elements such as UiCanvas
		// Batches are always accumulated, but we don't need to submit them to the GPU
		// If nothing in a UiCanvas's descendants changes, the UiCanvas can skip batch submission and reuse its texture from a previous frame
		// If at least one of the UiCanvas's descendants changes, the UiCanvas must redraw the texture entirely
		// The texture only needs to be replaced entirely if a resize occurs
	//}

	// Clamp radii
	float maxRadius{ std::min(absoluteElementRect.width, absoluteElementRect.height) * 0.5f };
	Vec4 clampedRadii = cornerRadii.Clamp(0.0f, maxRadius);

	// Get colours
	uint32_t packedBase = PackColour(baseColour);
	uint32_t packedOutline = PackColour(borderColour);

	// Centre for SDF calculation
	float cx{ absoluteElementRect.x + absoluteElementRect.width * 0.5f };
	float cy{ absoluteElementRect.y + absoluteElementRect.height * 0.5f };

	// Build a shape instance
	cachedInstance = {
		{ cx,cy },
		packedBase,
		packedOutline,
		{ absoluteElementRect.width,absoluteElementRect.height,borderThickness,0.0f },
		{ clampedRadii.x,clampedRadii.y,clampedRadii.z,clampedRadii.w },
		static_cast<uint32_t>(params.batches->finalClipChain.size()), // Index of new entry is end of SSBO clip chain
		static_cast<uint32_t>(params.batches->localClipStack.size())  // Length of new entry is same as local clip stack
	};
	params.batches->finalClipChain.insert(		// Inject current local clip stack into chain
		params.batches->finalClipChain.end(),	// Start at end of vector
		params.batches->localClipStack.begin(), // Inject from start of local stack
		params.batches->localClipStack.end()	// ^ to end of local stack
	);

	// Record draw instance into batch
	uint32_t instanceOffset{ static_cast<uint32_t>(params.batches->shapes.size()) };
	params.batches->shapes.push_back(cachedInstance);

	DrawRequest request;
	request.type = DrawRequestType::Shape;
	request.instanceIndex = instanceOffset;

	params.batches->renderList.push_back(request);
}