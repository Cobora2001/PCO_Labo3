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
      mutex(new PcoMutex()),
      mutexInterface(new PcoMutex),
      healedPatientsQueue(new std::array<int,NB_DAYS_OF_REST>())
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
        mutex.lock();
        if(getNumberSick < qty) {
            int totalCost = qty * getCostPerUnit(ItemType::PatientSick);

            getNumberSick -= qty;
            currentBeds -= qty;
            money += totalCost;
            mutex.unlock();

            mutexInterface.lock();
            interface->consoleAppendText(uniqueId, QString("Provided " + qty + " sick patient" + qty > 1 ? "s" : ""));
            updateInterface();
            mutexInterface.unlock();

            return totalCost;
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
    money += nbLetGo * PRICE_OF_HEALING;
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
    // TODO
}

int Hospital::send(ItemType it, int qty, int bill) {
    if(it == ItemType::PatientSick && qty > 0) {
        mutex.lock();
        if (money >= bill && qty <= (maxBeds - currentBeds)) {
            getNumberSick() += qty;
            currentBeds += qty;
            money -= bill;
            mutex.unlock();

            mutexInterface.lock();
            updateInterface();
            mutexInterface.unlock();

            nbHospitalised += qty;

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

    while (!finished) {
        transferPatientsFromClinic();

        freeHealedPatient();

        mutexInterface.lock();
        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
        mutexInterface.unlock();

        interface->simulateWork(); // Temps d'attente
    }

    interface->consoleAppendText(uniqueId, "[STOP] Hospital routine");
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
