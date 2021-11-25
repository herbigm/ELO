#ifndef ELODOCUMENTHANDLER_H
#define ELODOCUMENTHANDLER_H

#include <QObject>
#include <QFileInfo>
#include <QJsonObject>

#include "elowebview.h"
#include "elodocument.h"
#include "elosettings.h"

enum permissionMode {None, ReadOnly, ReadWrite}; // valid permission modes of users in a repository

typedef struct{
    QString Name;
    QString Path;
    permissionMode rights;
    QJsonObject settings;
} ELORepository;

class ELODocumentHandler : public QObject
{
    Q_OBJECT
public:
    explicit ELODocumentHandler(QObject *parent = nullptr);
    ~ELODocumentHandler();

    ELOWebView *openFile(const QString &filePath);
    void requestClosingCurrentDocument();

    // getter functions
    const QString &getCurrentTitle() const;

    // setter functions
    void setCurrentDocument(ELOWebView *widget);


public slots:
    void setCurrentDirectory(const QString &path);
    void performRenaming(const QString &oldpath, const QString &newpath);
    void deleteFile(const QString &filePath);
    void updateMetadata(const QJsonObject obj);
    void startFileCreation(const QString &parentPath);
    void saveCurrentDocument();
    void closeAllDocuments();

private:
    ELOSettings *settings;
    QVector<ELORepository> repos;
    QVector<ELODocument *> documents;
    ELODocument *currentDocument;
    int currentRepoIndex;
    QString currentDirectory;
    bool closeAll;

    bool isExperimentFile(const QString &filePath);
    void saveAndCloseCurrentDocument(bool modified);
    void closeCurrentDocument();

signals:
    void documentTitleChanged(ELOWebView *, const QString);
    void currentMetadataChanged(const QJsonObject);
    void askForMetadataForNewFile(const QJsonObject);
    void newFileCreatedView(ELOWebView *);
    void newFileCreatedModel(const QString);
    void allClosed();

};

#endif // ELODOCUMENTHANDLER_H
