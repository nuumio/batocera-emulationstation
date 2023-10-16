#include "guis/GuiTagRuleEditor.h"

#include "Log.h"
#include "Settings.h"
#include "components/OptionListComponent.h"
#include "SystemData.h"
#include "components/SwitchComponent.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "guis/GuiMsgBox.h"
#include "FileData.h"

GuiTagRuleEditor::GuiTagRuleEditor(Window* window, TagRuleSet* ruleSet) : GuiComponent(window), mMenu(window, _("RULE SET EDITOR"))
{
	mRuleSet = ruleSet;
	initializeMenu();
}

void GuiTagRuleEditor::initializeMenu()
{
	addChild(&mMenu);
	addRuleSetName(Utils::String::format(_("RULE SET: %s").c_str(), mRuleSet->getName().c_str()));

	addTagsToMenu(true);
	addTagsToMenu(false);

	mMenu.addButton(_("BACK"), "back", std::bind(&GuiTagRuleEditor::close, this));

	if (Renderer::isSmallScreen())
		mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, (Renderer::getScreenHeight() - mMenu.getSize().y()) / 2);
	else
		mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

GuiTagRuleEditor::~GuiTagRuleEditor()
{
}

bool GuiTagRuleEditor::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;

	if(config->isMappedTo(BUTTON_BACK, input) && input.value != 0)
		close();

	return false;
}

std::vector<HelpPrompt> GuiTagRuleEditor::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt(BUTTON_BACK, _("BACK")));
	return prompts;
}

void GuiTagRuleEditor::close()
{
	// NOTE: Settings are saved from GuiTagManager::close()
	auto finalize = mOnFinalizeFunc;

	delete this;

	if (finalize != nullptr)
		finalize();
}

void GuiTagRuleEditor::addTagsToMenu(bool include)
{
	mMenu.addEntry(_(include ? "INCLUDE TAGS" : "EXCLUDE TAGS"));
	auto settings = Settings::getInstance();
	auto knownTags = settings->getKnownTags();
	auto tagSet = include ? mRuleSet->getIncludeTags() : mRuleSet->getExcludeTags();
	for (auto tag : knownTags)
	{
		addTagSwitch(tag, tagSet.find(tag) != tagSet.end(), [this, tag, include](bool state)
		{
			auto settings = Settings::getInstance();
			if (state)
			{
				if (include)
					settings->addIncludeTag(tag, mRuleSet->getName());
				else
					settings->addExcludeTag(tag, mRuleSet->getName());
			}
			else
			{
				if (include)
					settings->removeIncludeTag(tag, mRuleSet->getName());
				else
					settings->removeExcludeTag(tag, mRuleSet->getName());
			}
		});
	}
}

void GuiTagRuleEditor::addRuleSetName(std::string ruleSetName)
{
	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	ComponentListRow row;

	auto text = std::make_shared<TextComponent>(mWindow, ruleSetName, font, color);
	row.addElement(text, true);
	text->setHorizontalAlignment(Alignment::ALIGN_CENTER);

	mMenu.addRow(row);
}

void GuiTagRuleEditor::addTagSwitch(std::string tag, bool switchState, const std::function<void(bool state)>& onChangeCallback)
{
		auto tagSwitch = std::make_shared<SwitchComponent>(mWindow);
		tagSwitch->setState(switchState);
		tagSwitch->setOnChangedCallback([onChangeCallback, tagSwitch]()
		{
			onChangeCallback(tagSwitch->getState());
		});

		auto theme = ThemeData::getMenuTheme();

		ComponentListRow row;

		auto text = std::make_shared<TextComponent>(mWindow, tag, theme->Text.font, theme->Text.color);
		text->setPadding(Vector4f(Renderer::getScreenWidth()*0.01, 0, 0, 0));
		row.addElement(text, true);

		row.addElement(tagSwitch, false);

		mMenu.addRow(row);
}
