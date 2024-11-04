#include "hospital.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

Hospital::Hospital(int uniqueId, int fund, int maxBeds)
    : SellerMutex(fund, uniqueId),
      maxBeds(maxBeds),
      currentBeds(0),
      nbHospitalised(0),
      nbFree(0),
      healedPatientsQueue({0})
{    
    lockMutex();
    std::vector<ItemType> initialStocks = { ItemType::PatientHealed, ItemType::PatientSick };

    for(const auto& item : initialStocks) {
        stocks[item] = 0;
    }
    unlockMutex();

    updateWithMessage("Hospital Created with " + QString::number(maxBeds) + " beds");
}

int& Hospital::getNumberSick() {
    return stocks[ItemType::PatientSick];
}

int& Hospital::getNumberHealed() {
    return stocks[ItemType::PatientHealed];
}

int Hospital::request(ItemType what, int qty){
    if (what == ItemType::PatientSick && qty > 0) {
        static int patientCost = getCostPerUnit(ItemType::PatientSick);
        int totalBenefit = qty * patientCost;
        lockMutex();
        if(getNumberSick() >= qty) {
            getNumberSick() -= qty;
            currentBeds -= qty;
            money += totalBenefit;
            unlockMutex();

            updateWithMessage("Provided " + QString::number(qty) + " sick patient" + (qty > 1 ? "s" : ""));

            return totalBenefit;
        }
        unlockMutex();
    }
    
    interfaceMessage("Refused request for " + QString::number(qty) + " " + getItemName(what));
    return 0;
}

void Hospital::freeHealedPatient() {
    lockMutex();
    int nbLetGo = healedPatientsQueue[0];
    nbFree += nbLetGo;
    getNumberHealed() -= nbLetGo;
    currentBeds -= nbLetGo;
    money += nbLetGo * BENEFIT_OF_HEALING;
    for(int i = 0; i < NB_DAYS_OF_REST - 1; ++i) {
        healedPatientsQueue[i] = healedPatientsQueue[i+1];
    }
    healedPatientsQueue[NB_DAYS_OF_REST - 1] = 0;
    unlockMutex();

    updateWithMessage("Let go " + QString::number(nbLetGo) + " healed patient" + (nbLetGo > 1 ? "s" : ""));
}

void Hospital::transferPatientsFromClinic() {
    static int employeeSalary = getEmployeeSalary(EmployeeType::Nurse);
    static int costPerHealed = getCostPerUnit(ItemType::PatientHealed);
    static int transferCost = costPerHealed + employeeSalary;
    lockMutex();
    if (currentBeds >= maxBeds || money < transferCost) {
        unlockMutex();
        interfaceMessage("No capacity to transfer patients from clinic");
        return;
    }
    unlockMutex();

    std::vector<Seller*> clinicsTried;
    bool enoughMoney = true;
    bool enoughBeds = true;

    while (clinicsTried.size() < clinics.size() && enoughBeds && enoughMoney) {
        Seller* chosenClinic = Seller::chooseRandomSeller(clinics);
        if (std::find(clinicsTried.begin(), clinicsTried.end(), chosenClinic) != clinicsTried.end()) {
            continue;
        }
        clinicsTried.push_back(chosenClinic);

        bool clinicAvailable = true;

        while (clinicAvailable && enoughBeds && enoughMoney) {

            lockMutex();
            if(currentBeds >= maxBeds) {
                enoughBeds = false;
                unlockMutex();
            } else if(money < transferCost) {
                enoughMoney = false;
                unlockMutex();
            } else {
                money -= transferCost;
                ++currentBeds;
                unlockMutex();
                
                int cost = chosenClinic->request(ItemType::PatientHealed, 1);

                if (cost > 0) {
                    if(cost != costPerHealed) {
                        std::cerr << "Error: cost of healing is not correct" << std::endl;
                    }
                    lockMutex();
                    ++getNumberHealed();
                    ++nbHospitalised;
                    ++healedPatientsQueue[NB_DAYS_OF_REST - 1]; // Ajouter un patient avec NB_DAYS_OF_REST jours de repos restants
                    unlockMutex();

                    updateWithMessage("Transferred a patient from clinic " + QString::number(chosenClinic->getUniqueId()));
                } else {
                    clinicAvailable = false;
                    lockMutex();
                    money += transferCost;
                    --currentBeds;
                    unlockMutex();

                    interfaceMessage("No healed patient available at clinic " + QString::number(chosenClinic->getUniqueId()));
                }
            }
        }
    }
}

int Hospital::send(ItemType it, int qty, int bill) {
    if(it == ItemType::PatientSick && qty > 0) {
        static int employeeSalary = getEmployeeSalary(EmployeeType::Nurse);
        int totalCost = qty * employeeSalary + bill;
        lockMutex();
        if (money >= totalCost && qty <= (maxBeds - currentBeds)) {

            getNumberSick() += qty;
            currentBeds += qty;
            money -= totalCost;
            nbHospitalised += qty;
            unlockMutex();

            updateWithMessage("Received " + QString::number(qty) + " sick patient" + (qty > 1 ? "s" : ""));

            return qty;
        }
        unlockMutex();
    }

    interfaceMessage("Refused request for " + QString::number(qty) + " " + getItemName(it));

    return 0;
}

void Hospital::run()
{
    if (clinics.empty()) {
        std::cerr << "You have to give clinics to a hospital before launching is routine" << std::endl;
        return;
    }

    interfaceMessage("[START] Hospital routine");

    while (!finished) {
        transferPatientsFromClinic();

        freeHealedPatient();

        updateInterface();

        simulateWork();
    }

    interfaceMessage("[STOP] Hospital routine");
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
        setLink(clinic->getUniqueId());
    }
}

int Hospital::getFundingFromHealed() {
    return nbFree * BENEFIT_OF_HEALING;
}
