#pragma once
#include <string>
#ifndef ES_APP_GUIS_GUI_TAG_RULE_EDITOR_H
#define ES_APP_GUIS_GUI_TAG_RULE_EDITOR_H

#include "components/MenuComponent.h"
#include "GuiComponent.h"
#include <functional>

class SystemData;

class GuiTagRuleEditor : public GuiComponent
{
public:
	GuiTagRuleEditor(Window* window, TagRuleSet* ruleSet);

	~GuiTagRuleEditor();
	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

	inline void onFinalize(const std::function<void()>& func) { mOnFinalizeFunc = func; };

private:
	std::function<void()> mOnFinalizeFunc;

	void initializeMenu();
	void close();
	void addTagsToMenu(bool include);

	void addRuleSetName(std::string ruleSetName);
	void addTagSwitch(std::string tag, bool switchState, const std::function<void(bool state)>& onChangeCallback);

	MenuComponent mMenu;
	TagRuleSet* mRuleSet;
};

#endif // ES_APP_GUIS_GUI_TAG_RULE_EDITOR_H
