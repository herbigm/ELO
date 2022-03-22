#include "elowebpage.h"

ELOWebPage::ELOWebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{

}

bool ELOWebPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    if (type == QWebEnginePage::NavigationTypeLinkClicked)
    {
        qDebug() << "navigate to: " << url;
        emit openFileRequest(url);
        return false;
    }
    return true;
}

QWebEnginePage *ELOWebPage::createWindow(WebWindowType type)
{
    ELOWebPage *page = new ELOWebPage(this->profile(), this->parent());
    connect(page, &ELOWebPage::urlChanged, this, &ELOWebPage::onUrlChanged);
    connect(page, &ELOWebPage::openFileRequest, this, &ELOWebPage::transmitOpenFileRequest);
    qDebug() << "createWindow";
    return page;
}

void ELOWebPage::onUrlChanged(const QUrl &url)
{
    if(ELOWebPage *page = qobject_cast<ELOWebPage *>(sender())){
        qDebug() << url;
        setUrl(url);
        page->deleteLater();
    }
}

void ELOWebPage::transmitOpenFileRequest(const QUrl &url)
{
    qDebug() << "transmitted";
    emit openFileRequest(url);
}
