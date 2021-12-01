#ifndef ELOLINKDIALOG_H
#define ELOLINKDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QFormLayout>
#include <QComboBox>
#include <QListView>

#include "elosettings.h"

class ELOLinkDialog : public QDialog
{
    Q_OBJECT
public:
    ELOLinkDialog(QWidget *parent=nullptr);
    void showShortcuts();

private:
    ELOSettings *settings;
    QDialogButtonBox *buttonBox;
    QTabWidget *tabWidget;
    QFormLayout *mainLayout;
    QLineEdit *linkTextEdit;
    QLineEdit *urlEdit;
    QComboBox *shortcutComboBox;
    QLineEdit *externalPathEdit;
    QListView *internalView;

private slots:
    void accept();
    void reject();

    void openFileFromShotcut();
    void openDirectoryFromShortcut();

signals:
    void setLinkRequest(const QString &text, const QString &path);
};

#endif // ELOLINKDIALOG_H
