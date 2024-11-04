#include "ambulance.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

Ambulance::Ambulance(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied, std::map<ItemType, int> initialStocks)
    : SellerInterface(fund, uniqueId), resourcesSupplied(resourcesSupplied), nbTransfer(0) 
{
    for (const auto& item : resourcesSupplied) {
        if (initialStocks.find(item) != initialStocks.end()) {
            stocks[item] = initialStocks[item];
        } else {
            stocks[item] = 0;
        }
    }

    interfaceMessage(QString("Ambulance Created"));
    updateInterface();
}

int& Ambulance::getNumberSick(){
    return stocks[ItemType::PatientSick];
}

void Ambulance::sendPatient(){
    if(getNumberPatients() <= 0){
        interfaceMessage(QString("No patient to send"));
        return;
    }

    Seller* chosenHospital = chooseRandomSeller(hospitals);

    if(!chosenHospital){
        interfaceMessage(QString("No hospital to send patient"));
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

        interfaceMessage(QString("Sent %1 patient to hospital %2")
            .arg(MAX_PATIENTS_PER_TRANSFER)
            .arg(chosenHospital->getUniqueId()));
    } else {
        interfaceMessage(QString("Failed to send patient to hospital"));
    }
}

void Ambulance::run() {
    interfaceMessage(QString("[START] Ambulance routine"));

    while (!finished && getNumberPatients() > 0) {
    
        sendPatient();
        
        simulateWork();

        updateInterface();
    }

    interfaceMessage(QString("[STOP] Ambulance routine"));
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

void Ambulance::setHospitals(std::vector<Seller*> hospitals){
    this->hospitals = hospitals;

    for (Seller* hospital : hospitals) {
        setLink(hospital->getUniqueId());
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
