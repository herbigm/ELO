#ifndef ELOASSOCIATEDFILEVIEW_H
#define ELOASSOCIATEDFILEVIEW_H

#include <QWidget>
#include <QListView>
#include <QFileSystemModel>

#include "elosettings.h"

class ELOAssociatedFileView : public QWidget
{
    Q_OBJECT
public:
    explicit ELOAssociatedFileView(QWidget *parent = nullptr);
    void changeModelRoot(const QString &repo, const QString &fileName);

private:
    QListView *listView;
    QFileSystemModel *fileModel;

    ELOSettings *settings;

    void addFile();
    void removeFile();
    void onFileDblClicked();

signals:
    void insertImageRequest(const QString &path);
    void insertLinkRequest(const QString &text, const QString &path);
};

#endif // ELOASSOCIATEDFILEVIEW_H
