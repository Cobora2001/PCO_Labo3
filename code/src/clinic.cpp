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
    mutex.lock();
    for(const auto& item : resourcesNeeded) {
        stocks[item] = 0;
    }
    mutex.unlock();

    updateWithMessage("Clinic Created");
}

bool Clinic::verifyResources() {
    for (auto item : resourcesNeeded) {
        if (stocks[item] <= 0) {
            return false;
        }
    }
    mutex.lock();
    if(money < getTreatmentCost()) {
        mutex.unlock();
        mutexInterface.lock();
        interface->consoleAppendText(uniqueId, "Not enough money to treat a patient");
        mutexInterface.unlock();
        return false;
    }
    money -= getTreatmentCost();
    mutex.unlock();
    return true;
}

int Clinic::request(ItemType what, int qty) {
    printf("Clinic %d requested %d %s\n", uniqueId, qty, getItemName(what).toStdString().c_str());
    if (what == ItemType::PatientHealed && qty > 0) {
        int benefit = getCostPerUnit(ItemType::PatientHealed) * qty;
        mutex.lock();
        if(stocks[ItemType::PatientHealed] >= qty) {
            stocks[ItemType::PatientHealed] -= qty;
            money += benefit;
            mutex.unlock();

            mutexInterface.lock();
            interface->consoleAppendText(uniqueId, QString("Sent %1 healed patient(s)").arg(qty));
            updateInterface();
            mutexInterface.unlock();

            return benefit;
        }
        mutex.unlock();
    }

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "Refused request for " + QString::number(qty) + " " + getItemName(what));
    mutexInterface.unlock();
    
    return 0;
}

void Clinic::treatPatient() {
    printf("Clinic %d treating patient\n", uniqueId);
    int cost = getTreatmentCost();
    mutex.lock();
    for(auto resource : resourcesNeeded) {
        if(resource != ItemType::PatientSick && resource != ItemType::PatientHealed) {
            stocks[resource] -= 1;
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

void Clinic::orderResources() {
    printf("Clinic %d ordering resources\n", uniqueId);
    for(auto resource : resourcesNeeded) {
        if(stocks[resource] == 0 && resource != ItemType::PatientHealed) {
            int costExpected = getCostPerUnit(resource) * MAX_ITEMS_PER_ORDER;
            mutex.lock();
            if(money < costExpected) {
                mutex.unlock();

                mutexInterface.lock();
                interface->consoleAppendText(uniqueId, "Not enough money to buy " + getItemName(resource) + " from " + (resource == ItemType::PatientSick ? "hospitals" : "suppliers"));
                mutexInterface.unlock();
            } else {
                money -= costExpected;
                mutex.unlock();

                std::vector<Seller*> sellersTried;
                std::vector<Seller*> sellers;

                if(resource == ItemType::PatientSick) {
                    sellers = hospitals;
                } else {
                    sellers = suppliers;
                }

                do {
                    Seller* seller;
                    do {
                        seller = Seller::chooseRandomSeller(sellers);
                    } while (std::find(sellersTried.begin(), sellersTried.end(), seller) != sellersTried.end());

                    int cost = seller->request(resource, MAX_ITEMS_PER_ORDER);

                    if (cost > 0) {
                        if(cost != costExpected) {
                            printf("Error: cost of resource is not correct\n");
                        }

                        mutex.lock();
                        stocks[resource] += MAX_ITEMS_PER_ORDER;
                        mutex.unlock();  // Unlock mutex after updating stocks and money

                        mutexInterface.lock();
                        interface->consoleAppendText(uniqueId, "Bought " + QString::number(MAX_ITEMS_PER_ORDER) + " " + getItemName(resource) + " from " + (resource == ItemType::PatientSick ? "hospital" : "supplier") + " " + QString::number(seller->getUniqueId()));
                        updateInterface();
                        mutexInterface.unlock();
                    } else {
                        sellersTried.push_back(seller);
                    }
                } while (stocks[resource] == 0 && sellersTried.size() < sellers.size());

                if(stocks[resource] == 0) {
                    mutex.lock();
                    money += costExpected;
                    mutex.unlock();

                    mutexInterface.lock();
                    interface->consoleAppendText(uniqueId, "No stock of " + getItemName(resource) + " in " + QString::number(MAX_ITEMS_PER_ORDER) + " quantity available from " + (resource == ItemType::PatientSick ? "hospitals" : "suppliers"));
                    mutexInterface.unlock();
                }
            }
        } 
    }
}

void Clinic::run() {
    if (hospitals.empty() || suppliers.empty()) {
        std::cerr << "You have to give to hospitals and suppliers to run a clinic" << std::endl;
        return;
    }

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "[START] Factory routine");
    mutexInterface.unlock();

    printf("Clinic %d started\n", uniqueId);

    while (!finished) {
        
        if (verifyResources()) {
            treatPatient();
        } else {
            orderResources();
        }
       
        interface->simulateWork();
    }

    printf("Clinic %d has finished\n", uniqueId);

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "[STOP] Factory routine");
    mutexInterface.unlock();
}


void Clinic::setHospitalsAndSuppliers(std::vector<Seller*> hospitals, std::vector<Seller*> suppliers) {
    this->hospitals = hospitals;
    this->suppliers = suppliers;

    mutexInterface.lock();
    for (Seller* hospital : hospitals) {
        interface->setLink(uniqueId, hospital->getUniqueId());
    }
    for (Seller* supplier : suppliers) {
        interface->setLink(uniqueId, supplier->getUniqueId());
    }
    mutexInterface.unlock();
}

int Clinic::getTreatmentCost() {
    return getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
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

std::map<ItemType, int> Clinic::getItemsForSale() {
    return stocks;
}


Pulmonology::Pulmonology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Thermometer}) {}

Cardiology::Cardiology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Syringe, ItemType::Stethoscope}) {}

Neurology::Neurology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::PatientSick, ItemType::Pill, ItemType::Scalpel}) {}
