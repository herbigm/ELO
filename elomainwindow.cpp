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
    gitCom = new ELOGitProcess();
    repoSettingsDialog = new ELORepoSettingsDialog(this);

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
        // commit and try to push
        upsync();
        emit setClose(true);
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
    setStatusBar(new QStatusBar());
    mainToolBar = addToolBar(tr("Main tool bar"));
    mainToolBar->setObjectName("mainToolBar");
    mainToolBar->addAction(actionNewFile);
    mainToolBar->addAction(actionSaveFile);
    mainToolBar->addAction(actionAddLink);
    mainToolBar->addAction(actionInsertImage);
    mainToolBar->addAction(actionPrintPDF);

    calculatorDock = new QDockWidget(tr("Calculator"), this);
    calculatorDock->setWidget(new ELOcalculatorWidget(calculatorDock));
    calculatorDock->setObjectName("calculatorDock");

    fileView = new ELOFileView();
    fileDock = new QDockWidget(tr("Files"), this);
    fileDock->setWidget(fileView);
    fileDock->setObjectName("fileDock");

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
    const QIcon showDockIcon = QIcon::fromTheme("dashboard-show", QIcon(":icons/icons/showDashBoard.svg"));
    actionShowCalculator = new QAction(showDockIcon, tr("Show calculator"), this);
    actionShowCalculator->setCheckable(true);

    actionShowFileTree = new QAction(showDockIcon, tr("Show files"), this);
    actionShowFileTree->setCheckable(true);

    actionShowAssociatedFiles = new QAction(showDockIcon, tr("Show associated files"), this);
    actionShowAssociatedFiles->setCheckable(true);

    const QIcon showProcessIcon = QIcon::fromTheme("preferences-system-startup", QIcon(":icons/icons/preferences-system-startup.svg"));
    actionShowProcess = new QAction(showProcessIcon, tr("show git process"), this);

    const QIcon aboutIcon = QIcon::fromTheme("help-about", QIcon(":icons/icons/info.svg"));
    actionAbout = new QAction(aboutIcon, tr("About this program"), this);

    const QIcon aboutQtIcon = QIcon::fromTheme("help-about", QIcon(":icons/icons/info.svg"));
    actionAboutQt = new QAction(aboutQtIcon, tr("About Qt"), this);

    const QIcon configureIcon = QIcon::fromTheme("configure", QIcon(":icons/icons/configure.svg"));
    actionSettings = new QAction(configureIcon, tr("Settings"), this);
    actionRepoSettings = new QAction(configureIcon, tr("Repositorium settings"), this);

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

    const QIcon newFileIcon = QIcon::fromTheme("document-new", QIcon(":icons/icons/document-new.svg"));
    actionNewFile = new QAction(newFileIcon, tr("New File"), this);
    actionNewFile->setEnabled(false);

    const QIcon upsyncIcon = QIcon::fromTheme("upload-media", QIcon(":icons/icons/upload-media.svg"));
    actionUpsync = new QAction(upsyncIcon, tr("Upsync"), this);
    actionUpsync->setEnabled(false);

    const QIcon logoutIcon = QIcon::fromTheme("im-kick-user", QIcon(":icons/icons/im-kick-user.svg"));
    actionLogout = new QAction(logoutIcon, tr("Logout"), this);
    actionLogout->setEnabled(false);

    const QIcon pdfIcon = QIcon::fromTheme("application-pdf", QIcon(":icons/icons/application-pdf.svg"));
    actionPrintPDF = new QAction(pdfIcon, tr("Print to PDF"), this);
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
    m_fileMenu->addAction(actionUpsync);
    m_fileMenu->addAction(actionLogout);
    m_fileMenu->addAction(actionShowProcess);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(actionExit);

    m_experimentMenu = menuBar()->addMenu(tr("Experiment"));
    m_experimentMenu->addAction(actionNewFile);
    m_experimentMenu->addAction(actionMetadata);
    m_experimentMenu->addAction(actionSaveFile);
    m_experimentMenu->addAction(actionAddLink);
    m_experimentMenu->addAction(actionInsertImage);
    m_experimentMenu->addAction(actionPrintPDF);

    m_settingsMenu = menuBar()->addMenu(tr("Settings"));
    m_settingsMenu->addAction(actionSettings);
    m_settingsMenu->addAction(actionRepoSettings);
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
    connect(actionRepoSettings, &QAction::triggered, repoSettingsDialog, &QDialog::show);
    connect(actionMetadata, &QAction::triggered, metadataDialog, &QDialog::show);

    connect(actionExit, &QAction::triggered, this, &ELOMainWindow::closeNow);

    connect(actionSaveFile, &QAction::triggered, this, &ELOMainWindow::saveCurrentFile);
    connect(actionAddLink, &QAction::triggered, linkDialog, [=]() {linkDialog->showShortcuts(); linkDialog->show();});
    connect(actionInsertImage, &QAction::triggered, this, &ELOMainWindow::openImage);

    connect(actionLoadUser, &QAction::triggered, this, &ELOMainWindow::loadNewUser);
    connect(actionChangePassword, &QAction::triggered, this, &ELOMainWindow::changePassword);

    connect(actionNewFile, &QAction::triggered, this, [=](bool b) {documentHandler->startFileCreation();});

    connect(actionUpsync, &QAction::triggered, this, &ELOMainWindow::upsync);
    connect(actionLogout, &QAction::triggered, this, &ELOMainWindow::logoutUser);
    connect(actionPrintPDF, &QAction::triggered, this, &ELOMainWindow::printToPDF);
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
    connect(documentHandler, &ELODocumentHandler::repoPermissionsChanged, this, &ELOMainWindow::applyRepoPermissions);
    connect(documentHandler, &ELODocumentHandler::filePermissionsChanged, this, &ELOMainWindow::applyFilePermissions);
    connect(documentHandler, &ELODocumentHandler::openExperimentRequest, this, &ELOMainWindow::openFile);
    connect(repoSettingsDialog, &ELORepoSettingsDialog::repoSettingsChanged, documentHandler, &ELODocumentHandler::updateRepoSettings);
    connect(this, &ELOMainWindow::setClose, gitCom,&ELOGitProcess::setClose);
    connect(settingsDialog, &ELOSettingsDialog::spellCheckChanged, documentHandler, &ELODocumentHandler::setSpellCheck);
    connect(settingsDialog, &ELOSettingsDialog::spellCheckLanguageChanged, documentHandler, &ELODocumentHandler::setSpellCheckLanguage);
    connect(user, &ELOUser::notConnected, this, &ELOMainWindow::notConnected);
}

