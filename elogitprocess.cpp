#include "elogitprocess.h"

#include <QVBoxLayout>
#include <QPushButton>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include <QCoreApplication>

ELOGitProcess::ELOGitProcess(QWidget *parent): QDialog(parent)
{
    this->setModal(true);
    this->hide();

    // resize
    resize(600,400);
    setWindowTitle(tr("ELO git process log"));

    // load settings
    settings = TheELOSettings::Instance();
    user = ELOUser::Instance();

    // create Widgets
    setLayout(new QVBoxLayout(this));
    textEdit = new QPlainTextEdit(this);
    textEdit->setStyleSheet("background-color: #000000; color: #dddddd; font-family: monospace; ");
    textEdit->setReadOnly(true);
    btn = new QPushButton(tr("close"), this);
    setClose(false);

    layout()->addWidget(textEdit);
    layout()->addWidget(btn);
}

QString ELOGitProcess::getUserInfo()
{
    unsigned long hostaddr;
    int rc, sock, i, auth_pw = 0;
    struct sockaddr_in sin;
    const char *fingerprint;
    char *userauthlist;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;
    char buffer[1024];
    int nbytes;
    QByteArray ba;

    show();
    textEdit->appendPlainText(tr("Start connecting via ssh to server with credentials of ") + user->getName());
    QCoreApplication::processEvents();

#ifdef WIN32
    WSADATA wsadata;
    int err;

    err = WSAStartup(MAKEWORD(2, 0), &wsadata);
    if(err != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", err);
        return QString();
    }
#endif

    hostaddr = inet_addr(user->getServer().toLocal8Bit());
    rc = libssh2_init(0);
    if(rc != 0) {
        fprintf(stderr, "libssh2 initialization failed (%d)\n", rc);
        textEdit->appendPlainText(tr("libssh2 initialization failed\n"));
        QCoreApplication::processEvents();
        return QString();
    }

    /* This code is responsible for creating the socket establishing the connection
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = hostaddr;
    if(::connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
        fprintf(stderr, "failed to connect!\n");
        textEdit->appendPlainText(tr("failed to connect!\n"));
        QCoreApplication::processEvents();
        return QString();
    }

    /* Create a session instance and start it up. This will trade welcome
     * banners, exchange keys, and setup crypto, compression, and MAC layers
     */
    session = libssh2_session_init();
    if(libssh2_session_handshake(session, sock)) {
        fprintf(stderr, "Failure establishing SSH session\n");
        textEdit->appendPlainText(tr("Failure establishing SSH session\n"));
        QCoreApplication::processEvents();
        return QString();
    }

    /* te with the public key. Using user gitolite, like assumed by the gitolite server
     *
     */
    if(libssh2_userauth_publickey_fromfile(session, "gitolite", user->getPubKeyFilePath().toLocal8Bit(), user->getPrivKeyFilePath().toLocal8Bit(), "")) {
        fprintf(stderr, "tAuthentication by public key failed!\n");
        textEdit->appendPlainText(tr("Authentication by public key failed!\n"));
        QCoreApplication::processEvents();
        goto shutdown;
    }
    else {
        fprintf(stderr, "Authentication by public key succeeded.\n");
        textEdit->appendPlainText(tr("Authentication by public key succeeded.\n"));
        QCoreApplication::processEvents();
    }

    channel = libssh2_channel_open_session(session);
    if(!channel) {
        fprintf(stderr, "Unable to open a session.\n");
        textEdit->appendPlainText(tr("Unable to open a session.\n"));
        QCoreApplication::processEvents();
        goto shutdown;
    }

    /* Open a SHELL on that pty */
    if(libssh2_channel_shell(channel)) {
        fprintf(stderr, "Unable to request shell.\n");
        textEdit->appendPlainText(tr("Unable to request shell.\n"));
        QCoreApplication::processEvents();
        goto shutdown;
    }

    // read the data from the channel
    while (!libssh2_channel_eof(channel)) {
        nbytes = libssh2_channel_read(channel, buffer, sizeof(buffer));
        if (nbytes > 0)
            ba.append(buffer);
    }
    textEdit->appendPlainText(ba);

    libssh2_channel_close(channel);

    if(channel) {
        libssh2_channel_free(channel);
        channel = nullptr;
    }

    shutdown:
        libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
        libssh2_session_free(session);

    #ifdef _WIN32
        closesocket(sock);
    #else
        ::close(sock);
    #endif
        fprintf(stderr, "all done!\n");
        libssh2_exit();

    textEdit->appendPlainText(tr("Closing ssh connection to server."));
    QCoreApplication::processEvents();

    return QString(ba);
}

