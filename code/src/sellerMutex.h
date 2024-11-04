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
    PcoMutex mutex;                     // Mutex pour la synchronisation des ressources partagées
    PcoMutex mutexInterface;            // Mutex pour la synchronisation de l'interface utilisateur

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
    
};

#endif // SELLERMUTEX_H