void ELOMainWindow::closeNow()
{
    settings->setGeometry(saveGeometry());
    settings->setState(saveState());

    // saving unaved Changes
    if (tabWidget->count() > 0) {
        connect(documentHandler, &ELODocumentHandler::allClosed, this, &ELOMainWindow::allClosed);
        documentHandler->closeAllDocuments();
    }

    // commit and try to push
    if (!user->getServer().isEmpty())
        upsync();
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
    //tabWidget->setCurrentIndex(tabIndex);
    //documentHandler->requestClosingCurrentDocument(); // closing the current document deletes the ELOWebView, so the Tab is closed automatically, befor closing, modification is checkt and saving asked
    documentHandler->requestClosingDocument(qobject_cast<ELOWebView *>(tabWidget->widget(tabIndex)));
}

void ELOMainWindow::renameTab(ELOWebView *view, const QString &newTitle)
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
        actionLoadUser->setEnabled(false);
        recentUserGroup->setEnabled(false);
        actionChangePassword->setEnabled(true);
        actionUpsync->setEnabled(true);
        actionLogout->setEnabled(true);
        if (user->canConnectToGit())
            user->createRepoPermissions(gitCom->getUserInfo());
        statusBar()->showMessage(tr("user loaded succesfully"),2000);
        QJsonObject repos = user->getRepos();
        QStringList repoNames = repos.keys();
        std::sort(repoNames.begin(), repoNames.end(), [repos](const QString &a, const QString &b) -> bool { if (a == QString("ELOtemplates")) { return -99999; } if (b == QString("ELOtemplates")) { return 99999; } return repos.value(a).toObject().value("permissions").toInt() > repos.value(b).toObject().value("permissions").toInt();});
        qDebug() << repoNames;
        foreach (QString repoName, repoNames) {
            QDir dir(settings->getWorkingDir() + QDir::separator() + repoName);
            if(dir.exists()) {
                // pull
                statusBar()->showMessage(tr("pulling ...") + repoName);
                if (user->getConnected())
                    gitCom->doThePull(repoName);
            } else {
                // clone
                statusBar()->showMessage(tr("cloning ...") + repoName);
                if (user->getConnected()) {
                    gitCom->doTheClone(repoName);
                } else {
                    QMessageBox::warning(this, tr("could not clone repository"), tr("The repository ") + repoName + tr(" could not be clones due to missing connection to the server."));
                }
            }
            qDebug() << repoName;
            if((repoName != "ELOtemplates") || ((repoName == "ELOtemplates") && (static_cast<permissionMode>(repos.value(repoName).toObject().value("permissions").toInt()) == ReadWrite))) {
                fileView->addView(repoName, settings->getWorkingDir() + QDir::separator() + repoName, static_cast<permissionMode>(repos.value(repoName).toObject().value("permissions").toInt()));
                linkDialog->insertToModel(settings->getWorkingDir() + QDir::separator() + repoName);
            }
        }
    } else {
        statusBar()->showMessage(tr("error loading user"),2000);
        actionLoadUser->setEnabled(true);
        recentUserGroup->setEnabled(true);
        actionChangePassword->setEnabled(false);
        actionUpsync->setEnabled(false);
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

void ELOMainWindow::logoutUser()
{
    actionLoadUser->setEnabled(true);
    recentUserGroup->setEnabled(true);
    actionChangePassword->setEnabled(false);
    actionUpsync->setEnabled(false);
    actionLogout->setEnabled(false);

    fileView->clearViews();
    documentHandler->closeAllDocuments();
    linkDialog->clearModel();

    user->unloadUser();
}

void ELOMainWindow::applyRepoPermissions(permissionMode permissions)
{
    // set file permissions related to the permissions of the shown repo in the fielView (like new file and new folder)
    // the permissions correspons to the opened file are not affected here
    if (permissions == ReadWrite) {
        actionNewFile->setEnabled(true);
    } else {
        actionNewFile->setEnabled(false);
    }
}

void ELOMainWindow::applyFilePermissions(permissionMode permissions)
{
    if (permissions == ReadWrite) {
        actionInsertImage->setEnabled(true);
        actionAddLink->setEnabled(true);
        actionSaveFile->setEnabled(true);
        actionRepoSettings->setEnabled(true);
        QString currentRepoName = documentHandler->getCurrentRepoName();
        repoSettingsDialog->setTitle(tr("ELO repo settings for: ") + currentRepoName);
        QJsonDocument jsonDoc(user->getRepos().value(currentRepoName).toObject().value("settings").toObject());
        repoSettingsDialog->setContent(jsonDoc.toJson(QJsonDocument::Indented));
    } else {
        actionInsertImage->setEnabled(false);
        actionAddLink->setEnabled(false);
        actionSaveFile->setEnabled(false);
        actionRepoSettings->setEnabled(false);
    }
}

void ELOMainWindow::upsync()
{
    if (!user->getServer().isEmpty()) {
        gitCom->show();
        QJsonObject repos = user->getRepos();
        QStringList repoNames = repos.keys();
        user->canConnectToGit();
        foreach(QString repoName, repoNames) {
            if(repos.value(repoName).toObject().value("permissions").toInt() == ReadWrite) {
                gitCom->doTheCommit(QDateTime::currentDateTime().toString(), repoName);
                if (user->getConnected())
                    gitCom->doThePush(repoName);
            }
        }
    }
}

void ELOMainWindow::notConnected()
{
    QMessageBox::warning(this,  tr("No connection to server"), tr("Cannot connect to the server, you're working offline. Be carefull with the upsync of your changes! ELO cannot handle file conflicts(yet)."));
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Image to Insert"), settings->getWorkingDir(), tr("Images (*.png *.jpg *.bmp *.gif *.svg)"));
    if (!fileName.isEmpty()) {
        if (!fileName.startsWith(settings->getWorkingDir())) {
            // copy file into associated files
            fileName = documentHandler->copyToAssociatedFiles(fileName);
        }
        documentHandler->insertImage(fileName);
    }
}

void ELOMainWindow::printToPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select file location"), settings->getWorkingDir(), tr("PDF (*.pdf)"));
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".pdf"))
            fileName += ".pdf";
        documentHandler->printToPDF(fileName);
    }
}
