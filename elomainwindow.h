#ifndef ELOMAINWINDOW_H
#define ELOMAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineProfile>
#include <QDockWidget>
#include <QSettings>

#include "elotabwidget.h"
#include "elosettingsdialog.h"
#include "elosettings.h"
#include "elometadatadialog.h"
#include "elofileview.h"
#include "elodocumenthandler.h"

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


    // widgets
    ELOTabWidget *tabWidget;
    QDockWidget *calculatorDock;
    QDockWidget *fileDock;

    ELOFileView *fileView;

    ELOSettingsDialog *settingsDialog;
    ELOMetadataDialog *metadataDialog;

    // actions and menus
    QMenu *m_fileMenu;
    QAction *actionUpsync;
    QAction *actionChangePassword;
    QAction *actionLoadUser;
    QAction *actionExit;

    QMenu *m_experimentMenu;
    QAction *actionMetadata;
    QAction *actionSaveFile;
    QAction *actionPrintPreview;
    QAction *actionPrint;
    QAction *actionPrintToPDF;
    QAction *actionAddComment;

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

signals:
    void setClose(bool b);
};
#endif // ELOMAINWINDOW_H
