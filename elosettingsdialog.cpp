#include "elosettingsdialog.h"

#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QCoreApplication>

ELOSettingsDialog::ELOSettingsDialog(QWidget *parent):
    QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint )
{
    this->setModal(true);
    this->hide();

    // resize
    resize(400,500);
    setWindowTitle(tr("ELO settings"));

    // load original settings
    originalSettings = TheELOSettings::Instance();

    // create Widgets
    // ButtonBox
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    centralTabWidget = new QTabWidget(this);
    mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    mainLayout->addWidget(centralTabWidget);
    mainLayout->addWidget(buttonBox);

    // working directory tab
    QWidget *widgetWd = new QWidget();
    QWidget *widgetWd2 = new QWidget();
    QVBoxLayout *layoutWd = new QVBoxLayout();
    QHBoxLayout *layoutWd2 = new QHBoxLayout();
    QLabel *descriptionWd = new QLabel(tr("The working directory ist the directory where all data ist saved.\n If you change the directory you have to restart ELO. All data ghave to be synchronised again."));
    QLabel *dtLabelWd = new QLabel(tr("Working directory"));
    editWd = new QLineEdit();
    editWd->setReadOnly(true);
    const QIcon buttonIconOpenFolder = QIcon::fromTheme("folder", QIcon(":icons/icons/openFolder.svg"));
    QPushButton *buttonWd = new QPushButton(buttonIconOpenFolder, tr("open Folder"), this);

    layoutWd->addWidget(descriptionWd);
    layoutWd->addWidget(widgetWd2);
    layoutWd->addStretch();
    dtLabelWd->setWordWrap(true);
    layoutWd2->addWidget(dtLabelWd);
    widgetWd2->setLayout(layoutWd2);
    layoutWd2->addWidget(editWd);
    layoutWd2->addWidget(buttonWd);
    widgetWd->setLayout(layoutWd);
    centralTabWidget->addTab(widgetWd, tr("Working directory"));

    connect(buttonWd, &QPushButton::pressed, this, &ELOSettingsDialog::openWd);

    // path shortcuts (Ps)
    QWidget *widgetPs = new QWidget();
    QVBoxLayout *layoutPs = new QVBoxLayout();
    QLabel *descriptionPs = new QLabel(tr("On each machine, external locations, e.g. network volumes, can be mountet in different paths. \nTo use universal links to these locations, a shortcut has to be defined. "));
    QGroupBox *groupBoxPs = new QGroupBox(tr("New shortcut"));
    QGroupBox *groupBoxPs2 = new QGroupBox(tr("Existing shortcut"));
    QGridLayout *layoutPs2 = new QGridLayout();
    QHBoxLayout *layoutPs3 = new QHBoxLayout();
    QFormLayout *layoutPs4 = new QFormLayout();
    QHBoxLayout *layoutPs5 = new QHBoxLayout();
    QWidget *widgetPs2 = new QWidget();
    QWidget *widgetPs3 = new QWidget();
    shortcutEdit = new QLineEdit();
    shortcutPathEdit = new QLineEdit();
    shortcutPathEdit->setReadOnly(true);
    shortcutDescriptionEdit = new QLineEdit();
    QPushButton *buttonOpenFolderPs = new QPushButton(buttonIconOpenFolder, tr("open Folder"), this);
    const QIcon buttonIconSave = QIcon::fromTheme("document-save", QIcon(":icons/icons/document-save.svg"));
    buttonSavePs = new QPushButton(buttonIconSave, tr("save"), this);
    buttonSavePs->setEnabled(false);
    shortcutList = new QListWidget();
    shortcutList->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));
    shortcutDescriptionLabel = new QLabel();
    shortcutPathLabel = new QLabel();
    const QIcon buttonIconEdit = QIcon::fromTheme("document-edit", QIcon(":icons/icons/document-edit.svg"));
    buttonEditPs = new QPushButton(buttonIconEdit, tr("edit"), this);
    buttonEditPs->setEnabled(false);
    const QIcon buttonIconDelete = QIcon::fromTheme("edit-delete", QIcon(":icons/icons/edit-delete.svg"));
    buttonDeletePs = new QPushButton(buttonIconDelete, tr("delete"), this);
    buttonDeletePs->setEnabled(false);

    widgetPs2->setLayout(layoutPs4);
    layoutPs4->addRow(new QLabel(tr("Path: ")), shortcutPathLabel);
    layoutPs4->addRow(new QLabel(tr("Description: ")), shortcutDescriptionLabel);
    layoutPs4->addRow(widgetPs3);

    widgetPs3->setLayout(layoutPs5);
    layoutPs5->addWidget(buttonEditPs);
    layoutPs5->addWidget(buttonDeletePs);

    layoutPs2->addWidget(new QLabel(tr("Shortcut: ")), 0, 0);
    layoutPs2->addWidget(new QLabel(tr("Path: ")), 1, 0);
    layoutPs2->addWidget(new QLabel(tr("Description: ")), 2, 0);
    layoutPs2->addWidget(shortcutEdit, 0, 1);
    layoutPs2->addWidget(shortcutPathEdit, 1, 1);
    layoutPs2->addWidget(shortcutDescriptionEdit, 2, 1);
    layoutPs2->addWidget(buttonOpenFolderPs, 1, 2);
    layoutPs2->addWidget(buttonSavePs, 3, 1);

    layoutPs3->addWidget(shortcutList);
    layoutPs3->addWidget(widgetPs2);

    groupBoxPs->setLayout(layoutPs2);
    groupBoxPs2->setLayout(layoutPs3);

    layoutPs->addWidget(descriptionPs);
    layoutPs->addWidget(groupBoxPs);
    layoutPs->addWidget(groupBoxPs2);
    widgetPs->setLayout(layoutPs);
    centralTabWidget->addTab(widgetPs, tr("Path shortcuts"));

    connect(shortcutEdit, &QLineEdit::textEdited, this, &ELOSettingsDialog::checkShortcutInput);
    connect(shortcutPathEdit, &QLineEdit::textChanged, this, &ELOSettingsDialog::checkShortcutInput);
    connect(buttonOpenFolderPs, &QPushButton::pressed, this, &ELOSettingsDialog::openDirectoryPs);
    connect(buttonSavePs, &QPushButton::pressed, this, &ELOSettingsDialog::savePathShortcut);
    connect(buttonEditPs, &QPushButton::pressed, this, &ELOSettingsDialog::editPathshortcut);
    connect(buttonDeletePs, &QPushButton::pressed, this, &ELOSettingsDialog::deletePathShortcut);
    connect(shortcutList, &QListWidget::currentRowChanged, this, &ELOSettingsDialog::shortcutSelected);

    // spell check
    QWidget *widgetSc = new QWidget();
    QVBoxLayout *layoutSc = new QVBoxLayout();
    QHBoxLayout *layoutSc2 = new QHBoxLayout();
    widgetSc->setLayout(layoutSc);
    checkboxSc = new QCheckBox(tr("enable spell check"), this);
    comboSc = new QComboBox();
    QDir d(QCoreApplication::applicationDirPath() + QDir::separator() + "qtwebengine_dictionaries");
    QStringList dicts = d.entryList({"*.bdic"});
    foreach (QString s, dicts) {
        comboSc->addItem(s.left(5));
    }

    layoutSc->addWidget(checkboxSc);
    layoutSc2->addWidget(new QLabel(tr("Language: ")));
    layoutSc2->addWidget(comboSc);
    layoutSc->addLayout(layoutSc2);
    layoutSc->addStretch();

    connect(checkboxSc, &QCheckBox::clicked, this, &ELOSettingsDialog::spellCheckState);

    centralTabWidget->addTab(widgetSc, tr("Spell Check"));

    // loadSettings
    tmp_workingDir = originalSettings->getWorkingDir();
    tmp_PathShortcuts = originalSettings->getPathShortCuts();
    comboSc->setCurrentText(originalSettings->getSpellCheckLanguage());
    checkboxSc->setChecked(originalSettings->getSpellCheck());

    // fill settings into form
    editWd->setText(tmp_workingDir);
    showPathShortcuts();

    // other initial settings
    shortcutEditIndex = -1;
}

