#pragma once
#include "window_text_editor.h"
#include "window_search.h"
#include "window_replace.h"
#include "window_mainmenu.h"

using Window = std::unique_ptr<BaseWindow>;
using Windows = std::vector<Window>;
using WindowsRegistry = std::unordered_map<std::string, bool>;
using WindowsIt = Windows::const_iterator;
enum class FocusDirection{up, down, left, right};
class WindowsManager {
public:
	void changeFocusUp();
	void changeFocusDown();
	void changeFocusLeft();
	void changeFocusRight();
	void setFocus(const int newFocus);

	WindowsIt showMainMenuWindow(const COORD& consoleSize);
	WindowsIt showSearchWindow(const COORD& consoleSize);
	WindowsIt showReplaceWindow(const COORD& consoleSize);
	WindowsIt showTextEditorWindow(const COORD& consoleSize);
	void destroyLastWindow();

	const Window& getFocusedWindow() const;
	const Window& getTextEditor() const;
	const Windows& getWindows() const;
private:
	WindowsIt findWindow(const std::string& name) const;

	int focus = 0;
	Windows windows;
	WindowsRegistry windowsRegistry;
};