#include "elosettings.h"

ELOSettings *ELOSettings::m_Instance = nullptr; // ELOSettings is a Singleton!

ELOSettings::ELOSettings()
{
    // init the QSettings
    settings = new QSettings("TU Bergakademie Freiberg", "Elektronisches LaborjOurnal");

    /*
     * get all settings from QSettings, which can be defined directly by the user
     *
     * the Geometry and windowState are not got because the user cannot change them directly
     */

    // get or set the correct working dir
    if(!settings->value("workingDir").toString().isEmpty()) {
        workingDir = settings->value("workingDir").toString();
    } else {
        QDir dir(QDir::homePath() + QDir::separator() + "ELO" + QDir::separator());
        workingDir = dir.path();
        settings->setValue("workingDir", workingDir);
        settings->sync();
    }

    // get the last opened directory
    lastOpenDir = settings->value("lastOpenDir").toString();

    // read the path shortcuts
    PathShortcuts.clear();
    QStringList str = settings->value("pathShortCuts").toStringList();
    for(int i = 0; i < str.length(); i++) {
        PathShortcut psc;
        QStringList parts = str[i].split("::");
        if(parts.length() == 3){
            psc.Name = parts[0];
            psc.Path = parts[1];
            psc.Description = parts[2];
            PathShortcuts.append(psc);
        }
    }
}

ELOSettings::~ELOSettings()
{
    settings->sync();
    delete settings;
}

void ELOSettings::savePathShortcuts()
{
    QStringList str;
    for(int i = 0; i < PathShortcuts.length(); i++) {
        str << PathShortcuts[i].Name + "::" + PathShortcuts[i].Path + "::" + PathShortcuts[i].Description;
    }
    settings->setValue("pathShortCuts", str);
    settings->sync();
}
