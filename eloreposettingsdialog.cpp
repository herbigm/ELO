#include "eloreposettingsdialog.h"

#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>

ELORepoSettingsDialog::ELORepoSettingsDialog(QWidget *parent): QDialog(parent)
{
    this->setModal(true);
    this->hide();

    // resize
    resize(600,400);
    setWindowTitle(tr("ELO repo settings"));

    // create Widgets
    // ButtonBox
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ELORepoSettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ELORepoSettingsDialog::reject);

    textEdit = new QPlainTextEdit(this);
    setLayout(new QVBoxLayout());
    layout()->addWidget(textEdit);
    layout()->addWidget(buttonBox);
}

void ELORepoSettingsDialog::accept()
{
    emit repoSettingsChanged(QJsonDocument::fromJson(textEdit->toPlainText().toUtf8()).object());
    hide();
}

void ELORepoSettingsDialog::reject()
{
    setContent(oldContent);
    hide();
}
