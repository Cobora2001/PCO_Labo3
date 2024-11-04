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

    /**
     * @brief buyFromSeller
     * @param seller The seller to buy from
     * @param item The item to buy
     * @param qty The quantity to buy
     * @param costExpected The expected cost of the transaction
     * @return true if the transaction was successful, false otherwise
     * Buys an item from a seller (we expect that the costExpected was already checked to be less or equal than money, and the money was already locked)
     */
    bool buyFromSeller(Seller* seller, ItemType item, int qty, int costExpected);

    /**
     * @brief buyFromSellers
     * @param sellers The list of sellers to buy from
     * @param item The item to buy
     * @param maxQty The maximum quantity to buy
     * @param numberPerOrder The number of items to buy per order
     * @return The total quantity bought
     * Buys an item from a list of sellers (we expect other conditions to be checked and reserved before calling this function)
     */
    int buyFromSellers(std::vector<Seller*> sellers, ItemType item, int maxQty, int costPerOrder = -1, int numberPerOrder = 1);

private:
    PcoMutex mutex;                     // Mutex pour la synchronisation des ressources partagées
    PcoMutex mutexInterface;            // Mutex pour la synchronisation de l'interface utilisateur
};

#endif // SELLERMUTEX_H
