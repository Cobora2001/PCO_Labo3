#include "sellerMutex.h"

SellerMutex::SellerMutex(int money, int uniqueId) : SellerInterface(money, uniqueId), mutex(), mutexInterface() {}

void SellerMutex::updateInterface() {
    mutexInterface.lock();
    SellerInterface::updateInterface();
    mutexInterface.unlock();
}

void SellerMutex::interfaceMessage(QString message) {
    mutexInterface.lock();
    SellerInterface::interfaceMessage(message);
    mutexInterface.unlock();
}

void SellerMutex::updateWithMessage(QString message) {
    mutexInterface.lock();
    SellerInterface::updateInterface();
    SellerInterface::interfaceMessage(message);
    mutexInterface.unlock();
}

void SellerMutex::updateStock() {
    mutexInterface.lock();
    SellerInterface::updateStock();
    mutexInterface.unlock();
}

void SellerMutex::updateMoney() {
    mutexInterface.lock();
    SellerInterface::updateMoney();
    mutexInterface.unlock();
}
