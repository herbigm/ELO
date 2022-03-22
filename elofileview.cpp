#include "elofileview.h"

#include "elofilemodel.h"
#include <QDir>
#include <qheaderview.h>
#include <QInputDialog>
#include <QMessageBox>

ELOFileView::ELOFileView(QWidget *parent) : QTabWidget(parent)
{
    setTabPosition(QTabWidget::West);
    setTabsClosable(false);

    // contextMenu
    contextMenu = new QMenu(this);
    contextMenu->clear();
    contextMenuPath = QString();
    contextMenuItem = nullptr;

    const QIcon newFileIcon = QIcon::fromTheme("document-new", QIcon(":icons/icons/document-new.svg"));
    actionNewFile = new QAction(newFileIcon, tr("New File"), this);
    connect(actionNewFile, &QAction::triggered, this, &ELOFileView::requestNewFile);

    const QIcon newFolderIcon = QIcon::fromTheme("folder-new", QIcon(":icons/icons/folder-new.svg"));
    actionNewFolder = new QAction(newFolderIcon, tr("New Subfolder"), this);
    connect(actionNewFolder, &QAction::triggered, this, &ELOFileView::addNewFolder);

    const QIcon deleteFileIcon = QIcon::fromTheme("document-delete", QIcon(":icons/icons/document-delete.svg"));
    actionDeleteFile = new QAction(deleteFileIcon, tr("Delete File"), this);
    connect(actionDeleteFile, &QAction::triggered, this, &ELOFileView::removeFile);

    const QIcon deleteFolderIcon = QIcon::fromTheme("folder-delete", QIcon(":icons/icons/folder-delete.svg"));
    actionDeleteFolder = new QAction(deleteFolderIcon, tr("Delete Folder"), this);
    connect(actionDeleteFolder, &QAction::triggered, this, &ELOFileView::removeFolder);

    const QIcon renameFolderIcon = QIcon::fromTheme("folder-rename", QIcon(":icons/icons/rename.svg"));
    actionRenameFolder = new QAction(renameFolderIcon, tr("Rename Folder"), this);
    connect(actionRenameFolder, &QAction::triggered, this, &ELOFileView::renameFolder);

    const QIcon renameFileIcon = QIcon::fromTheme("document-rename", QIcon(":icons/icons/rename.svg"));
    actionRenameFile = new QAction(renameFileIcon, tr("Rename File"), this);
    connect(actionRenameFile, &QAction::triggered, this, &ELOFileView::renameFile);

    connect(this, &QTabWidget::currentChanged, this, &ELOFileView::currentTabChanged);
}

void ELOFileView::addView(const QString &title, const QString &path, permissionMode permissions)
{
    ELOFileModel *model = new ELOFileModel(permissions);
    model->setRootPath(path);
    views.append(new QTreeView());
    views.last()->setEditTriggers(QAbstractItemView::NoEditTriggers);
    views.last()->header()->hide();
    views.last()->setModel(model);
    addTab(views.last(), title);

    views.last()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(views.last(), &QTreeView::customContextMenuRequested, this, &ELOFileView::treeViewContextMenu);
    connect(views.last(), &QTreeView::doubleClicked, this, &ELOFileView::onModelSelected);
    connect(views.last(), &QTreeView::clicked, this, &ELOFileView::onModelItemClicked);
}

void ELOFileView::clearViews()
{
    disconnect(this, &QTabWidget::currentChanged, this, &ELOFileView::currentTabChanged);
    clear();
    qDeleteAll(views);
    views.clear();
}

void ELOFileView::addNewFile(const QString filePath)
{
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    model->addNewFile(filePath);
}

void ELOFileView::onModelSelected(const QModelIndex &index)
{
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    QStandardItem *item = model->itemFromIndex(index);
    QString filePath = item->accessibleDescription();
    QFileInfo finfo(filePath);
    if (finfo.isFile()) {
        emit itemSelected(filePath);
    }
}

void ELOFileView::onModelItemClicked(const QModelIndex &index)
{
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    QStandardItem *item = model->itemFromIndex(index);
    QString filePath = item->accessibleDescription();

    emit itemClicked(filePath);
}

void ELOFileView::requestNewFile()
{
    emit newFileRequested(contextMenuPath);
}

void ELOFileView::addNewFolder()
{
    bool ok;
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    QString folderName = QInputDialog::getText(this, tr("New Folder"), tr("Folder Name"), QLineEdit::Normal, tr("New Folder"), &ok);
    if(ok) {
        QStandardItem *parentItem = model->itemFromIndex(views[currentIndex()]->currentIndex());
        QString currentFolder = parentItem->accessibleDescription();
        QDir dir;
        QString newPath = currentFolder + QDir::separator() + folderName;
        dir.mkpath(newPath);
        model->addItem(newPath, parentItem);
    }
}

