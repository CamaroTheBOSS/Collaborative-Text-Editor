#pragma once
#include "window_text_editor.h"
#include "window_search.h"
#include "window_replace.h"
#include "window_mainmenu.h"
#include "window_help.h"
#include "window_createdoc.h"
#include "window_loaddoc.h"

#include <array>

using Window = std::unique_ptr<BaseWindow>;
using Windows = std::vector<Window>;
using WindowsRegistry = std::unordered_map<std::string, bool>;
using WindowsIt = Windows::const_iterator;
enum class FocusDirection { up, down, left, right };

class WindowsManager {
public:
	WindowsManager();
	void changeFocusUp();
	void changeFocusDown();
	void changeFocusLeft();
	void changeFocusRight();
	void setFocus(const int newFocus);
	template <typename T>
	WindowsIt showWindow(const COORD& consoleSize) {
		if (windowsRegistry.find(T::className) != windowsRegistry.cend()) {
			return findWindow(T::className);
		}
		auto builder = builderMap.find(T::className);
		if (builder == builderMap.cend()) {
			return windows.cend();
		}
		builder->second.setConsoleSize({consoleSize.X, consoleSize.Y});
		auto window = std::make_unique<T>(builder->second);
		windowsRegistry[window->name()] = true;
		windows.emplace_back(std::move(window));
		setFocus(windows.size() - 1);
		return windows.cend() - 1;
	}
	void destroyLastWindow(const TCPClient& client);
	void destroyWindow(const std::string& name, const TCPClient& client);

	const Window& getFocusedWindow() const;
	const Window& getTextEditor() const;
	const Windows& getWindows() const;
	bool processEvent(const TCPClient& client, const Event& pEvent);
private:
	WindowsIt findWindow(const std::string& name) const;

	int focus = 0;
	Windows windows;
	WindowsRegistry windowsRegistry;
	std::unordered_map<std::string, ScrollableScreenBufferBuilder> builderMap;
};