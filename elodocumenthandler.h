#ifndef ELODOCUMENTHANDLER_H
#define ELODOCUMENTHANDLER_H

#include <QObject>
#include <QFileInfo>
#include <QJsonObject>

#include "elodocument.h"
#include "elosettings.h"
#include "elouser.h"

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
    void requestClosingDocument(ELOWebView *widget);

    // getter functions
    const QString &getCurrentTitle() const;
    const QString &getCurrentRepoName() const;

    // setter functions
    void setCurrentDocument(ELOWebView *widget);

    QString copyToAssociatedFiles(const QString &path);
    void insertImage(const QString &path);
    void printToPDF(const QString &path);


public slots:
    void setCurrentDirectory(const QString &path);
    void performRenaming(const QString &oldpath, const QString &newpath);
    void deleteFile(const QString &filePath);
    void updateMetadata(QJsonObject obj);
    void startFileCreation(const QString &parentPath = QString());
    void saveCurrentDocument();
    void closeAllDocuments();
    void performOpenFileRequest(const QUrl &url);
    void insertLink(const QString &text, const QString &url);
    void updateRepoSettings(QJsonObject settings);
    void setSpellCheck(bool enabled);
    void setSpellCheckLanguage(QString language);

private:
    ELOSettings *settings;
    QVector<ELORepository> repos;
    QVector<ELODocument *> documents;
    ELODocument *currentDocument;
    int currentRepoIndex;
    QString currentDirectory;
    bool closeAll;
    ELOUser *user;

    bool isExperimentFile(const QString &filePath);
    void saveAndCloseDocument(ELODocument * document, bool modified);
    void closeDocument(ELODocument* document);
    QString getRepoNameFromPath(QString path);

signals:
    void documentTitleChanged(ELOWebView *, const QString);
    void currentMetadataChanged(const QJsonObject);
    void currentFileChanged(const QString &repo, const QString &fileName);
    void askForMetadataForNewFile(const QJsonObject);
    void newFileCreatedView(ELOWebView *);
    void newFileCreatedModel(const QString);
    void allClosed();
    void repoPermissionsChanged(permissionMode p);
    void filePermissionsChanged(permissionMode p);
    void openExperimentRequest(const QString &);

};

#endif // ELODOCUMENTHANDLER_H
