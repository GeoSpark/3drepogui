/**
 *  Copyright (C) 2014 3D Repo Ltd
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//------------------------------------------------------------------------------
// Qt
#include <QMessageBox>

//------------------------------------------------------------------------------
// Core
#include <RepoGraphHistory>
#include <RepoLogger>

//------------------------------------------------------------------------------
// GUI
#include "repogui.h"
#include "ui_repogui.h"
#include "widgets/repo_widgetrepository.h"
#include "widgets/repo_textbrowser.h"
#include "workers/repo_workercommit.h"
#include "dialogs/repo_dialogcommit.h"
#include "dialogs/repo_dialogconnect.h"
#include "dialogs/repo_dialoghistory.h"
#include "dialogs/repodialogoculus.h"
#include "dialogs/repodialogusermanager.h"
#include "primitives/repo_fontawesome.h"
#include "oculus/repo_oculus.h"



//------------------------------------------------------------------------------

const QString repo::gui::RepoGUI::REPO_SETTINGS_GUI_GEOMETRY    = "RepoGUI/geometry";
const QString repo::gui::RepoGUI::REPO_SETTINGS_GUI_STATE       = "RepoGUI/state";
const QString repo::gui::RepoGUI::REPO_SETTINGS_LINK_WINDOWS    = "RepoGUI/link";

repo::gui::RepoGUI::RepoGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RepoGUI)
{
    ui->setupUi(this);
    restoreSettings();

    core::RepoLogger::instance().addListener(ui->logTextBrowser);

    this->setWindowIcon(
                RepoFontAwesome::getInstance().getIcon(
                            RepoFontAwesome::fa_database,
                            QColor(246, 101, 60)));

    //--------------------------------------------------------------------------
    // For docks and windows not to update as they are slow to repaint.
    this->setAnimated(false);

    //--------------------------------------------------------------------------
    // Force opengl format settings by default.
    QGLFormat format(QGL::SampleBuffers);
    format.setSwapInterval(1); // vsync
    format.setSamples(16); // Antialiasing (multisample)
    QGLFormat::setDefaultFormat(format);

    //--------------------------------------------------------------------------
    //
    // File
    //
    //--------------------------------------------------------------------------
    // Open
    QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    ui->actionOpen->setIcon(repo::gui::RepoFontAwesome::getInstance().getIcon(RepoFontAwesome::fa_folder_open));

    // Save As
    QObject::connect(ui->actionSave_As, SIGNAL(triggered()), this,
                     SLOT(saveAs()));
    ui->actionSave_As->setIcon(repo::gui::RepoFontAwesome::getInstance().getIcon(RepoFontAwesome::fa_floppy_o));


    //--------------------------------------------------------------------------
    // Exit
    QObject::connect(ui->actionExit, SIGNAL(triggered()),
                     QApplication::instance(), SLOT(quit()));
    ui->actionExit->setIcon(
                RepoFontAwesome::getInstance().getIcon(
                    RepoFontAwesome::fa_sign_out, QColor(Qt::darkRed)));


    //--------------------------------------------------------------------------
    //
    // Repository
    //
    //--------------------------------------------------------------------------
    // Connect
    QObject::connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(connect()));
    ui->actionConnect->setIcon(RepoDialogConnect::getIcon());

    // Refresh
    QObject::connect(ui->actionRefresh, SIGNAL(triggered()), this, SLOT(refresh()));
    ui->actionRefresh->setIcon(
                RepoFontAwesome::getInstance().getIcon(
                    RepoFontAwesome::fa_refresh,
                    QColor(Qt::darkGreen)));

    // Head
    QObject::connect(ui->actionHead, SIGNAL(triggered()), this, SLOT(fetchHead()));
    ui->actionHead->setIcon(
                RepoFontAwesome::getInstance().getIcon(
                    RepoFontAwesome::fa_download));

    // History
    QObject::connect(ui->actionHistory, SIGNAL(triggered()), this, SLOT(history()));
    ui->actionHistory->setIcon(RepoDialogHistory::getIcon());


    // Commit
    QObject::connect(ui->actionCommit, SIGNAL(triggered()), this, SLOT(commit()));
    ui->actionCommit->setIcon(RepoDialogCommit::getIcon());

    //--------------------------------------------------------------------------
    // Drop
    QObject::connect(ui->actionDrop, SIGNAL(triggered()), this, SLOT(dropDatabase()));
    ui->actionDrop->setIcon(RepoFontAwesome::getInstance().getIcon(RepoFontAwesome::fa_trash_o));




    //--------------------------------------------------------------------------
    //
    // Rendering
    //
    //--------------------------------------------------------------------------
    // Link
    QObject::connect(ui->actionLink, SIGNAL(triggered(bool)), ui->mdiArea, SLOT(chainSubWindows(bool)));
    ui->actionLink->setIcon(
                RepoFontAwesome::getInstance().getIcon(
                    RepoFontAwesome::fa_link,
                    RepoFontAwesome::fa_chain_broken));


    QObject::connect(ui->actionOculus, SIGNAL(triggered()), this, SLOT(oculus()));
    ui->actionOculus->setIcon(
                RepoFontAwesome::getInstance().getIcon(
                    RepoFontAwesome::fa_eye));

    //--------------------------------------------------------------------------
    //
    // Tools
    //
    //--------------------------------------------------------------------------
    // User Management...
    QObject::connect(ui->actionUserManager, SIGNAL(triggered()), this, SLOT(openUserManager()));
    ui->actionUserManager->setIcon(RepoDialogUserManager::getIcon());



    //--------------------------------------------------------------------------
    //
    // Window
    //
    //--------------------------------------------------------------------------
    // Full Screen
    QObject::connect(ui->actionFull_Screen,
                     SIGNAL(triggered()),
                     this, SLOT(toggleFullScreen()));
    ui->actionFull_Screen->setIcon(
                RepoFontAwesome::getInstance().getIcon(
                    RepoFontAwesome::fa_arrows_alt));

    // Panels
    QMenu *menuPanels = QMainWindow::createPopupMenu();
    menuPanels->setTitle(QString("Panels"));
    menuPanels->setIcon(RepoFontAwesome::getInstance().getIcon(RepoFontAwesome::fa_columns));
    ui->menuWindow->addMenu(menuPanels);


    //--------------------------------------------------------------------------
    //
    // Help
    //
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    // Email Technical Support
    QObject::connect(ui->actionEmail_Technical_Support, SIGNAL(triggered()),
                    this, SLOT(openSupportEmail()));
    ui->actionEmail_Technical_Support->setIcon(
                RepoFontAwesome::getInstance().getIcon(
                    RepoFontAwesome::fa_envelope_o));

    //--------------------------------------------------------------------------
    // Report Issue
    QObject::connect(ui->actionReport_Issue, SIGNAL(triggered()),
                    this, SLOT(reportIssue()));
    ui->actionReport_Issue->setIcon(
                RepoFontAwesome::getInstance().getIcon(
                    RepoFontAwesome::fa_globe));






    //--------------------------------------------------------------------------
    // Context menus
     QObject::connect(
        ui->widgetRepository->getDatabasesTreeView(),
        &QWidget::customContextMenuRequested,
        this,
        &RepoGUI::showDatabaseContextMenu);

    QObject::connect(
        ui->widgetRepository->getCollectionTreeView(),
        &QTreeView::customContextMenuRequested,
        this,
        &RepoGUI::showCollectionContextMenuSlot);
}

repo::gui::RepoGUI::~RepoGUI()
{
    delete ui;
}


//------------------------------------------------------------------------------
//
// Public
//
//------------------------------------------------------------------------------

void repo::gui::RepoGUI::commit()
{
    RepoMdiSubWindow *activeWindow = ui->mdiArea->activeSubWindow();
    const RepoGLCWidget *widget = getActiveWidget();

    if (activeWindow && widget)
    {
        const core::RepoGraphScene *repoScene = widget->getRepoScene();
        // TODO: fix !!!
        std::cerr << "TEMPORARY COMMIT ONLY" << std::endl;

        // MongoDB cannot have dots in database names, hence replace with underscores
        // (and remove file extension if any)
        // http://docs.mongodb.org/manual/reference/limits/#naming-restrictions
        QFileInfo path(activeWindow->windowTitle());
        QString dbName = path.completeBaseName();
        //dbName = dbName.mid(0, dbName.indexOf("_"));
        dbName.replace(".", "_");
        repo::core::RepoGraphHistory *history = new repo::core::RepoGraphHistory();

        core::MongoClientWrapper mongo = ui->widgetRepository->getSelectedConnection();
        std::string username = mongo.getUsername(dbName.toStdString());
        username = username.empty() ? "anonymous" : username;

        core::RepoNodeRevision *revision = new core::RepoNodeRevision(username);
        revision->setCurrentUniqueIDs(repoScene->getUniqueIDs());
        history->setCommitRevision(revision);

        // http://docs.mongodb.org/manual/reference/connection-string/
        repo::gui::RepoDialogCommit commitDialog(
            QString::fromStdString(username + "@" + mongo.getHostAndPort()),
            dbName,
            "master {00000000-0000-0000-0000-000000000000}", // TODO: get currently active branch from QSettings
            repoScene,
            revision,
            this,
            Qt::Window);
        commitDialog.setWindowTitle(commitDialog.windowTitle() + " " + dbName);

        if(!commitDialog.exec())
            std::cout << "Commit dialog cancelled by user" << std::endl;
        else // Clicked "OK"
        {
            // TODO: move this to the commit dialog.
            std::cout << tr("Uploading, please wait...").toStdString() << std::endl;
            //------------------------------------------------------------------
            // Establish and connect the new worker.
            RepoWorkerCommit *worker = new RepoWorkerCommit(
                        mongo,
                        dbName,
                        history,
                        widget->getRepoScene());

            QObject::connect(worker, SIGNAL(progress(int, int)), activeWindow, SLOT(progress(int, int)));
            QObject::connect(worker, SIGNAL(finished()), this, SLOT(refresh()));
            //connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));

            //------------------------------------------------------------------
            // Fire up the asynchronous calculation.
            QThreadPool::globalInstance()->start(worker);
        }
    }
}

void repo::gui::RepoGUI::connect()
{
    RepoDialogConnect connectionDialog(this);

    if(!connectionDialog.exec()) // if not clicked "Connect"
    {
        std::cout<< "Connection dialog cancelled by user" << std::endl;
    }
    else
    {
        // TODO move mongo creation outside of this main GUI to repository widget
        // or similar
        core::MongoClientWrapper mongo;

        //----------------------------------------------------------------------
        // if not successfully connected
        if (!mongo.connect(
                    connectionDialog.getHost().toStdString(),
                    connectionDialog.getPort()))
        {
            std::cerr << "Connection error" << std::endl;
        }
        else
        {
            if (!connectionDialog.getUsername().isEmpty())
            {
                mongo.authenticate(
                    connectionDialog.getUsername().toStdString(),
                    connectionDialog.getPassword().toStdString());
            }
            ui->widgetRepository->fetchDatabases(mongo);

            //-----------------------------------------------------------------
            // enable buttons
            ui->actionRefresh->setEnabled(true);
            ui->actionHead->setEnabled(true);
            ui->actionHistory->setEnabled(true);
            ui->actionCommit->setEnabled(true);
            ui->actionDrop->setEnabled(true);
        }
    }
}

void repo::gui::RepoGUI::dropDatabase()
{
    QString dbName = ui->widgetRepository->getSelectedDatabase();
    if (dbName.isNull() || dbName.isEmpty())
        std::cout << "A database must be selected." << std::endl;
    else if (dbName == "local" || dbName == "admin")
        std::cout << "You are not allowed to delete 'local' or 'admin' databases." << std::endl;
    else
    {
        switch (QMessageBox::warning(this,
            "Drop Database?",
            "Are you sure you want to drop '" + dbName + "' repository?",
            "&Yes",
            "&No",
            QString::null, 1, 1))
        {
            case 0: // yes

                // TODO: create a DB manager separate from repositories widget.
                core::MongoClientWrapper mongo = ui->widgetRepository->getSelectedConnection();
                mongo.reconnectAndReauthenticate();
                if (mongo.dropDatabase(dbName.toStdString()))
                {
                    std::cout << dbName.toStdString() << " deleted successfully."
                                 << std::endl;
                }
                else
                   std::cout << "Delete unsuccessful" << std::endl;
                refresh();
                break;
            }
    }
}

void repo::gui::RepoGUI::fetchHead()
{
    QString database = ui->widgetRepository->getSelectedDatabase();
    ui->mdiArea->addSubWindow(
                ui->widgetRepository->getSelectedConnection(),
                database); // head revision from master branch

    // make sure to hook controls if chain is on
    ui->mdiArea->chainSubWindows(ui->actionLink->isChecked());
}

const repo::gui::RepoGLCWidget * repo::gui::RepoGUI::getActiveWidget()
{
    RepoGLCWidget *widget = ui->mdiArea->activeSubWidget<repo::gui::RepoGLCWidget *>();
    if (!widget)
        std::cerr << "A 3D window has to be open." << std::endl;
    return widget;
}

const repo::core::RepoGraphScene *repo::gui::RepoGUI::getActiveScene()
{
    const core::RepoGraphScene *scene = 0;
    if (const RepoGLCWidget *widget = getActiveWidget())
        scene = widget->getRepoScene();
    return scene;
}

void repo::gui::RepoGUI::history()
{
    QString database = ui->widgetRepository->getSelectedDatabase();
    core::MongoClientWrapper mongo =
            ui->widgetRepository->getSelectedConnection();
    RepoDialogHistory historyDialog(mongo, database, this);

    if(!historyDialog.exec()) // if not OK
        std::cout << "Revision History dialog cancelled by user." << std::endl;
    else
    {
        QList<QUuid> revisions = historyDialog.getSelectedRevisions();
        for (QList<QUuid>::iterator uid = revisions.begin();
             uid != revisions.end();
             ++uid)
            ui->mdiArea->addSubWindow(mongo, database, *uid, false);
        //---------------------------------------------------------------------
        // make sure to hook controls if chain is on
        ui->mdiArea->chainSubWindows(ui->actionLink->isChecked());
    }
}

void repo::gui::RepoGUI::loadFile(const QString &filePath)
{
    if (!filePath.isEmpty())
    {
        QFileInfo pathInfo(filePath);
        string fileName = pathInfo.fileName().toStdString();
        std::cout << "Loading " << fileName << " ..." << std::endl;

        // TODO: get assimp post processing flags from settings dialog box.
        unsigned int postProcessingFlags = 0;
        ui->mdiArea->addSubWindow(filePath, postProcessingFlags);
        ui->mdiArea->chainSubWindows(ui->actionLink->isChecked());
    }
}

void repo::gui::RepoGUI::loadFiles(const QStringList &filePaths)
{
    QStringList::ConstIterator it = filePaths.begin();
    while(it != filePaths.end())
    {
        loadFile(*it);
        ++it;
    }
}

void repo::gui::RepoGUI::loadFiles(const QList<QUrl> &urls)
{
    QList<QUrl>::ConstIterator it = urls.begin();
    while(it != urls.end())
    {
        QUrl url = *it;
        loadFile(url.toLocalFile());
        ++it;
    }
}


void repo::gui::RepoGUI::oculus()
{

    RepoGLCWidget *activeSubWidget = ui->mdiArea->activeSubWidget<RepoGLCWidget*>();
    if (!activeSubWidget)
        std::cout << "A 3D window has to be open." << std::endl;
    else
    {
        RepoDialogOculus oculusDialog(activeSubWidget, this);
        oculusDialog.exec();
        //ui->mdiArea->activeSubWindowToOculus();
    }

}

void repo::gui::RepoGUI::openFile()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        tr("Select one or more files to open"),
        QString::null,
        repo::core::AssimpWrapper::getImportFormats().c_str());
    loadFiles(filePaths);
}

void repo::gui::RepoGUI::openSupportEmail() const
{
    QString email = "support@3drepo.org";
    QString subject = "GUI Support Request";

    // TODO: get system state printout directly from About dialog.
    QString body;
    body += QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion();
    body += "\n";
    body += QString("Qt ") + QT_VERSION_STR;
    body += "\n";
    body += "OpenGL " + QString::number(QGLFormat::defaultFormat().majorVersion());
    body += "." + QString::number(QGLFormat::defaultFormat().minorVersion());

    QDesktopServices::openUrl(
                QUrl("mailto:" + email +
                     "?subject=" + subject +
                     "&body=" + body));
}

void repo::gui::RepoGUI::openUserManager() const
{
    core::MongoClientWrapper mongo = ui->widgetRepository->getSelectedConnection();
    RepoDialogUserManager um(mongo, (QWidget*) this);
    um.exec();
}

void repo::gui::RepoGUI::refresh()
{
    ui->widgetRepository->refresh();
}

void repo::gui::RepoGUI::reportIssue() const
{
    QDesktopServices::openUrl(QUrl("https://github.com/3drepo/3drepogui/issues"));
}

void repo::gui::RepoGUI::saveAs()
{
    if (const RepoGLCWidget *widget = getActiveWidget())
    {
        // TODO: create export worker
        QString path = QFileDialog::getSaveFileName(
            this,
            tr("Select a file to save"),
            QString(QDir::separator()) + widget->windowTitle(),
            tr(core::AssimpWrapper::getExportFormats().c_str()));
        QFileInfo fileInfo(path);
        QDir directory = fileInfo.absoluteDir();
        QString fileExtension = fileInfo.completeSuffix();
        const core::RepoGraphScene *repoScene = widget->getRepoScene();

        std::cout << "Exporting to " << path.toStdString() << std::endl;

        string embeddedTextureExtension = ".jpg";
        aiScene *scene = new aiScene();
        scene->mFlags = 0; //getPostProcessingFlags(); // TODO FIX ME!
        repoScene->toAssimp(scene);
        core::AssimpWrapper exporter;

        bool successful = exporter.exportModel(scene,
            core::AssimpWrapper::getExportFormatID(fileExtension.toStdString()),
            path.toStdString(),
            embeddedTextureExtension);

        if (!successful)
           std::cerr << "Export unsuccessful." << std::endl;
        else
        {
           std::vector<core::RepoNodeTexture *> textures = repoScene->getTextures();
           for (size_t i = 0; i < textures.size(); ++i)
           {
               core::RepoNodeTexture *repoTex = textures[i];
                const unsigned char *data = (unsigned char*) repoTex->getData();
                QImage image = QImage::fromData(data, repoTex->getDataSize());
                QString filename = QString::fromStdString(repoTex->getName());

                if (scene->HasTextures())
                {
                    string name = repoTex->getName();
                    name = name.substr(1, name.size()); // remove leading '*' char
                    filename = QString::fromStdString(name + embeddedTextureExtension);
                }
                QFileInfo fi(directory,filename);
                image.save(fi.absoluteFilePath());
            }
            /* TODO: textures
            const map<string, QImage> textures = widget->getTextures();
            for (map<string, QImage>::const_iterator it = textures.begin(); it != textures.end(); it++)
            {
                QString filename(((*it).first).c_str());

                // if embedded textures
                if (scene->HasTextures())
                {
                    string name = (*it).first;
                    name = name.substr(1, name.size()); // remove leading '*' char
                    filename = QString((name + embeddedTextureExtension).c_str());
                }
                QFileInfo fi(directory,filename);
                QImage image = (*it).second;
                image.save(fi.absoluteFilePath());
            }*/
            std::cout << "Export successful." << std::endl;
        }
        delete scene;
    }
}

