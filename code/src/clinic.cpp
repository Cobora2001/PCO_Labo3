#include "clinic.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>

IWindowInterface* Clinic::interface = nullptr;

Clinic::Clinic(int uniqueId, int fund, std::vector<ItemType> resourcesNeeded)
    : Seller(fund, uniqueId),
    nbTreated(0),
    resourcesNeeded(resourcesNeeded),
    mutex(),
    mutexInterface()
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Factory created");

    for(const auto& item : resourcesNeeded) {
        stocks[item] = 0;
    }
}

bool Clinic::verifyResources() {
    for (auto item : resourcesNeeded) {
        if (stocks[item] <= 0) {
            return false;
        }
    }
    return true;
}

void Clinic::updateInterface() {
    interface->updateFund(uniqueId, money);
    interface->updateStock(uniqueId, &stocks);
}

int Clinic::request(ItemType what, int qty) {
    if (what == ItemType::PatientHealed && qty > 0) {
        int cost = getCostPerUnit(ItemType::PatientHealed) * qty;
        mutex.lock();
        if(stocks[ItemType::PatientHealed] >= qty) {
            stocks[ItemType::PatientHealed] -= qty;
            money += cost;
            mutex.unlock();

            mutexInterface.lock();
            interface->consoleAppendText(uniqueId, QString("Sent %1 healed patient(s)").arg(qty));
            updateInterface();
            mutexInterface.unlock();

            return qty;
        }
        mutex.unlock();
    }

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "Refused request for " + QString::number(qty) + " " + getItemName(what));
    mutexInterface.unlock();
    
    return 0;
}

void Clinic::treatPatient() {
    int cost = getTreatmentCost();
    mutex.lock();
    if (money < cost) {
        mutex.unlock();
        mutexInterface.lock();
        interface->consoleAppendText(uniqueId, "Not enough money to treat a patient");
        mutexInterface.unlock();
        return;
    } else {
        money -= cost;
        for(auto resource : resourcesNeeded) {
            if(resource != ItemType::PatientSick) {
                stocks[resource] -= 1;
            }
        }
        mutex.unlock();
    }

    //Temps simulant un traitement
    interface->simulateWork();

    mutex.lock();
    ++nbTreated;
    stocks[ItemType::PatientHealed] += 1;
    stocks[ItemType::PatientSick] -= 1;
    mutex.unlock();

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "Treated a patient");
    updateInterface();
    mutexInterface.unlock();
}

bool Clinic::canPurchaseResources(ItemType resource) {
    
    if (stocks[resource] == 0 && money >= getCostPerUnit(resource)) {
        return true;
    }

    if (stocks[resource] == 0) {
        mutexInterface.lock();
        interface->consoleAppendText(uniqueId, "Not enough money to buy " + getItemName(resource));
        mutexInterface.unlock();
    }
    return false;
}

bool attemptPurchaseResources(ItemType resource, Seller* supplier) {
    int cost = supplier->request(resource, MAX_ITEMS_PER_ORDER);
    if (cost > 0) {
        stocks[resource] += MAX_ITEMS_PER_ORDER;
        money -= cost;
        mutex.unlock(); 

        mutexInterface.lock();
        interface->consoleAppendText(uniqueId, "Bought " + QString::number(MAX_ITEMS_PER_ORDER) + " " + getItemName(resource) + " from " + (resource == ItemType::PatientSick ? "hospital" : "supplier") + " " + QString::number(seller->getUniqueId()));
        updateInterface();
        mutexInterface.unlock();

        return true;
    }
    return false;
}

void Clinic::orderResources() {
    for(auto resource : resourcesNeeded) {
        mutex.lock();

        if(canPurchaseResources(resource)) {
            std::vector<Seller*> sellersTried;
            std::vector<Seller*> sellers = (resource == ItemType::PatientSick) ? hospitals : suppliers;

            while(stocks[resource] == 0 && sellersTried.size() < sellers.size()) {
                Seller* seller;
                do {
                    seller = Seller::chooseRandomSeller(sellers);
                } while (std::find(sellersTried.begin(), sellersTried.end(), seller) != sellersTried.end());
                
                if(attemptPurchaseResources(resource, seller)) break;

                sellersTried.push_back(seller);
            }

            if(stocks[resource] == 0) {
                mutexInterface.lock();
                interface->consoleAppendText(uniqueId, "No stock of " + getItemName(resource) + " available from " + (resource == ItemType::PatientSick ? "hospitals" : "suppliers"));
                mutexInterface.unlock();
            }
        }
    }
}

void Clinic::run() {
    if (hospitals.empty() || suppliers.empty()) {
        std::cerr << "You have to give to hospitals and suppliers to run a clinic" << std::endl;
        return;
    }
    interface->consoleAppendText(uniqueId, "[START] Factory routine");

    while (!finished) {
        
        if (verifyResources()) {
            treatPatient();
        } else {
            orderResources();
        }
       
        interface->simulateWork();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "[STOP] Factory routine");
    mutexInterface.unlock();
}


void Clinic::setHospitalsAndSuppliers(std::vector<Seller*> hospitals, std::vector<Seller*> suppliers) {
    this->hospitals = hospitals;
    this->suppliers = suppliers;

    for (Seller* hospital : hospitals) {
        interface->setLink(uniqueId, hospital->getUniqueId());
    }
    for (Seller* supplier : suppliers) {
        interface->setLink(uniqueId, supplier->getUniqueId());
    }
}

int Clinic::getTreatmentCost() {
    return getCostPerUnit(ItemType::PatientHealed);
}

int Clinic::getWaitingPatients() {
    return stocks[ItemType::PatientSick];
}

int Clinic::getNumberPatients(){
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed];
}

int Clinic::send(ItemType it, int qty, int bill){
    return 0;
}

int Clinic::getAmountPaidToWorkers() {
    return nbTreated * getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
}

void Clinic::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}

std::map<ItemType, int> Clinic::getItemsForSale() {
    return stocks;
}


Pulmonology::Pulmonology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Thermometer}) {}

Cardiology::Cardiology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Syringe, ItemType::Stethoscope}) {}

Neurology::Neurology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Scalpel}) {}
