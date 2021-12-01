#include "elouser.h"

#include <QJsonDocument>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>

#include "qblowfish.h"

ELOUser *ELOUser::m_Instance = nullptr; // ELOSettings is a Singleton!

ELOUser::ELOUser()
{
    privKeyFile = new QTemporaryFile();
    privKeyFile->setAutoRemove(true);
    pubKeyFile = new QTemporaryFile();
    pubKeyFile->setAutoRemove(true);

    m_isLoaded = false;
    configSavePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

ELOUser::~ELOUser()
{
    delete pubKeyFile;
    delete privKeyFile;
}

QString ELOUser::getAdditionalInformation()
{
    QStringList keys = userSettings.keys();
    QJsonObject obj;
    foreach (QString key, keys) {
        if (key != "username" && key != "name" && key != "email" && key != "server") {
            obj.insert(key, userSettings.value(key));
        }
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Indented);
}

void ELOUser::setAdditionalInformation(QString json)
{
    QJsonDocument doc;
    QJsonObject obj = doc.fromJson(json.toUtf8()).object();
    QStringList keys = obj.keys();
    foreach (QString key, keys) {
        userSettings.insert(key, obj.value(key));
    }
}

void ELOUser::setRepoPermissions(QJsonObject rp)
{
    repos = rp;
}

bool ELOUser::tryPassword(const QString &filename, const QByteArray pw)
{
    QFile file(filename);
    if (file.open(QFile::ReadOnly)) {
        QByteArray content = qUncompress(file.readAll());
        if(pw.length() > 0){
            QBlowfish bf(pw);
            bf.setPaddingEnabled(true);
            content = bf.decrypted(content);
        }
        content = QByteArray::fromBase64(content);
        QString c(content);
        if(c.indexOf("PRIVKEY") > -1) {
            return true;
        }
    }
    return false;
}

void ELOUser::loadUserFile(const QString &filename, const QByteArray pw)
{
    /* Reads all information from a file */
    QFile file(filename);
    if (file.open(QFile::ReadOnly)) {
        QByteArray content = qUncompress(file.readAll());
        if(pw.length() > 0){
            QBlowfish bf(pw);
            bf.setPaddingEnabled(true);
            content = bf.decrypted(content);
            passwd = pw;
        }
        content = QByteArray::fromBase64(content);
        readUserFile(content);

        // create keyfile

        if (privKeyFile->open()) {
            QTextStream out(privKeyFile);
            out.setEncoding(QStringConverter::Utf8);
            out << privKey;
            out.flush();
            privKeyFilePath = privKeyFile->fileName();
        }

        if (pubKeyFile->open()) {
            QTextStream out(pubKeyFile);
            out.setEncoding(QStringConverter::Utf8);
            out << pubKey;
            out.flush();
            pubKeyFilePath = pubKeyFile->fileName();
        }

        // if file is not in the settings directory copy it to this directory
        QFileInfo finfo = QFileInfo(file);
        if(finfo.dir().path() != configSavePath) {
            saveUserFile(pw);
        }
        emit userLoaded(true);
    } else {
        emit userLoaded(false);
    }
}

void ELOUser::readUserFile(const QByteArray content)
{
    QTextStream in(content);

    in.setEncoding(QStringConverter::Utf8);
    QString line = in.readLine();
    bool privkeystart = false;
    bool pubkeystart = false;
    QString userData;
    bool privkeystop = false;
    bool pubkeystop = false;
    while(!in.atEnd()) {
        if(line != "PRIVKEY" && !privkeystart && !privkeystop && !pubkeystart && !pubkeystop) {
            userData += line;
        } else if(line == "PRIVKEY" && !privkeystart && !privkeystop && !pubkeystart && !pubkeystop) {
            privkeystart = true;
        } else if(privkeystart && line == "ENDPRIVKEY" && !privkeystop && !pubkeystart && !pubkeystop) {
            privkeystop = true;
        } else if(privkeystart && !privkeystop && !pubkeystart && !pubkeystop) {
            privKey += line + "\n";
        } else if(line != "PUBKEY" && privkeystart && privkeystop && !pubkeystart && !pubkeystop) {
            userData += line;
        } else if(line == "PUBKEY" && privkeystart && privkeystop && !pubkeystart && !pubkeystop) {
            pubkeystart = true;
        } else if(pubkeystart && line == "ENDPUBKEY" && privkeystop && privkeystart && !pubkeystop) {
            pubkeystop = true;
        } else if(privkeystart && privkeystop && pubkeystart && !pubkeystop) {
            pubKey += line + "\n";
        }
        line = in.readLine();
    }
    QJsonDocument jUser = QJsonDocument::fromJson(userData.toUtf8());
    userSettings = jUser.object();
}

void ELOUser::saveUserFile(QByteArray pw, QString path)
{
    QFile file;
    if (path.isEmpty()) {
        file.setFileName(configSavePath + QDir::separator() +  userSettings.value("username").toString() + ".elouser");
    } else {
        file.setFileName(path);
    }
    if (file.open(QFile::WriteOnly)) {
        QJsonDocument jUser(userSettings);
        QJsonDocument jRepo(repos);
        QByteArray  content;
        QTextStream out(&content);
        out.setEncoding(QStringConverter::Utf8);
        out << jUser.toJson(QJsonDocument::Compact) << Qt::endl << Qt::endl;
        out << "PRIVKEY" << Qt::endl;
        out << privKey;
        out << "ENDPRIVKEY" << Qt::endl << Qt::endl;
        out << "PUBKEY" << Qt::endl;
        out << pubKey;
        out << "ENDPUBKEY" << Qt::endl << Qt::endl;
        out << jRepo.toJson(QJsonDocument::Compact) << Qt::endl;

        QByteArray c = content.toBase64();
        if(pw.length() <= 0)
            if(passwd.length() > 0)
                pw = passwd;
        if(pw.length() > 0 ) {
            QBlowfish bf(pw);
            bf.setPaddingEnabled(true);
            c = bf.encrypted(c);
            passwd = pw;
        }
        file.write(qCompress(c, 9));
    } else {
        qDebug() << "no file written";
    }
}

void ELOUser::changePassword(const QString &newFile, const QByteArray pw)
{
    saveUserFile(pw); // Update passwort in internal file
    QFile file(getKeyFile());
    file.copy(newFile); // copy internal file to new path
}
