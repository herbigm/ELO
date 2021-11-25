#include "elodocumenthandler.h"

#include <QDesktopServices>
#include <QMessageBox>

ELODocumentHandler::ELODocumentHandler(QObject *parent) : QObject(parent)
{
    currentDocument = nullptr;
    currentRepoIndex = -1;
    settings = TheELOSettings::Instance();
    closeAll = false;
}

ELOWebView *ELODocumentHandler::openFile(const QString &filePath)
{
    QFileInfo finfo(filePath);
    if (!finfo.exists()) {
        qErrnoWarning("File not found");
        return nullptr;
    }
    // test, if file is an experiment file
    if (!isExperimentFile(filePath)) {
        // open the file with system defaults
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        return nullptr;
    }

    // test, if document is already opened
    for (int i = 0; i < documents.length(); i++) {
        if (documents[i]->getFileInfo().filePath() == finfo.filePath()) {
            // document is already opened
            currentDocument = documents[i]; // the current document
            return currentDocument->getWebView(); // return the webview
        }
    }

    // create new ELODocument
    ELODocument *doc = new ELODocument(filePath);

    // extract repoName from filePath
    QString repoName;
    QString path = finfo.filePath().replace("\\", "/"); // replace Windows path separators
    path = path.remove(settings->getWorkingDir().replace("\\", "/"), Qt::CaseInsensitive);
    int pos = path.indexOf("/", 1);
    if(pos == -1) {
        repoName = path.mid(1);
    } else {
        repoName = path.mid(1, pos-1);
    }
    doc->setRepoName(repoName);
    documents.append(doc);
    currentDocument = doc; // the new document is the current document

    return doc->getWebView(); // return the webview

    return nullptr;
}

ELODocumentHandler::~ELODocumentHandler()
{
    qDeleteAll(documents);
}

void ELODocumentHandler::setCurrentDocument(ELOWebView *widget)
{
    for (int i = 0; i < documents.length(); i++) {
        if (documents[i]->getWebView() == widget) {
            // this is the current document
            currentDocument = documents[i];
            emit currentMetadataChanged(currentDocument->getMetaData());
            return;
        }
    }
    emit currentMetadataChanged(QJsonObject());
}

void ELODocumentHandler::setCurrentDirectory(const QString &path)
{
    QFileInfo finfo(path);
    if (finfo.isDir()) {
        currentDirectory = path;
    } else {
        currentDirectory = finfo.dir().path();
    }
}

void ELODocumentHandler::performRenaming(const QString &oldpath, const QString &newpath)
{
    // a file or a folder was renamed -> update all path (if nessaccery)
    QFileInfo newFinfo(newpath);
    for (int i = 0; i < documents.length(); i++) {
        QFileInfo finfo = documents[i]->getFileInfo();
        if (finfo.filePath().startsWith(oldpath)) {
            // path has to be changed
            documents[i]->updateFileInfoPath(finfo.filePath().replace(oldpath, newpath));
            if (newFinfo.isFile()) {
                emit documentTitleChanged(documents[i]->getWebView(), documents[i]->getTitle());
            }
        }
    }
}

void ELODocumentHandler::deleteFile(const QString &filePath)
{
    for (int i = 0; i < documents.length(); i++) {
        if (documents[i]->getFileInfo().filePath() == filePath) {
            // this is the right document
            ELODocument *tmp = documents[i];
            documents.remove(i);
            delete tmp;
            break;
        }
    }
}

void ELODocumentHandler::updateMetadata(const QJsonObject obj)
{
    if (obj.contains("forNewFile")) { // create new file and add metadata
        QString path = currentDirectory + QDir::separator() + obj.value("experiment number").toString() + ".html";
        QFile newFile(path);
        QFileInfo finfo(newFile);
        newFile.open(QIODevice::WriteOnly);
        newFile.write("<!-- {} -->\n<html></html>"); // TODO: insert template here
        newFile.close();
        ELODocument *doc = new ELODocument(path);
        doc->setMetadata(obj);
        // extract repoName from filePath
        QString repoName;
        path = finfo.filePath().replace("\\", "/"); // replace Windows path separators
        path = path.remove(settings->getWorkingDir().replace("\\", "/"), Qt::CaseInsensitive);
        int pos = path.indexOf("/", 1);
        if(pos == -1) {
            repoName = path.mid(1);
        } else {
            repoName = path.mid(1, pos-1);
        }
        doc->setRepoName(repoName);
        documents.append(doc);
        currentDocument = doc; // the new document is the current document
        emit newFileCreatedView(doc->getWebView());
        emit newFileCreatedModel(doc->getFileInfo().filePath());

    } else { // simply update the metadata
        currentDocument->setMetadata(obj);
    }
}

void ELODocumentHandler::startFileCreation(const QString &parentPath)
{
    // TODO: Repo-Standards einfügen
    QJsonObject obj;
    obj.insert("author", "");
    obj.insert("date", QDate::currentDate().toString("yyyy-MM-dd"));
    obj.insert("description", "");
    obj.insert("title", "");
    obj.insert("experiment number", "");
    obj.insert("template", "");
    obj.insert("forNewFile", true);
    emit askForMetadataForNewFile(obj);
}

void ELODocumentHandler::saveCurrentDocument()
{
    if (!currentDocument) // if no document is opened, abort!
        return;
    currentDocument->startSaveing();
}

const QString &ELODocumentHandler::getCurrentTitle() const
{
    return currentDocument->getTitle();
}

bool ELODocumentHandler::isExperimentFile(const QString &filePath)
{
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly)) {
        QString firstLine = file.readLine().trimmed();
        if(firstLine.startsWith("<!--") && firstLine.endsWith("-->"))
            return true;
    }
    return false;
}

void ELODocumentHandler::requestClosingCurrentDocument()
{
    connect(currentDocument, &ELODocument::wasModified, this, &ELODocumentHandler::saveAndCloseCurrentDocument);
    currentDocument->checkModified();
}

void ELODocumentHandler::saveAndCloseCurrentDocument(bool modified)
{
    if (!modified) {
        closeCurrentDocument();
    } else {
        QMessageBox::StandardButton btn = QMessageBox::question(nullptr, tr("Save modified file?"), tr("The file was modified. Do you like to save the modificated file?"));
        if (btn == QMessageBox::Yes) {
            connect(currentDocument, &ELODocument::wasSaved, this, &ELODocumentHandler::closeCurrentDocument);
            currentDocument->startSaveing();
        } else {
            closeCurrentDocument();
        }
    }
}

void ELODocumentHandler::closeCurrentDocument()
{
    for (int i = 0; i < documents.length(); i++) {
        if (documents[i] == currentDocument) {
            // this is the current document
            documents.remove(i);
            delete currentDocument;
            break;
        }
    }
    if (documents.length() > 0) {
        currentDocument = documents[0];
        if (closeAll) {
            requestClosingCurrentDocument();
        }
    } else {
        emit allClosed();
    }
}

void ELODocumentHandler::closeAllDocuments()
{
    closeAll = true;
    requestClosingCurrentDocument();
}
