#include "supplier.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

Supplier::Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied)
    : SellerMutex(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbSupplied(0) 
{
    for (const auto& item : resourcesSupplied) {    
        stocks[item] = 0;    
    }

    updateWithMessage("Supplier Created");
}

int Supplier::request(ItemType it, int qty) {
    if (stocks.find(it) != stocks.end()) {
        lockMutex();
        if (stocks[it] >= qty) {
            stocks[it] -= qty;
            int cost = getCostPerUnit(it) * qty;
            money += cost;
            unlockMutex();

            updateWithMessage(QString("Sold %1 %2").arg(qty).arg(getItemName(it)));

            return cost;
        }
        unlockMutex();
    }

    interfaceMessage(QString("Refused request for %1 %2").arg(qty).arg(getItemName(it)));

    return 0;
}

void Supplier::run() {
    interfaceMessage("[START] Supplier routine");

    while (!finished) {
        ItemType resourceSupplied = chooseAdequateItem();
        int supplierCost = getEmployeeSalary(getEmployeeThatProduces(resourceSupplied));

        bool hasEnoughMoney = false;

        lockMutex();
        if (money >= supplierCost) {
            money -= supplierCost;
            hasEnoughMoney = true;
        }
        unlockMutex();

        simulateWork();

        if(hasEnoughMoney) {
            ++nbSupplied;
            lockMutex();
            ++stocks[resourceSupplied];
            unlockMutex();

            updateWithMessage(QString("Supplied 1 %1").arg(getItemName(resourceSupplied)));
        }
    }

    interfaceMessage("[STOP] Supplier routine");    
}


std::map<ItemType, int> Supplier::getItemsForSale() {
    return stocks;
}

int Supplier::getMaterialCost() {
    int totalCost = 0;
    for (const auto& item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

int Supplier::getAmountPaidToWorkers() {
    return nbSupplied * getEmployeeSalary(EmployeeType::Supplier);
}

std::vector<ItemType> Supplier::getResourcesSupplied() const
{
    return resourcesSupplied;
}

int Supplier::send(ItemType it, int qty, int bill){
    return 0;
}

ItemType Supplier::chooseAdequateItem() {
    if (stocks.empty()) {
        throw std::runtime_error("Stock is empty.");
    }

    int minQuantity = std::numeric_limits<int>::max();
    ItemType leastPresentItem = ItemType::Nothing;

    // Parcours des stocks pour trouver les items avec la plus petite quantité
    for (const auto& item : stocks) {
        if (leastPresentItem == ItemType::Nothing || item.second < minQuantity) {
            minQuantity = item.second;
            leastPresentItem = item.first;
        } 
    }

    return leastPresentItem;
}
