#ifndef ELOREPOSETTINGSDIALOG_H
#define ELOREPOSETTINGSDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QDialogButtonBox>

class ELORepoSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    ELORepoSettingsDialog(QWidget *parent = nullptr);
    void setTitle(const QString &title) { setWindowTitle(title);}
    void setContent(const QString &jsonText) { textEdit->setPlainText(jsonText); oldContent = jsonText; }

private:
    QPlainTextEdit *textEdit;
    QDialogButtonBox *buttonBox;
    QString oldContent;

private slots:
    void accept();
    void reject();

signals:
    void repoSettingsChanged(QJsonObject);
};

#endif // ELOREPOSETTINGSDIALOG_H
