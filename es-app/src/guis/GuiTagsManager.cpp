#include "guis/GuiTagsManager.h"

#include "Log.h"
#include "Settings.h"
#include "components/OptionListComponent.h"
#include "SystemData.h"
#include "guis/GuiTagRuleEditor.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "guis/GuiMsgBox.h"
#include "FileData.h"

GuiTagsManager::GuiTagsManager(Window* window, SystemData* system) : GuiComponent(window), mMenu(window, _("TAGS MANAGER")), mSystem(system)
{
	mCurrentTagSelect = std::make_shared<OptionListComponent<std::string>>(mWindow, _("CURRENT TAG"), false, false, false);
	mCurrentRuleSetSelect = std::make_shared<OptionListComponent<std::string>>(mWindow, _("CURRENT RULE SET"), false, false, false);
	initializeMenu();
}

void GuiTagsManager::initializeMenu()
{
	addChild(&mMenu);

	addCurrentTagSelectorToMenu();
	addAddTagToMenu();
	addDeleteTagToMenu();

	addCurrentRuleSetSelectorToMenu();
	addEditRuleSetToMenu();
	addAddRuleSetToMenu();
	addDeleteRuleSetToMenu();

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
	auto settings = Settings::getInstance();
	auto currentTag = mCurrentTagSelect->getSelected();
	settings->setCurrentTag(currentTag);
	auto currentRuleSet = mCurrentRuleSetSelect->getSelected();
	settings->setCurrentRuleSet(currentRuleSet);
	settings->saveFile();

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

void GuiTagsManager::addAddTagToMenu()
{
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

	mMenu.addEntry(_("ADD TAG"), true, [this, addNewTag]
	{
		if (Settings::getInstance()->getBool("UseOSK"))
			mWindow->pushGui(new GuiTextEditPopupKeyboard(mWindow, _("ADD TAG"), "", addNewTag, false));
		else
			mWindow->pushGui(new GuiTextEditPopup(mWindow, _("ADD TAG"), "", addNewTag, false));
	});
}

void GuiTagsManager::addDeleteTagToMenu()
{
	auto deleteCurrentTag = [this]()
	{
		auto settings = Settings::getInstance();
		auto tagToDelete = mCurrentTagSelect->getSelected();
		settings->setCurrentTag(tagToDelete);
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

	mMenu.addEntry(_("DELETE CURRENT TAG"), true, [this, deleteCurrentTag]
	{
		auto tagToDelete = mCurrentTagSelect->getSelected();
		auto settings = Settings::getInstance();
		if (tagToDelete.empty())
			return;
		mWindow->pushGui(new GuiMsgBox(mWindow, Utils::String::format(_("REALLY DELETE CURRENT TAG?\nCURRENT TAG IS: %s\nIT WILL BE REMOVED FROM EVERYWHERE.").c_str(), tagToDelete.c_str()), _("YES"), [this, tagToDelete, deleteCurrentTag]
		{
			mWindow->pushGui(new GuiMsgBox(mWindow, Utils::String::format(_("REALLY REALLY DELETE CURRENT TAG?\nCURRENT TAG IS: %s\nIT WILL BE REMOVED FROM KNOWN TAGS,\nGAMELISTS ANS TAG RULE SETS.\nTHIS ACTION CANNOT BE UNDONE!").c_str(), tagToDelete.c_str()), _("YES"), [this, deleteCurrentTag]
			{
				deleteCurrentTag();
			}, 
			_("NO"), nullptr, ICON_WARNING, false, true, 2000));
		}, 
		_("NO"), nullptr, ICON_WARNING, false, true, 2000));
	});
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

void GuiTagsManager::addCurrentRuleSetSelectorToMenu()
{
	updateCurrentRuleSetSelector();
	mMenu.addWithLabel(_("CURRENT RULE SET"), mCurrentRuleSetSelect);
}

void GuiTagsManager::addEditRuleSetToMenu()
{
	mMenu.addEntry(_("EDIT RULE SET"), true, [this]()
	{
		auto currentRuleSetName = mCurrentRuleSetSelect->getSelected();
		auto currentRuleSet = Settings::getInstance()->getRuleSet(currentRuleSetName);
		if (currentRuleSet == nullptr)
			return;
		GuiTagRuleEditor* gtm = new GuiTagRuleEditor(mWindow, currentRuleSet);
		mWindow->pushGui(gtm);
	});
}

void GuiTagsManager::addAddRuleSetToMenu()
{
	auto addNewRuleSet = [this](const std::string& newRuleSet)
	{
		auto trimmed = Utils::String::trim(newRuleSet);
		// only reason to ban < and > is to keep "<default>" clearly distinguishable from real rule sets.
		if (trimmed.find_first_of("<>") != std::string::npos || trimmed.empty())
		{
			mWindow->displayNotificationMessage(_U("\uF05E ") +
				Utils::String::format(_(
					"INVALID RULE SET NAME: %s").c_str(), trimmed.c_str()), 4000);
			return;
		}
		auto settings = Settings::getInstance();
		if (settings->hasRuleSet(trimmed))
		{
			mWindow->displayNotificationMessage(_U("\uF05E ") +
				Utils::String::format(_(
					"RULE SET ALREADY EXISTS: %s").c_str(), trimmed.c_str()), 4000);
			return;
		}

		bool wasAdded;
		TagRuleSet *addedRuleSet;
		std::tie(wasAdded, addedRuleSet) = settings->addRuleSet(trimmed);
		if (!wasAdded)
			return;

		settings->saveFile();
		mWindow->displayNotificationMessage(_U("\uF02B ") +
			Utils::String::format(_(
				"RULE SET ADDED: %s").c_str(), trimmed.c_str()), 4000);
		updateCurrentRuleSetSelector();
	};

	mMenu.addEntry(_("ADD RULE SET"), true, [this, addNewRuleSet]
	{
		if (Settings::getInstance()->getBool("UseOSK"))
			mWindow->pushGui(new GuiTextEditPopupKeyboard(mWindow, _("ADD RULE SET"), "", addNewRuleSet, false));
		else
			mWindow->pushGui(new GuiTextEditPopup(mWindow, _("ADD RULE SET"), "", addNewRuleSet, false));
	});
}

void GuiTagsManager::addDeleteRuleSetToMenu()
{
	auto deleteCurrentRuleSet = [this]()
	{
		auto settings = Settings::getInstance();
		auto ruleSetToDelete = mCurrentRuleSetSelect->getSelected();
		if (ruleSetToDelete.empty())
			return;
		settings->setCurrentRuleSet(ruleSetToDelete);

		bool wasDeleted;
		TagRuleSet *deletedRuleSet;
		std::tie(wasDeleted, deletedRuleSet) = settings->deleteRuleSet(ruleSetToDelete);
		if (!wasDeleted)
		{
			if (ruleSetToDelete == TagRuleSet::defaultRuleSetName)
			{
				// We can use the same message here as in the case default was actually cleared.
				mWindow->displayNotificationMessage(_U("\uF014 ") +
					Utils::String::format(_(
						"CLEARED DEFAULT RULE SET").c_str()), 4000);
			}
			return;
		}

		settings->saveFile();
		if (deletedRuleSet->getName() == TagRuleSet::defaultRuleSetName)
		{
			mWindow->displayNotificationMessage(_U("\uF014 ") +
				Utils::String::format(_(
					"CLEARED DEFAULT RULE SET").c_str()), 4000);
		}
		else
		{
			mWindow->displayNotificationMessage(_U("\uF014 ") +
				Utils::String::format(_(
					"RULE SET DELETED: %s").c_str(), ruleSetToDelete.c_str()), 4000);
		}
		updateCurrentRuleSetSelector();
	};

	mMenu.addEntry(_("DELETE CURRENT RULE SET"), true, [this, deleteCurrentRuleSet]
	{
		auto ruleSetToDelete = mCurrentRuleSetSelect->getSelected();
		if (ruleSetToDelete.empty())
			return;
		auto settings = Settings::getInstance();
		mWindow->pushGui(new GuiMsgBox(mWindow, Utils::String::format(_("REALLY DELETE CURRENT RULE SET?\nCURRENT RULE SET IS: %s").c_str(), ruleSetToDelete.c_str()), _("YES"), [this, ruleSetToDelete, deleteCurrentRuleSet]
		{
			mWindow->pushGui(new GuiMsgBox(mWindow, Utils::String::format(_("REALLY REALLY DELETE CURRENT RULE SET?\nCURRENT RULE SET IS: %s\nTHIS ACTION CANNOT BE UNDONE!").c_str(), ruleSetToDelete.c_str()), _("YES"), [this, deleteCurrentRuleSet]
			{
				deleteCurrentRuleSet();
			}, 
			_("NO"), nullptr, ICON_WARNING, false, true, 2000));
		}, 
		_("NO"), nullptr, ICON_WARNING, false, true, 2000));
	});
}

void GuiTagsManager::updateCurrentRuleSetSelector()
{
	auto currentRuleSetName = Settings::getInstance()->getCurrentRuleSet()->getName();
	mCurrentRuleSetSelect->clear();
	for (auto ruleSetName : Settings::getInstance()->getRuleSetNames())
	{
		mCurrentRuleSetSelect->add(ruleSetName, ruleSetName, currentRuleSetName == ruleSetName);
	}
}
