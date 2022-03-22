#ifndef ELOWEBPAGE_H
#define ELOWEBPAGE_H

#include <QWebEnginePage>
#include <QObject>
#include <QWebEngineView>

/*
Subclassing QWebEngineView to prwevent default behavior to send createWindow
*/
class ELOWebView : public QWebEngineView
{
    Q_OBJECT

public:
    ELOWebView() {}
};


class ELOWebPage : public QWebEnginePage
{
     Q_OBJECT

public:
    explicit ELOWebPage(QWebEngineProfile *profile, QObject *parent = nullptr);

    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;

protected:
    QWebEnginePage *createWindow(WebWindowType type) override;

private slots:
    void onUrlChanged(const QUrl &url);
    void transmitOpenFileRequest(const QUrl &url);

signals:
    void openFileRequest(const QUrl &url);
};

#endif // ELOWEBPAGE_H
