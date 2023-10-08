#pragma once
#ifndef ES_APP_GUIS_GUI_TAGS_MANAGER_H
#define ES_APP_GUIS_GUI_TAGS_MANAGER_H

#include "components/MenuComponent.h"
#include "GuiComponent.h"
#include <functional>

template<typename T>
class OptionListComponent;
class SystemData;

class GuiTagsManager : public GuiComponent
{
public:
	GuiTagsManager(Window* window, SystemData* system);

	~GuiTagsManager();
	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

	inline void onFinalize(const std::function<void()>& func) { mOnFinalizeFunc = func; };

private:
	std::function<void()> mOnFinalizeFunc;

	void initializeMenu();
	void close();
	void addCurrentTagSelectorToMenu();

	MenuComponent mMenu;
	SystemData* mSystem;

	std::shared_ptr<OptionListComponent<std::string>> mCurrentTagSelect;
};

#endif // ES_APP_GUIS_GUI_TAGS_MANAGER_H
