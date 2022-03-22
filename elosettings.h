#ifndef ELOSETTINGS_H
#define ELOSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QDebug>

typedef struct{
    QString Name;
    QString Path;
    QString Description;
} PathShortcut;

class ELOSettings : public QObject
{
    Q_OBJECT
public:
    // ELOSettings is a Singleton!
    static ELOSettings* Instance() {
        if (m_Instance == nullptr) {
            m_Instance = new ELOSettings();
        }
        return m_Instance;
    }

    ~ELOSettings();
    inline QString getWorkingDir() {
        return workingDir;
    }
    inline QByteArray getGeometry() {
        return settings->value("geometry").toByteArray();
    }
    inline QByteArray getState() {
        return settings->value("windowState").toByteArray();
    }
    inline QString getLastOpenDir() {
        return lastOpenDir;
    }
    inline QVector<PathShortcut> getPathShortCuts() {
        return PathShortcuts;
    }
    inline QString getDictionary() {
        return settings->value("dictionary").toString();
    }
    inline bool getSpellCheck() {
        return settings->value("enableSpellCheck").toBool();
    }
    inline QString getSpellCheckLanguage() {
        return settings->value("spellCheckLanguage").toString();
    }

    inline void setLastOpenDir(QString lod) {
        lastOpenDir = lod;
        settings->value("lastOpenDir", lod);
        settings->sync();
    }
    inline void setGeometry(QByteArray geom) {
        settings->setValue("geometry", geom);
        settings->sync();
    }
    inline void setState(QByteArray state) {
        settings->setValue("windowState", state);
        settings->sync();
    }
    inline void setWorkingDir(QString wd) {
        workingDir = wd;
        settings->setValue("workingDir", wd);
        settings->sync();
    }
     inline void addPathShortCut(PathShortcut psc) {
        PathShortcuts.append(psc);
        savePathShortcuts();
    }

    inline void clearPathShortcuts() {
        PathShortcuts.clear();
    }

    inline void setDictionary(QString path) {
        settings->setValue("dictionary", path);
        settings->sync();
    }

    inline void setSpellCheck(bool b) {
        settings->setValue("enableSpellCheck", b);
        settings->sync();
    }
    inline void setSpellCheckLanguage(const QString &lang) {
        settings->setValue("spellCheckLanguage", lang);
        settings->sync();
    }

private:
    explicit ELOSettings(); // ELOSettings is a Singleton!

    QSettings *settings;
    QString workingDir;
    QString lastOpenDir;
    QVector<PathShortcut> PathShortcuts;

    static ELOSettings *m_Instance; // ELOSettings is a Singleton!
    void savePathShortcuts();

signals:

};

typedef ELOSettings TheELOSettings; // ELOSettings is an Singleton!

#endif // ELOSETTINGS_H
