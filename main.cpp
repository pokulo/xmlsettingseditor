#include <QtGui/QApplication>
#include "xmlsettingseditorwrapper.h"
#include <QFileDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //ToDo maybe file selection dialog or parameter for filename
    QFile file(QFileDialog::getOpenFileName(0,"Open XML file", "../testdata/"));

    XMLSettingsEditorWrapper * w = new XMLSettingsEditorWrapper(&file,0);
    w->show();

    return a.exec();
}
