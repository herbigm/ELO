#ifndef ELOGITPROCESS_H
#define ELOGITPROCESS_H

#include <QDialog>
#include <QPlainTextEdit>

#include <libssh2.h>
#include <git2.h>

#include "elosettings.h"
#include "elouser.h"

typedef struct{
    ELOUser *user;
    QPlainTextEdit *textEdit;
} payloadData;


class ELOGitProcess : public QDialog
{
    Q_OBJECT
public:
    ELOGitProcess(QWidget *parent = nullptr);

    QString getUserInfo();
    QString doThePull(const QString &repoName);
    QString doThePush(const QString &repoName);
    QString doTheCommit(const QString &msg, const QString &repoName);
    QString doInitialCommit(const QString &repoName);
    QString doTheClone(const QString &repoName);

public slots:
    void setClose(bool b);

private:
    ELOSettings *settings;
    QPlainTextEdit *textEdit;
    ELOUser *user;
    QPushButton *btn;

    static int credentialCallback(git_cred **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload);
    static int fetchProgress(const git_transfer_progress *stats, void *payload);
    static void checkoutProgress(const char *path, size_t cur, size_t tot, void *payload);
    static int getFetchHead(const char *ref_name, const char *remote_url, const git_oid *oid, unsigned int is_merge, void *payload);
    static int countIndex(const char *path, const char *matched_pathspec, void *payload);
};

#endif // ELOGITPROCESS_H
