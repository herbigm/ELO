#include "elomainwindow.h"
#include "elocalculatorwidget.h"

#include <QMessageBox>
#include <QMenuBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QInputDialog>
#include <QStandardPaths>

ELOMainWindow::ELOMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // initalisize variables
    settings = TheELOSettings::Instance();
    user = ELOUser::Instance();
    documentHandler = new ELODocumentHandler();
    gitCom = new ELOGitProcess(this);

    // create main widgets
    tabWidget = new QTabWidget(this);
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

void ELOMainWindow::openNewCreatedFile(QWebEngineView *view)
{
    tabWidget->addTab(view, documentHandler->getCurrentTitle());
    tabWidget->setCurrentWidget(view);
}

void ELOMainWindow::createWidgets()
{
    setStatusBar(new QStatusBar());
    mainToolBar = addToolBar(tr("Main tool bar"));
    mainToolBar->setObjectName("mainToolBar");
    mainToolBar->addAction(actionSaveFile);
    mainToolBar->addAction(actionAddLink);
    mainToolBar->addAction(actionInsertImage);

    calculatorDock = new QDockWidget(tr("Calculator"), this);
    calculatorDock->setWidget(new ELOcalculatorWidget(calculatorDock));
    calculatorDock->setObjectName("calculatorDock");

    fileView = new ELOFileView();
    fileDock = new QDockWidget(tr("Files"), this);
    fileDock->setWidget(fileView);
    fileDock->setObjectName("fileDock");
    fileView->addView("Hallo", "/home/marcus/ELO/Marcus_Herbig");

    associatedFileView = new ELOAssociatedFileView(this);
    associatedFileDock = new QDockWidget(tr("Associated Files"), this);
    associatedFileDock->setObjectName("associatedFileDock");
    associatedFileDock->setWidget(associatedFileView);

    settingsDialog = new ELOSettingsDialog(this);
    metadataDialog = new ELOMetadataDialog(this);
    linkDialog = new ELOLinkDialog(this);
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
    actionShowComments;

    actionDatabaseChemicals;
    actionDatabaseDevice;*/

    const QIcon showDockIcon = QIcon::fromTheme("dashboard-show", QIcon(":icons/icons/showDashBoard.svg"));
    actionShowCalculator = new QAction(showDockIcon, tr("Show calculator"), this);
    actionShowCalculator->setCheckable(true);

    actionShowFileTree = new QAction(showDockIcon, tr("Show files"), this);
    actionShowFileTree->setCheckable(true);

    actionShowAssociatedFiles = new QAction(showDockIcon, tr("Show associated files"), this);
    actionShowAssociatedFiles->setCheckable(true);

    const QIcon showProcessIcon = QIcon::fromTheme("preferences-system-startup", QIcon(":icons/icons/preferences-system-startup.svg"));
    actionShowProcess = new QAction(showProcessIcon, tr("show git process"), this);
    actionShowProcess->setCheckable(true);

    const QIcon aboutIcon = QIcon::fromTheme("help-about", QIcon(":icons/icons/info.svg"));
    actionAbout = new QAction(aboutIcon, tr("About this program"), this);

    const QIcon aboutQtIcon = QIcon::fromTheme("help-about", QIcon(":icons/icons/info.svg"));
    actionAboutQt = new QAction(aboutQtIcon, tr("About Qt"), this);

    const QIcon configureIcon = QIcon::fromTheme("configure", QIcon(":icons/icons/configure.svg"));
    actionSettings = new QAction(configureIcon, tr("Settings"), this);

    const QIcon metadataIcon = QIcon::fromTheme("description", QIcon(":icons/icons/description.svg"));
    actionMetadata = new QAction(metadataIcon, tr("View metadata"), this);

    const QIcon exitIcon = QIcon::fromTheme("application-exit", QIcon(":icons/icons/exit.svg"));
    actionExit = new QAction(exitIcon, tr("Exit"), this);

    const QIcon saveFileIcon = QIcon::fromTheme("document-save", QIcon(":icons/icons/document-save.svg"));
    actionSaveFile = new QAction(saveFileIcon, tr("&Save"), this);
    actionSaveFile->setShortcut(Qt::CTRL | Qt::Key_S);

    const QIcon linkIcon = QIcon::fromTheme("insert-link", QIcon(":icons/icons/insert-link.svg"));
    actionAddLink = new QAction(linkIcon, tr("Insert link"), this);

    const QIcon insertImgaeIcon = QIcon::fromTheme("insert-image", QIcon(":icons/icons/insert-image.svg"));
    actionInsertImage = new QAction(insertImgaeIcon, tr("Insert image"), this);

    const QIcon loadUserIcon = QIcon::fromTheme("user-others", QIcon(":icons/icons/user-others.svg"));
    actionLoadUser = new QAction(loadUserIcon, tr("Load user"), this);

    const QIcon changePasswordIcon = QIcon::fromTheme("document-decrypt", QIcon(":icons/icons/document-decrypt.svg"));
    actionChangePassword = new QAction(changePasswordIcon, tr("change password"), this);
    actionChangePassword->setEnabled(false);
}

void ELOMainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("File"));
    m_fileMenu->addAction(actionLoadUser);
    QDir ConfigSaveDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QFileInfoList finfo = ConfigSaveDir.entryInfoList(QStringList("*.elouser"), QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
    recentUserGroup = new QActionGroup(this);
    connect(recentUserGroup, &QActionGroup::triggered, this, &ELOMainWindow::loadRecentUser);
    if (finfo.length() > 0){
        m_fileMenu->addSection(tr("recent user files"));
        foreach(QFileInfo s, finfo) {
            recentUserGroup->addAction(s.fileName());
        }
        m_fileMenu->addActions(recentUserGroup->actions());
    }
    m_fileMenu->addAction(actionChangePassword);
    m_fileMenu->addAction(actionShowProcess);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(actionExit);

    m_experimentMenu = menuBar()->addMenu(tr("Experiment"));
    m_experimentMenu->addAction(actionMetadata);
    m_experimentMenu->addAction(actionSaveFile);
    m_experimentMenu->addAction(actionAddLink);
    m_experimentMenu->addAction(actionInsertImage);

    m_settingsMenu = menuBar()->addMenu(tr("Settings"));
    m_settingsMenu->addAction(actionSettings);
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(actionShowCalculator);
    m_settingsMenu->addAction(actionShowFileTree);
    m_settingsMenu->addAction(actionShowAssociatedFiles);

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

    connect(actionShowAssociatedFiles, &QAction::triggered, associatedFileDock, [=](bool checked) { if (checked) associatedFileDock->show();  else associatedFileDock->hide();});
    connect(associatedFileDock, &QDockWidget::visibilityChanged, actionShowAssociatedFiles, &QAction::setChecked);

    connect(actionShowProcess, &QAction::triggered, gitCom, &ELOGitProcess::show);

    connect(actionAbout, &QAction::triggered, this, [this](){ QMessageBox::about(this, tr("about this Program"), tr("This programm is written by Marcus Herbig.")); });
    connect(actionAboutQt, &QAction::triggered, this, [this](){ QMessageBox::aboutQt(this, tr("About Qt")); });

    connect(actionSettings, &QAction::triggered, settingsDialog, &QDialog::show);
    connect(actionMetadata, &QAction::triggered, metadataDialog, &QDialog::show);

    connect(actionExit, &QAction::triggered, this, &ELOMainWindow::closeNow);

    connect(actionSaveFile, &QAction::triggered, this, &ELOMainWindow::saveCurrentFile);
    connect(actionAddLink, &QAction::triggered, linkDialog, [=]() {linkDialog->showShortcuts(); linkDialog->show();});
    connect(actionInsertImage, &QAction::triggered, this, &ELOMainWindow::openImage);

    connect(actionLoadUser, &QAction::triggered, this, &ELOMainWindow::loadNewUser);
    connect(actionChangePassword, &QAction::triggered, this, &ELOMainWindow::changePassword);
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
    connect(linkDialog, &ELOLinkDialog::setLinkRequest, documentHandler, &ELODocumentHandler::insertLink);
    connect(documentHandler, &ELODocumentHandler::currentFileChanged, associatedFileView, &ELOAssociatedFileView::changeModelRoot);
    connect(associatedFileView, &ELOAssociatedFileView::insertLinkRequest, documentHandler, &ELODocumentHandler::insertLink);
    connect(associatedFileView, &ELOAssociatedFileView::insertImageRequest, documentHandler, &ELODocumentHandler::insertImage);
    connect(user, &ELOUser::userLoaded, this, &ELOMainWindow::userLoaded);
}

void ELOMainWindow::closeNow()
{
    settings->setGeometry(saveGeometry());
    settings->setState(saveState());

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
    QWebEngineView *view = documentHandler->openFile(filePath);
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
    documentHandler->setCurrentDocument(qobject_cast<QWebEngineView *>(tabWidget->currentWidget()));
    Q_UNUSED(tabIndex);
}

