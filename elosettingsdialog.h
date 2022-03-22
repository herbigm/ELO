#ifndef ELOSETTINGSDIALOG_H
#define ELOSETTINGSDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>

#include "elosettings.h"

class ELOSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    ELOSettingsDialog(QWidget *parent=nullptr);

private:
    QVBoxLayout *mainLayout;
    QDialogButtonBox *buttonBox;
    QTabWidget *centralTabWidget;
    QLineEdit *editWd;

    QLineEdit *shortcutEdit;
    QLineEdit *shortcutPathEdit;
    QLineEdit *shortcutDescriptionEdit;
    QListWidget *shortcutList;
    QLabel *shortcutPathLabel;
    QLabel *shortcutDescriptionLabel;
    QPushButton *buttonSavePs;
    QPushButton *buttonEditPs;
    QPushButton *buttonDeletePs;

    ELOSettings *originalSettings;
    QString tmp_workingDir;
    QVector<PathShortcut> tmp_PathShortcuts;

    QCheckBox *checkboxSc;
    QComboBox *comboSc;

    int shortcutEditIndex;


private slots:
    void accept();
    void reject();

    void openWd();
    void checkShortcutInput(const QString &text);
    void openDirectoryPs();
    void savePathShortcut();
    void showPathShortcuts();
    void editPathshortcut();
    void deletePathShortcut();
    void shortcutSelected(int currentRow);
    void spellCheckState(bool checked);

signals:
    void spellCheckChanged(bool);
    void spellCheckLanguageChanged(QString);
};

#endif // ELOSETTINGSDIALOG_H
