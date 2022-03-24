#include "eloassociatedfileview.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QDesktopServices>

ELOAssociatedFileView::ELOAssociatedFileView(QWidget *parent) : QWidget(parent)
{
    settings = TheELOSettings::Instance();

    setLayout(new QVBoxLayout());
    listView = new QListView(this);
    connect(listView, &QListView::doubleClicked, this, &ELOAssociatedFileView::onFileDblClicked);
    fileModel = new QFileSystemModel(this);
    layout()->addWidget(listView);

    QWidget *btnBox = new QWidget(this);
    const QIcon addButtonIcon = QIcon::fromTheme("list-add", QIcon(":icons/icons/list-add.svg"));
    const QIcon removeButtonIcon = QIcon::fromTheme("list-add", QIcon(":icons/icons/list-remove.svg"));
    QPushButton *addButton = new QPushButton(addButtonIcon, tr("Add associated file"), this);
    connect(addButton, &QPushButton::pressed, this, &ELOAssociatedFileView::addFile);
    QPushButton *removeButton = new QPushButton(removeButtonIcon, tr("Remove selected file"), this);
    connect(removeButton, &QPushButton::pressed, this, &ELOAssociatedFileView::removeFile);
    btnBox->setLayout(new QHBoxLayout());
    btnBox->layout()->addWidget(addButton);
    btnBox->layout()->addWidget(removeButton);

    layout()->addWidget(btnBox);
}

void ELOAssociatedFileView::changeModelRoot(const QString &repo, const QString &fileName)
{
    QDir dir(settings->getWorkingDir() + QDir::separator() + repo + QDir::separator() + ".AssociatedFiles" + QDir::separator() + fileName);
    if(!dir.exists()) {
        dir.mkpath(dir.path());
    }
    QFileSystemModel *oldModel = fileModel;
    fileModel = new QFileSystemModel(this);
    oldModel->deleteLater();
    listView->setModel(fileModel);
    listView->setRootIndex(fileModel->setRootPath(dir.path()));
}

void ELOAssociatedFileView::addFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), QDir::homePath());
    if (!fileName.isEmpty()) {
        // copy file into directory
        QFile oldFile(fileName);
        QFileInfo finfo(oldFile);
        QFileInfo toSaveFile(fileModel->rootPath() + QDir::separator() + finfo.fileName());
        int i = 1;
        while(toSaveFile.exists()) {
            toSaveFile.setFile(toSaveFile.filePath() + QDir::separator() + QChar(i) + "_" + finfo.fileName());
            i++;
        }
        oldFile.copy(toSaveFile.filePath());
    }
}

void ELOAssociatedFileView::removeFile()
{
    QFile f(fileModel->filePath(listView->currentIndex()));
    QFileInfo finfo(f);
    QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Confirm deletion"), tr(QString("Do you really want to delete %1?").arg(finfo.fileName()).toUtf8()));
    if (btn == QMessageBox::Yes) {
        f.remove();
    }
}

void ELOAssociatedFileView::onFileDblClicked()
{
    QString filePath = fileModel->filePath(listView->currentIndex());
    QFileInfo finfo(filePath);

    // maybe it's an image?!
    QMimeDatabase mimeDB;
    QMimeType mime = mimeDB.mimeTypeForFile(filePath);

    // ask what to do with the file
    QMessageBox msgBox;
    msgBox.setText(tr("What do you like to do with the selected file?"));
    msgBox.setIcon(QMessageBox::Question);
    QPushButton *openButton = msgBox.addButton(tr("Open File"), QMessageBox::ActionRole);
    QPushButton *insertLinkButton = msgBox.addButton(tr("Insert Link to this File"), QMessageBox::ActionRole);
    QPushButton *insertImageButton = msgBox.addButton(tr("Insert File as Image"), QMessageBox::ActionRole);
    insertImageButton->setEnabled(false);
    if(mime.preferredSuffix() == "jpeg" || mime.preferredSuffix() == "jpg" || mime.preferredSuffix() == "gif" || mime.preferredSuffix() == "png" || mime.preferredSuffix() == "svg") {
        insertImageButton->setEnabled(true);
    }
    QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
    msgBox.setDefaultButton(abortButton);

    msgBox.exec();

    if (msgBox.clickedButton() == openButton) {
        // open the file with system defaults
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    } else if (msgBox.clickedButton() == insertLinkButton) {
        emit insertLinkRequest(finfo.fileName(), "file-getter.php?path=" + QDir(settings->getWorkingDir()).relativeFilePath(filePath) + "&internal=true");
    } else if (msgBox.clickedButton() == insertImageButton) {
        emit insertImageRequest(finfo.filePath());
    }
}
