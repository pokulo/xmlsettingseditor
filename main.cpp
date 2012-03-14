#include <QtGui/QApplication>
#include "xmlsettingseditorwrapper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    XMLSettingsEditorWrapper * w = new XMLSettingsEditorWrapper(0);
    w->show();

    return a.exec();
}
