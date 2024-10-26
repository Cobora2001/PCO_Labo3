#include "ambulance.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

IWindowInterface* Ambulance::interface = nullptr;

Ambulance::Ambulance(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied, std::map<ItemType, int> initialStocks)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbTransfer(0) 
{
    interface->consoleAppendText(uniqueId, QString("Ambulance Created"));

    for (const auto& item : resourcesSupplied) {
        if (initialStocks.find(item) != initialStocks.end()) {
            stocks[item] = initialStocks[item];
        } else {
            stocks[item] = 0;
        }
    }

    interface->updateFund(uniqueId, fund);
}

int& Ambulance::getNumberSick(){
    return stocks[ItemType::PatientSick];
}

void Ambulance::sendPatient(){
    if(getNumberPatients() <= 0){
        interface->consoleAppendText(uniqueId, QString("No patient to send"));
        return;
    }

    Seller* chosenHospital = chooseRandomSeller(hospitals);

    if(!chosenHospital){
        interface->consoleAppendText(uniqueId, QString("No hospital to send patient"));
        return;
    }

    static int patientCost = getCostPerUnit(ItemType::PatientSick);
    int sent = chosenHospital->send(ItemType::PatientSick,
                                    MAX_PATIENTS_PER_TRANSFER,
                                    MAX_PATIENTS_PER_TRANSFER * patientCost);

    if(sent > 0){
        static int employeeSalary = getEmployeeSalary(EmployeeType::Supplier);

        --getNumberSick();
        money += patientCost;
        money -= employeeSalary;
        ++nbTransfer;

        interface->consoleAppendText(uniqueId, QString("Sent %1 patient to hospital %2")
            .arg(MAX_PATIENTS_PER_TRANSFER)
            .arg(chosenHospital->getUniqueId()));
    } else {
        interface->consoleAppendText(uniqueId, QString("Failed to send patient to hospital"));
    }
}

void Ambulance::run() {
    interface->consoleAppendText(uniqueId, "[START] Ambulance routine");

    while (!finished && getNumberPatients() > 0) {
    
        sendPatient();
        
        interface->simulateWork();

        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
    }

    interface->consoleAppendText(uniqueId, "[STOP] Ambulance routine");
}

std::map<ItemType, int> Ambulance::getItemsForSale() {
    return stocks;
}

int Ambulance::getMaterialCost() {
    int totalCost = 0;
    for (const auto& item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

int Ambulance::getAmountPaidToWorkers() {
    return nbTransfer * getEmployeeSalary(EmployeeType::Supplier);
}

int Ambulance::getNumberPatients(){
    return stocks[ItemType::PatientSick];
}

void Ambulance::setInterface(IWindowInterface *windowInterface) {
    interface = windowInterface;
}


void Ambulance::setHospitals(std::vector<Seller*> hospitals){
    this->hospitals = hospitals;

    for (Seller* hospital : hospitals) {
        interface->setLink(uniqueId, hospital->getUniqueId());
    }
}

int Ambulance::send(ItemType it, int qty, int bill) {
    return 0;
}


int Ambulance::request(ItemType what, int qty){
    return 0;
}

std::vector<ItemType> Ambulance::getResourcesSupplied() const
{
    return resourcesSupplied;
}
