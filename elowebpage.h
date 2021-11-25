#ifndef ELOWEBPAGE_H
#define ELOWEBPAGE_H

#include <QWebEnginePage>
#include <QObject>

class ELOWebPage : public QWebEnginePage
{
     Q_OBJECT

public:
    explicit ELOWebPage(QWebEngineProfile *profile, QObject *parent = nullptr);
};

#endif // ELOWEBPAGE_H
