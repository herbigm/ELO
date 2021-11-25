#ifndef ELOTABWIDGET_H
#define ELOTABWIDGET_H

#include <QTabWidget>
#include <QWebEnginePage>

class ELOWebView;

class ELOTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ELOTabWidget(QWebEngineProfile *profile, QWidget *parent = nullptr);

    ELOWebView *currentWebView() const;

public slots:
    ELOWebView *createTab();
    ELOWebView *createBackgroundTab();

    void setUrl(const QUrl &url);

private:
    ELOWebView *webView(int index) const;

    QWebEngineProfile *m_profile;
};

#endif // ELOTABWIDGET_H
