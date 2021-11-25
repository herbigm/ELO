#include "elodocument.h"


ELODocument::ELODocument(const QString &filePath)
{
    fileInfo = QFileInfo(filePath);
    title = fileInfo.fileName().left(fileInfo.fileName().length()-5);
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

    // create webView
    webView = new ELOWebView();
    webPage = new ELOWebPage(QWebEngineProfile::defaultProfile(), webView);

    QFile experimentFile(fileInfo.filePath());
    experimentFile.open(QIODevice::ReadOnly);
    QString experimentHTML = experimentFile.readAll();
    experimentFile.close();

    QFile editorHTMLFile(QCoreApplication::applicationDirPath() + QDir::separator() + "ckeditor5" + QDir::separator() + "index.html");
    editorHTMLFile.open(QIODevice::ReadOnly);
    QString editorHTML = editorHTMLFile.readAll();
    editorHTMLFile.close();

    QFile cssFile(QCoreApplication::applicationDirPath() + QDir::separator() + "ckeditor5" + QDir::separator() + "styles.css");
    cssFile.open(QIODevice::ReadOnly);
    QString css = cssFile.readAll();
    cssFile.close();

    webView->page()->setHtml(editorHTML.replace("{{Style}}", css).replace("{{CONTENT}}", experimentHTML), QUrl::fromLocalFile(fileInfo.filePath()));
    connect(webView, &ELOWebView::loadFinished, this, &ELODocument::loadJS);
    //webView->page()->runJavaScript(ckeditorJS1 + "\n" + ckeditorJS2);
    //webView->page()->runJavaScript(ckeditorJS2);
    //webView->setUrl(QUrl::fromLocalFile(fileInfo.filePath()));
    //webView->page()->runJavaScript("getEditorData();", [this](const QVariant &v) { qDebug()<<v.toString();});
    //webView->page()->runJavaScript("function hi() {return  \"Hallo\";} hi();", [this](const QVariant &v) { qDebug()<<v.toString();});
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

void ELODocument::saveDocument(const QString content)
{
    QString fileContent = "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\" />\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>{{TITLE}}</title>\n</head>\n<body>\n{{CONTENT}}\n</body>\n</html>";
    fileContent = fileContent.replace("{{TITLE}}", metaData.value("title").toString());
    fileContent = fileContent.replace("{{CONTENT}}", content);
    fileContent = "<!-- " + QJsonDocument(metaData).toJson(QJsonDocument::Compact) + " -->\n" + fileContent;

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
