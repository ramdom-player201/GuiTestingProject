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
	clip.rTL = cornerRadii.x;
	clip.rTR = cornerRadii.y;
	clip.rBR = cornerRadii.z;
	clip.rBL = cornerRadii.w;
	return clip;
}

void UiShape::DrawElement(UiPassParams& params, bool needsRedraw) {
	if (params.batches == nullptr) { return; }

	// only recalculate if dirty
	if (needsRedraw) {

		// Clamp radii
		float maxRadius{ std::min(absoluteElementRect.width, absoluteElementRect.height) * 0.5f };
		Vec4 clampedRadii = cornerRadii.Clamp(0.0f,maxRadius);

		// Get colours
		uint32_t packedBase = PackColour(baseColour);
		uint32_t packedOutline = PackColour(borderColour);

		// Get four vertices
		float x0 = absoluteElementRect.x;
		float y0 = absoluteElementRect.y;
		float x1 = absoluteElementRect.x + absoluteElementRect.width;
		float y1 = absoluteElementRect.y + absoluteElementRect.height;

		// Centre for SDF calculation
		float cx{ x0 + absoluteElementRect.width * 0.5f };
		float cy{ y0 + absoluteElementRect.height * 0.5f };

		LogService::Log(LogType::CATCH, className, FUNC_NAME, "REDRAW");

		// Top left
		cachedVertices[0] = {
			{x0, y0},{cx, cy},
			packedBase, packedOutline,
			{absoluteElementRect.width, absoluteElementRect.height, borderThickness, 0.0f},
			{clampedRadii.x,clampedRadii.y,clampedRadii.z,clampedRadii.w}
		};
		// Top right
		cachedVertices[1] = {
			{x1, y0}, {cx, cy},
			packedBase, packedOutline,
			{absoluteElementRect.width, absoluteElementRect.height, borderThickness, 0.0f},
			{clampedRadii.x,clampedRadii.y,clampedRadii.z,clampedRadii.w}
		};
		// Bottom right
		cachedVertices[2] = {
			  {x1, y1}, {cx, cy},
			packedBase, packedOutline,
			{absoluteElementRect.width, absoluteElementRect.height, borderThickness, 0.0f},
			{clampedRadii.x,clampedRadii.y,clampedRadii.z,clampedRadii.w}
		};
		// Bottom left
		cachedVertices[3] = {
			{x0, y1}, {cx, cy},
			packedBase, packedOutline,
			{absoluteElementRect.width, absoluteElementRect.height, borderThickness, 0.0f},
			{clampedRadii.x,clampedRadii.y,clampedRadii.z,clampedRadii.w}
		};
	}

	// always draw
	auto& bucket = params.batches->zBuckets[params.currentZBucket];
	bucket.clipStack = params.clipStack;

	bucket.shapes.push_back(cachedVertices[0]); // TL
	bucket.shapes.push_back(cachedVertices[1]); // TR
	bucket.shapes.push_back(cachedVertices[3]); // BL (Order for CCW)

	bucket.shapes.push_back(cachedVertices[1]); // TR
	bucket.shapes.push_back(cachedVertices[2]); // BR
	bucket.shapes.push_back(cachedVertices[3]); // BL
}