#pragma once
#include "window_helpers.h"

using Window = std::unique_ptr<BaseWindow>;
using Windows = std::vector<Window>;
using WindowsRegistry = std::unordered_map<std::string, bool>;
using WindowsIt = Windows::const_iterator;
using TextInputHandler = TextInputWindow::TextInputHandler;
enum class FocusDirection { up, down, left, right };

class WindowsManager {
public:
	WindowsManager(const COORD& consoleSize);
	void changeFocusUp();
	void changeFocusDown();
	void changeFocusLeft();
	void changeFocusRight();
	void setFocus(const std::string& winName);
	void setFocus(const int newFocus);

	template <typename WindowClass, typename... Args>
	WindowsIt showWindow(const ScrollableScreenBufferBuilder& builder, Args&&... args) {
		std::string title = builder.getTitle();
		if (windowsRegistry.find(title) != windowsRegistry.cend()) {
			return findWindow(title);
		}
		auto window = std::make_unique<WindowClass>(builder, std::forward<Args>(args)...);
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
	WindowsIt findWindow(const std::string& name) const;
	bool processEvent(const TCPClient& client, const Event& pEvent);
private:
	int focus = 0;
	Windows windows;
	WindowsRegistry windowsRegistry;
	std::unordered_map<std::string, ScrollableScreenBufferBuilder> builderMap;
};