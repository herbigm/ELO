#ifndef ELOMAINWINDOW_H
#define ELOMAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineProfile>
#include <QDockWidget>
#include <QSettings>
#include <QToolBar>
#include <QWebEngineView>
#include <QActionGroup>

#include "elosettingsdialog.h"
#include "elosettings.h"
#include "elometadatadialog.h"
#include "elofileview.h"
#include "elodocumenthandler.h"
#include "elolinkdialog.h"
#include "eloassociatedfileview.h"
#include "elouser.h"
#include "elogitprocess.h"

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
     void openNewCreatedFile(QWebEngineView *view);
     void saveCurrentFile();
     void allClosed();
     void openImage();

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
    void renameTab(QWebEngineView *view, const QString &newTitle);

    void loadUser(QString fileName);
    void loadNewUser();
    void loadRecentUser(QAction *a);
    void userLoaded(bool success);
    void changePassword();


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

    QToolBar *mainToolBar;

    // actions and menus
    QMenu *m_fileMenu;
    QAction *actionUpsync;
    QAction *actionChangePassword;
    QAction *actionLoadUser;
    QActionGroup *recentUserGroup;
    QAction *actionExit;
    QAction *actionShowProcess;

    QMenu *m_experimentMenu;
    QAction *actionMetadata;
    QAction *actionSaveFile;
    QAction *actionPrintPreview;
    QAction *actionPrint;
    QAction *actionPrintToPDF;
    QAction *actionAddComment;
    QAction *actionAddLink;
    QAction *actionInsertImage;

    QMenu *m_settingsMenu;
    QAction *actionSettings;
    QAction *actionRepoOptions;
    QAction *actionSpellCheck;
    QAction *actionShowMetadata;
    QAction *actionShowAssociatedFiles;
    QAction *actionShowFileTree;
    QAction *actionShowComments;
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

signals:
    void setClose(bool b);
};
#endif // ELOMAINWINDOW_H
