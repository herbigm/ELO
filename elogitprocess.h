#ifndef ELOGITPROCESS_H
#define ELOGITPROCESS_H

#include <QDialog>
#include <QPlainTextEdit>

#include "elosettings.h"

class ELOGitProcess : public QDialog
{
    Q_OBJECT
public:
    ELOGitProcess(QWidget *parent = nullptr);

private:
    ELOSettings *settings;
    QPlainTextEdit *textEdit;
};

#endif // ELOGITPROCESS_H
