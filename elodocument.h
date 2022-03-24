#ifndef ELODOCUMENT_H
#define ELODOCUMENT_H

#include <QObject>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QWebEngineProfile>
#include <QDir>
#include <QCoreApplication>
#include <QPrinter>
#include <QTextDocument>

#include "elowebpage.h"
#include "elouser.h"

class ELODocument: public QObject
{
    Q_OBJECT
public:
    explicit ELODocument(const QString &filePath, permissionMode permissions);

    ~ELODocument() {
        delete webPage;
        delete webView;
    }

    void loadJS(bool ok);

    const QString &getRepoName() const { return repoName; }
    const QString &getFileTitle() const { return fileTitle; }
    const QFileInfo &getFileInfo() const { return fileInfo; }
    QJsonObject getMetaData() { return metaData; }
    ELOWebView *getWebView() { return webView; }
    ELOWebPage *getWebPage() { return webPage; }

    inline void setRepoName(const QString &newRepoName) { repoName = newRepoName; }
    inline void updateFileInfoPath(const QString &newPath) {
        fileInfo.setFile(newPath);
        fileTitle = fileInfo.fileName().left(fileInfo.fileName().length()-5);
    }
    inline void setMetadata(const QJsonObject obj) { metaData = obj; }
    void startSaveing();
    void printToPdf(const QString &path);
    void saveDocument(const QString content);
    void checkModified();

private:
    QString repoName;
    QString fileTitle;
    QFileInfo fileInfo;
    ELOWebView *webView;
    ELOWebPage *webPage;
    QJsonObject metaData;
    QString lastSavedContent;

signals:
    void wasModified(bool modified);
    void wasSaved();
};

#endif // ELODOCUMENT_H
