#ifndef ELOUSER_H
#define ELOUSER_H

#include <QObject>
#include <QTemporaryFile>
#include <QJsonObject>
#include <QDir>

#include "elosettings.h"

enum permissionMode {None, ReadOnly, ReadWrite}; // valid permission modes of users in a repository

class ELOUser : public QObject
{
    Q_OBJECT
public:
    // ELOUser is a Singleton!
    static ELOUser* Instance() {
        if (m_Instance == nullptr) {
            m_Instance = new ELOUser();
        }
        return m_Instance;
    }
    ~ELOUser();

    // getter functions
    QString getUserName() {return userSettings.value("username").toString(); };
    QString getName() {return userSettings.value("name").toString(); };
    QString getEMail() {return userSettings.value("email").toString(); };
    QString getServer() {return userSettings.value("server").toString(); }
    QString getPubKeyFilePath() {return pubKeyFilePath; };
    QString getPrivKeyFilePath() {return privKeyFilePath; };
    QString getAdditionalInformation();
    QString getWebPassword() {
        if (userSettings.value("webPassword").isUndefined())
            return "$2y$10$6SaIY115fEVQcCuoldIM/uRcU96qPcvtHUnF039g6BxpHk5UqOgq6";
        return userSettings.value("webPassword").toString();
    }
    QJsonObject getRepos() {return repos; }
    QString getKeyFile() {return configSavePath + QDir::separator() +  userSettings.value("username").toString() + ".user"; }
    bool getConnected() { return isConnected; }

    // setter functions
    void setAdditionalInformation(QString json);
    void setUserName(QString s) { userSettings.insert("username", s); };
    void setName(QString s) { userSettings.insert("name", s); };
    void setEMail(QString s) { userSettings.insert("email", s); };
    void setServer(QString s) { userSettings.insert("server", s); }
    void setPrivateKey(QString s) { privKey = s; }
    void setPublicKey(QString s) { pubKey = s; }
    void setWebPassword(QString s) { userSettings.insert("webPassword", s); }
    void setRepoPermissions(QJsonObject rp);

    // other functions
    bool isLoaded() {return m_isLoaded;}
    bool tryPassword(const QString &filename, const QByteArray pw);
    void loadUserFile(const QString &filename, const QByteArray pw = QByteArray());
    void readUserFile(const QByteArray content);
    void saveUserFile(QByteArray pw = QByteArray(), QString path = QString());
    void changePassword(const QString &newFile, const QByteArray pw);
    void createRepoPermissions(const QString &gitoliteOutput);
    void updateRepoSettings(QJsonObject repoSettings, const QString &repoName);
    void unloadUser();

    bool canConnectServer(const QString addr, uint port);
    bool canConnectToGit();

private:
    explicit ELOUser();
    static ELOUser *m_Instance; // ELOUser is a Singleton!

    QJsonObject userSettings;
    QString privKey;
    QString pubKey;
    QString pubKeyFilePath;
    QString privKeyFilePath;
    QString configSavePath;
    QTemporaryFile *privKeyFile;
    QTemporaryFile *pubKeyFile;
    QJsonObject repos;
    QByteArray passwd;
    bool m_isLoaded;
    bool isConnected;
    ELOSettings *settings;

signals:
    void userLoaded(bool);
    void notConnected();

};

#endif // ELOUSER_H