QString ELOGitProcess::doThePull(const QString &repoName)
{
    /*
     * function to implement the git pull command (fetch + merge + checkout)
     */

    show();

    payloadData pd = {user, textEdit};

    textEdit->appendPlainText(tr("pulling repo...") + repoName);
    QCoreApplication::processEvents();

    git_libgit2_init();

    QString path = settings->getWorkingDir() + QDir::separator() + repoName;
    git_repository *repo = nullptr;
    git_remote *remote = nullptr;

    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = &ELOGitProcess::checkoutProgress;
    checkout_opts.progress_payload = &pd;
    checkout_opts.disable_filters = 0;

    fetch_opts.callbacks.transfer_progress = &ELOGitProcess::fetchProgress;
    fetch_opts.callbacks.payload = &pd;
    fetch_opts.callbacks.credentials = &ELOGitProcess::credentialCallback;

    clone_opts.checkout_opts = checkout_opts;
    clone_opts.fetch_opts = fetch_opts;

    // open repo
    int error = git_repository_open(&repo, path.toUtf8()); // pull = fetch + merge + checkout
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // look for remote 'origin'
    error = git_remote_lookup(&remote, repo, "origin");
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // fetch
    error = git_remote_fetch(remote, NULL, &fetch_opts, NULL);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // merge -- should be fast-forward
    git_repository_state_t state;
    git_merge_analysis_t analysis;
    git_merge_preference_t preference;
    git_annotated_commit **annotatedCommit = (git_annotated_commit **) calloc(1, sizeof(git_annotated_commit *));
    git_oid target_oid;
    git_reference *target_ref = nullptr;
    git_reference *new_target_ref = nullptr;
    git_object *target = nullptr;

    // check state of the repo
    state = static_cast<git_repository_state_t>(git_repository_state(repo));
    if (state != GIT_REPOSITORY_STATE_NONE) {
        textEdit->appendPlainText(tr("repository is in unexpected state ") + QString::number(state));
        QCoreApplication::processEvents();
    }

    // getting heads to merge

    error = git_repository_fetchhead_foreach(repo, ELOGitProcess::getFetchHead, &target_oid);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        QCoreApplication::processEvents();
        free(annotatedCommit);
        return QString(giterr_last()->message);
    }

    error = git_annotated_commit_lookup(annotatedCommit, repo, (const git_oid *) &target_oid);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        QCoreApplication::processEvents();
        free(annotatedCommit);
        return QString(giterr_last()->message);
    }

    // anlyse merge possibilitis
    error = git_merge_analysis(&analysis, &preference, repo, (const git_annotated_commit **) annotatedCommit, 1);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        QCoreApplication::processEvents();
        free(annotatedCommit);
        return QString(giterr_last()->message);
    }

    if (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
        textEdit->appendPlainText(tr("Already up-to-date"));
        QCoreApplication::processEvents();
    } else {
      // HEAD exists, just lookup and resolve
      error = git_repository_head(&target_ref, repo);
      if (error != 0) {
          textEdit->appendPlainText(giterr_last()->message);
          QCoreApplication::processEvents();
          free(annotatedCommit);
          return QString(giterr_last()->message);
      }

      // Lookup the target object
      error = git_object_lookup(&target, repo, &target_oid, GIT_OBJECT_COMMIT);
      if (error != 0) {
          textEdit->appendPlainText(giterr_last()->message);
          QCoreApplication::processEvents();
          free(annotatedCommit);
          return QString(giterr_last()->message);
      }

      // Checkout the result so the workdir is in the expected state
      error = git_checkout_tree(repo, target, &checkout_opts);
      if (error != 0) {
          textEdit->appendPlainText(giterr_last()->message);
          QCoreApplication::processEvents();
          free(annotatedCommit);
          return QString(giterr_last()->message);
      }

      // Move the target reference to the target OID
      error = git_reference_set_target(&new_target_ref, target_ref, &target_oid, NULL);
      if (error != 0) {
          textEdit->appendPlainText(giterr_last()->message);
          QCoreApplication::processEvents();
          free(annotatedCommit);
          return QString(giterr_last()->message);
      }
    }

    // clean up
    cleanup:
        git_reference_free(target_ref);
        git_object_free(target);
        git_reference_free(new_target_ref);
        git_remote_free(remote);
        git_repository_free(repo);
        git_libgit2_shutdown();
        free(annotatedCommit);

    textEdit->appendPlainText(tr("pulling repo ") + repoName + tr(" done.") + "\n");
    QCoreApplication::processEvents();

    return QString("pull done.");
}

