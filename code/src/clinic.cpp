#include "clinic.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>

IWindowInterface* Clinic::interface = nullptr;

Clinic::Clinic(int uniqueId, int fund, std::vector<ItemType> resourcesNeeded)
    : Seller(fund, uniqueId), nbTreated(0), resourcesNeeded(resourcesNeeded)
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

int Clinic::request(ItemType what, int qty){
    // TODO 
    PcoMutexLocker lock(mutex);

    if(what != ItemType::PatientHealed){
        interface->consoleAppendText(uniqueId, "Request denied: Clinic only provides healed patients");
        return 0;
    }

    if(stocks[ItemType::PatientHealed] < qty){
        interface->consoleAppendText(uniqueId, "Request denied: Not enough healed patients");
        return 0;
    }

    stocks[ItemType::PatientHealed] -= qty;

    int totalCost = qty * getCostPerUnit(ItemType::PatientHealed);

    interface->consoleAppendText(uniqueId, QString("Provided %1 healed patients").arg(qty));
    
    return totalCost;
}

void Clinic::treatPatient() {
    // TODO 
    PcoMutexLocker lock(mutex);

    if(stocks[ItemType::PatientSick] <= 0){
        interface->consoleAppendText(uniqueId, "No patient to treat");
        return;
    }

    if(!verifyResources()){
        interface->consoleAppendText(uniqueId, "Not enough resources to treat patient");
        return;
    }

    for (auto item : resourcesNeeded) {
        stocks[item]--;
    }
    
    stocks[ItemType::PatientSick]--;
    stocks[ItemType::PatientHealed]++;

    nbTreated++;

    int treatmentCost = getEmployeeSalary(getEmployeeThatProduces(ItemType::PatientHealed));
    money -= treatmentCost;

    //Temps simulant un traitement 
    interface->simulateWork();

    // TODO 
    
    interface->consoleAppendText(uniqueId, "Clinic have healed a new patient");
}

void Clinic::orderResources() {
    // TODO 
        for (auto item : resourcesNeeded) {
        {
            PcoMutexLocker lock(mutex); 
            if (stocks[item] > 0) {
                continue; 
            }
        }

        
        Seller* chosenSupplier = chooseRandomSeller(suppliers);
        if (!chosenSupplier) {
            interface->consoleAppendText(uniqueId, "No supplier to order resources");
            continue;
        }

        int cost = chosenSupplier->request(item, 1);
        {
            PcoMutexLocker lock(mutex);

            if (cost > 0 && money >= cost) {
                money -= cost;
                stocks[item]++;
                interface->consoleAppendText(uniqueId, QString("Ordered %1 from supplier %2")
                                             .arg(getItemName(item))
                                             .arg(chosenSupplier->getUniqueId()));
            } else {
                interface->consoleAppendText(uniqueId, "Failed to order resources");
            }
        }
    }

    // commande d'un patient
    {
        PcoMutexLocker lock(mutex);

        if (stocks[ItemType::PatientSick] > 0) {
            return;
        }
    }

    Seller* chosenHospital = chooseRandomSeller(hospitals);
    if (!chosenHospital) {
        interface->consoleAppendText(uniqueId, "No hospital to send patient");
        return;
    }

    int cost = chosenHospital->request(ItemType::PatientSick, 1);
    {
        PcoMutexLocker lock(mutex);
        if (cost > 0 && money >= cost) {
            money -= cost;
            stocks[ItemType::PatientSick]++;
            interface->consoleAppendText(uniqueId, QString("Ordered patient from hospital %1")
                                         .arg(chosenHospital->getUniqueId()));
        } else {
            interface->consoleAppendText(uniqueId, "Failed to order patient");
        }
    }
}

/*added*/
bool Clinic::shouldStop() {
    PcoMutexLocker lock(mutex);
    return stocks[ItemType::PatientSick] == 0 && !verifyResources();
}

void Clinic::run() {
    if (hospitals.empty() || suppliers.empty()) {
        std::cerr << "You have to give to hospitals and suppliers to run a clinic" << std::endl;
        return;
    }
    interface->consoleAppendText(uniqueId, "[START] Factory routine");

    /*TODO*/
    while (isStillRunning()) {
        
        if (verifyResources()) {
            treatPatient();
        } else {
            orderResources();
        }

        /*added*/
        if(shouldStop()){
            stop();
        }
       
        interface->simulateWork();

        /*changed*/
        {
            PcoMutexLocker lock(mutex);
            interface->updateFund(uniqueId, money);
            interface->updateStock(uniqueId, &stocks);
        }

    }
    interface->consoleAppendText(uniqueId, "[STOP] Factory routine");
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
