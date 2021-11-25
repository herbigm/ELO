#ifndef ELOFILEMODEL_H
#define ELOFILEMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QApplication>
#include <QStyle>

class ELOFileModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit ELOFileModel(QObject *parent = nullptr);

    void setRootPath(const QString &path);
    bool isReadOnly() { return m_isReadOnly; }
    void setReadOnly(bool b) { m_isReadOnly = b; }
    bool isRootsFirstChild(const QModelIndex &index);
    void addItem(const QString &path, QStandardItem *parent);
    void addNewFile(const QString &path, QStandardItem *parent = nullptr);
    void updateChildsPath(const QString &oldPath, const QString &newPath, QStandardItem *parentItem);
    void deleteItem(QStandardItem *item);

private:
    QStandardItem *rootItem;
    QIcon dirIcon;
    QIcon fileIcon;

    bool m_isReadOnly;

    void getChildNodes(const QString &path, QStandardItem *parentItem);
};

#endif // ELOFILEMODEL_H
