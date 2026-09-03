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
		uint32_t packedBase = PackColour(baseColour);
		uint32_t packedOutline = PackColour(borderColour);

		float x0 = absoluteElementRect.x;
		float y0 = absoluteElementRect.y;
		float x1 = absoluteElementRect.x + absoluteElementRect.width;
		float y1 = absoluteElementRect.y + absoluteElementRect.height;

		// Centre for SDF calculation
		float cx{ x0 + absoluteElementRect.width * 0.5f };
		float cy{ y0 + absoluteElementRect.height * 0.5f };

		// Top left
		cachedVertices[0] = {
			{x0, y0},{cx, cy},
			packedBase, packedOutline,
			{absoluteElementRect.width, absoluteElementRect.height, borderThickness, 0.0f},
			{cornerRadii.x, cornerRadii.y, cornerRadii.z, cornerRadii.w}
		};
		// Top right
		cachedVertices[1] = {
			{x1, y0}, {cx, cy},
			packedBase, packedOutline,
			{absoluteElementRect.width, absoluteElementRect.height, borderThickness, 0.0f},
			{cornerRadii.x, cornerRadii.y, cornerRadii.z, cornerRadii.w}
		};
		// Bottom right
		cachedVertices[2] = {
			  {x1, y1}, {cx, cy},
			packedBase, packedOutline,
			{absoluteElementRect.width, absoluteElementRect.height, borderThickness, 0.0f},
			{cornerRadii.x, cornerRadii.y, cornerRadii.z, cornerRadii.w}
		};
		// Bottom left
		cachedVertices[3] = {
			{x0, y1}, {cx, cy},
			packedBase, packedOutline,
			{absoluteElementRect.width, absoluteElementRect.height, borderThickness, 0.0f},
			{cornerRadii.x, cornerRadii.y, cornerRadii.z, cornerRadii.w}
		};
	}

	// always draw
	auto& bucket = params.batches->zBuckets[params.currentZBucket];
	for (int i{ 0 }; i < 4; i++) {
		bucket.shapes.push_back(cachedVertices[i]);
	}
}