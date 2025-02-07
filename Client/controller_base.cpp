#include "controller_base.h"

BaseController::BaseController(TCPClient& client, Terminal& terminal) :
    client(client),
    terminal(terminal),
    doc() {}

bool BaseController::saveDoc() const {
    std::ofstream file(doc.getFilename(), std::ios::out);
    if (!file) {
        return false;
    }
    file << doc.getText();
    return true;
}

ClientSiteDocument& BaseController::getDoc() {
    return doc;
}