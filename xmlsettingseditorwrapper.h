#ifndef XMLSETTINGSEDITORWRAPPER_H
#define XMLSETTINGSEDITORWRAPPER_H

#include <QMainWindow>
#include <QModelIndex>
#include <QSettings>

namespace Ui {
    class XMLSettingsEditorWrapper;
}

class XMLSettingsEditorWrapper : public QMainWindow
{
    Q_OBJECT

public:
    explicit XMLSettingsEditorWrapper(QWidget *parent = 0);
    ~XMLSettingsEditorWrapper();

private:
    Ui::XMLSettingsEditorWrapper *ui;
};

#endif // XMLSETTINGSEDITORWRAPPER_H