void ELOSettingsDialog::accept()
{
    // set the new working directory
    originalSettings->setWorkingDir(tmp_workingDir);

    // set the new path shortcuts
    originalSettings->clearPathShortcuts();;
    for (int i = 0; i < tmp_PathShortcuts.length(); i++) {
        originalSettings->addPathShortCut(tmp_PathShortcuts[i]);
    }

    // spell check
    if (originalSettings->getSpellCheck() != checkboxSc->isChecked()) {
        emit spellCheckChanged(checkboxSc->isChecked());
        originalSettings->setSpellCheck(checkboxSc->isChecked());
    }
    if (originalSettings->getSpellCheckLanguage() != comboSc->currentText()) {
        originalSettings->setSpellCheckLanguage(comboSc->currentText());
        emit spellCheckLanguageChanged(comboSc->currentText());
    }

    // reset shortcutEditIndex
    shortcutEditIndex = -1;

    // hide the dialog
    hide();
}

void ELOSettingsDialog::reject()
{
    // remove made changes by restoring the settings from the source
    tmp_workingDir = originalSettings->getWorkingDir();
    tmp_PathShortcuts = originalSettings->getPathShortCuts();

    // reset shortcutEditIndex
    shortcutEditIndex = -1;

    // reset UI
    showPathShortcuts();

    // hide the dialog
    hide();
}

