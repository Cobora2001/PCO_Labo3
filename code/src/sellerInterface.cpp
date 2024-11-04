#include "sellerInterface.h"

IWindowInterface* SellerInterface::interface = nullptr;

void SellerInterface::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

void SellerInterface::updateInterface() {
    SellerInterface::updateMoney();
    SellerInterface::updateStock();
}

void SellerInterface::interfaceMessage(QString message) {
    interface->consoleAppendText(uniqueId, message);
}

void SellerInterface::simulateWork() {
    interface->simulateWork();
}

void SellerInterface::updateStock() {
    interface->updateStock(uniqueId, &stocks);
}

void SellerInterface::updateMoney() {
    interface->updateFund(uniqueId, money);
}

void SellerInterface::setLink(int id) {
    interface->setLink(uniqueId, id);
}
