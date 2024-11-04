#ifndef SELLERINTERFACE_H
#define SELLERINTERFACE_H

#include "seller.h"
#include "iwindowinterface.h"

// Classe SellerMutex is a subclass of Seller

class SellerInteface extends Seller {
public:
    /**
     * @brief SellerInteface
     * @param money money money !
     */
    SellerInteface(int money, int uniqueId) {
        super(money, uniqueId);
    }

    /**
     * @brief setInterface
     * @param windowInterface Pointeur vers l'interface graphique utilisée pour afficher les logs et mises à jour
     * Configure l'interface pour l'affichage des actions du SellerInterface.
     */
    static void setInterface(IWindowInterface* windowInterface);

    /**
     * @brief updateInterface
     * Updates the interface with the current state of the seller
     */
    void updateInterface();

    /**
     * @brief interfaceMessage
     * @param message
     * Sends a message to the interface
     */
    void interfaceMessage(QString message);

protected:
    static IWindowInterface* interface; // Pointeur statique vers l'interface utilisateur pour les logs et mises à jour visuelles

};

#endif // SELLERINTERFACE_H