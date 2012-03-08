#include <QtGui/QApplication>
#include "xmlsettingseditorwrapper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //ToDo maybe file selection dialog or parameter for filename
    QFile file("./../testdata/Config-Demo.xml");

    XMLSettingsEditorWrapper * w = new XMLSettingsEditorWrapper(file,0);
    w->show();

    return a.exec();
}
