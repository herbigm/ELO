#include "elogitprocess.h"

#include <QVBoxLayout>
#include <QPushButton>

ELOGitProcess::ELOGitProcess(QWidget *parent): QDialog(parent)
{
    this->setModal(true);
    this->hide();

    // resize
    resize(600,400);
    setWindowTitle(tr("ELO git process log"));

    // load settings
    settings = TheELOSettings::Instance();

    // create Widgets
    setLayout(new QVBoxLayout(this));
    textEdit = new QPlainTextEdit(this);
    textEdit->setStyleSheet("background-color: #000000; color: #0064a8;");
    textEdit->setReadOnly(true);
    QPushButton *btn = new QPushButton(tr("close"), this);
    connect(btn, &QPushButton::pressed, this, &ELOGitProcess::hide);

    layout()->addWidget(textEdit);
    layout()->addWidget(btn);
}
