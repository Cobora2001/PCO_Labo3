#include "supplier.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

Supplier::Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied)
    : SellerMutex(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbSupplied(0) 
{
    for (const auto& item : resourcesSupplied) {    
        stocks[item] = 0;    
    }

    interface->consoleAppendText(uniqueId, QString("Supplier Created"));
    interface->updateFund(uniqueId, fund);
}

int Supplier::request(ItemType it, int qty) {
    if (stocks.find(it) != stocks.end()) {
        mutex.lock();
        if (stocks[it] >= qty) {
            stocks[it] -= qty;
            int cost = getCostPerUnit(it) * qty;
            money += cost;
            mutex.unlock();

            mutexInterface.lock();
            interface->consoleAppendText(uniqueId, QString("Sold %1 %2").arg(qty).arg(getItemName(it)));
            updateInterface();
            mutexInterface.unlock();

            return cost;
        }
        mutex.unlock();
    }

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, QString("Refused request for %1 %2").arg(qty).arg(getItemName(it)));
    mutexInterface.unlock();

    return 0;
}

void Supplier::run() {
    interface->consoleAppendText(uniqueId, "[START] Supplier routine");

    printf("Supplier %d started\n", uniqueId);

    while (!finished) {
        ItemType resourceSupplied = getRandomItemFromStock();
        int supplierCost = getEmployeeSalary(getEmployeeThatProduces(resourceSupplied));

        bool hasEnoughMoney = false;

        mutex.lock();
        if (money >= supplierCost) {
            money -= supplierCost;
            hasEnoughMoney = true;
            mutex.unlock();
        } else {
            mutex.unlock();
        }


        /* Temps aléatoire borné qui simule l'attente du travail fini ou d'attente qu'on lui achète des produits*/
        interface->simulateWork();

        if(hasEnoughMoney) {
            nbSupplied++;
            mutex.lock();
            ++stocks[resourceSupplied];
            mutex.unlock();

            mutexInterface.lock();
            interface->consoleAppendText(uniqueId, QString("Supplied 1 %1").arg(getItemName(resourceSupplied)));
            updateInterface();
            mutexInterface.unlock();
        }
    }

    printf("Supplier %d finished\n", uniqueId);
    
    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "[STOP] Supplier routine");
    mutexInterface.unlock();
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
