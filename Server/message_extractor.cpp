#include "message_extractor.h"
#include "logging.h"

constexpr int defaultBuffSize = 128;

std::vector<msg::Buffer> MessageExtractor::extractMessages(const SOCKET client) {
    msg::Buffer recvBuff{defaultBuffSize};
    recvBuff.size = recv(client, recvBuff.get(), recvBuff.capacity, 0);
    if (recvBuff.size > 0) {
        auto [it, newOne] = clientFramerMap.try_emplace(client, Framer{ defaultBuffSize });
        auto msgBuffers = it->second.extractMessages(recvBuff);
        if (!msgBuffers.empty()) {
            server::logger.logDebug("Received", msgBuffers.size(), "messages from client", client);
        }
        return msgBuffers;
    }
    return { std::move(recvBuff) };
}
void MessageExtractor::reset(const SOCKET client) {
	clientFramerMap.erase(client);
}