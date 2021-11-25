#ifndef ELOFILEVIEW_H
#define ELOFILEVIEW_H

#include <QTabWidget>
#include <QVector>
#include <QTreeView>
#include <QMenu>
#include <QStandardItem>

class ELOFileView : public QTabWidget
{
    Q_OBJECT
public:
    explicit ELOFileView(QWidget *parent = nullptr);
    void addView(const QString &title, const QString &path);

private:
    QVector<QTreeView *> views;
    QMenu *contextMenu;
    QAction *actionNewFile;
    QAction *actionNewFolder;
    QAction *actionDeleteFile;
    QAction *actionDeleteFolder;
    QAction *actionRenameFolder;
    QAction *actionRenameFile;

    QString contextMenuPath;
    QStandardItem *contextMenuItem;

public slots:
    void addNewFile(const QString filePath);

private slots:
    void onModelSelected(const QModelIndex &index);
    void treeViewContextMenu(const QPoint &point);
    void onModelItemClicked(const QModelIndex &index);
    void requestNewFile();
    void addNewFolder();
    void removeFile();
    void removeFolder();
    void renameFolder();
    void renameFile();

signals:
    void itemSelected(const QString);
    void itemClicked(const QString);
    void newFileRequested(const QString);
    void fileRemoved(const QString);
    void haveRenamed(const QString, const QString);
};

#endif // ELOFILEVIEW_H
