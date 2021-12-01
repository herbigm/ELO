#ifndef ELOWEBPAGE_H
#define ELOWEBPAGE_H

#include <QWebEnginePage>
#include <QObject>

class ELOWebPage : public QWebEnginePage
{
     Q_OBJECT

public:
    explicit ELOWebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

protected:
    QWebEnginePage *createWindow(WebWindowType type);

private slots:
    void onUrlChanged(const QUrl &url);
    void transmitOpenFileRequest(const QUrl &url);

signals:
    void openFileRequest(const QUrl &url);
};

#endif // ELOWEBPAGE_H
