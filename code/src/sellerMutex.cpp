#include "sellerMutex.h"
#include <iostream>

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

bool SellerMutex::buyFromSeller(Seller* seller, ItemType item, int qty, int costExpected) {

    int bill = seller->request(item, qty);

    if(bill > 0) {
        if(bill > costExpected) { // The bill can be lower given personnel costs and other such things
            std::cerr << "Error: cost of resource is not correct" << std::endl;
        }
        lockMutex();
        stocks[item] += qty;
        unlockMutex();

        updateWithMessage("Bought " + QString::number(qty) + " " + getItemName(item) + " from " + QString::number(seller->getUniqueId()));

        return true;
    } else {
        lockMutex();
        money += costExpected;
        unlockMutex();

        interfaceMessage("Not enough " + getItemName(item) + " available at " + QString::number(seller->getUniqueId()));

        return false;
    }
}

int SellerMutex::buyFromSellers(std::vector<Seller*> sellers, ItemType item, int maxQty, int costPerOrder, int numberPerOrder) {
    int qty = 0;

    costPerOrder = costPerOrder == -1 ? getCostPerUnit(item) * numberPerOrder : costPerOrder;

    bool enoughMoney = true;

    std::vector<Seller*> sellersTried;
    do {
        Seller* seller;
        do {
            seller = Seller::chooseRandomSeller(sellers);
        } while (std::find(sellersTried.begin(), sellersTried.end(), seller) != sellersTried.end());

        bool sellerAvailable = true;

        while(qty <= maxQty - numberPerOrder && enoughMoney && sellerAvailable) {

            lockMutex();
            if(money < costPerOrder) {
                enoughMoney = false;
                unlockMutex();
                interfaceMessage("Not enough money to buy " + getItemName(item) + " from " + QString::number(seller->getUniqueId()));
            } else {
                money -= costPerOrder;
                unlockMutex();

                if(buyFromSeller(seller, item, numberPerOrder, costPerOrder)) {
                    qty += numberPerOrder;
                } else {
                    sellerAvailable = false;
                    sellersTried.push_back(seller);
                }
            }
        }
    } while(enoughMoney && qty <= maxQty - numberPerOrder && sellersTried.size() < sellers.size());

    return qty;
}
