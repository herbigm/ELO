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
    user = ELOUser::Instance();
}

ELOWebView *ELODocumentHandler::openFile(const QString &filePath)
{
    QFileInfo finfo(filePath);
    if (!finfo.exists()) {
        qErrnoWarning("File not found");
        return nullptr;
    }
    // test, if file is an experiment file and not in the template folder
    if (!isExperimentFile(filePath) && getRepoNameFromPath(filePath) != "ELOtemplates") {
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
            insertLink(finfo.fileName(), "file-getter.php?path=" + QDir(settings->getWorkingDir()).relativeFilePath(filePath) + "&internal=true");
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
            emit filePermissionsChanged(static_cast<permissionMode>(user->getRepos().value(currentDocument->getRepoName()).toObject().value("permissions").toInt()));
            return currentDocument->getWebView(); // return the webview
        }
    }

    // create new ELODocument with the right repoName
    QString repoName = getRepoNameFromPath(filePath);
    ELODocument *doc = new ELODocument(filePath, static_cast<permissionMode>(user->getRepos().value(repoName).toObject().value("permissions").toInt()));
    if (settings->getSpellCheck()) {
        doc->getWebPage()->profile()->setSpellCheckEnabled(true);
        doc->getWebPage()->profile()->setSpellCheckLanguages({settings->getSpellCheckLanguage()});
    }
    doc->setRepoName(repoName);

    documents.append(doc);
    currentDocument = doc; // the new document is the current document
    emit filePermissionsChanged(static_cast<permissionMode>(user->getRepos().value(currentDocument->getRepoName()).toObject().value("permissions").toInt()));

    connect(doc->getWebPage(), &ELOWebPage::openFileRequest, this, &ELODocumentHandler::performOpenFileRequest);

    return doc->getWebView(); // return the webview
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
            emit filePermissionsChanged(static_cast<permissionMode>(user->getRepos().value(currentDocument->getRepoName()).toObject().value("permissions").toInt()));
            emit currentMetadataChanged(currentDocument->getMetaData());
            emit currentFileChanged(currentDocument->getRepoName(), currentDocument->getFileTitle());
            return;
        }
    }
    emit currentMetadataChanged(QJsonObject());
    emit filePermissionsChanged(static_cast<permissionMode>(user->getRepos().value(currentDocument->getRepoName()).toObject().value("permissions").toInt()));
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

