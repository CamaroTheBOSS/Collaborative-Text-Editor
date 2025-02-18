#include "window_base.h"

BaseWindow::BaseWindow() :
    doc() {}

BaseWindow::BaseWindow(const ScrollableScreenBufferBuilder& ssbBuilder) :
    doc(),
    buffer(ssbBuilder.getResult()) {}

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

bool BaseWindow::isActive() const {
    return active;
}

void BaseWindow::activate() {
    active = true;
}

void BaseWindow::deactivate() {
    active = false;
}