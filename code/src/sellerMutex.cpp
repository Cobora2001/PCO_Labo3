#include "sellerMutex.h"

void SellerMutex::updateInterface() {
    mutexInterface.lock();
    super.updateInterface();
    mutexInterface.unlock();
}

void SellerMutex::interfaceMessage(QString message) {
    mutexInterface.lock();
    super.interfaceMessage(message);
    mutexInterface.unlock();
}

void SellerMutex::updateWithMessage(QString message) {
    mutexInterface.lock();
    super.updateInterface();
    super.interfaceMessage(message);
    mutexInterface.unlock();
}
