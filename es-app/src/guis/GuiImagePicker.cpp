#include "guis/GuiImagePicker.h"

#include "Log.h"
#include "Settings.h"
#include "components/OptionListComponent.h"
#include "SystemData.h"
#include "guis/GuiTagRuleEditor.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "guis/GuiMsgBox.h"
#include "FileData.h"
#include "views/ViewController.h"

GuiImagePicker::GuiImagePicker(Window* window, Vector2f gridSize) : GuiComponent(window), mMenu(window, _("IMAGE PICKER")), mGrid(window)
{
	setPosition(0, 0);
	setSize(Renderer::getScreenWidth(), Renderer::getScreenHeight());
	mGrid.setPosition(0, 0);
	mGrid.setSize(mSize);
	mGrid.setGridSizeOverride(gridSize);
	mGrid.setDefaultZIndex(200);

	addChild(&mGrid);

	auto theme = ThemeData::getMenuTheme();
	float sh = (float)Math::min(Renderer::getScreenHeight(), Renderer::getScreenWidth());
	sh = (float) theme->TextSmall.font->getSize() / sh;

	std::string xml =
		"<theme defaultView=\"Tiles\">"
		"<formatVersion>7</formatVersion>"
		"<view name = \"grid\">"
		"<imagegrid name=\"gamegrid\">"
		"  <margin>0 0</margin>"
		"  <padding>0.02 0.05</padding>"
		"  <pos>0 0</pos>"
		"  <size>1 1</size>"
		"  <scrollDirection>vertical</scrollDirection>"
		"  <autoLayout>1 1</autoLayout>"
		"  <autoLayoutSelectedZoom>1</autoLayoutSelectedZoom>"
		"  <animateSelection>false</animateSelection>"
		"  <centerSelection>false</centerSelection>"
		"  <scrollLoop>true</scrollLoop>"
		"  <showVideoAtDelay>10</showVideoAtDelay>"
		"</imagegrid>"
		"<gridtile name=\"default\">"
		"  <backgroundColor>FFFFFF00</backgroundColor>"
		"  <selectionMode>image</selectionMode>"
		"  <padding>4 4</padding>"
		"  <margin>1 1</margin>"
		"  <imageColor>FFFFFFFF</imageColor>"
		"</gridtile>"
		"<gridtile name=\"selected\">"
		"  <backgroundColor>22FF22FF</backgroundColor>"
		"</gridtile>"
		"<text name=\"gridtile\">"
		"  <color>" + Utils::String::toHexString(theme->Text.color) + "</color>"
		"  <backgroundColor>00000000</backgroundColor>"
		"  <fontPath>" + theme->Text.font->getPath() +"</fontPath>"
		"  <fontSize>" + std::to_string(sh) + "</fontSize>"
		"  <alignment>center</alignment>"
		"  <singleLineScroll>false</singleLineScroll>"
		"  <size>1 0.30</size>"
		"  <padding>0.03 0</padding>"
		"</text>"
		"<text name=\"gridtile:selected\">"
		"  <color>" + Utils::String::toHexString(theme->Text.selectedColor) + "</color>"
		"</text>"
		"<image name=\"gridtile.image\">"
		"  <linearSmooth>true</linearSmooth>"
		"</image>"
		"</view>"
		"</theme>";

	mTheme = std::shared_ptr<ThemeData>(new ThemeData());
	std::map<std::string, std::string> emptyMap;
	mTheme->loadFile("imageviewer", emptyMap, xml, false);

	mGrid.applyTheme(mTheme, "grid", "gamegrid", 0);

	animateTo(Vector2f(0, Renderer::getScreenHeight()), Vector2f(0.05, 0.05));
}

GuiImagePicker::~GuiImagePicker()
{
}

bool GuiImagePicker::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;

	if(config->isMappedTo(BUTTON_OK, input) && input.value != 0)
		close(true);

	if(config->isMappedTo(BUTTON_BACK, input) && input.value != 0)
		close(false);

	return false;
}

std::vector<HelpPrompt> GuiImagePicker::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt(BUTTON_OK, _("PICK IMAGE")));
	prompts.push_back(HelpPrompt(BUTTON_BACK, _("BACK")));
	return prompts;
}

void GuiImagePicker::close(bool pickSelectedImage)
{
	auto onImagePicked = mOnImagedPicked;
	auto window = mWindow;
	auto selected = mGrid.getSelected();

	delete this;

	if (pickSelectedImage && onImagePicked != nullptr)
		onImagePicked(selected);
}

void GuiImagePicker::add(const std::string imagePath, const std::string name)
{
	mGrid.add(name, imagePath, imagePath, "", false, false, false, false, imagePath);
}

void GuiImagePicker::setCursor(const std::string imagePath)
{
	mGrid.resetLastCursor();
	mGrid.setCursor(imagePath);
	mGrid.onShow();
}