void repo::gui::RepoGUI::showCollectionContextMenuSlot(const QPoint &pos)
{
    QMenu menu(ui->widgetRepository->getCollectionTreeView());
    QAction *a = menu.addAction(
                tr("Copy"),
                ui->widgetRepository,
                SLOT(copySelectedCollectionCellToClipboard()));
    a->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    menu.addAction(
                tr("Expand all"),
                ui->widgetRepository,
                SLOT(expandAllCollectionRecords()));
    menu.addSeparator();

    a = menu.addAction(tr("Delete record"), this, SLOT(deleteRecordSlot()));
    a->setEnabled(false);
    a = menu.addAction(tr("Delete all records"), this, SLOT(deleteAllRecordsSlot()));
    a->setEnabled(false);

    menu.exec(ui->widgetRepository->mapToGlobalCollectionTreeView(pos));
}

void repo::gui::RepoGUI::showDatabaseContextMenu(const QPoint &pos)
{
    QMenu menu(ui->widgetRepository->getDatabasesTreeView());
    menu.addAction(ui->actionConnect);
    menu.addAction(ui->actionRefresh);
    menu.addSeparator();
    menu.addAction(ui->actionHead);
    menu.addAction(ui->actionHistory);
    menu.addAction(ui->actionSwitch);
    menu.addSeparator();
    menu.addAction(ui->actionDrop);
    menu.exec(ui->widgetRepository->mapToGlobalDatabasesTreeView(pos));
}

