#include "guis/GuiTagsManager.h"

#include "Log.h"
#include "Settings.h"
#include "components/OptionListComponent.h"
#include "SystemData.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "guis/GuiMsgBox.h"
#include "FileData.h"

GuiTagsManager::GuiTagsManager(Window* window, SystemData* system) : GuiComponent(window), mMenu(window, _("TAGS MANAGER")), mSystem(system)
{
	mCurrentTagSelect = std::make_shared<OptionListComponent<std::string>>(mWindow, _("CURRENT TAG"), false);
	initializeMenu();
}

void GuiTagsManager::initializeMenu()
{
	addChild(&mMenu);

	addCurrentTagSelectorToMenu();

	auto addNewTag = [this](const std::string& newTag)
	{
		auto trimmed = Utils::String::trim(newTag);
		// only reason to ban < and > is to keep "<no tag>" clearly distinguishable from real tags.
		if (trimmed.find_first_of("<>") != std::string::npos || trimmed.empty())
		{
			mWindow->displayNotificationMessage(_U("\uF05E ") +
				Utils::String::format(_(
					"INVALID TAG: %s").c_str(), trimmed.c_str()), 4000);
			return;
		}
		auto settings = Settings::getInstance();
		if (settings->isKnownTag(trimmed))
		{
			mWindow->displayNotificationMessage(_U("\uF05E ") +
				Utils::String::format(_(
					"TAG ALREADY EXISTS: %s").c_str(), trimmed.c_str()), 4000);
			return;
		}
		settings->addKnownTag(trimmed);
		settings->saveFile();
		mWindow->displayNotificationMessage(_U("\uF02B ") +
			Utils::String::format(_(
				"TAG ADDED: %s").c_str(), trimmed.c_str()), 4000);
		updateCurrentTagSelector();
	};

	auto deleteCurrentTag = [this]()
	{
		auto settings = Settings::getInstance();
		auto tagToDelete = settings->getCurrentTag();
		if (tagToDelete.empty())
			return;

		SystemData::forEachSystem([tagToDelete](SystemData* system){
			if (!system->isCollection())
			{
				auto items = system->getRootFolder()->getFilesRecursive(GAME | FOLDER);
				for (auto data : items)
				{
					data->getMetadata().removeTag(tagToDelete);
				}
			}
		});

		if(settings->deleteTag(tagToDelete))
			settings->saveFile();

		mWindow->displayNotificationMessage(_U("\uF014 ") +
			Utils::String::format(_(
				"TAG DELETED: %s").c_str(), tagToDelete.c_str()), 4000);
		updateCurrentTagSelector();
	};

	mMenu.addEntry(_("ADD TAG"), true, [this, addNewTag]
	{
		if (Settings::getInstance()->getBool("UseOSK"))
			mWindow->pushGui(new GuiTextEditPopupKeyboard(mWindow, _("ADD TAG"), "", addNewTag, false));
		else
			mWindow->pushGui(new GuiTextEditPopup(mWindow, _("ADD TAG"), "", addNewTag, false));
	});

	mMenu.addEntry(_("DELETE CURRENT TAG"), true, [this, deleteCurrentTag]
	{
		auto settings = Settings::getInstance();
		if (settings->getCurrentTag().empty())
			return;
		mWindow->pushGui(new GuiMsgBox(mWindow, _("REALLY DELETE CURRENT TAG?\nIt will be removed from everywhere."), _("YES"), [this, deleteCurrentTag]
		{
			mWindow->pushGui(new GuiMsgBox(mWindow, _("REALLY REALLY DELETE CURRENT TAG?\nIt will be removed from known tags,\ngamelists and tag rule sets.\nThis action cannot be undone!"), _("YES"), [this, deleteCurrentTag]
			{
				deleteCurrentTag();
			}, 
			_("NO"), nullptr, ICON_WARNING, false, true, 2000));
		}, 
		_("NO"), nullptr, ICON_WARNING, false, true, 2000));
	});

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
	updateCurrentTagSelector();
	mMenu.addWithLabel(_("CURRENT TAG"), mCurrentTagSelect);
}

void GuiTagsManager::updateCurrentTagSelector()
{
	auto currentTag = Settings::getInstance()->getCurrentTag();
	mCurrentTagSelect->clear();
	mCurrentTagSelect->add(_("<no tag>"), "", currentTag.empty());
	for (auto tag : Settings::getInstance()->getKnownTags())
	{
		mCurrentTagSelect->add(tag, tag, currentTag == tag);
	}	
}
