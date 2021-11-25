#include "elomainwindow.h"
#include "elowebview.h"
#include "elocalculatorwidget.h"

#include <QMessageBox>
#include <QMenuBar>

ELOMainWindow::ELOMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // initalisize variables
    settings = TheELOSettings::Instance();
    documentHandler = new ELODocumentHandler();

    // create main widgets
    tabWidget = new ELOTabWidget(QWebEngineProfile::defaultProfile(), this);
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);

    createActions();
    createMenus();
    createWidgets();
    connectActions();
    connectOther();

    // load settings
    restoreGeometry(settings->getGeometry());
    restoreState(settings->getState());

    actionShowCalculator->setChecked(!calculatorDock->isHidden());
    actionShowFileTree->setChecked(!fileDock->isHidden());
}

ELOMainWindow::~ELOMainWindow()
{
    delete documentHandler;
}

void ELOMainWindow::closeEvent(QCloseEvent *event)
{
    settings->setGeometry(saveGeometry());
    settings->setState(saveState());

    if (tabWidget->count() > 0) {
        connect(documentHandler, &ELODocumentHandler::allClosed, this, &ELOMainWindow::allClosed);
        documentHandler->closeAllDocuments();
        event->ignore();
    } else {
        event->accept();
        QMainWindow::closeEvent(event);
    }
}

void ELOMainWindow::startNewFile(const QJsonObject obj)
{
    metadataDialog->updateFormContent(obj);
    metadataDialog->show();
}

void ELOMainWindow::openNewCreatedFile(ELOWebView *view)
{
    tabWidget->addTab(view, documentHandler->getCurrentTitle());
    tabWidget->setCurrentWidget(view);
}

void ELOMainWindow::createWidgets()
{
    calculatorDock = new QDockWidget(tr("Calculator"), this);
    calculatorDock->setWidget(new ELOcalculatorWidget(calculatorDock));
    calculatorDock->setObjectName("calculatorDock");

    fileView = new ELOFileView();
    fileDock = new QDockWidget(tr("Files"), this);
    fileDock->setWidget(fileView);
    fileDock->setObjectName("fileDock");
    fileView->addView("Hallo", "/home/marcus/ELO/Marcus_Herbig");


    settingsDialog = new ELOSettingsDialog(this);
    metadataDialog = new ELOMetadataDialog(this);
}

void ELOMainWindow::createActions()
{
    /*actionUpsync;
    actionChangePassword;
    actionLoadUser;
    actionExit;

    actionPrintPreview;
    actionPrint;
    actionPrintToPDF;
    actionAddComment;

    actionRepoOptions;
    actionSpellCheck;
    actionShowMetadata;
    actionShowAssociatedFiles;
    actionShowComments;

    actionDatabaseChemicals;
    actionDatabaseDevice;*/

    const QIcon showDockIcon = QIcon::fromTheme("dashboard-show", QIcon(":icons/icons/showDashBoard.svg"));
    actionShowCalculator = new QAction(showDockIcon, tr("Show calculator"), this);
    actionShowCalculator->setCheckable(true);

    actionShowFileTree = new QAction(showDockIcon, tr("Show files"), this);
    actionShowFileTree->setCheckable(true);

    const QIcon aboutIcon = QIcon::fromTheme("help-about", QIcon(":icons/icons/info.svg"));
    actionAbout = new QAction(aboutIcon, tr("About this program"), this);

    const QIcon aboutQtIcon = QIcon::fromTheme("help-about", QIcon(":icons/icons/info.svg"));
    actionAboutQt = new QAction(aboutQtIcon, tr("About Qt"), this);

    const QIcon configureIcon = QIcon::fromTheme("configure", QIcon(":icons/icons/configure.svg"));
    actionSettings = new QAction(configureIcon, tr("Settings"), this);

    const QIcon metadataIcon = QIcon::fromTheme("description", QIcon(":icons/icons/description.svg"));
    actionMetadata = new QAction(metadataIcon, tr("view metadata"), this);

    const QIcon exitIcon = QIcon::fromTheme("application-exit", QIcon(":icons/icons/exit.svg"));
    actionExit = new QAction(exitIcon, tr("Exit"), this);

    const QIcon saveFileIcon = QIcon::fromTheme("document-save", QIcon(":icons/icons/document-save.svg"));
    actionSaveFile = new QAction(saveFileIcon, tr("&Save"), this);
    actionSaveFile->setShortcut(Qt::CTRL | Qt::Key_S);
    connect(actionSaveFile, &QAction::triggered, this, &ELOMainWindow::saveCurrentFile);
}

void ELOMainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("File"));
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(actionExit);

    m_experimentMenu = menuBar()->addMenu(tr("Experiment"));
    m_experimentMenu->addAction(actionMetadata);
    m_experimentMenu->addAction(actionSaveFile);

    m_settingsMenu = menuBar()->addMenu(tr("Settings"));
    m_settingsMenu->addAction(actionSettings);
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(actionShowCalculator);
    m_settingsMenu->addAction(actionShowFileTree);

    m_databasMenu = menuBar()->addMenu(tr("Databases"));

    m_infoMenu = menuBar()->addMenu(tr("About"));
    m_infoMenu->addAction(actionAbout);
    m_infoMenu->addAction(actionAboutQt);
}

