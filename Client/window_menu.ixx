module;

#include <functional>
#include <string>

export module window.menu;
import window.base;

class MenuWindow;
export struct Option {
	std::string name;
	std::function<Event(MenuWindow&)> handler;
};

export class MenuWindow : public BaseWindow {
public:
	MenuWindow(const ScrollableScreenBufferBuilder& ssbBuilder, std::vector<Option>&& options);
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override;
private:
	Event goUp();
	Event goDown();
	void selectOption();
	Event executeOption();
	int selectedOption;
	std::vector<Option> options;
	const std::string winName;
};