#include <QApplication>

#include "utils.h"
#include "iwindowinterface.h"
#ifdef TESTING_MODE
#include "fakeinterface.h"
#else
#include "windowinterface.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    IWindowInterface* windowInterface;

    #ifdef TESTING_MODE
        windowInterface = new FakeInterface();
    #else
        WindowInterface::initialize(NB_SUPPLIER, NB_CLINICS, NB_HOSPITALS);
        windowInterface = new WindowInterface();
    #endif

    SellerInterface::setInterface(windowInterface);

    Utils utils = Utils(NB_SUPPLIER, NB_CLINICS, NB_HOSPITALS);
    windowInterface->setUtils(&utils);

    return a.exec();
}