void ELOMainWindow::connectActions()
{
    connect(actionShowCalculator, &QAction::triggered, calculatorDock, [=](bool checked) { if (checked) calculatorDock->show();  else calculatorDock->hide();});
    connect(calculatorDock, &QDockWidget::visibilityChanged, actionShowCalculator, &QAction::setChecked);

    connect(actionShowFileTree, &QAction::triggered, fileDock, [=](bool checked) { if (checked) fileDock->show();  else fileDock->hide();});
    connect(fileDock, &QDockWidget::visibilityChanged, actionShowFileTree, &QAction::setChecked);

    connect(actionAbout, &QAction::triggered, this, [this](){ QMessageBox::about(this, tr("about this Program"), tr("This programm is written by Marcus Herbig.")); });
    connect(actionAboutQt, &QAction::triggered, this, [this](){ QMessageBox::aboutQt(this, tr("About Qt")); });

    connect(actionSettings, &QAction::triggered, settingsDialog, &QDialog::show);

    connect(actionMetadata, &QAction::triggered, metadataDialog, &QDialog::show);

    connect(actionExit, &QAction::triggered, this, &ELOMainWindow::closeNow);
}

void ELOMainWindow::connectOther()
{
    connect(fileView, &ELOFileView::itemSelected, this, &ELOMainWindow::openFile);
    connect(tabWidget, &QTabWidget::currentChanged, this, &ELOMainWindow::selectCurrentDocument);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &ELOMainWindow::closeTab);
    connect(fileView, &ELOFileView::itemClicked, documentHandler, &ELODocumentHandler::setCurrentDirectory);
    connect(fileView, &ELOFileView::haveRenamed, documentHandler, &ELODocumentHandler::performRenaming);
    connect(fileView, &ELOFileView::fileRemoved, documentHandler, &ELODocumentHandler::deleteFile);
    connect(fileView, &ELOFileView::newFileRequested, documentHandler, &ELODocumentHandler::startFileCreation);
    connect(documentHandler, &ELODocumentHandler::documentTitleChanged, this, &ELOMainWindow::renameTab);
    connect(documentHandler, &ELODocumentHandler::currentMetadataChanged, metadataDialog, &ELOMetadataDialog::updateFormContent);
    connect(metadataDialog, &ELOMetadataDialog::metadataChanged, documentHandler, &ELODocumentHandler::updateMetadata);
    connect(documentHandler, &ELODocumentHandler::askForMetadataForNewFile, this, &ELOMainWindow::startNewFile);
    connect(documentHandler, &ELODocumentHandler::newFileCreatedView, this, &ELOMainWindow::openNewCreatedFile);
    connect(documentHandler, &ELODocumentHandler::newFileCreatedModel, fileView, &ELOFileView::addNewFile);
}

void ELOMainWindow::closeNow()
{
    settings->setGeometry(saveGeometry());
    settings->setState(saveState());

    // save all opened files
    /*while(tabWidget->count() > 0) {
        documentHandler->saveFile();
        ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    }*/
    // commit and try to push
    /*QJsonObject repos = activeUser->getRepoPermissions();
    QStringList keys = repos.keys();
    foreach(QString key, keys) {
        if(repos.value(key).toInt() == ReadWrite) {
            gitCommunication->doTheCommit(activeUser, QDateTime::currentDateTime().toString(), key);
            if(checkOnlineState())
                    gitCommunication->doThePush(activeUser, key);
        }
    }*/
    emit setClose(true);
    close();
}

void ELOMainWindow::openFile(const QString &filePath)
{
    // get ELOWebView for file (new one or known one)
    ELOWebView *view = documentHandler->openFile(filePath);
    if (!view) { // on error or the file is not an experiment file, nullptr ist returned by the documentHandler
        return;
    }
    int index = tabWidget->indexOf(view);
    if (index == -1) { // file not opend yet
        tabWidget->addTab(view, documentHandler->getCurrentTitle());
        tabWidget->setCurrentWidget(view);
    } else { // file is opened
        tabWidget->setCurrentWidget(view);
    }

}

void ELOMainWindow::selectCurrentDocument(int tabIndex)
{
    // select current document at the documentHandler when the tab is changed
    documentHandler->setCurrentDocument(qobject_cast<ELOWebView *>(tabWidget->currentWidget()));
    Q_UNUSED(tabIndex);
}

void ELOMainWindow::closeTab(int tabIndex)
{
    tabWidget->setCurrentIndex(tabIndex);
    documentHandler->requestClosingCurrentDocument(); // closing the current document deletes the ELOWebView, so the Tab is closed automatically, befor closing, modification is checkt and saving asked
}

void ELOMainWindow::renameTab(ELOWebView *view, const QString &newTitle)
{
    int index = tabWidget->indexOf(view);
    if (index >= 0)
        tabWidget->setTabText(index, newTitle);
}

void ELOMainWindow::saveCurrentFile()
{
    documentHandler->saveCurrentDocument();
}

void ELOMainWindow::allClosed()
{
    disconnect(documentHandler, &ELODocumentHandler::allClosed, this, &ELOMainWindow::allClosed);
    close();
}
