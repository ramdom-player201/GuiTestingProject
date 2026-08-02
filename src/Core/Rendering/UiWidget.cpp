#include "UiWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// TREE MANAGEMENT
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void UiWidget::AddChild(std::unique_ptr<UiWidget> child) {
	if (!child) { return; }

	child->parent = this;
	// When adding a child to a parent, the layout becomes invalid
	SetFlag(child->flags, UiWidgetFlags::LayoutDirty, true);
	children.push_back(std::move(child));
}

void UiWidget::RemoveChild(UiWidget* child) {
	if (!child) { return; }

	auto it = std::find_if(
		children.begin(), children.end(),
		[child](const std::unique_ptr<UiWidget>& ptr) {return ptr.get() == child;}
	)

		// Not sure why auto-format indents this up here
		if (it != children.end()) {
			(*it)->parent = nullptr;
			children.erase(it);
		}
}

UiWidget* UiWidget::FindWidgetById(const std::string& searchId) {
	if (id == searchId) { return this; }

	for (auto& child : children) {
		UiWidget* found = child->FindWidgetById(searchId);
		if (found) { return found; }
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CORE LIFECYCLE
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void UiWidget::CalculateLayout(DrawRect parentAbsoluteRect, float globalUiScale) {
	// Optimisation: if this is not dirty, skip the recalculation, but still call on children incase there is a change downstream
	if (!HasFlag(flags, UiWidgetFlags::LayoutDirty)) {
		for (auto& child : children) {
			child->CalculateLayout(absoluteRect, globalUiScale);
		}
		return;
	}
	// Else (this layout is dirty)

	absoluteRect.width = (size.x.scale * parentAbsoluteRect.width) + (size.x.offset * globalUiScale);
	absoluteRect.height = (size.y.scale * parentAbsoluteRect.height) + (size.y.offset * globalUiScale);

	float baseX{ (position.x.scale * parentAbsoluteRect.width) + (position.x.offset * globalUiScale) };
	float baseY{ (position.y.scale * parentAbsoluteRect.height) + (position.y.offset * globalUiScale) };

	absoluteRect.x = baseX - (anchorPoint.x * absoluteRect.width);
	absoluteRect.y = baseY - (anchorPoint.y * absoluteRect.height);

	SetFlag(flags, UiWidgetFlags::LayoutDirty, false);

	for (auto& child : children) {
		child->CalculateLayout(absoluteRect, globalUiScale);
	}
}

void UiWidget::Render() const {
	// The root widget renders nothing, child subclasses should override this
}

InputEventResult UiWidget::ProcessInput(const InputEvent& event, DrawRect parentAbsoluteRect) {
	// Root widget consumes no input, child subclasses should override this
	return InputEventResult();
}