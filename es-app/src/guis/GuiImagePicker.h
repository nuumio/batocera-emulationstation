#pragma once
#ifndef ES_APP_GUIS_GUI_IMAGE_PICKER_H
#define ES_APP_GUIS_GUI_IMAGE_PICKER_H

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/ImageGridComponent.h"
#include <functional>
#include <string>

class GuiImagePicker : public GuiComponent
{
public:
	GuiImagePicker(Window* window, Vector2f gridSize = Vector2f(5,3));

	~GuiImagePicker();
	bool input(InputConfig* config, Input input) override;
	virtual std::vector<HelpPrompt> getHelpPrompts() override;
	inline void onImagePicked(const std::function<void(std::string)>& func) { mOnImagedPicked = func; };
	inline void onCancel(const std::function<void()>& func) { mOnCancel = func; };

	void add(const std::string imagePath, const std::string name);
	void setCursor(const std::string imagePath);

private:
	std::function<void(std::string)> mOnImagedPicked;
	std::function<void()> mOnCancel;

	void close(bool pickSelectedImage);

	MenuComponent mMenu;
	ImageGridComponent<std::string> mGrid;
	std::shared_ptr<ThemeData> mTheme;
};

#endif // ES_APP_GUIS_GUI_IMAGE_PICKER_H