void ELODocumentHandler::printToPDF(const QString &path)
{
    if (currentDocument) {
        currentDocument->printToPdf(path);
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
    QString currentRepo = getRepoNameFromPath(currentDirectory);
    emit repoPermissionsChanged(static_cast<permissionMode>(user->getRepos().value(currentRepo).toObject().value("permissions").toInt()));
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

void ELODocumentHandler::updateMetadata(QJsonObject obj)
{
    if (obj.contains("forNewFile")) { // create new file and add metadata
        QString path;
        QFile newFile;
        if (currentDirectory.endsWith("ELOtemplates")) {
            // a new template should be created
            path = currentDirectory + QDir::separator() + obj.value("title").toString() + ".html";
            newFile.setFileName(path);
            newFile.open(QIODevice::WriteOnly);
            newFile.write("<html></html>");
            newFile.close();
        } else {
            path = currentDirectory + QDir::separator() + obj.value("experiment number").toString() + ".html";
            newFile.setFileName(path);
            newFile.open(QIODevice::WriteOnly);
            QString templateName = obj.value("template").toString();
            if (!templateName.isEmpty()) {
                templateName = templateName.remove(".tmplt").remove(".html");
                QFile templateHTML(TheELOSettings::Instance()->getWorkingDir() + QDir::separator() + "ELOtemplates" + QDir::separator() + templateName + ".html");
                templateHTML.open(QIODevice::ReadOnly);
                QString tmplt = templateHTML.readAll();
                templateHTML.close();
                tmplt = tmplt.replace("{{experiment number}}", obj.value("experiment number").toString());
                tmplt = tmplt.replace("{{date}}", obj.value("date").toString());
                tmplt = tmplt.replace("{{title}}", obj.value("title").toString());
                newFile.write("<!-- {} -->\n");
                newFile.write(tmplt.toUtf8());
            } else {
                newFile.write("<!-- {} -->\n<html></html>");
            }
            newFile.close();
        }
        QFileInfo finfo(newFile);
        ELODocument *doc = new ELODocument(path, static_cast<permissionMode>(user->getRepos().value(getRepoNameFromPath(currentDirectory)).toObject().value("permissions").toInt()));
        obj.remove("forNewFile");
        doc->setMetadata(obj);
        // extract repoName from filePath
        QString repoName = getRepoNameFromPath(finfo.filePath());
        doc->setRepoName(repoName);
        documents.append(doc);
        currentDocument = doc; // the new document is the current document

        if (!currentDirectory.endsWith("ELOtemplates")) {
            // only update reposetting if ithe new file is not inside the ELOtemplates repo
            int experimentNumber = obj.value("experiment number").toString().remove(QRegularExpression("[^\\d]")).toInt();
            QJsonObject repoSettings = user->getRepos().value(currentDocument->getRepoName()).toObject().value("settings").toObject();
            repoSettings.insert("lastExperimentNumber", experimentNumber);
            user->updateRepoSettings(repoSettings, currentDocument->getRepoName());
        }

        emit filePermissionsChanged(static_cast<permissionMode>(user->getRepos().value(currentDocument->getRepoName()).toObject().value("permissions").toInt()));
        emit newFileCreatedView(doc->getWebView());
        emit newFileCreatedModel(doc->getFileInfo().filePath());
        emit currentMetadataChanged(currentDocument->getMetaData());

    } else { // simply update the metadata
        currentDocument->setMetadata(obj);
    }
}

void ELODocumentHandler::startFileCreation(const QString &parentPath)
{
    if (!parentPath.isEmpty())
        setCurrentDirectory(parentPath);

    QJsonObject repoSettings = user->getRepos().value(getRepoNameFromPath(currentDirectory)).toObject().value("settings").toObject();

    QJsonObject obj;
    obj.insert("author", user->getName());
    obj.insert("date", QDate::currentDate().toString("yyyy-MM-dd"));
    obj.insert("description", "");
    obj.insert("title", "");
    obj.insert("experiment number", repoSettings.value("prefix").toString() + QString::number(repoSettings.value("lastExperimentNumber").toInt() + 1));
    obj.insert("template", repoSettings.value("template"));
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

const QString &ELODocumentHandler::getCurrentRepoName() const
{
    return currentDocument->getRepoName();
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

void ELODocumentHandler::requestClosingDocument(ELOWebView *widget)
{
    for (int i = 0; i < documents.length(); i++) {
        if (documents[i]->getWebView() == widget) {
            // this is the document which should be closed
            if (static_cast<permissionMode>(user->getRepos().value(documents[i]->getRepoName()).toObject().value("permissions").toInt()) == ReadWrite) {
                ELODocument *document = documents[i];
                connect(documents[i], &ELODocument::wasModified, this, [this, document](bool modified) { saveAndCloseDocument(document, modified); });
                documents[i]->checkModified();
            } else {
                closeDocument(documents[i]);
            }
            break;
        }
    }
}

void ELODocumentHandler::saveAndCloseDocument(ELODocument *document, bool modified)
{
    if (!modified) {
        closeDocument(document);
    } else {
        QMessageBox::StandardButton btn = QMessageBox::question(nullptr, tr("Save modified file?"), tr(QString("The file %1 was modified. Do you like to save the modificated file?").arg(document->getFileTitle()).toUtf8()));
        if (btn == QMessageBox::Yes) {
            connect(document, &ELODocument::wasSaved, this, [this, document] { closeDocument(document); });
            document->startSaveing();
        } else {
            closeDocument(document);
        }
    }
}

void ELODocumentHandler::closeDocument(ELODocument *document)
{
    for (int i = 0; i < documents.length(); i++) {
        if (documents[i] == document) {
            // this is the current document
            documents.remove(i);
            delete document;
            break;
        }
    }
    if (documents.length() > 0) {
        if (closeAll) {
            requestClosingDocument(documents[0]->getWebView());
        }
    } else {
        emit allClosed();
    }
}

QString ELODocumentHandler::getRepoNameFromPath(QString path)
{
    QString repoName;
    path = path.replace("\\", "/"); // replace Windows path separators
    path = path.remove(settings->getWorkingDir().replace("\\", "/"), Qt::CaseInsensitive);
    int pos = path.indexOf("/", 1);
    if(pos == -1) {
        repoName = path.mid(1);
    } else {
        repoName = path.mid(1, pos-1);
    }
    return repoName;
}

void ELODocumentHandler::closeAllDocuments()
{
    closeAll = true;
    requestClosingDocument(documents[0]->getWebView());
}

void ELODocumentHandler::performOpenFileRequest(const QUrl &url)
{
    qDebug() << "file request for" << url;
    QString path = url.toString();
    if (path.contains("file-getter.php") && path.endsWith("&internal=true")) {
        // internal file
        int index = path.indexOf("file-getter.php?path=") + QString("file-getter.php?path=").length();
        path = path.replace(path.left(index), settings->getWorkingDir());
        path = path.remove("&internal=true");
        if (isExperimentFile(path)) {
            emit openExperimentRequest(path);
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    } else if (path.contains("file-getter.php") && path.endsWith("&external=true")) {
        int index = path.indexOf("file-getter.php?path=") + QString("file-getter.php?path=").length();
        path = path.remove(path.left(index));
        path = path.remove("&external=true");
        // replace shortcuts
        QVector<PathShortcut> pathShortcuts = settings->getPathShortCuts();
        for (int i = 0; i < pathShortcuts.length(); i++) {
            if (path.startsWith(pathShortcuts[i].Name)) {
                path = path.replace(pathShortcuts[i].Name, pathShortcuts[i].Path);
                break;
            }
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    } else {
        QDesktopServices::openUrl(url);
    }

}

void ELODocumentHandler::insertLink(const QString &text, const QString &url)
{
    if (currentDocument) {
        currentDocument->getWebPage()->runJavaScript("editor.model.change( writer => { const insertPosition = editor.model.document.selection.getFirstPosition(); writer.insertText( '" + text + "', { linkHref: '" + url + "' }, insertPosition ); } );");
    }
}

void ELODocumentHandler::updateRepoSettings(QJsonObject settings)
{
    user->updateRepoSettings(settings, currentDocument->getRepoName());
}

void ELODocumentHandler::setSpellCheck(bool enabled)
{
    for (int i = 0; i < documents.length(); i++) {
        documents[i]->getWebPage()->profile()->setSpellCheckEnabled(enabled);
    }
}

void ELODocumentHandler::setSpellCheckLanguage(QString language)
{
    for (int i = 0; i < documents.length(); i++) {
        documents[i]->getWebPage()->profile()->setSpellCheckLanguages({language});
    }
}