void repo::gui::RepoGUI::startup()
{
    RepoDialogConnect connectionDialog(this);
    if (connectionDialog.isShowAtStartup())
        connect();
}

void repo::gui::RepoGUI::toggleFullScreen()
{
    if (ui->actionFull_Screen->isChecked())
    {
        ui->menuBar->hide();
        ui->dockWidgetRepositories->hide();
        ui->dockWidgetLog->hide();
        ui->repositoriesToolBar->hide();
        ui->openGLToolBar->hide();
        showFullScreen();
    }
    else
    {
        ui->menuBar->show();
        ui->dockWidgetRepositories->show();
        ui->dockWidgetLog->show();
        ui->repositoriesToolBar->show();
        ui->openGLToolBar->show();
        showNormal();
    }
}

//------------------------------------------------------------------------------
//
// Protected
//
//------------------------------------------------------------------------------

void repo::gui::RepoGUI::closeEvent(QCloseEvent *event)
{
    storeSettings();
    QMainWindow::closeEvent(event);
}

void repo::gui::RepoGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // accept only drag events that provide urls (paths) to files/resources
   if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void repo::gui::RepoGUI::dropEvent(QDropEvent *event)
{
    // a list of full paths to files dragged onto the window
    loadFiles(event->mimeData()->urls());
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    event->acceptProposedAction();
}

void repo::gui::RepoGUI::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_F11:
            ui->actionFull_Screen->setChecked(!ui->actionFull_Screen->isChecked());
            toggleFullScreen();
            break;
        case Qt::Key_Escape:
            ui->actionFull_Screen->setChecked(false);
            toggleFullScreen();
            break;
    }
    QMainWindow::keyPressEvent(event);
}

void repo::gui::RepoGUI::restoreSettings()
{
    QSettings settings;
    restoreGeometry(settings.value(REPO_SETTINGS_GUI_GEOMETRY).toByteArray());
    restoreState(settings.value(REPO_SETTINGS_GUI_STATE).toByteArray());
    ui->actionLink->setChecked(settings.value(REPO_SETTINGS_LINK_WINDOWS, false).toBool());
}

void repo::gui::RepoGUI::storeSettings()
{
    QSettings settings;
    settings.setValue(REPO_SETTINGS_GUI_GEOMETRY, saveGeometry());
    settings.setValue(REPO_SETTINGS_GUI_STATE, saveState());
    settings.setValue(REPO_SETTINGS_LINK_WINDOWS, ui->actionLink->isChecked());
}

