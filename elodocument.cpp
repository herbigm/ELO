#include "elodocument.h"

#include <QWebEngineSettings>
#include "elosettings.h"

ELODocument::ELODocument(const QString &filePath, permissionMode permissions)
{
    fileInfo = QFileInfo(filePath);
    fileTitle = fileInfo.fileName().left(fileInfo.fileName().length()-5);
    // read file
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        // read metaData
        QString line = in.readLine();
        QJsonDocument jdoc = QJsonDocument::fromJson(line.mid(4, line.length() - 7).trimmed().toUtf8());
        metaData = jdoc.object();
    }

    // create QWebEngineProfile and change settings
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->settings()->setUnknownUrlSchemePolicy(QWebEngineSettings::AllowAllUnknownUrlSchemes);
    profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);

    // create webView
    webView = new ELOWebView();
    webPage = new ELOWebPage(profile, webView);
    webView->setPage(webPage);

    QFile experimentFile(fileInfo.filePath());
    experimentFile.open(QIODevice::ReadOnly);
    QString experimentHTML = experimentFile.readAll();
    experimentFile.close();
    experimentHTML = experimentHTML.replace("internal:", "internal/");

    QFile editorHTMLFile(QCoreApplication::applicationDirPath() + QDir::separator() + "ckeditor5" + QDir::separator() + "index.html");
    editorHTMLFile.open(QIODevice::ReadOnly);
    QString editorHTML = editorHTMLFile.readAll();
    editorHTMLFile.close();

    QFile cssFile(QCoreApplication::applicationDirPath() + QDir::separator() + "ckeditor5" + QDir::separator() + "styles.css");
    cssFile.open(QIODevice::ReadOnly);
    QString css = cssFile.readAll();
    cssFile.close();

    if (metaData.contains("template")) {
        QString templateName = metaData.value("template").toString();
        templateName = templateName.remove(".tmplt").remove(".html");
        QFile cssFile2(TheELOSettings::Instance()->getWorkingDir() + QDir::separator() + "ELOtemplates" + QDir::separator() + templateName + ".css");
        if (cssFile2.open(QIODevice::ReadOnly)) {
            css += "\n" + cssFile2.readAll();
            cssFile2.close();
        }
    }

    webView->page()->setHtml(editorHTML.replace("{{Style}}", css).replace("{{CONTENT}}", experimentHTML), QUrl::fromLocalFile(fileInfo.filePath()));
    if (permissions == ReadWrite) // only load the editor, if the file could be changed
        connect(webView, &ELOWebView::loadFinished, this, &ELODocument::loadJS);
}

void ELODocument::loadJS(bool ok)
{
    QFile ckeditorFile(QCoreApplication::applicationDirPath() + QDir::separator() + "ckeditor5" + QDir::separator() + "ckeditor.js");
    ckeditorFile.open(QIODevice::ReadOnly);
    QString ckeditorJS1 = ckeditorFile.readAll();
    ckeditorFile.close();

    QFile ckeditorFile2(QCoreApplication::applicationDirPath() + QDir::separator() + "ckeditor5" + QDir::separator() + "editorSettings.js");
    ckeditorFile2.open(QIODevice::ReadOnly);
    QString ckeditorJS2 = ckeditorFile2.readAll();
    ckeditorFile2.close();

    webView->page()->runJavaScript(ckeditorJS1 + "\n" + ckeditorJS2);
    webView->page()->runJavaScript("window.editor.getData();", [this](const QVariant &v) { lastSavedContent  = v.toString();});
}

void ELODocument::startSaveing()
{
    webView->page()->runJavaScript("window.editor.getData();", [this](const QVariant &v) { saveDocument(v.toString());});
}

void ELODocument::printToPdf(const QString &path)
{
    webView->page()->runJavaScript("window.editor.getData();", [this, path](const QVariant &v) {
        QString fileContent = "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\" />\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>{{TITLE}}</title>\n</head>\n<body>\n{{CONTENT}}\n</body>\n</html>";
        fileContent = fileContent.replace("{{TITLE}}", metaData.value("title").toString());
        fileContent = fileContent.replace("{{CONTENT}}", v.toString());
        fileContent = "<!-- " + QJsonDocument(metaData).toJson(QJsonDocument::Compact) + " -->\n" + fileContent;

        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(path);

        QTextDocument doc;
        doc.setHtml(fileContent);
        doc.print(&printer);
    });
}

void ELODocument::saveDocument(const QString content)
{
    QString fileContent;
    if (repoName != "ELOtemplates") {
        fileContent = "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\" />\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>{{TITLE}}</title>\n</head>\n<body>\n{{CONTENT}}\n</body>\n</html>";
        fileContent = fileContent.replace("{{TITLE}}", metaData.value("title").toString());
        fileContent = fileContent.replace("{{CONTENT}}", content);
        fileContent = "<!-- " + QJsonDocument(metaData).toJson(QJsonDocument::Compact) + " -->\n" + fileContent;
    } else {
        fileContent = "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\" />\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>{{experiment number}}</title>\n</head>\n<body>\n{{CONTENT}}\n</body>\n</html>";
        fileContent = fileContent.replace("{{CONTENT}}", content);
    }

    QFile f(fileInfo.filePath());
    if (f.open(QIODevice::WriteOnly)) {
        QTextStream out(&f);
        out.setEncoding(QStringConverter::Utf8);
        out << fileContent;
        out.flush();
        f.close();
        lastSavedContent = content;
        emit wasSaved();
    } else {
        qDebug() << "cannot open file for writing!";
    }
}

void ELODocument::checkModified()
{
    webView->page()->runJavaScript("window.editor.getData();", [this](const QVariant &v) { if (lastSavedContent == v.toString()) { emit wasModified(false); } else { emit wasModified(true); } });
}
