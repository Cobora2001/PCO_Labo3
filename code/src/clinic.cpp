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
        if (stocks[item] == 0) {
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
    if(stocks[ItemType::PatientSick] == 0) {
        int cost = getCostPerUnit(ItemType::PatientSick);
        mutex.lock();
        if(money >= cost) {
            std::vector<Seller*> hospitalsTried;
            do {
                Seller* hospital;
                do {
                    hospital = Seller::chooseRandomSeller(hospitals);
                } while (std::find(hospitalsTried.begin(), hospitalsTried.end(), hospital) != hospitalsTried.end());
                int bill = hospital->request(ItemType::PatientSick, 1);
                if (bill > 0) {
                    stocks[ItemType::PatientSick] += 1;
                    money -= bill;
                    mutex.unlock();  // Unlock mutex after updating stocks and money

                    mutexInterface.lock();
                    interface->consoleAppendText(uniqueId, "Bought 1 patient from hospital " + QString::number(hospital->getUniqueId()));
                    updateInterface();
                    mutexInterface.unlock();
                } else {
                    hospitalsTried.push_back(hospital);
                }
            } while (stocks[ItemType::PatientSick] == 0 && hospitalsTried.size() < hospitals.size());

            if(stocks[ItemType::PatientSick] == 0) {
                mutex.unlock();  // Unlock mutex if no stock was obtained after trying hospitals

                mutexInterface.lock();
                interface->consoleAppendText(uniqueId, "No patient available from hospitals");
                mutexInterface.unlock();
            }
        } else {
            mutex.unlock();  // Unlock mutex if not enough money to buy patient from hospitals

            mutexInterface.lock();
            interface->consoleAppendText(uniqueId, "Not enough money to buy patient from hospitals");
            mutexInterface.unlock();
        }
    }

    //Temps simulant un traitement ou l'attente de l'arrivée d'un patient à l'hôpital pour retenter de traiter une prochaine fois
    interface->simulateWork();

    if(stocks[ItemType::PatientSick] >= 1) {
        ++nbTreated;
        mutex.lock();
        stocks[ItemType::PatientSick] -= 1;
        stocks[ItemType::PatientHealed] += 1;
        money -= getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
        mutex.unlock();

        mutexInterface.lock();
        updateInterface();
        interface->consoleAppendText(uniqueId, "Treated a patient");
        mutexInterface.unlock();                
    }
}

void Clinic::orderResources() {
    for(auto resource : resourcesNeeded) {
        mutex.lock();
        if(stocks[resource] == 0 && money >= getCostPerUnit(resource)) {
            std::vector<Seller*> suppliersTried;
            do {
                Seller* supplier;
                do {
                    supplier = Seller::chooseRandomSeller(suppliers);
                } while (std::find(suppliersTried.begin(), suppliersTried.end(), supplier) != suppliersTried.end());
                int cost = supplier->request(resource, MAX_ITEMS_PER_ORDER);
                if (cost > 0) {
                    stocks[resource] += MAX_ITEMS_PER_ORDER;
                    money -= cost;
                    mutex.unlock();  // Unlock mutex after updating stocks and money

                    mutexInterface.lock();
                    interface->consoleAppendText(uniqueId, "Bought " + QString::number(MAX_ITEMS_PER_ORDER) + " " + getItemName(resource) + " from supplier " + QString::number(supplier->getUniqueId()));
                    updateInterface();
                    mutexInterface.unlock();
                } else {
                    suppliersTried.push_back(supplier);
                }
            } while (stocks[resource] == 0 && suppliersTried.size() < suppliers.size());

            if(stocks[resource] == 0) {
                mutex.unlock();  // Unlock mutex if no stock was obtained after trying suppliers

                mutexInterface.lock();
                interface->consoleAppendText(uniqueId, "No stock of " + getItemName(resource) + " in " + QString::number(MAX_ITEMS_PER_ORDER) + " quantity available from suppliers");
                mutexInterface.unlock();
            }
        } else {
            mutex.unlock();  // Unlock mutex if conditions are not met for resource acquisition

            if(stocks[resource] == 0) {
                mutexInterface.lock();
                interface->consoleAppendText(uniqueId, "Not enough money to buy " + getItemName(resource) + " from suppliers");
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
    return 0;
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
