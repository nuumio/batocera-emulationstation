#pragma once

#include "GuiComponent.h"

class Window;

class MultiStateInput
{
public:
	MultiStateInput(const std::string& buttonName);

	bool isShortPressed(InputConfig* config, Input input);
	bool isLongPressed(int deltaTime);
	bool isDown(InputConfig* config, Input input);
	// Call release* methods to reset down/pressed state for example when another component
	// is opened and catches the input up event or down/pressed might otherwise get mixed.
	void releaseDown() {mIsDown = false;};
	void releasePressed() {mIsPressed = false;};

private:
	std::string mButtonName;
	int mTimeHoldingButton;
	bool mIsPressed;
	bool mIsDown;
};