module;

#include <unordered_map>
#include <string>
#include <vector>
#include "document_base.h"

export module window.base;
export import screen.buffers.builder;
export import keypack;
export import tcp_client;
import client.document;
import canvas;

export struct Event {
	std::string name;
	std::string src;
	std::string target;
	std::vector<std::string> params;
	bool empty() {
		return name.empty();
	}
};

export template <typename T>
using EventHandlersMap = std::unordered_map<std::string, void(T::*)(const TCPClient&, const std::vector<std::string>&)>;

export class BaseWindow {
public:
	BaseWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	virtual Event onDelete() { return Event{}; };
	virtual Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData = "") = 0;
	bool saveDoc() const;
	const ClientSiteDocument& getDoc() const;
	ClientSiteDocument& getDocMutable();
	const ScrollableScreenBuffer& getBuffer() const;
	unsigned int getDocBufferWidth();
	bool isActive() const;
	void activate();
	void deactivate();
	virtual std::string name() const = 0;
	virtual void render(Canvas& canvas);
	virtual void processEvent(const TCPClient& client, const Event& pEvent) {};
	void updateScroll();
	void updateConsoleSize(const COORD& newSize);
	void clearContent();
	COORD getFocusAnchor() const;
protected:
	ClientSiteDocument doc;
	ScrollableScreenBuffer buffer;
	bool active;
};