QString ELOGitProcess::doThePush(const QString &repoName)
{
    show();
    payloadData pd = {user, textEdit};

    textEdit->appendPlainText(tr("pushing repo... ") + repoName);

    git_libgit2_init();

    QString path = settings->getWorkingDir() + QDir::separator() + repoName;
    git_repository *repo = NULL;
    git_remote *remote;

    git_push_options  push_opts = GIT_PUSH_OPTIONS_INIT;
    push_opts.callbacks.credentials = &ELOGitProcess::credentialCallback;
    push_opts.callbacks.transfer_progress = &ELOGitProcess::fetchProgress;
    push_opts.callbacks.payload = &pd;

    // open repo
    int error = git_repository_open(&repo, path.toUtf8());
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git open error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // look for remote 'origin'
    error = git_remote_lookup(&remote, repo, "origin");
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git origin error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // push
    error = git_remote_push(remote, NULL, &push_opts);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git push error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    git_remote_free(remote);
    git_repository_free(repo);
    git_libgit2_shutdown();

    textEdit->appendPlainText(tr("pushing repo ") + repoName + tr(" done.") + "\n");
    QCoreApplication::processEvents();

    return QString("push done.");
}

QString ELOGitProcess::doTheCommit(const QString &msg, const QString &repoName)
{
    show();
    textEdit->appendPlainText(tr("committing repo...") + repoName);

    int count = 0;

    // using libgit2
    git_libgit2_init();

    QString path = settings->getWorkingDir() + QDir::separator() + repoName;

    // open repo
    git_repository *repo = NULL;
    int error = git_repository_open(&repo, path.toUtf8());
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git open error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // create signature
    git_signature *me = NULL;
    error = git_signature_now(&me, user->getName().toUtf8(), user->getEMail().toUtf8());
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git signature error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // open index of repo
    git_index *idx = NULL;
    error = git_repository_index(&idx, repo);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git index error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // prepare git add
    const char *paths[] = {"*"};
    git_strarray arr = {(char **)paths, 1};

    // add files to index
    error = git_index_add_all(idx, &arr, GIT_INDEX_ADD_DEFAULT, &ELOGitProcess::countIndex, &count);
    //error = git_index_add_all(idx, &arr, GIT_INDEX_ADD_DEFAULT, NULL, &count);
    if (error < 0){
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git add error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    if(count > 0){
        // write index to repo
        error = git_index_write(idx);
        if (error < 0) {
            textEdit->appendPlainText(giterr_last()->message);
            QCoreApplication::processEvents();
            return QString(giterr_last()->message);
        }

        // preparing commit
        git_oid parent_id, commit_id, tree_id;
        git_tree *tree;
        git_commit *gitParent;

        // get HEAD id
        error = git_reference_name_to_id(&parent_id, repo, "HEAD");
        if (error < 0) {
            textEdit->appendPlainText(giterr_last()->message);
            textEdit->appendPlainText("git HEAD error");
            QCoreApplication::processEvents();
            return QString(giterr_last()->message);
        }

        // get last commit
        error = git_commit_lookup(&gitParent, repo, &parent_id);
        if (error < 0) {
            textEdit->appendPlainText(giterr_last()->message);
            textEdit->appendPlainText("git last commit error");
            QCoreApplication::processEvents();
            return QString(giterr_last()->message);
        }

        // write a new tree id for the index
        error = git_index_write_tree(&tree_id, idx);
        if (error < 0) {
            textEdit->appendPlainText(giterr_last()->message);
            textEdit->appendPlainText("git new tree error");
            QCoreApplication::processEvents();
            return QString(giterr_last()->message);
        }

        // get the new tree
        error = git_tree_lookup(&tree, repo, &tree_id);
        if (error < 0) {
            textEdit->appendPlainText(giterr_last()->message);
            textEdit->appendPlainText("git get tree error");
            QCoreApplication::processEvents();
            return QString(giterr_last()->message);
        }

        // finally: do the commit
        error = git_commit_create_v(&commit_id, repo, "HEAD", me, me, "UTF-8", msg.toUtf8(), tree, 1, gitParent);
        if (error < 0) {
            textEdit->appendPlainText(giterr_last()->message);
            textEdit->appendPlainText("git commit error");
            QCoreApplication::processEvents();
            return QString(giterr_last()->message);
        }

        // clean up
        git_commit_free(gitParent);
        git_tree_free(tree);
    }

    // clean up
    git_signature_free(me);
    git_index_free(idx);
    git_repository_free(repo);
    git_libgit2_shutdown();

    textEdit->appendPlainText(tr("committing repo ") + repoName + tr(" done.") + "\n");
    QCoreApplication::processEvents();

    return QString("commit done.");
}

QString ELOGitProcess::doInitialCommit(const QString &repoName)
{
    show();
    textEdit->appendPlainText(tr("committing repo...") + repoName);

    int count = 0;

    // using libgit2
    git_libgit2_init();

    QString path = settings->getWorkingDir() + QDir::separator() + repoName;

    // create .ELOconfig file
    QFile f(path + QDir::separator() + ".ELOconfig");
    if (f.open(QIODevice::WriteOnly)) {
        QTextStream out(&f);
        out.setEncoding(QStringConverter::Utf8);
        out << "{" << Qt::endl;
        out << "\"lastExperimentNumber\": 0," << Qt::endl;
        out << "\"prefix\": \"\"," << Qt::endl;
        out << "\"template\": \"AGSi\"" << Qt::endl;
        out << "}" << Qt::endl;
        out.flush();
        f.close();
    }

    // open repo
    git_repository *repo = NULL;
    int error = git_repository_open(&repo, path.toUtf8());
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git open error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // create signature
    git_signature *me = NULL;
    error = git_signature_now(&me, user->getName().toUtf8(), user->getEMail().toUtf8());
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git sigiture error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // open index of repo
    git_index *idx = NULL;
    error = git_repository_index(&idx, repo);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git index error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // prepare git add
    const char *paths[] = {"*"};
    git_strarray arr = {(char **)paths, 1};

    // add files to index
    error = git_index_add_all(idx, &arr, GIT_INDEX_ADD_DEFAULT, &ELOGitProcess::countIndex, &count);
    //error = git_index_add_all(idx, &arr, GIT_INDEX_ADD_DEFAULT, NULL, &count);
    if (error < 0){
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git add error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    error = git_index_write(idx);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // preparing commit
    git_oid commit_id, tree_id;
    git_tree *tree;

    // write a new tree id for the index
    error = git_index_write_tree(&tree_id, idx);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git new tree error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // get the new tree
    error = git_tree_lookup(&tree, repo, &tree_id);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git get tree error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // finally: do the commit
    error = git_commit_create_v(&commit_id, repo, "HEAD", me, me, "UTF-8", "initial commit", tree, 0, NULL);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        textEdit->appendPlainText("git commit error");
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // clean up
    git_tree_free(tree);
    git_signature_free(me);
    git_index_free(idx);
    git_repository_free(repo);
    git_libgit2_shutdown();

    textEdit->appendPlainText(tr("committing repo ") + repoName + tr(" done.") + "\n");
    QCoreApplication::processEvents();

    return QString("initial commit done.");
}

QString ELOGitProcess::doTheClone(const QString &repoName)
{
    show();
    payloadData pd = {user, textEdit};

    textEdit->appendPlainText(tr("cloning repo...") + repoName);
    QCoreApplication::processEvents();

    // using libgit2
    git_libgit2_init();

    QString url = "ssh://gitolite@" + user->getServer() + "/" + repoName;
    QString path = settings->getWorkingDir() + QDir::separator() + repoName;
    git_repository *repo = nullptr;
    git_remote *remote = nullptr;

    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = ELOGitProcess::checkoutProgress;
    checkout_opts.progress_payload = &pd;
    checkout_opts.disable_filters = 0;

    fetch_opts.callbacks.transfer_progress = ELOGitProcess::fetchProgress;
    fetch_opts.callbacks.credentials = ELOGitProcess::credentialCallback;
    fetch_opts.callbacks.payload = &pd;

    clone_opts.checkout_opts = checkout_opts;
    clone_opts.fetch_opts = fetch_opts;

    int error = git_clone(&repo, url.toUtf8(), path.toUtf8(), &clone_opts);
    if (error < 0) {
        textEdit->appendPlainText(giterr_last()->message);
        QCoreApplication::processEvents();
        return QString(giterr_last()->message);
    }

    // set refspecs
    git_remote_add_push(repo, "origin", "refs/heads/master:refs/heads/master");

    git_repository_free(repo);
    git_libgit2_shutdown();

    textEdit->appendPlainText(tr("cloning repo ") + repoName + tr(" done.") + "\n");
    QCoreApplication::processEvents();

    return QString(tr("repo cloned."));
}

void ELOGitProcess::setClose(bool b)
{
    if (b) {
        connect(btn, &QPushButton::clicked, this, &ELOGitProcess::close);
        disconnect(btn, &QPushButton::clicked, this, &ELOGitProcess::hide);
    } else {
        connect(btn, &QPushButton::clicked, this, &ELOGitProcess::hide);
        disconnect(btn, &QPushButton::clicked, this, &ELOGitProcess::close);
    }
}

int ELOGitProcess::credentialCallback(git_cred **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)
{
    Q_UNUSED(url);
    Q_UNUSED(username_from_url);
    Q_UNUSED(allowed_types);

    payloadData *pl = (payloadData *)payload; // sended userdata

    pl->textEdit->appendPlainText(tr("logging in with credentials of ") + pl->user->getName());
    QCoreApplication::processEvents();

    return git_cred_ssh_key_new(out, "gitolite", pl->user->getPubKeyFilePath().toUtf8(), pl->user->getPrivKeyFilePath().toUtf8(), "");
}

int ELOGitProcess::fetchProgress(const git_transfer_progress *stats, void *payload)
{
    payloadData *pl = (payloadData *)payload; // sended userdata

    int fetch_percent = (100 * stats->received_objects) / stats->total_objects;
    int index_percent = (100 * stats->indexed_objects) / stats->total_objects;
    int kbytes = stats->received_bytes / 1024;

    pl->textEdit->appendPlainText("\rnetwork " + QString::number(fetch_percent) + " % (" + QString::number(kbytes) + " kb, " +
                      QString::number(stats->received_objects) + "/" + QString::number(stats->total_objects) + ")  /  index " +
                      QString::number(index_percent) + " % (" + QString::number(stats->indexed_objects) + "/" + QString::number(stats->total_objects) + ")\r");
    QCoreApplication::processEvents();

    return 0;
}

void ELOGitProcess::checkoutProgress(const char *path, size_t cur, size_t tot, void *payload)
{
    payloadData *pl = (payloadData *)payload; // sended userdata

    size_t percent = (100 * cur) / tot;
    pl->textEdit->appendPlainText(QString(path) + "\t" + QString::number(percent) + " %");
    QCoreApplication::processEvents();
}

int ELOGitProcess::getFetchHead(const char *ref_name, const char *remote_url, const git_oid *oid, unsigned int is_merge, void *payload)
{
    git_oid_cpy((git_oid *)payload, oid);
    return 0;
}

int ELOGitProcess::countIndex(const char *path, const char *matched_pathspec, void *payload)
{
    int *i = (int *) payload;
    *i += 1;
    Q_UNUSED(path);
    Q_UNUSED(matched_pathspec);
    return 0;
}
