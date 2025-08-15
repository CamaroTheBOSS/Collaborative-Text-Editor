module;

#include <functional>
#include <string>

export module window.text_input.obsfucated;
import window.text_input;

export class ObsfucatedTextInputWindow : public TextInputWindow {
public:
	using TextInputHandler = std::function<Event(const TextInputWindow&, const ClientSiteDocument&)>;
	ObsfucatedTextInputWindow(const ScrollableScreenBufferBuilder& ssbBuilder, TextInputHandler onSubmitHandler = funcNoop(),
		TextInputHandler onModifyHandler = funcNoop(), TextInputHandler onDeleteHandler = funcNoop());
	Event onDelete() override;
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	void render(Canvas& canvas);
private:
	ClientSiteDocument obsfucated;
};