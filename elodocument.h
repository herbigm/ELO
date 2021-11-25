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

#include "elowebview.h"
#include "elowebpage.h"

class ELODocument: public QObject
{
    Q_OBJECT
public:
    explicit ELODocument(const QString &filePath);

    ~ELODocument() {
        delete webPage;
        delete webView;
    }

    void loadJS(bool ok);

    const QString &getRepoName() const { return repoName; }
    const QString &getTitle() const { return title; }
    const QFileInfo &getFileInfo() const { return fileInfo; }
    QJsonObject getMetaData() { return metaData; }
    ELOWebView *getWebView() { return webView; }

    inline void setRepoName(const QString &newRepoName) { repoName = newRepoName; }
    inline void updateFileInfoPath(const QString &newPath) {
        fileInfo.setFile(newPath);
        title = fileInfo.fileName().left(fileInfo.fileName().length()-5);
    }
    inline void setMetadata(const QJsonObject obj) { metaData = obj; }
    void startSaveing();
    void saveDocument(const QString content);
    void checkModified();

private:
    QString repoName;
    QString title;
    QFileInfo fileInfo;
    ELOWebView *webView;
    ELOWebPage *webPage;
    QJsonObject metaData;
    QString lastSavedContent;

signals:
    void wasModified(bool);
    void wasSaved();
};

#endif // ELODOCUMENT_H
