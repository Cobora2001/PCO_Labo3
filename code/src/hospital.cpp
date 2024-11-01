#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

IWindowInterface* Hospital::interface = nullptr;

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : Seller(fund, uniqueId),
      maxBeds(maxBeds),
      currentBeds(0),
      nbHospitalised(0),
      nbFree(0),
      mutex(),
      mutexInterface(),
      healedPatientsQueue({0})
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Hospital Created with " + QString::number(maxBeds) + " beds");
    
    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
}

int& Hospital::getNumberSick() {
    return stocks[ItemType::PatientSick];
}

int& Hospital::getNumberHealed() {
    return stocks[ItemType::PatientHealed];
}

void Hospital::updateInterface() {
    interface->updateFund(uniqueId, money);
    interface->updateStock(uniqueId, &stocks);
}

int Hospital::request(ItemType what, int qty){
    if (what == ItemType::PatientSick && qty > 0) {
        static int patientCost = getCostPerUnit(ItemType::PatientSick);
        int totalBenefit = qty * patientCost;
        mutex.lock();
        if(getNumberSick() >= qty) {
            getNumberSick() -= qty;
            currentBeds -= qty;
            money += totalBenefit;
            mutex.unlock();

            mutexInterface.lock();
            interface->consoleAppendText(uniqueId, QString("Provided " + QString::number(qty) + " sick patient" + (qty > 1 ? "s" : "")));
            updateInterface();
            mutexInterface.unlock();

            return totalBenefit;
        }
        mutex.unlock();
    }
    
    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "Refused request for " + QString::number(qty) + " " + getItemName(what));
    mutexInterface.unlock();
    return 0;
}

void Hospital::freeHealedPatient() {
    mutex.lock();
    int nbLetGo = healedPatientsQueue[0];
    nbFree += nbLetGo;
    getNumberHealed() -= nbLetGo;
    money += nbLetGo * BENEFIT_OF_HEALING;
    for(int i = 0; i < NB_DAYS_OF_REST - 1; ++i) {
        healedPatientsQueue[i] = healedPatientsQueue[i+1];
    }
    healedPatientsQueue[NB_DAYS_OF_REST - 1] = 0;
    mutex.unlock();

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "Let go " + QString::number(nbLetGo) + " healed patient" + nbLetGo > 1 ? "s" : "");
    updateInterface();
    mutexInterface.unlock();
}

void Hospital::transferPatientsFromClinic() {
    static int employeeSalary = getEmployeeSalary(EmployeeType::Nurse);
    static int transferCost = getCostPerUnit(ItemType::PatientHealed) + employeeSalary;
    mutex.lock();
    if (currentBeds >= maxBeds || money < transferCost) {
        mutex.unlock();
        mutexInterface.lock();
        interface->consoleAppendText(uniqueId, "No capacity to transfer patients from clinic");
        mutexInterface.unlock();
        return;
    }

    std::vector<Seller*> clinicsTried;
    do
    {
        Seller* chosenClinic = Seller::chooseRandomSeller(clinics);
        if (std::find(clinicsTried.begin(), clinicsTried.end(), chosenClinic) != clinicsTried.end()) {
            continue;
        }
        clinicsTried.push_back(chosenClinic);

        bool clinicAvailable = true;

        do
        {
            int cost = chosenClinic->request(ItemType::PatientHealed, 1);
            if (cost > 0) {
                if(cost != getCostPerUnit(ItemType::PatientHealed)) {
                    printf("Error: cost of healing is not correct\n");
                }
                ++getNumberHealed();
                ++currentBeds;
                money -= cost;
                money -= employeeSalary;
                ++nbHospitalised;
                ++healedPatientsQueue[NB_DAYS_OF_REST - 1]; // Ajouter un patient avec NB_DAYS_OF_REST jours de repos restants

                mutexInterface.lock();
                interface->consoleAppendText(uniqueId, QString("Transferred a patient from clinic %1").arg(chosenClinic->getUniqueId()));
                updateInterface();
                mutexInterface.unlock();
            } else {
                clinicAvailable = false;

                mutexInterface.lock();
                interface->consoleAppendText(uniqueId, "Failed to transfer patient from clinic");
                mutexInterface.unlock();
            }
        } while (clinicAvailable && currentBeds < maxBeds && money >= transferCost);
        
    } while (clinicsTried.size() < clinics.size() && currentBeds < maxBeds && money >= transferCost);
    
    mutex.unlock();
}

int Hospital::send(ItemType it, int qty, int bill) {
    if(it == ItemType::PatientSick && qty > 0) {
        static int employeeSalary = getEmployeeSalary(EmployeeType::Nurse);
        int totalCost = qty * employeeSalary + bill;
        mutex.lock();
        if (money >= totalCost && qty <= (maxBeds - currentBeds)) {
            getNumberSick() += qty;
            currentBeds += qty;
            money -= totalCost;
            nbHospitalised += qty;
            mutex.unlock();

            mutexInterface.lock();
            updateInterface();
            mutexInterface.unlock();

            return qty;
        }
        mutex.unlock();
    }

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "Refused request for " + QString::number(qty) + " " + getItemName(it));
    mutexInterface.unlock();

    return 0;
}

void Hospital::run()
{
    if (clinics.empty()) {
        std::cerr << "You have to give clinics to a hospital before launching is routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Hospital routine");

    printf("Hospital %d started\n", uniqueId);

    while (!finished) {
        transferPatientsFromClinic();

        freeHealedPatient();

        mutexInterface.lock();
        updateInterface();
        mutexInterface.unlock();

        interface->simulateWork(); // Temps d'attente
    }

    printf("Hospital %d has finished\n", uniqueId);

    mutexInterface.lock();
    interface->consoleAppendText(uniqueId, "[STOP] Hospital routine");
    mutexInterface.unlock();
}

int Hospital::getAmountPaidToWorkers() {
    return nbHospitalised * getEmployeeSalary(EmployeeType::Nurse);
}

int Hospital::getNumberPatients(){
    return getNumberSick() + getNumberHealed() + nbFree;
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

int Hospital::getFundingFromHealed() {
    return nbFree * BENEFIT_OF_HEALING;
}
