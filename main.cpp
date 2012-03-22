#include <QtGui/QApplication>
#include "xmlsettingseditor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    XMLSettingsEditor * w = new XMLSettingsEditor(0);
    w->show();

    return a.exec();
}
