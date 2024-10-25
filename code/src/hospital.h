#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <vector>
#include <pcosynchro/pcomutex.h>

#include "iwindowinterface.h"
#include "seller.h"

/**
 * @brief The Hospital class
 * Gère un hôpital qui reçoit des patients malades des ambulances et des patients soignés des cliniques.
 * Hérite de la classe Seller, car les patients peuvent être "échangés" comme des ressources.
 */
class Hospital : public Seller 
{
public:
    /**
     * @brief Hospital Constructor
     * @param uniqueId L'identifiant unique de l'hôpital
     * @param fund L'argent initial de l'hôpital
     * @param maxBeds Le nombre maximum de lits disponibles à l'hôpital
     */
    Hospital(int uniqueId, int fund, int maxBeds);

    /**
     * @brief run
     * La boucle principale de l'hôpital, qui gère l'import de patients des ambulances et des cliniques.
     * Cette fonction est responsable de la gestion des transferts et de la logistique des patients.
     */
    void run();

    /**
    * @brief getItemsForSale
    * @return Retourne la map des patients présents à l'hôpital, avec la clé étant le type de patient (malade ou soigné) et la valeur la quantité.
    */
    std::map<ItemType, int> getItemsForSale() override;

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
     * @brief setClinics
     * @param clinics Une liste de cliniques avec lesquelles l'hôpital va interagir
     * Cette fonction configure les cliniques avec lesquelles l'hôpital va échanger des patients soignés.
     */
    void setClinics(std::vector<Seller*> clinics);

    int getNumberPatients();

    /**
     * @brief getAmountPaidToWorkers
     * @return Le montant total payé aux travailleurs de l'ambulance.
     */
    int getAmountPaidToWorkers();

    /**
     * @brief setInterface
     * @param windowInterface Pointeur vers l'interface graphique utilisée pour l'affichage des logs et mises à jour
     * Configure l'interface pour afficher les actions de l'hôpital, comme les transferts de patients.
     */
    static void setInterface(IWindowInterface* windowInterface);

private:
    /**
     * @brief transferPatientsFromClinic
     * Transfère des patients d'une clinique vers l'hôpital.
     * Cette fonction est appelée dans le cadre de l'intégration avec les cliniques et gère l'arrivée de patients soignés.
     */
    void transferPatientsFromClinic();

    /**
     * @brief buyResources
     * Fonction pour acheter des ressources, ici des patients, soit malades (d'ambulances) soit soignés (de cliniques).
     * Cette méthode est utilisée pour gérer l'approvisionnement en "ressources" humaines.
     */
    void buyResources();

    void freeHealedPatient();

    std::vector<Seller*> clinics;     // Liste des cliniques liées à l'hôpital, qui renvoient des patients soignés

    int maxBeds;        // Nombre maximum de lits disponibles à l'hôpital
    int currentBeds;    // Nombre actuel de lits occupés, représente le nombre de patients présents

    int nbHospitalised; //Nombre de transfert réussi vers l'hôpital (nombre de fois ou un(e) infirmier/infirmière est payé)

    int nbFree; // Nombre de personnes qui sont sorties soignées de l'hôpital.

    static IWindowInterface* interface;  // Pointeur statique vers l'interface utilisateur pour les logs et mises à jour visuelles
};

#endif // HOSPITAL_H
