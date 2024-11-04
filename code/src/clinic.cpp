#include "clinic.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>

Clinic::Clinic(int uniqueId, int fund, std::vector<ItemType> resourcesNeeded)
    : SellerMutex(fund, uniqueId),
    nbTreated(0),
    resourcesNeeded(resourcesNeeded)
{
    lockMutex();
    for(const auto& item : resourcesNeeded) {
        stocks[item] = 0;
    }
    unlockMutex();

    updateWithMessage("Clinic Created");
}

bool Clinic::verifyResources() {
    for (auto item : resourcesNeeded) {
        if (stocks[item] <= 0) {
            return false;
        }
    }
    lockMutex();
    if(money < getTreatmentCost()) {
        unlockMutex();

        interfaceMessage("Not enough money to treat a patient");
        return false;
    }
    money -= getTreatmentCost();
    unlockMutex();
    return true;
}

int Clinic::request(ItemType what, int qty) {
    if (what == ItemType::PatientHealed && qty > 0) {
        int benefit = getCostPerUnit(ItemType::PatientHealed) * qty;
        lockMutex();
        if(stocks[ItemType::PatientHealed] >= qty) {
            stocks[ItemType::PatientHealed] -= qty;
            money += benefit;
            unlockMutex();

            updateWithMessage("Provided " + QString::number(qty) + " healed patient" + (qty > 1 ? "s" : ""));

            return benefit;
        }
        unlockMutex();
    }

    interfaceMessage("Refused request for " + QString::number(qty) + " " + getItemName(what));

    return 0;
}

void Clinic::treatPatient() {
    int cost = getTreatmentCost();
    lockMutex();
    for(auto resource : resourcesNeeded) {
        if(resource != ItemType::PatientSick && resource != ItemType::PatientHealed) {
            stocks[resource] -= 1;
        }
    }
    unlockMutex();

    //Temps simulant un traitement
    simulateWork();

    lockMutex();
    ++nbTreated;
    stocks[ItemType::PatientHealed] += 1;
    stocks[ItemType::PatientSick] -= 1;
    unlockMutex();

    updateWithMessage("Treated a patient");
}

void Clinic::orderResources() {
    for(auto resource : resourcesNeeded) {
        if(stocks[resource] == 0 && resource != ItemType::PatientHealed) {
            std::vector<Seller*> sellers = resource == ItemType::PatientSick ? hospitals : suppliers;

            int qty = buyFromSellers(sellers, resource, MAX_ITEMS_PER_ORDER);

            if(qty > 0) {
                updateWithMessage("Bought " + QString::number(qty) + " " + getItemName(resource) + " from supplier" + (suppliers.size() > 1 ? "s" : ""));
            } else {
                interfaceMessage("No stock of " + getItemName(resource) + " in " + QString::number(MAX_ITEMS_PER_ORDER) + " quantity available from supplier" + (suppliers.size() > 1 ? "s" : ""));
            }
        } 
    }
}

void Clinic::run() {
    if (hospitals.empty() || suppliers.empty()) {
        std::cerr << "You have to give to hospitals and suppliers to run a clinic" << std::endl;
        return;
    }

    interfaceMessage("[START] Factory routine");

    while (!finished) {
        
        if (verifyResources()) {
            treatPatient();
        } else {
            orderResources();
        }
       
        simulateWork();
    }

    interfaceMessage("[STOP] Factory routine");
}


void Clinic::setHospitalsAndSuppliers(std::vector<Seller*> hospitals, std::vector<Seller*> suppliers) {
    this->hospitals = hospitals;
    this->suppliers = suppliers;

    for (Seller* hospital : hospitals) {
        setLink(hospital->getUniqueId());
    }
    for (Seller* supplier : suppliers) {
        setLink(supplier->getUniqueId());
    }
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
