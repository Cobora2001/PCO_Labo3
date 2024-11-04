#ifndef SELLERINTERFACE_H
#define SELLERINTERFACE_H

#include "seller.h"
#include "iwindowinterface.h"

// Classe SellerMutex is a subclass of Seller

class SellerInterface : public Seller {
public:
    /**
     * @brief SellerInteface
     * @param money money money !
     * @param uniqueId Identifiant unique du vendeur
     */
    SellerInterface(int money, int uniqueId) : Seller(money, uniqueId) {}

    /**
     * @brief setInterface
     * @param windowInterface Pointeur vers l'interface graphique utilisée pour afficher les logs et mises à jour
     * Configure l'interface pour l'affichage des actions du SellerInterface.
     */
    static void setInterface(IWindowInterface* windowInterface);

protected:
    static IWindowInterface* interface; // Pointeur statique vers l'interface utilisateur pour les logs et mises à jour visuelles

    /**
     * @brief updateInterface
     * Updates the interface with the current state of the seller
     */
    virtual void updateInterface();

    /**
     * @brief interfaceMessage
     * @param message
     * Sends a message to the interface
     */
    virtual void interfaceMessage(QString message);

    /**
     * @brief simulateWork
     * Simulates work for the interface
     */
    void simulateWork();

};

#endif // SELLERINTERFACE_H