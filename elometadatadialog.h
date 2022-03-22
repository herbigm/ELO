#ifndef ELOMETADATADIALOG_H
#define ELOMETADATADIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QJsonObject>

#include "elosettings.h"

class ELOMetadataDialog : public QDialog
{
    Q_OBJECT
public:
    ELOMetadataDialog(QWidget *parent=nullptr);

public slots:
    void updateFormContent(const QJsonObject metadata);

private:
    QDialogButtonBox *buttonBox;
    QLineEdit *authorEdit;
    QLineEdit *experimentNumberEdit;
    QLineEdit *titleEdit;
    QDateEdit *dateEdit;
    QPlainTextEdit *descriptionEdit;
    QLabel *templateLabel;
    QPushButton *buttonTemplate;
    ELOSettings *settings;
    QJsonObject lastMetadata;
    bool forNewFile;

private slots:
    void accept();
    void reject();

    void openTemplate();

signals:
    void metadataChanged(const QJsonObject);
};

#endif // ELOMETADATADIALOG_H
