#include "elolinkdialog.h"

#include <QTabWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>

#include "elofilemodel.h"

ELOLinkDialog::ELOLinkDialog(QWidget *parent):
    QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint )
{
    this->setModal(true);
    this->hide();

    // resize
    resize(600,400);
    setWindowTitle(tr("ELO metadata"));

    // load settings
    settings = TheELOSettings::Instance();

    // create Widgets
    // ButtonBox
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout = new QFormLayout(this);
    linkTextEdit = new QLineEdit(this);
    urlEdit = new QLineEdit(this);
    externalPathEdit = new QLineEdit(this);
    externalPathEdit->setReadOnly(true);
    shortcutComboBox = new QComboBox(this);
    showShortcuts();
    internalView = new QTreeView(this);
    internalView->setModel(new ELOFileModel(ReadOnly, this));
    internalView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabWidget = new QTabWidget(this);
    QWidget *internalLinkWidget = new QWidget(this);
    QWidget *externalLinkWidget = new QWidget(this);
    QWidget *external2 = new QWidget(this);
    QWidget *webLinkWidget = new QWidget(this);
    const QIcon buttonIconOpenFile = QIcon::fromTheme("document-open", QIcon(":icons/icons/document-open.svg"));
    const QIcon buttonIconOpenFolder = QIcon::fromTheme("folder", QIcon(":icons/icons/openFolder.svg"));
    QPushButton *selectFileButton = new QPushButton(buttonIconOpenFile, tr("select a file"), this);
    selectFileButton->setToolTip(tr("select a file"));
    connect(selectFileButton, &QPushButton::pressed, this, &ELOLinkDialog::openFileFromShotcut);
    QPushButton *selectDirectoryButton = new QPushButton(buttonIconOpenFolder, tr("select a directory"), this);
    selectDirectoryButton->setToolTip(tr("select a directory"));
    connect(selectDirectoryButton, &QPushButton::pressed, this, &ELOLinkDialog::openDirectoryFromShortcut);

    external2->setLayout(new QHBoxLayout());
    external2->layout()->addWidget(externalPathEdit);
    external2->layout()->addWidget(selectFileButton);
    external2->layout()->addWidget(selectDirectoryButton);

    internalLinkWidget->setLayout(new QVBoxLayout());
    internalLinkWidget->layout()->addWidget(new QLabel(tr("Internal links are links to files inside the accessible lab book.\nInternal files are syncronized with the server. \nHuge amounts of internal files may result in prolonged loading times at login and synchronisation. ")));
    internalLinkWidget->layout()->addWidget(internalView);
    qobject_cast<QVBoxLayout *>(internalLinkWidget->layout())->addStretch();

    externalLinkWidget->setLayout(new QFormLayout());
    qobject_cast<QFormLayout *>(externalLinkWidget->layout())->addRow(new QLabel(tr("External links are links to files outside the accessible lab book.\nExternal files are not synchronized with the server!\nYou may use shortcuts to access these files.")));
    qobject_cast<QFormLayout *>(externalLinkWidget->layout())->addRow(tr("Shortcut: "), shortcutComboBox);
    qobject_cast<QFormLayout *>(externalLinkWidget->layout())->addRow(tr("File/Directory: "), external2);

    webLinkWidget->setLayout(new QFormLayout());
    qobject_cast<QFormLayout *>(webLinkWidget->layout())->addRow(new QLabel(tr("Web links are links to webpages. Insert the right url.")));
    qobject_cast<QFormLayout *>(webLinkWidget->layout())->addRow(tr("Url: "), urlEdit);

    tabWidget->addTab(internalLinkWidget, tr("internal link"));
    tabWidget->addTab(externalLinkWidget, tr("external link"));
    tabWidget->addTab(webLinkWidget, tr("web link"));

    mainLayout->addRow(tr("Link text"), linkTextEdit);
    mainLayout->addRow(tabWidget);
    mainLayout->addRow(buttonBox);
}

void ELOLinkDialog::showShortcuts()
{
    QVector<PathShortcut> pathShortcuts = settings->getPathShortCuts();
    for (int i = 0; i < pathShortcuts.length(); i++) {
        shortcutComboBox->addItem(pathShortcuts[i].Name);
    }
}