void ELOSettingsDialog::openWd()
{
    QString newWorkingDirectory = QFileDialog::getExistingDirectory(this, tr("Choose new working directory"), QDir::homePath());
    if (!newWorkingDirectory.isEmpty()) {
        tmp_workingDir = newWorkingDirectory;
        editWd->setText(newWorkingDirectory);
    }
}

void ELOSettingsDialog::checkShortcutInput(const QString &text)
{
    const QString s1 = shortcutEdit->text();
    const QString s2 = shortcutPathEdit->text();

    if (s1.isEmpty() || s2.isEmpty()) {
        buttonSavePs->setEnabled(false);
    } else {
        buttonSavePs->setEnabled(true);
    }

    Q_UNUSED(text);
}

void ELOSettingsDialog::openDirectoryPs()
{
    QString newPath = QFileDialog::getExistingDirectory(this, tr("Choose a path for the shortcut"), QDir::homePath());
    if (!newPath.isEmpty()) {
        shortcutPathEdit->setText(newPath);
    }
}

void ELOSettingsDialog::savePathShortcut()
{
    const QString name = shortcutEdit->text();
    const QString path = shortcutPathEdit->text();
    const QString description = shortcutDescriptionEdit->text();

    if(!name.isEmpty() && !path.isEmpty()) {
        if(shortcutEditIndex >=0) {
            tmp_PathShortcuts[shortcutEditIndex].Name = name;
            tmp_PathShortcuts[shortcutEditIndex].Path = path;
            tmp_PathShortcuts[shortcutEditIndex].Description = description;
        } else {
            PathShortcut psc;
            psc.Name = name;
            psc.Path = path;
            psc.Description = description;
            tmp_PathShortcuts.append(psc);
        }
        showPathShortcuts();
    }
    shortcutEditIndex = -1;
    shortcutEdit->clear();
    shortcutPathEdit->clear();
    shortcutDescriptionEdit->clear();
}

void ELOSettingsDialog::showPathShortcuts()
{
    shortcutList->clear();
    for (int i = 0; i < tmp_PathShortcuts.length(); i++) {
        shortcutList->addItem(tmp_PathShortcuts[i].Name);
    }
}

void ELOSettingsDialog::editPathshortcut()
{
    shortcutEditIndex = shortcutList->currentRow();
    shortcutEdit->setText(tmp_PathShortcuts[shortcutEditIndex].Name);
    shortcutPathEdit->setText(tmp_PathShortcuts[shortcutEditIndex].Path);
    shortcutDescriptionEdit->setText(tmp_PathShortcuts[shortcutEditIndex].Description);
}

void ELOSettingsDialog::deletePathShortcut()
{
    if(QMessageBox::question(this, tr("confrm deletion"), tr("Do you really want to delet this shortcut?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        tmp_PathShortcuts.remove(shortcutList->currentRow());
        showPathShortcuts();
        shortcutEditIndex = -1;
    }
}

void ELOSettingsDialog::shortcutSelected(int currentRow)
{
    if(currentRow >=0 ){
        shortcutPathLabel->setText(tmp_PathShortcuts[currentRow].Path);
        shortcutDescriptionLabel->setText(tmp_PathShortcuts[currentRow].Description);
        buttonEditPs->setEnabled(true);
        buttonDeletePs->setEnabled(true);
    } else {
        shortcutPathLabel->clear();
        shortcutDescriptionLabel->clear();
        buttonEditPs->setEnabled(false);
        buttonDeletePs->setEnabled(false);
    }
}

void ELOSettingsDialog::spellCheckState(bool checked)
{
    if (checked) {
        comboSc->setEnabled(true);
    } else {
        comboSc->setEnabled(false);
    }
}