void ELOMainWindow::closeTab(int tabIndex)
{
    tabWidget->setCurrentIndex(tabIndex);
    documentHandler->requestClosingCurrentDocument(); // closing the current document deletes the ELOWebView, so the Tab is closed automatically, befor closing, modification is checkt and saving asked
}

void ELOMainWindow::renameTab(QWebEngineView *view, const QString &newTitle)
{
    int index = tabWidget->indexOf(view);
    if (index >= 0)
        tabWidget->setTabText(index, newTitle);
}

void ELOMainWindow::loadUser(QString fileName)
{
    if (fileName.isEmpty()) {
        // what is the last opened dir?
        if(!settings->getLastOpenDir().isEmpty()) {
            fileName = QFileDialog::getOpenFileName(this, tr("UserFile"), settings->getLastOpenDir(), tr("All files (*)"));
        } else {
            fileName = QFileDialog::getOpenFileName(this, tr("UserFile"), QDir::homePath(), tr("All files (*)"));
        }
    }

    if(!fileName.isEmpty()) { // file selected
        // ask for the password
        bool ok;
        QString text = QInputDialog::getText(this, tr("Password required"), tr("Password"), QLineEdit::Password, "", &ok);
        if (ok) {
            // password dialog successfull
            if(user->tryPassword(fileName, text.toUtf8())) { // check the password
                // open the file and read
                QFileInfo finfo(fileName);
                settings->setLastOpenDir(finfo.filePath());
                user->loadUserFile(fileName, text.toUtf8());
            } else { // wrong password
                QMessageBox::critical(this, tr("wrong password"), tr("The entered password is wrong. Aborting!"));
            }
        }
    } else { //no file selected
        statusBar()->showMessage(tr("error loading user"),2000);
    }
}

void ELOMainWindow::loadNewUser()
{
    QString fileName;
    // what is the last opened dir?
    if(!settings->getLastOpenDir().isEmpty()) {
        fileName = QFileDialog::getOpenFileName(this, tr("UserFile"), settings->getLastOpenDir(), tr("All files (*)"));
    } else {
        fileName = QFileDialog::getOpenFileName(this, tr("UserFile"), QDir::homePath(), tr("All files (*)"));
    }
    if (!fileName.isEmpty()) {
        loadUser(fileName);
    }
}

void ELOMainWindow::loadRecentUser(QAction *a)
{
    loadUser(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + a->text());
}

void ELOMainWindow::userLoaded(bool success)
{ // TODO: handle logout action (setEnabled)
    if (success) {
        statusBar()->showMessage(tr("user loaded succesfully"),2000);
        actionLoadUser->setEnabled(false);
        recentUserGroup->setEnabled(false);
        actionChangePassword->setEnabled(true);
    } else {
        statusBar()->showMessage(tr("error loading user"),2000);
        actionLoadUser->setEnabled(true);
        recentUserGroup->setEnabled(true);
        actionChangePassword->setEnabled(false);
    }
}

void ELOMainWindow::changePassword()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Old password required"), tr("Old password"), QLineEdit::Password, "", &ok);
    if (ok) {
        if(user->tryPassword(user->getKeyFile(), text.toUtf8())) {
            bool ok2, ok3;
            QString text2 = QInputDialog::getText(this, tr("New password required"), tr("new password"), QLineEdit::Password, "", &ok2);
            QString text3 = QInputDialog::getText(this, tr("New password required"), tr("repeat new password"), QLineEdit::Password, "", &ok3);
            if(ok2 && ok3) {
                if(text2 == text3) {
                     // change the password
                     // get place to save the new user file
                    QString fileName = QFileDialog::getSaveFileName(this, tr("Where to save the new user file?"), QDir::homePath());
                     //changing the password
                    user->changePassword(fileName, text2.toUtf8());
                    QMessageBox::information(this, tr("Password changed"), tr("Your Password is changed now."));
                } else {
                    // password repetition not successfull
                    QMessageBox::critical(this, tr("Verification failed"), tr("The new passwords were not the same. Aborting!"));
                }
            }
        } else {
            QMessageBox::critical(this, tr("wrong password"), tr("The entered password is wrong. Aborting!"));
        }
    }
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

void ELOMainWindow::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Image to Insert"), settings->getWorkingDir(), tr("Images (*.png *.jpg *.bmp *.gif *.svg"));
    if (!fileName.isEmpty()) {
        if (!fileName.startsWith(settings->getWorkingDir())) {
            // copy file into associated files
            fileName = documentHandler->copyToAssociatedFiles(fileName);
        }
        documentHandler->insertImage(fileName);
    }
}
