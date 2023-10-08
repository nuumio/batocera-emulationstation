#include "guis/GuiTagsManager.h"

#include "Settings.h"
#include "components/OptionListComponent.h"
#include "SystemData.h"

GuiTagsManager::GuiTagsManager(Window* window, SystemData* system) : GuiComponent(window), mMenu(window, _("TAGS MANAGER")), mSystem(system)
{
	mCurrentTagSelect = std::make_shared<OptionListComponent<std::string>>(mWindow, _("CURRENT TAG"), false);
	initializeMenu();
}

void GuiTagsManager::initializeMenu()
{
	addChild(&mMenu);

	addCurrentTagSelectorToMenu();

	mMenu.addButton(_("BACK"), "back", std::bind(&GuiTagsManager::close, this));

	if (Renderer::isSmallScreen())
		mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, (Renderer::getScreenHeight() - mMenu.getSize().y()) / 2);
	else
		mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

GuiTagsManager::~GuiTagsManager()
{
}

bool GuiTagsManager::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;

	if(config->isMappedTo(BUTTON_BACK, input) && input.value != 0)
		close();

	return false;
}

std::vector<HelpPrompt> GuiTagsManager::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt(BUTTON_BACK, _("BACK")));
	return prompts;
}

void GuiTagsManager::close()
{
	auto currentTag = mCurrentTagSelect->getSelected();
	if (Settings::getInstance()->setCurrentTag(currentTag))
	{
		Settings::getInstance()->saveFile();
	}

	auto finalize = mOnFinalizeFunc;

	delete this;

	if (finalize != nullptr)
		finalize();
}

void GuiTagsManager::addCurrentTagSelectorToMenu()
{
	auto currentTag = Settings::getInstance()->getCurrentTag();
	mCurrentTagSelect->add(_("<no tag>"), "", currentTag.empty());
	for (auto tag : Settings::getInstance()->getKnownTags())
	{
		mCurrentTagSelect->add(tag, tag, currentTag == tag);
	}

	mMenu.addWithLabel(_("CURRENT TAG"), mCurrentTagSelect);
}