void ELOFileView::removeFile()
{
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Deletion confirmation"), tr("Are you sure to delete the file?"));
    if (btn == QMessageBox::Yes) {
        model->deleteItem(contextMenuItem);
        QFile f(contextMenuPath);
        f.remove();
        emit fileRemoved(contextMenuPath);
    }
}

void ELOFileView::removeFolder()
{
    // delete an empty Folder
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    QStandardItem *item = model->itemFromIndex(views[currentIndex()]->currentIndex());
    if(item->hasChildren()) {
        QMessageBox::critical(this, tr("Cannot delete folder"), tr("This folder cannot be removed because it is not empty."));
    } else {
        QDir dir;
        dir.rmdir(model->itemFromIndex(views[currentIndex()]->currentIndex())->accessibleDescription()); // delete in file system
        item->parent()->removeRow(item->row()); // delete from model
    }
}

void ELOFileView::renameFolder()
{
    bool ok;
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    QStandardItem *item = model->itemFromIndex(views[currentIndex()]->currentIndex());
    QString currentFolder = item->accessibleDescription();
    QDir dir(currentFolder);
    QString folderName = QInputDialog::getText(this, tr("Rename Folder"), tr("New Folder Name"), QLineEdit::Normal, dir.dirName(), &ok);
    if(ok) {
        dir.cdUp();
        QString newPath = dir.path() + QDir::separator() + folderName;
        dir.rename(currentFolder, newPath);
        item->setText(folderName);
        item->setAccessibleDescription(newPath);
        model->updateChildsPath(currentFolder, newPath, item);
        emit haveRenamed(currentFolder, newPath);
    }
}

void ELOFileView::renameFile()
{
    bool ok;
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    QStandardItem *item = model->itemFromIndex(views[currentIndex()]->currentIndex());
    QString currentFile = item->accessibleDescription();
    QFileInfo finfo(currentFile);
    QDir dir(finfo.dir());
    QString newName;
    if (finfo.fileName().endsWith(".html")) {
        newName = QInputDialog::getText(this, tr("Rename File"), tr("New File Name"), QLineEdit::Normal, finfo.fileName().left(finfo.fileName().length()-5), &ok);
    } else {
        newName = QInputDialog::getText(this, tr("Rename File"), tr("New File Name"), QLineEdit::Normal, finfo.fileName(), &ok);
    }
    if(ok) {
        //Update File System and FileModel
        if (finfo.fileName().endsWith(".html")) {
            dir.rename(currentFile, newName + ".html");
            item->setAccessibleDescription(dir.path() + QDir::separator() + newName + ".html");
            emit haveRenamed(currentFile, dir.path() + QDir::separator() + newName + ".html");
        } else {
            dir.rename(currentFile, newName);
            item->setAccessibleDescription(dir.path() + QDir::separator() + newName);
            emit haveRenamed(currentFile, dir.path() + QDir::separator() + newName);
        }
        item->setText(newName);
    }
}

void ELOFileView::currentTabChanged(int index)
{
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[index]->model());
    QStandardItem *item = model->itemFromIndex(views[index]->currentIndex());
    if (!item) // if no item was selected, try to get the roots first child item
        item = model->getRootsFirstChild();
    if (item) {
        QString filePath = item->accessibleDescription();
        emit itemClicked(filePath);
    }
}

void ELOFileView::treeViewContextMenu(const QPoint &point)
{
    QModelIndex index = views[currentIndex()]->indexAt(point);
    ELOFileModel *model = qobject_cast<ELOFileModel *>(views[currentIndex()]->model());
    if (index.isValid()) {
        contextMenu->clear();
        QStandardItem *item = model->itemFromIndex(index);
        QString filePath = item->accessibleDescription();
        contextMenuItem = item;
        contextMenuPath = filePath;
        emit itemClicked(filePath);
        if(!model->isReadOnly()) {
            QFileInfo finfo(filePath);
            if(finfo.isDir()) {
                contextMenu->addAction(actionNewFile);
                contextMenu->addAction(actionNewFolder);
                contextMenu->addSeparator();
                if (!model->isRootsFirstChild(index)) {
                    contextMenu->addAction(actionRenameFolder);
                    contextMenu->addSeparator();
                    contextMenu->addAction(actionDeleteFolder);
                }
            } else {
                contextMenu->addAction(actionDeleteFile);
                contextMenu->addSeparator();
                contextMenu->addAction(actionRenameFile);
            }
        } else {
            contextMenu->addAction(tr("No actions avaible"));
        }
        contextMenu->exec(views[currentIndex()]->viewport()->mapToGlobal(point));
    }
}
