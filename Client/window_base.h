#pragma once
#include "tcp_client.h"
#include "screen_buffers_builder.h"
#include "keypack.h"
#include "canvas.h"
#include "events.h"

#include <unordered_map>

struct Event {
	std::string name;
	std::string src;
	std::string target;
	std::vector<std::string> params;
	bool empty() {
		return name.empty();
	}
};

template <typename T>
using EventHandlersMap = std::unordered_map<std::string, void(T::*)(const TCPClient&, const std::vector<std::string>&)>;

class BaseWindow {
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
protected:
	ClientSiteDocument doc;
	ScrollableScreenBuffer buffer;

	std::vector<std::unique_ptr<BaseWindow>> children;
	int activeChildrenIndex;
	bool active;
};
