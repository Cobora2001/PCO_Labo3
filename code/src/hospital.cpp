#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

IWindowInterface* Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId), maxBeds(maxBeds), currentBeds(0), nbHospitalised(0), nbFree(0)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");
    
    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
}

int Hospital::request(ItemType what, int qty){
    // TODO 
    PcoMutexLocker lock(mutex);

    if (what != ItemType::PatientSick || stocks[ItemType::PatientSick] < qty) {
        interface->consoleAppendText(uniqueId, "Not enough sick patients available");
        return 0;
    }

    stocks[ItemType::PatientSick] -= qty;
    currentBeds -= qty;
    int totalCost = qty * getCostPerUnit(ItemType::PatientSick);
    interface->consoleAppendText(uniqueId, QString("Provided %1 sick patients").arg(qty));

    return totalCost;
}

void Hospital::freeHealedPatient() {
    // TODO 
    PcoMutexLocker lock(mutex);

    if (!healedPatientsQueue.empty()) {
        int daysRemaining = healedPatientsQueue.front();
        daysRemaining--;

        if (daysRemaining <= 0) {
            healedPatientsQueue.pop();
            stocks[ItemType::PatientHealed]--;
            nbFree++;
            currentBeds--;
            interface->consoleAppendText(uniqueId, "Freed a healed patient");
        } else {
            healedPatientsQueue.front() = daysRemaining; // Mise à jour du compteur
        }
    }
}

void Hospital::transferPatientsFromClinic() {
    // TODO
        PcoMutexLocker lock(mutex);

    if (currentBeds >= maxBeds) {
        interface->consoleAppendText(uniqueId, "No available beds");
        return;
    }

    Seller* chosenClinic = chooseRandomSeller(clinics);
    if (!chosenClinic) {
        interface->consoleAppendText(uniqueId, "No clinic available to transfer patients");
        return;
    }

    int cost = chosenClinic->request(ItemType::PatientHealed, 1);
    if (cost > 0 && currentBeds < maxBeds) {
        stocks[ItemType::PatientHealed]++;
        currentBeds++;
        money -= cost;
        nbHospitalised++;
        healedPatientsQueue.push(5); // Ajouter un patient avec 5 jours de repos restants
        interface->consoleAppendText(uniqueId, "Transferred a patient from clinic " + QString::number(chosenClinic->getUniqueId()));
    } else {
        interface->consoleAppendText(uniqueId, "Failed to transfer patient from clinic");
    }
}

int Hospital::send(ItemType it, int qty, int bill) {
    // TODO
    PcoMutexLocker locker(mutex);

    if(it == ItemType::PatientSick|| qty >  (maxBeds - currentBeds)) {
        interface->consoleAppendText(uniqueId, "Refused to take patients");
        return 0;
    }

    stocks[ItemType::PatientSick] += qty;
    currentBeds += qty;
    money -= bill;

    nbHospitalised += qty;

    return qty;
}

/*added*/
bool Hospital::shouldStop() {
    PcoMutexLocker lock(mutex);
    return (stocks[ItemType::PatientSick] == 0 
            && stocks[ItemType::PatientHealed] == 0 
            && currentBeds == 0 
            && healedPatientsQueue.empty());
}

void Hospital::run()
{
    if (clinics.empty()) {
        std::cerr << "You have to give clinics to a hospital before launching is routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Hospital routine");

    /*added*/
    while (isStillRunning()) {
        transferPatientsFromClinic();

        freeHealedPatient();

        if(shouldStop()) {
            stop();
        }

        /*changed*/

        {
            PcoMutexLocker lock(mutex);
            interface->updateFund(uniqueId, money);
            interface->updateStock(uniqueId, &stocks);
        }
        
        interface->simulateWork(); // Temps d'attente
    }

    interface->consoleAppendText(uniqueId, "[STOP] Hospital routine");
}

int Hospital::getAmountPaidToWorkers() {
    return nbHospitalised * getEmployeeSalary(EmployeeType::Nurse);
}

int Hospital::getNumberPatients(){
    return stocks[ItemType::PatientSick] + stocks[ItemType::PatientHealed] + nbFree;
}

std::map<ItemType, int> Hospital::getItemsForSale()
{
    return stocks;
}

void Hospital::setClinics(std::vector<Seller*> clinics){
    this->clinics = clinics;

    for (Seller* clinic : clinics) {
        interface->setLink(uniqueId, clinic->getUniqueId());
    }
}

void Hospital::setInterface(IWindowInterface* windowInterface){
    interface = windowInterface;
}
