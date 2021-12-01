#include "elodocumenthandler.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QMimeDatabase>

ELODocumentHandler::ELODocumentHandler(QObject *parent) : QObject(parent)
{
    currentDocument = nullptr;
    currentRepoIndex = -1;
    settings = TheELOSettings::Instance();
    closeAll = false;
}

QWebEngineView *ELODocumentHandler::openFile(const QString &filePath)
{
    QFileInfo finfo(filePath);
    if (!finfo.exists()) {
        qErrnoWarning("File not found");
        return nullptr;
    }
    // test, if file is an experiment file
    if (!isExperimentFile(filePath)) {
        // maybe it's an image?!
        QMimeDatabase mimeDB;
        QMimeType mime = mimeDB.mimeTypeForFile(filePath);

        // ask what to do with the file
        QMessageBox msgBox;
        msgBox.setText(tr("The selected file is not an experiment file. What do you like to do?"));
        msgBox.setIcon(QMessageBox::Question);
        QPushButton *openButton = msgBox.addButton(tr("Open File"), QMessageBox::ActionRole);
        QPushButton *insertLinkButton = msgBox.addButton(tr("Insert Link to this File"), QMessageBox::ActionRole);
        QPushButton *insertImageButton = msgBox.addButton(tr("Insert File as Image"), QMessageBox::ActionRole);
        insertImageButton->setEnabled(false);
        if(mime.preferredSuffix() == "jpeg" || mime.preferredSuffix() == "jpg" || mime.preferredSuffix() == "gif" || mime.preferredSuffix() == "png" || mime.preferredSuffix() == "svg") {
            insertImageButton->setEnabled(true);
        }
        QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
        msgBox.setDefaultButton(abortButton);

        msgBox.exec();

        if (msgBox.clickedButton() == openButton) {
            // open the file with system defaults
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        } else if (msgBox.clickedButton() == insertLinkButton) {
            insertLink(finfo.fileName(), currentDocument->getFileInfo().dir().relativeFilePath(filePath));
        } else if (msgBox.clickedButton() == insertImageButton) {
            insertImage(finfo.filePath());
        }
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
    connect(doc->getWebPage(), &ELOWebPage::openFileRequest, this, &ELODocumentHandler::performOpenFileRequest);

    return doc->getWebView(); // return the webview
}

ELODocumentHandler::~ELODocumentHandler()
{
    qDeleteAll(documents);
}

void ELODocumentHandler::setCurrentDocument(QWebEngineView *widget)
{
    for (int i = 0; i < documents.length(); i++) {
        if (documents[i]->getWebView() == widget) {
            // this is the current document
            currentDocument = documents[i];
            emit currentMetadataChanged(currentDocument->getMetaData());
            emit currentFileChanged(currentDocument->getRepoName(), currentDocument->getFileTitle());
            return;
        }
    }
    emit currentMetadataChanged(QJsonObject());
}

QString ELODocumentHandler::copyToAssociatedFiles(const QString &path)
{
    // get the right directory name
    QDir dir(settings->getWorkingDir() + QDir::separator() + currentDocument->getRepoName() + QDir::separator() + ".AssociatedFiles" + QDir::separator() + currentDocument->getFileTitle());
    if(!dir.exists()) {
        dir.mkpath(dir.path());
    }

    // copy file into directory
    QFile oldFile(path);
    QFileInfo finfo(oldFile);
    QFileInfo toSaveFile(dir.path() + QDir::separator() + finfo.fileName());
    int i = 1;
    while(toSaveFile.exists()) {
        toSaveFile.setFile(toSaveFile.filePath() + QDir::separator() + QChar(i) + "_" + finfo.fileName());
        i++;
    }
    oldFile.copy(toSaveFile.filePath());
    return toSaveFile.filePath();
}

void ELODocumentHandler::insertImage(const QString &path)
{
    if (currentDocument) {
        QString relativePath = currentDocument->getFileInfo().dir().relativeFilePath(path);
        currentDocument->getWebPage()->runJavaScript("editor.execute( 'insertImage', { source: '" + relativePath + "' } );");
    }
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
                emit documentTitleChanged(documents[i]->getWebView(), documents[i]->getFileTitle());
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
        QString templateName = obj.value("template").toString();
        if (!templateName.isEmpty()) {
            templateName = templateName.left(templateName.length()-6);
            QFile templateHTML(TheELOSettings::Instance()->getWorkingDir() + QDir::separator() + "ELOtemplates" + QDir::separator() + templateName + ".html");
            templateHTML.open(QIODevice::ReadOnly);
            QString tmplt = templateHTML.readAll();
            templateHTML.close();
            tmplt = tmplt.replace("{{experiment number}}", obj.value("experiment number").toString());
            tmplt = tmplt.replace("{{date}}", obj.value("date").toString());
            tmplt = tmplt.replace("{{title}}", obj.value("title").toString());
            newFile.write(tmplt.toUtf8());
        } else {
            newFile.write("<!-- {} -->\n<html></html>");
        }
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
    return currentDocument->getFileTitle();
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
        QMessageBox::StandardButton btn = QMessageBox::question(nullptr, tr("Save modified file?"), tr(QString("The file %1 was modified. Do you like to save the modificated file?").arg(currentDocument->getFileTitle()).toUtf8()));
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

void ELODocumentHandler::performOpenFileRequest(const QUrl &url)
{
    qDebug() << "file request for" << url;
    if (isExperimentFile(url.toString())) {
        openFile(url.toString());
    } else {
        QDesktopServices::openUrl(url);
    }
}

void ELODocumentHandler::insertLink(const QString &text, const QString &url)
{
    if (currentDocument) {
        QString newUrl;
        if (!url.startsWith("http")) {
            newUrl = currentDocument->getFileInfo().dir().relativeFilePath(url);
        } else {
            newUrl = url;
        }
        currentDocument->getWebPage()->runJavaScript("editor.model.change( writer => { const insertPosition = editor.model.document.selection.getFirstPosition(); writer.insertText( '" + text + "', { linkHref: '" + newUrl + "' }, insertPosition ); } );");
    }
}
