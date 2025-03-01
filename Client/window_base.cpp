#include "window_base.h"
#include "renderer.h"

BaseWindow::BaseWindow(const ScrollableScreenBufferBuilder& ssbBuilder) :
    doc(),
    buffer(ssbBuilder.getResult()),
    active(false) {}

bool BaseWindow::saveDoc() const {
    std::ofstream file(doc.getFilename(), std::ios::out);
    if (!file) {
        return false;
    }
    file << doc.getText();
    return true;
}

const ClientSiteDocument& BaseWindow::getDoc() const {
    return doc;
}

ClientSiteDocument& BaseWindow::getDocMutable() {
    return doc;
}

const ScrollableScreenBuffer& BaseWindow::getBuffer() const {
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

void BaseWindow::render(Canvas& canvas) {
    updateScroll();
    Renderer::addToCanvas(canvas, *this);
}

void BaseWindow::updateScroll() {
    buffer.scrollToCursor(buffer.getMyTerminalCursor(doc));
}

void BaseWindow::updateConsoleSize(const COORD& newSize) {
    buffer.setNewConsoleSize({ newSize.X, newSize.Y });
}

void BaseWindow::clearContent() {
    doc.clearContent();
}