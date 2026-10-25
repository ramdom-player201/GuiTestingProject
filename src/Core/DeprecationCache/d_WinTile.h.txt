#pragma once

#include "UiPanel.h"
#include <memory>

enum class TileType {
	side,				// left|right -> upper,lower,corner
	centred				// centre|bottom -> left,right
};

enum class TileSubplacement {
	upper,
	lower,
	corner,
	left,
	right,
};

enum class TileInsertPoint {
	top, 
	bottom,
	left,
	right,
	into
};

class WinTile {
private:

public:
	WinTile(TileType type);

	void AddPanel(std::shared_ptr<UiPanel> panel, TileInsertPoint insertPoint);	// insert panel into tile at position
	void RemovePanel(std::shared_ptr<UiPanel> panel);							// remove panel from tile
	std::shared_ptr<UiPanel> GetPanelAtLocation(TileSubplacement location);
};