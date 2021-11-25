#include "elotabwidget.h"
#include "elowebview.h"
#include "elowebpage.h"

ELOTabWidget::ELOTabWidget(QWebEngineProfile *profile, QWidget *parent)
    : QTabWidget(parent)
    , m_profile(profile)
{

}

ELOWebView *ELOTabWidget::currentWebView() const
{
    return webView(currentIndex());
}

ELOWebView *ELOTabWidget::createTab()
{
    ELOWebView *webView = createBackgroundTab();
    setCurrentWidget(webView);
    return webView;
}

ELOWebView *ELOTabWidget::createBackgroundTab()
{
    ELOWebView *webView = new ELOWebView;
    ELOWebPage *webPage = new ELOWebPage(m_profile, webView);
    webView->setPage(webPage);
    //setupView(webView);
    addTab(webView, tr("(Untitled)"));
    // Workaround for QTBUG-61770
    webView->resize(currentWidget()->size());
    webView->show();
    return webView;
}

void ELOTabWidget::setUrl(const QUrl &url)
{
    if (ELOWebView *view = currentWebView()) {
        view->setUrl(url);
        view->setFocus();
    }
}

ELOWebView *ELOTabWidget::webView(int index) const
{
    return qobject_cast<ELOWebView*>(widget(index));
}
