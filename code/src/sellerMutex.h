#ifndef SELLERMUTEX_H
#define SELLERMUTEX_H

#include "sellerInterface.h"
#include <pcosynchro/pcomutex.h>

// Classe SellerMutex is a subclass of SellerInterface

class SellerMutex : public SellerInterface {
public:
    /**
     * @brief SellerMutex
     * @param money money money !
     * @param uniqueId Identifiant unique du vendeur
     */
    SellerMutex(int money, int uniqueId);

protected:

    /**
     * @brief updateInterface
     * @param message message to print to the inteface
     * Updates the interface with the current state of the seller and prints a message
     */
    void updateWithMessage(QString message);

    /**
     * @brief updateInterface
     * Updates the interface with the current state of the seller
     */
    void updateInterface() override;

    /**
     * @brief interfaceMessage
     * @param message
     * Sends a message to the interface
     */
    void interfaceMessage(QString message) override;

    /**
     * @brief lockMutex
     * Locks the mutex
     */
    void lockMutex() { mutex.lock(); }

    /**
     * @brief unlockMutex
     * Unlocks the mutex
     */
    void unlockMutex() { mutex.unlock(); }

    /**
     * @brief updateStock
     * Updates the interface with the current stock of the seller
     */
    void updateStock() override;

    /**
     * @brief updateMoney
     * Updates the interface with the current money of the seller
     */
    void updateMoney() override;

private:
    PcoMutex mutex;                     // Mutex pour la synchronisation des ressources partagées
    PcoMutex mutexInterface;            // Mutex pour la synchronisation de l'interface utilisateur
};

#endif // SELLERMUTEX_H
