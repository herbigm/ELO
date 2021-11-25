#ifndef ELOWEBVIEW_H
#define ELOWEBVIEW_H

#include <QWebEngineView>
#include <QObject>
#include "elowebpage.h"

class ELOWebView : public QWebEngineView
{
    Q_OBJECT

public:
    ELOWebView();
};

#endif // ELOWEBVIEW_H
