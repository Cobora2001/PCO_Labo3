#ifndef SELLERMUTEX_H
#define SELLERMUTEX_H

#include "sellerInteface.h"
#include <pcosynchro/pcomutex.h>

// Classe SellerMutex is a subclass of SellerInterface

class SellerMutex extends SellerInterface {
public:
    /**
     * @brief SellerMutex
     * @param money money money !
     */
    SellerMutex(int money, int uniqueId) {
        super(money, uniqueId);
    }

    /**
     * @brief updateInterface
     * @param message message to print to the inteface
     * Updates the interface with the current state of the seller and prints a message
     */
    void updateWithMessage(QString message);
    
protected:
    PcoMutex mutex;                     // Mutex pour la synchronisation des ressources partagées
    PcoMutex mutexInterface;            // Mutex pour la synchronisation de l'interface utilisateur

};

#endif // SELLERMUTEX_H
