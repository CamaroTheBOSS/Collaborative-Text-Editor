#include "window_base.h"

BaseWindow::BaseWindow(TCPClient& client) :
    client(client),
    doc() {}

bool BaseWindow::saveDoc() const {
    std::ofstream file(doc.getFilename(), std::ios::out);
    if (!file) {
        return false;
    }
    file << doc.getText();
    return true;
}

ClientSiteDocument& BaseWindow::getDoc() {
    return doc;
}

ScrollableScreenBuffer& BaseWindow::getBuffer() {
    return buffer;
}

unsigned int BaseWindow::getDocBufferWidth() {
    return buffer.width();
}