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
    /**
     * @brief updateInterface
     * Updates the interface with the current state of the seller
     */
    virtual void updateInterface();

    /**
     * @brief updateStoock
     * Updates the interface with the current stock of the seller
     */
    virtual void updateStock();

    /**
     * @brief updateMoney
     * Updates the interface with the current money of the seller
     */
    virtual void updateMoney();

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

    /**
     * @brief setLink
     * @param id ID from the seller with which the link is established
     * Establishes a link between the seller and another seller
     */
    void setLink(int id);

private:
    static IWindowInterface* interface; // Pointeur statique vers l'interface utilisateur pour les logs et mises à jour visuelles

};

#endif // SELLERINTERFACE_H