#ifndef SUPPLIER_H
#define SUPPLIER_H
#include <QTimer>
#include <pcosynchro/pcomutex.h>

#include "iwindowinterface.h"

#include "costs.h"
#include "seller.h"

class Supplier : public Seller {
public:
    /**
     * @brief Supplier Constructor
     * @param uniqueId : ID du fournisseur
     * @param fund : Argent initial
     * @param resourcesSupplied : Liste des ressources fournies par ce Supplier
     */
    Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied);

    /**
     * @brief Obtenir les items à vendre
     * @return Les items dans les stocks à vendre sous forme d'une map (clé : type d'item, valeur : quantité)
     */
    std::map<ItemType, int> getItemsForSale() override;

    /**
     * @brief Fonction permettant de proposer des ressources au vendeur
     * @param what Le type de resource à acheter
     * @param qty Nombre de ressources voulant être achetées
     * @param bill Le coût de la transaction
     * @return La quantité acceptée ou la facture (peu dépendre de votre logique) et 0 si la transaction n'est pas acceptée.
     */
    int send(ItemType it, int qty, int bill) override;

    /**
     * @brief Fonction permettant de proposer des ressources au vendeur
     * @param what Le type de resource
     * @param qty Nombre de ressources
     * @param bill Le coût de la transaction
     * @return La quantité acceptée ou la facture (peu dépendre de votre logique) et 0 si la transaction n'est pas acceptée.
     */
    int request(ItemType what, int qty) override;

    /**
     * @brief Gérer l'opération du fournisseur, mise à jour des stocks et paiement des employés
     * Cette fonction gère l'augmentation des stocks, les transactions, ainsi que la gestion des employés.
     */
    void run();

    /**
     * @brief Obtenir le coût des matériaux
     * @return Le coût total des matériaux pour les items fournis
     */
    int getMaterialCost();

    /**
     * @brief Obtenir le montant total payé aux employés
     * @return Le montant total des paiements effectués pour les travailleurs
     */
    int getAmountPaidToWorkers();

    /**
     * @brief Configurer l'interface graphique pour l'affichage des informations
     * @param windowInterface : Pointeur vers l'interface graphique utilisée pour afficher les logs et mises à jour
     */
    static void setInterface(IWindowInterface* windowInterface);

    /**
     * @brief Obtenir la liste des ressources fournies par ce fournisseur
     * @return Un vecteur contenant les types d'items fournis
     */
    std::vector<ItemType> getResourcesSupplied() const;

protected:
    std::vector<ItemType> resourcesSupplied;  // Liste des items que ce fournisseur gère
    int nbSupplied;  // Nombre total d'items fournis
    static IWindowInterface* interface;  // Interface pour les logs et mises à jour
};



class MedicalDeviceSupplier : public Supplier {
public:
    /**
     * @brief Constructeur de MedicalDeviceSupplier
     * Initialise un fournisseur spécialisé dans les dispositifs médicaux.
     * @param uniqueId : ID du fournisseur
     * @param fund : Argent initial disponible pour ce fournisseur
     */
    MedicalDeviceSupplier(int uniqueId, int fund)
        : Supplier(uniqueId, fund, {ItemType::Scalpel, ItemType::Thermometer, ItemType::Stethoscope}) {
        // Log de création spécifique à un fournisseur d'outils médicaux
        interface->consoleAppendText(uniqueId, QString("Medical Tool Supplier Created"));
    }
};

class Pharmacy : public Supplier {
public:
    /**
     * @brief Constructeur de Pharmacy
     * Initialise un fournisseur spécialisé dans les articles de pharmacie.
     * @param uniqueId : ID du fournisseur
     * @param fund : Argent initial disponible pour ce fournisseur
     */
    Pharmacy(int uniqueId, int fund)
        : Supplier(uniqueId, fund, {ItemType::Syringe, ItemType::Pill}) {
        // Log de création spécifique à une pharmacie
        interface->consoleAppendText(uniqueId, QString("Pharmacy Created"));
    }
};

#endif // SUPPLIER_H