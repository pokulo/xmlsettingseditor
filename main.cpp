#include <QtGui/QApplication>
#include "xmlsettingseditorwrapper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XMLSettingsEditorWrapper w;
    w.show();

    return a.exec();
}
