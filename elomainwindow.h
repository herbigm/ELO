#ifndef ELOMAINWINDOW_H
#define ELOMAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineProfile>
#include <QDockWidget>
#include <QSettings>
#include <QToolBar>
#include <QActionGroup>
#include <QTranslator>

#include "elosettingsdialog.h"
#include "elosettings.h"
#include "elometadatadialog.h"
#include "elofileview.h"
#include "elodocumenthandler.h"
#include "elolinkdialog.h"
#include "eloassociatedfileview.h"
#include "elouser.h"
#include "elogitprocess.h"
#include "eloreposettingsdialog.h"

class ELOMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ELOMainWindow(QWidget *parent = nullptr);
    ~ELOMainWindow();

protected:
     void closeEvent(QCloseEvent *event);

private slots:
     void startNewFile(const QJsonObject obj);
     void openNewCreatedFile(ELOWebView *view);
     void saveCurrentFile();
     void allClosed();
     void openImage();
     void printToPDF();

private:
    void createWidgets();
    void createActions();
    void createMenus();
    void connectActions();
    void connectOther();
    void closeNow();

    void openFile(const QString &filePath);

    void selectCurrentDocument(int tabIndex);
    void closeTab(int tabIndex);
    void renameTab(ELOWebView *view, const QString &newTitle);

    void loadUser(QString fileName);
    void loadNewUser();
    void loadRecentUser(QAction *a);
    void userLoaded(bool success);
    void changePassword();
    void logoutUser();

    void applyRepoPermissions(permissionMode permissions);
    void applyFilePermissions(permissionMode permissions);
    void upsync();

    void notConnected();


    // widgets
    QTabWidget *tabWidget;
    QDockWidget *calculatorDock;
    QDockWidget *fileDock;
    QDockWidget *associatedFileDock;

    ELOFileView *fileView;
    ELOAssociatedFileView *associatedFileView;

    ELOSettingsDialog *settingsDialog;
    ELOMetadataDialog *metadataDialog;
    ELOLinkDialog *linkDialog;
    ELOGitProcess *gitCom;
    ELORepoSettingsDialog *repoSettingsDialog;

    QToolBar *mainToolBar;

    // actions and menus
    QMenu *m_fileMenu;
    QAction *actionUpsync;
    QAction *actionChangePassword;
    QAction *actionLoadUser;
    QActionGroup *recentUserGroup;
    QAction *actionExit;
    QAction *actionShowProcess;
    QAction *actionLogout;

    QMenu *m_experimentMenu;
    QAction *actionNewFile;
    QAction *actionMetadata;
    QAction *actionSaveFile;
    QAction *actionAddLink;
    QAction *actionInsertImage;
    QAction *actionPrintPDF;

    QMenu *m_settingsMenu;
    QAction *actionSettings;
    QAction *actionRepoSettings;
    QAction *actionShowAssociatedFiles;
    QAction *actionShowFileTree;
    QAction *actionShowCalculator;

    QMenu *m_databasMenu;
    QAction *actionDatabaseChemicals;
    QAction *actionDatabaseDevice;

    QMenu *m_infoMenu;
    QAction *actionAboutQt;
    QAction *actionAbout;

    // setting
    ELOSettings *settings;

    // documentHandler
    ELODocumentHandler *documentHandler;

    // user
    ELOUser *user;

    // translator
    QTranslator m_translator;
    QTranslator m_translatorQt;

signals:
    void setClose(bool b);
};
#endif // ELOMAINWINDOW_H
