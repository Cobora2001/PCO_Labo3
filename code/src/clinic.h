#ifndef CLINIC_H
#define CLINIC_H

#include <vector>
#include <pcosynchro/pcomutex.h>

#include "iwindowinterface.h"
#include "seller.h"

/**
 * @brief La classe Clinic permet l'implémentation d'une clinique et de ses fonctions
 *        de gestion des patients, héritant de la classe Seller.
 */
class Clinic : public Seller
{
public:
    /**
     * @brief Constructeur de la classe Clinic
     * @param uniqueId Identifiant unique de la clinique
     * @param fund Capital initial de la clinique
     * @param resourcesNeeded Liste des ressources nécessaires au fonctionnement de la clinique
     */
    Clinic(int uniqueId, int fund, std::vector<ItemType> resourcesNeeded);

    /**
     * @brief run
     * La routine principale de la clinique, qui appelle les fonctions privées pour que la clinique fonctionne continuellement.
     * Cette méthode est exécutée dans un thread séparé pour permettre un fonctionnement asynchrone.
     */
    void run();

    /**
     * @brief getItemsForSale
     * @return Retourne une map représentant les items (patients) disponibles à la clinique,
     *         avec le type d'item comme clé et la quantité comme valeur.
     */
    std::map<ItemType, int> getItemsForSale() override;

    /**
     * @brief getWaitingPatients
     * @return Le nombre de patients en attente d'être traités dans la clinique.
     */
    int getWaitingPatients();

    /**
     * @brief Fonction permettant de proposer des ressources au vendeur
     * @param what Le type de resource
     * @param qty Nombre de ressources
     * @param bill Le coût de la transaction
     * @return La quantité acceptée ou la facture (peu dépendre de votre logique) et 0 si la transaction n'est pas acceptée.
     */
    int send(ItemType it, int qty, int bill) override;

    /**
     * @brief Fonction permettant d'acheter des ressources au vendeur
     * @param what Le type de resource à acheter
     * @param qty Nombre de ressources voulant être achetées
     * @return La facture : côut de la resource * le nombre, 0 si indisponible
     */
    int request(ItemType what, int qty) override;

    /**
     * @brief getTreatmentCost
     * @return Le coût du traitement d'un patient dans la clinique.
     */
    int getTreatmentCost();

    /**
     * @brief setHospitalsAndSuppliers
     * Permet d'affecter plusieurs hôpitaux et fournisseurs à la clinique pour faciliter les échanges.
     * @param hospitals Vecteur d'hôpitaux avec lesquels la clinique va interagir
     * @param suppliers Vecteur de fournisseurs avec lesquels la clinique va travailler
     */
    void setHospitalsAndSuppliers(std::vector<Seller*> hospitals, std::vector<Seller*> suppliers);

    int getNumberPatients();

    /**
     * @brief getAmountPaidToWorkers
     * @return Le montant total payé aux travailleurs de la clinique.
     */
    int getAmountPaidToWorkers();

    /**
     * @brief setInterface
     * @param windowInterface Pointeur vers l'interface graphique utilisée pour afficher les logs et mises à jour
     * Configure l'interface pour l'affichage des actions de la clinique.
     */
    static void setInterface(IWindowInterface* windowInterface);

private:
    std::vector<Seller*> suppliers;    // Liste des fournisseurs de ressources nécessaires à la clinique
    std::vector<Seller*> hospitals;     // Liste des hôpitaux associés à la clinique

    const std::vector<ItemType> resourcesNeeded; // Liste des ressources requises pour le fonctionnement de la clinique

    int nbTreated;                      // Nombre total de patients traités par la clinique

    static IWindowInterface* interface; // Pointeur statique vers l'interface utilisateur pour les logs et mises à jour visuelles

    /**
     * @brief orderResources
     * Fonction pour acheter des ressources nécessaires au traitement des patients chez les fournisseurs.
     */
    void orderResources();

    /**
     * @brief treatPatient
     * Gère le traitement d'un patient dans la clinique, incluant la vérification des ressources nécessaires.
     */
    void treatPatient();

    /**
     * @brief verifyResources
     * Vérifie si les ressources nécessaires au traitement des patients sont disponibles.
     * @return true si les ressources sont suffisantes, false sinon.
     */
    bool verifyResources();
};

class Pulmonology : public Clinic {
public:
    /**
     * @brief Constructeur de la spécialisation Pulmonology
     * Initialise une clinique spécialisée en pneumologie.
     * @param uniqueId Identifiant unique de la clinique
     * @param fund Capital initial de la clinique
     */
    Pulmonology(int uniqueId, int fund);
};

class Cardiology : public Clinic {
public:
    /**
     * @brief Constructeur de la spécialisation Cardiology
     * Initialise une clinique spécialisée en cardiologie.
     * @param uniqueId Identifiant unique de la clinique
     * @param fund Capital initial de la clinique
     */
    Cardiology(int uniqueId, int fund);
};

class Neurology : public Clinic {
public:
    /**
     * @brief Constructeur de la spécialisation Neurology
     * Initialise une clinique spécialisée en neurologie.
     * @param uniqueId Identifiant unique de la clinique
     * @param fund Capital initial de la clinique
     */
    Neurology(int uniqueId, int fund);
};

#endif // CLINIC_H
