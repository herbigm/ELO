#include "elofilemodel.h"

#include <QFileInfo>
#include <QDir>

ELOFileModel::ELOFileModel(permissionMode permissions, QObject *parent): QStandardItemModel(parent)
{
    rootItem = this->invisibleRootItem();
    dirIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);      //icon for directories
    fileIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);    //icon for files
    if (permissions == ReadWrite) {
        m_isReadOnly = false;
    } else {
        m_isReadOnly = true;
    }
}

void ELOFileModel::setRootPath(const QString &path)
{
    while (rootItem->hasChildren()) { // delete all items
        delete rootItem->takeChild(0);
    }
    addPath(path);
}

void ELOFileModel::addPath(const QString &path)
{
    QStandardItem* child;
    QFileInfo finfo(path);
    child = new QStandardItem(dirIcon ,finfo.fileName());  //create the parent directory item
    child->setAccessibleDescription(finfo.filePath());     //set actual path to item
    rootItem->appendRow(child);            //add the parent item to root item
    getChildNodes(path, child);          //Iterate and populate the contents
}

void ELOFileModel::removePath(const QString &path)
{
    QFileInfo finfo(path);
    if (rootItem->hasChildren()) {
        for (int i = 0; i < rootItem->rowCount(); i++) {
            if (rootItem->child(i)->accessibleDescription() == finfo.dir().path()) {
                deleteItem(rootItem->child(i));
                return;
            }
        }
    }
}

bool ELOFileModel::isRootsFirstChild(const QModelIndex &index)
{
    if (index == indexFromItem(rootItem->child(0))) {
        return true;
    }
    return false;
}

void ELOFileModel::addItem(const QString &path, QStandardItem *parent)
{
    // create the new Item
    QFileInfo finfo(path);
    QStandardItem *newItem;
    if(finfo.isFile()) {
        if (finfo.fileName().endsWith(".html")) {
            newItem = new QStandardItem(fileIcon, finfo.fileName().mid(0,finfo.fileName().size()-5));                 //Append a file
        } else {
            newItem = new QStandardItem(fileIcon, finfo.fileName());
        }
    } else {
        newItem = new QStandardItem(dirIcon, finfo.fileName()); // Append a folder
    }
    newItem->setAccessibleDescription(finfo.filePath());
    newItem->setAccessibleText(finfo.fileName());

    // insert the new Item at the right position
    QDir dir(parent->accessibleDescription());
    QFileInfoList subFolders = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsFirst);
    for(int i = 0; i < subFolders.length(); i++) {
        if(subFolders[i].fileName() == newItem->accessibleText()) {
            parent->insertRow(i, newItem);
            break;
        }
    }
}

void ELOFileModel::addNewFile(const QString &path, QStandardItem *parent)
{
    QFileInfo finfo(path);
    if (!parent)
        parent = rootItem;
    if (parent->hasChildren()) {
        for (int i = 0; i < parent->rowCount(); i++) {
            if (parent->child(i)->accessibleDescription() == finfo.dir().path()) {
                addItem(path, parent->child(i));
                return;
            }
            if (parent->child(i)->hasChildren()) {
                addNewFile(path, parent->child(i));
            }
        }
    }
}

void ELOFileModel::updateChildsPath(const QString &oldPath, const QString &newPath, QStandardItem *parentItem)
{
    QStandardItem *currentChild;
    for(int r = 0; r < parentItem->rowCount(); ++r) {
        currentChild = parentItem->child(r);
        currentChild->setAccessibleDescription(currentChild->accessibleDescription().replace(oldPath, newPath));
        if (currentChild->hasChildren()) {
            updateChildsPath(oldPath, newPath, currentChild);
        }
    }
}

void ELOFileModel::deleteItem(QStandardItem *item)
{
    QStandardItem *parent = item->parent();
    QList<QStandardItem *> itemlist = parent->takeRow(item->row());
    qDeleteAll(itemlist);
}

QStandardItem *ELOFileModel::getRootsFirstChild()
{
    if (rootItem->hasChildren())
        return rootItem->child(0);
    return nullptr;
}

void ELOFileModel::getChildNodes(const QString &path, QStandardItem *parentItem)
{
    QDir dir(path);
    QFileInfoList subFolders;
    QFileInfo folderName;
    QStandardItem* child;
    subFolders = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsFirst);    //get all the files and sub folders
    foreach (folderName, subFolders) {
        if(folderName.isFile() && !folderName.fileName().startsWith(".")) {             // exclude .-Files (needed for Windows)
            if (folderName.fileName().endsWith(".html")) {
                child = new QStandardItem(fileIcon, folderName.fileName().mid(0,folderName.fileName().size()-5));                 //Append a file
            } else {
                child = new QStandardItem(fileIcon, folderName.fileName());
            }
            child->setAccessibleDescription(folderName.filePath());                     //set actual path to item
        } else if (folderName.isDir() && !folderName.fileName().startsWith(".")) {
            child = new QStandardItem(dirIcon, folderName.fileName());                  //Append a folder
            child->setAccessibleDescription(folderName.filePath());                     //set actual path to item
            getChildNodes(folderName.filePath(), child);                              //Recurse its subdirectories
        } else {
            continue;
        }
        parentItem->appendRow(child);
    }
}
