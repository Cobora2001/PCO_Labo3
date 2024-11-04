#include "sellerInterface.h"

IWindowInterface* SellerInteface::interface = nullptr;

void SellerInterface::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

void SellerInterface::updateInterface() {
    interface->updateFund(uniqueId, money);
    interface->updateStock(uniqueId, &stocks);
}

void SellerInterface::interfaceMessage(QString message) {
    interface->consoleAppendText(uniqueId, message);
}
