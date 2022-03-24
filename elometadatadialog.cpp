#include "elometadatadialog.h"

#include <QFormLayout>
#include <QIcon>
#include <QPushButton>
#include <QDir>
#include <QFileDialog>

ELOMetadataDialog::ELOMetadataDialog(QWidget *parent):
    QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint )
{
    this->setModal(true);
    this->hide();

    // resize
    resize(600,400);
    setWindowTitle(tr("ELO metadata"));

    // load settings
    settings = TheELOSettings::Instance();

    // create Widgets
    // ButtonBox
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QFormLayout *mainLayout = new QFormLayout();
    setLayout(mainLayout);

    authorEdit = new QLineEdit();
    experimentNumberEdit = new QLineEdit();
    titleEdit = new QLineEdit();
    dateEdit = new QDateEdit();
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateEdit->setDate(QDate::currentDate());
    descriptionEdit = new QPlainTextEdit();
    templateLabel = new QLabel();
    QWidget *widget1 = new QWidget();
    QHBoxLayout *layout1 = new QHBoxLayout();
    const QIcon buttonIconOpenFolder = QIcon::fromTheme("folder", QIcon(":icons/icons/openFolder.svg"));
    QPushButton *button = new QPushButton(buttonIconOpenFolder, tr("open template"), this);

    widget1->setLayout(layout1);
    layout1->addWidget(templateLabel);
    layout1->addWidget(button);

    mainLayout->addRow(new QLabel(tr("Author: ")), authorEdit);
    mainLayout->addRow(new QLabel(tr("Experiment number:")), experimentNumberEdit);
    mainLayout->addRow(new QLabel(tr("Title: ")), titleEdit);
    mainLayout->addRow(new QLabel(tr("Date: ")), dateEdit);
    mainLayout->addRow(new QLabel(tr("Description: ")), descriptionEdit);
    mainLayout->addRow(new QLabel(tr("Template: ")), widget1);
    mainLayout->addRow(buttonBox);

    connect(button, &QPushButton::pressed, this, &ELOMetadataDialog::openTemplate);
    forNewFile = false;
}

void ELOMetadataDialog::updateFormContent(const QJsonObject metadata)
{
    if (!metadata.contains("forNewFile")) {
        lastMetadata = metadata;
    } else {
        forNewFile = true;
    }
    if (metadata.contains("author")) {
        authorEdit->setText(metadata.value("author").toString());
    }
    if (metadata.contains("date")) {
        dateEdit->setDate(QDate::fromString(metadata.value("date").toString(), "yyyy-MM-dd"));
    }
    if (metadata.contains("description")) {
        descriptionEdit->setPlainText(metadata.value("description").toString());
    }
    if (metadata.contains("title")) {
        titleEdit->setText(metadata.value("title").toString());
    }
    if (metadata.contains("experiment number")) {
        experimentNumberEdit->setText(metadata.value("experiment number").toString());
    }
    if (metadata.contains("template")) {
        templateLabel->setText(metadata.value("template").toString());
    }
}

void ELOMetadataDialog::accept()
{
    QJsonObject obj;
    obj.insert("author", authorEdit->text());
    obj.insert("date", dateEdit->date().toString("yyyy-MM-dd"));
    obj.insert("description", descriptionEdit->toPlainText());
    obj.insert("title", titleEdit->text());
    obj.insert("experiment number", experimentNumberEdit->text());
    obj.insert("template", templateLabel->text());
    if (!forNewFile) {
        lastMetadata = obj;
    } else { // for a new file the metadata is set by opening the created file later (by documentHandler and MainWindow)
        obj.insert("forNewFile", true);
        forNewFile = false;
    }
    emit metadataChanged(obj);
    hide();
}

void ELOMetadataDialog::reject()
{
    authorEdit->clear();
    experimentNumberEdit->clear();
    titleEdit->clear();
    dateEdit->clear();
    descriptionEdit->clear();
    templateLabel->clear();
    updateFormContent(lastMetadata);
    forNewFile = false;
    hide();
}

void ELOMetadataDialog::openTemplate()
{
    QString dgl = QFileDialog::getOpenFileName(this, tr("open template"), settings->getWorkingDir() + QDir::separator() + "ELOtemplates", tr("HTML templates (*.html)"));
    if(!dgl.isEmpty()) {
        dgl = dgl.remove(settings->getWorkingDir() + QDir::separator() + "ELOtemplates" + QDir::separator());
        templateLabel->setText(dgl);
    }
}