void ELOLinkDialog::insertToModel(const QString &path)
{
    qobject_cast<ELOFileModel *>(internalView->model())->addPath(path);
}

void ELOLinkDialog::clearModel()
{
    qobject_cast<ELOFileModel *>(internalView->model())->clear();
}

void ELOLinkDialog::accept()
{
    // there have to be a link text!
    QString linkText = linkTextEdit->text();
    if (linkText.isEmpty()) {
        QMessageBox::warning(this, tr("No link text"), tr("Please fill in a link text."));
        return;
    }
    if (tabWidget->currentIndex() == 0) { // internal link
        QModelIndex index = internalView->currentIndex();
        QString pathText = qobject_cast<ELOFileModel *>(internalView->model())->itemFromIndex(index)->accessibleDescription();
        QString proceededPath;
        pathText = pathText.replace(settings->getWorkingDir(), "");
        if (pathText.startsWith("/"))
            pathText = pathText.mid(1);
        proceededPath = "file-getter.php?path=" + pathText + "&internal=true";
        // emit to insert link!
        emit setLinkRequest(linkText, proceededPath);
    } else if (tabWidget->currentIndex() == 1) { // external link
        QString pathText = externalPathEdit->text();
        QString proceededPath;
        if (pathText.isEmpty()) {
            QMessageBox::warning(this, tr("No path given"), tr("Please choose a path as link destination."));
            return;
        }
        // replace shortcuts
        QVector<PathShortcut> pathShortcuts = settings->getPathShortCuts();
        for (int i = 0; i < pathShortcuts.length(); i++) {
            if (pathText.startsWith(pathShortcuts[i].Path)) {
                pathText = pathText.replace(pathShortcuts[i].Path, pathShortcuts[i].Name);
                if (pathText.startsWith("/"))
                    pathText = pathText.mid(1);
                proceededPath = "file-getter.php?path=" + pathText + "&external=true";
                break;
            }
        }
        if (proceededPath.isEmpty()) {
            // the path is NOT inside any shortcut directory
            // maybe is is an internal link?
            if (pathText.startsWith(settings->getWorkingDir())) {
                pathText = pathText.replace(settings->getWorkingDir(), "");
                if (pathText.startsWith("/"))
                    pathText = pathText.mid(1);
                proceededPath = "file-getter.php?path=" + pathText + "&internal=true";
                QMessageBox::information(this, tr("Found internal link"), tr("The given path is a internal path and will be handled as so."));
            }
        }
        if (proceededPath.isEmpty()) {
            // it is an external path. Ask user to continue
            QMessageBox::StandardButton btn = QMessageBox::question(this, tr("External file"), tr("The given path is neither in the working directory nor in any yet set shortcut directory. Such links are not reliable on different systems. Would you like to continue?"));
            if (btn == QMessageBox::Yes) {
                proceededPath = "file-getter.php?path=" + pathText + "&external=true";
            } else {
                return;
            }
        }
        // emit to insert link
        emit setLinkRequest(linkText, proceededPath);

    } else if (tabWidget->currentIndex() == 2) { // web link
        QString urlText = urlEdit->text();
        if (urlText.isEmpty()) {
            QMessageBox::warning(this, tr("No url given"), tr("Please fill in an url."));
            return;
        }
        // emit to insert link!
        emit setLinkRequest(linkText, urlText);
    }

    // clear
    linkTextEdit->clear();
    urlEdit->clear();
    externalPathEdit->clear();

    hide();
}

void ELOLinkDialog::reject()
{
    // clear
    linkTextEdit->clear();
    urlEdit->clear();
    externalPathEdit->clear();

    hide();
}

void ELOLinkDialog::openFileFromShotcut()
{
    QVector<PathShortcut> pathShortcuts = settings->getPathShortCuts();
    QString newPath = QFileDialog::getOpenFileName(this, tr("Choose a file"), pathShortcuts[shortcutComboBox->currentIndex()].Path);
    if (!newPath.isEmpty()) {
        externalPathEdit->setText(newPath);
    }
}

void ELOLinkDialog::openDirectoryFromShortcut()
{
    QVector<PathShortcut> pathShortcuts = settings->getPathShortCuts();
    QString newPath = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), pathShortcuts[shortcutComboBox->currentIndex()].Path);
    if (!newPath.isEmpty()) {
        externalPathEdit->setText(newPath);
    }
}
