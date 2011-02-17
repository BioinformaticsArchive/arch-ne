////////////////////////////////////////////////////////////////////////////////
//
//   Copyright 2011 Mario Negri Institute & Orobix Srl
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include <QtGui>
#include <QDockWidget>
#include <QGridLayout>

#include "mainwindow.h"
#include "editorarea.h"
#include "workspace.h"
#include "DataCollector/datacollector.h"
#include "dock.h"

#include <QTextStream>
using namespace std;

QTextStream mainout(stdout);

MainWindow::MainWindow()
{
    editorArea = new EditorArea;
    setCentralWidget(editorArea);

    tabs = new QTabWidget;
    connect(tabs, SIGNAL(currentChanged(int)),this, SLOT(tabsContentChanged()));

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();

    readSettings(); // It restores user's preferences.

    setCurrentFile(""); // It sets the window title.
    setUnifiedTitleAndToolBarOnMac(true);
}

EditorArea* MainWindow::getEditorArea()
{
    return editorArea;
}

void MainWindow::insertDataCollectorToDock(DataCollector* theDataCollector, QPoint elementRequest)
{
    QString element;

    if (elementRequest.x() == 1) {
        element = "node";
    } else {
        element = "edge";
    }

    element.append(" ");
    QString idString;
    idString.setNum(elementRequest.y());
    element.append(idString);

    dataCollectorList.insert(theDataCollector, elementRequest);

    tabs->insertTab(0, theDataCollector, element);
    tabs->setCurrentWidget(theDataCollector);
    emit editingEl2Ws(elementRequest);
    dock->setWidget(tabs);
}

void MainWindow::removeAllDataCollectorFromDock()
{
    tabs->clear();
    dataCollectorList.clear();
    QPoint temp(-1, -1);
    emit editingEl2Ws(temp);
}

void MainWindow::removeDataCollectorFromDock()
{
    dataCollectorList.remove(tabs->currentWidget());
    tabs->removeTab(tabs->currentIndex());
    if (tabs->count() == 0) {
        hideDock();
    }
}

void MainWindow::setPageInTab(DataCollector* theDataCollector)
{
    tabs->setCurrentWidget(theDataCollector);
    emit editingEl2Ws(dataCollectorList.value(tabs->currentWidget()));
}

void MainWindow::tabsContentChanged()
{
    emit editingEl2Ws(dataCollectorList.value(tabs->currentWidget()));
}

void MainWindow::mouseEnteredInDock()
{
    emit editingEl2Ws(dataCollectorList.value(tabs->currentWidget()));
}

void MainWindow::dockClosed()
{
    removeAllDataCollectorFromDock();
}

void MainWindow::elementsBeenHit(QVector<QPoint> hitEls)
{
    if (hitEls[0].x() != -1) {
        QMapIterator<QWidget*, QPoint> dataColIter(dataCollectorList);
        while (dataColIter.hasNext()) {
            dataColIter.next();
            if (dataColIter.value() == hitEls[0]) {
                tabs->setCurrentWidget(dataColIter.key());
            }
        }
    }
}

void MainWindow::hideDock()
{
    dock->hide();
}

void MainWindow::showDock()
{
    dock->resize(850,800);
    dock->show();
}

void MainWindow::createActions()
{
    newNetworkAct = new QAction(QIcon(":/images/new.png"), tr("&New Network"), this);
    newNetworkAct->setShortcuts(QKeySequence::New);
    newNetworkAct->setStatusTip(tr("Create a new network"));
    connect(newNetworkAct, SIGNAL(triggered()), this, SLOT(newNetwork()));

    openNetworkAct = new QAction(QIcon(":/images/openNet.png"), tr("&Open Network"), this);
    openNetworkAct->setShortcuts(QKeySequence::Open);
    openNetworkAct->setStatusTip(tr("Open an existing network"));
    connect(openNetworkAct, SIGNAL(triggered()), this, SLOT(openNetwork()));

    importNetworkAct = new QAction(QIcon(":/images/importNet.png"), tr("&Import Network"), this);
    importNetworkAct->setStatusTip(tr("Import an existing network"));
    connect(importNetworkAct, SIGNAL(triggered()), this, SLOT(importNetwork()));

    importMeshAct = new QAction(QIcon(":/images/importMesh.png"), tr("Import Mesh"), this);
    importMeshAct->setStatusTip(tr("Import an existing mesh"));
    connect(importMeshAct, SIGNAL(triggered()), this, SLOT(importMesh()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save File"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the netwrok"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the network under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("Exit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the ARCHNetworkEditor"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    /////

    undoAct = new QAction(QIcon(":/images/undo.png"), tr("Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    connect(undoAct, SIGNAL(triggered()), this, SIGNAL(undoPressed()));

    redoAct = new QAction(QIcon(":/images/redo.png"), tr("Redo"), this);
    redoAct->setShortcut(tr("Ctrl+shift+Z"));
    connect(redoAct, SIGNAL(triggered()), this, SIGNAL(redoPressed()));

    bcAct = new QAction(QIcon(":/images/BC.png"), tr("Set Boundary Conditions"), this);
    bcAct->setStatusTip(tr("Set Boundary Conditions"));
    connect(bcAct, SIGNAL(triggered()), this, SIGNAL(bcPressed()));

    spAct = new QAction(QIcon(":/images/SP.png"), tr("Set Simulation Parameters"), this);
    spAct->setStatusTip(tr("Set Simulation Parameters"));
    connect(spAct, SIGNAL(triggered()), this, SIGNAL(spPressed()));
    /////

    addSegmentAct = new QAction(QIcon(":/images/addSegment.png"), tr("&Add Segment"), this);
    addSegmentAct->setStatusTip(tr("Create a new vascular segment"));
    connect(addSegmentAct, SIGNAL(triggered()), this, SIGNAL(addSegmentPressed()));

    removeSegmentAct = new QAction(QIcon(":/images/removeSegment.png"), tr("&Remove Segment"), this);
    removeSegmentAct->setShortcut(tr("Ctrl+R"));
    removeSegmentAct->setStatusTip(tr("Remove an existing vascular segment"));
    connect(removeSegmentAct, SIGNAL(triggered()), this, SIGNAL(removeSegmentPressed()));

    splitSegmentAct = new QAction(QIcon(":/images/splitSegment.png"), tr("Spli&t Segment"), this);
    splitSegmentAct->setShortcut(tr("Ctrl+T"));
    splitSegmentAct->setStatusTip(tr("Split an existing vascular segment"));
    connect(splitSegmentAct, SIGNAL(triggered()), this, SIGNAL(splitSegmentPressed()));

    superEdgeAct = new QAction(QIcon(":/images/superEdge.png"), tr("Super Edge"), this);
    superEdgeAct->setStatusTip(tr("Create a super edge"));
    connect(superEdgeAct, SIGNAL(triggered()), this, SIGNAL(superEdgePressed()));

    selectElementsAct = new QAction(QIcon(":/images/moveGraph.png"), tr("Select Elements"), this);
    selectElementsAct->setShortcut(tr("Ctrl+shift+M"));
    selectElementsAct->setStatusTip(tr("Select part of the network"));
    connect(selectElementsAct, SIGNAL(triggered()), this, SIGNAL(selectElementsPressed()));

    meshAct = new QAction(QIcon(":/images/mesh.png"), tr("Mesh the network"), this);
    meshAct->setStatusTip(tr("Create the mesh for the network"));
    connect(meshAct, SIGNAL(triggered()), this, SLOT(meshPressed()));

    defaultMeshAct = new QAction(QIcon(":/images/defaultMesh.png"), tr("Apply default mesh"), this);
    defaultMeshAct->setShortcut(tr("Ctrl+shift+D"));
    defaultMeshAct->setStatusTip(tr("Apply the default mesh to the network"));
    connect(defaultMeshAct, SIGNAL(triggered()), this, SIGNAL(defaultMeshPressed()));

    infoAct = new QAction(QIcon(":/images/info.png"), tr("&Edit Information"), this);
    infoAct->setShortcut(tr("Ctrl+I"));
    infoAct->setStatusTip(tr("Edit element properties"));
    connect(infoAct, SIGNAL(triggered()), this, SIGNAL(infoPressed()));
    /////

    zoomInAct = new QAction(QIcon(":/images/zoomIn.png"), tr("Zoom In"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setStatusTip(tr("Zoom in"));
    connect(zoomInAct, SIGNAL(triggered()), this, SIGNAL(zoomInPressed()));

    zoomOutAct = new QAction(QIcon(":/images/zoomOut.png"), tr("Zoom Out"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setStatusTip(tr("Zoom out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SIGNAL(zoomOutPressed()));

    translateAct = new QAction(QIcon(":/images/translate.png"), tr("Translate"), this);
    translateAct->setStatusTip(tr("Translate the scene"));
    connect(translateAct, SIGNAL(triggered()), this, SIGNAL(translatePressed()));

    homeViewAct = new QAction(QIcon(":/images/home.png"), tr("Reset View"), this);
    homeViewAct->setShortcut(tr("Ctrl+0"));
    homeViewAct->setStatusTip(tr("Reset the scene view"));
    connect(homeViewAct, SIGNAL(triggered()), this, SIGNAL(homeViewPressed()));

    showGridAct = new QAction(QIcon(":/images/grid.png"), tr("Grid"), this);
    showGridAct->setStatusTip(tr("Grid"));
    connect(showGridAct, SIGNAL(triggered()), this, SIGNAL(showGridPressed()));

    snapToGridAct = new QAction(QIcon(":/images/snapToGrid.png"), tr("Snap To Grid"), this);
    snapToGridAct->setStatusTip(tr("Snap To Grid"));
    connect(snapToGridAct, SIGNAL(triggered()), this, SIGNAL(snapToGridPressed()));

    showLabelsAct = new QAction(QIcon(":/images/showLabel.png"), tr("Show labels"), this);
    showLabelsAct->setStatusTip(tr("Show labels"));
    connect(showLabelsAct, SIGNAL(triggered()), this, SIGNAL(showLabelsPressed()));

    unravelNetAct = new QAction(QIcon(":/images/unravel.png"), tr("Unravel Network"), this);
    unravelNetAct->setShortcut(tr("Ctrl+U"));
    unravelNetAct->setStatusTip(tr("Unravel the network"));
    connect(unravelNetAct, SIGNAL(triggered()), this, SIGNAL(unravelNetPressed()));

    blockNodesAct = new QAction(QIcon(":/images/blockNodes.png"), tr("Block node position"), this);
    blockNodesAct->setShortcut(tr("Ctrl+B"));
    blockNodesAct->setStatusTip(tr("Block the nodes position"));
    connect(blockNodesAct, SIGNAL(triggered()), this, SIGNAL(blockNodesPressed()));
    /////

    aboutAct = new QAction(tr("About ARCHNetworkEditor"), this);
    aboutAct->setStatusTip(tr("Show the ARCHNetworkEditor's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    preferencesAct = new QAction(tr("preferences..."), this);
    preferencesAct->setShortcuts(QKeySequence::Preferences);
    //preferencesAct->setShortcut(tr("Ctrl+,"));
    preferencesAct->setStatusTip(tr("Set application preferences"));
    connect(preferencesAct, SIGNAL(triggered()), this, SIGNAL(setPrefPressed()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newNetworkAct);
    fileMenu->addAction(openNetworkAct);
    fileMenu->addAction(importNetworkAct);
    fileMenu->addAction(importMeshAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(aboutAct);
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(bcAct);
    editMenu->addAction(spAct);
    editMenu->addSeparator();
    editMenu->addAction(preferencesAct);

    menuBar()->addSeparator();

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(translateAct);
    viewMenu->addAction(homeViewAct);
    viewMenu->addAction(showGridAct);
    viewMenu->addAction(snapToGridAct);
    viewMenu->addAction(showLabelsAct);
    viewMenu->addAction(unravelNetAct);
    viewMenu->addAction(blockNodesAct);

    operationMenu = menuBar()->addMenu(tr("&Operations"));
    operationMenu->addAction(addSegmentAct);
    operationMenu->addAction(removeSegmentAct);
    operationMenu->addAction(splitSegmentAct);
    operationMenu->addAction(superEdgeAct);
    operationMenu->addAction(selectElementsAct);
    operationMenu->addAction(meshAct);
    operationMenu->addAction(defaultMeshAct);
    operationMenu->addAction(infoAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction(aboutAct);

    addSegmentAct->setCheckable(true);
    removeSegmentAct->setCheckable(true);
    splitSegmentAct->setCheckable(true);
    superEdgeAct->setCheckable(true);
    selectElementsAct->setCheckable(true);
    infoAct->setCheckable(true);

    showLabelsAct->setCheckable(true);
    showLabelsAct->setChecked(false);

    selectElementsAct->setChecked(true);

    translateAct->setCheckable(true);
    showGridAct->setCheckable(true);

    showGridAct->setChecked(true);
    snapToGridAct->setCheckable(true);
    snapToGridAct->setChecked(true);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newNetworkAct);
    fileToolBar->addAction(openNetworkAct);
    fileToolBar->addAction(importNetworkAct);
    fileToolBar->addAction(importMeshAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->setMovable(false);

    viewToolBar = addToolBar(tr("View"));
    viewToolBar->setAllowedAreas(Qt::TopToolBarArea);
    viewToolBar->addAction(zoomInAct);
    viewToolBar->addAction(zoomOutAct);
    viewToolBar->addAction(translateAct);
    viewToolBar->addAction(homeViewAct);
    viewToolBar->addAction(showGridAct);
    viewToolBar->addAction(snapToGridAct);
    viewToolBar->addAction(showLabelsAct);
    viewToolBar->addAction(unravelNetAct);
    viewToolBar->addAction(blockNodesAct);

    operationToolBar = addToolBar(tr("Operations"));
    addToolBar(Qt::LeftToolBarArea, operationToolBar);
    operationToolBar->setMovable(true);

    operationGroup = new QActionGroup(this);
    operationGroup->addAction(addSegmentAct);
    operationGroup->addAction(removeSegmentAct);
    operationGroup->addAction(splitSegmentAct);
    operationGroup->addAction(infoAct);
    operationGroup->addAction(translateAct);
    operationGroup->addAction(superEdgeAct);
    operationGroup->addAction(selectElementsAct);
    operationGroup->setExclusive(true);

    operationToolBar->setMovable(true);
    operationToolBar->addAction(addSegmentAct);
    operationToolBar->addAction(removeSegmentAct);
    operationToolBar->addAction(splitSegmentAct);
    operationToolBar->addAction(superEdgeAct);
    operationToolBar->addAction(selectElementsAct);
    operationToolBar->addAction(meshAct);
    operationToolBar->addAction(defaultMeshAct);
    operationToolBar->addAction(infoAct);
    operationToolBar->addAction(bcAct);
    operationToolBar->addAction(spAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWindows()
{
    dock = new Dock(this);
    connect(dock, SIGNAL(mouseEnteredInDock()), this, SLOT(mouseEnteredInDock()));
    connect(dock, SIGNAL(dockClosed()), this, SLOT(dockClosed()));
    connect(dock, SIGNAL(dockClosed()), this, SIGNAL(dockClosedSig()));
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->hide();
}


void MainWindow::readSettings()
{
    QSettings settings("archTk", "ARCHNetworkEditor");
    QPoint pos = settings.value("pos", QPoint(0, 0)).toPoint();
    QSize size = settings.value("size", QSize(800, 600)).toSize();

    resize(size);
    move(pos);
}

void MainWindow::writeSettings()
{   
    QSettings settings("archTk", "ARCHNetworkEditor");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

bool MainWindow::maybeSave()
{
    if (isWindowModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("ARCHNetworkEditor"),
                                   tr("The network has been modified.\n"
                                      "Do you want to save the changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return save();
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::loadNetwork()
{
    removeAllDataCollectorFromDock();
    hideDock();
    emit loadGraphFromLayout();
}

void MainWindow::loadNetWithoutLayout()
{
    removeAllDataCollectorFromDock();
    hideDock();
    emit loadGraphFromGraph();
}

void MainWindow::loadMesh()
{
    emit meshToBeLoaded();
}

bool MainWindow::saveFile(const QString& fileName)
{
    emit saveNetwork(fileName);

    curFile = fileName;

    setWindowModified(false);
    return true;
}

void MainWindow::setCurrentFile(const QString& fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty()) {
        shownName = "untitled";
    }
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::newNetwork()
{
    if (maybeSave()) {
        editorArea->clear();
        setCurrentFile("");

        removeAllDataCollectorFromDock();
        hideDock();
        initNewCase();
    }
}

void MainWindow::openNetwork()
{
    if (maybeSave()) {
        loadNetwork();
    }
}

void MainWindow::importNetwork()
{
    if (maybeSave()) {
        loadNetWithoutLayout();
    }
}

void MainWindow::importMesh()
{
    if (!editorArea->getWorkspace()->dataInGraph()) {
        QMessageBox msgBox;
        msgBox.setText("You need to have a network\n"
                       "before importing a mesh.");
        msgBox.exec();
        return;
    }

    loadMesh();
}

void MainWindow::meshPressed()
{
    emit meshToBeGenerated(curFile);
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty()) {
        return false;
    }

    return saveFile(fileName);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About ARCHNetworkEditor"),
                       tr("The ARCHNetworkEditor has been\n"
                          "developed in the context of the FP7.\n\n"
                          "Powered by OROBIX"));
}

void MainWindow::documentWasModified()
{
    setWindowModified(true);
}

void MainWindow::updateMainWindow()
{
    bool gridStatus = editorArea->getWorkspace()->getGridStatus();
    bool snapStatus = editorArea->getWorkspace()->getSnapToGridStatus();
    bool labelsStatus = editorArea->getWorkspace()->getShowLabelsStatus();
    int selectedTool = editorArea->getWorkspace()->getSelectedTool();

    if (gridStatus) {
        showGridAct->setChecked(true);
    } else {
        showGridAct->setChecked(false);
    }

    if (snapStatus) {
        snapToGridAct->setChecked(true);
    } else {
        snapToGridAct->setChecked(false);
    }

    if (labelsStatus) {
        showLabelsAct->setChecked(true);
    } else {
        showLabelsAct->setChecked(false);
    }

    addSegmentAct->setChecked(false);
    removeSegmentAct->setChecked(false);
    splitSegmentAct->setChecked(false);
    superEdgeAct->setChecked(false);
    selectElementsAct->setChecked(false);
    infoAct->setChecked(false);
    translateAct->setChecked(false);

    switch (selectedTool) {
    case 0:
        addSegmentAct->setChecked(true);
        break;
    case 1:
        removeSegmentAct->setChecked(true);
        break;
    case 2:
        splitSegmentAct->setChecked(true);
        break;
    case 3:
        superEdgeAct->setChecked(true);
        break;
    case 4:
        selectElementsAct->setChecked(true);
        break;
    case 5:
        infoAct->setChecked(true);
        break;
    case 6:
        translateAct->setChecked(true);
        break;
    }
}

void MainWindow::initNewCase()
{
    QString theCase("<case>\n"
                     "  <patient_id>10001</patient_id>\n"
                     "  <visit>V0 (pre-OP)</visit>\n"
                     "</case>\n");
    editorArea->getWorkspace()->getGraphProperties()->setCase(theCase);

    QString theSuperedges("<superedges>\n"
                          "</superedges>\n");
    editorArea->getWorkspace()->getGraphProperties()->setSuperedges(theSuperedges);

    QString theTransformations("<transformations>\n"
                               "</transformations>\n");
    editorArea->getWorkspace()->getGraphProperties()->setTransformations(theTransformations);
}

void MainWindow::showStatusBarMessage(QString theMessage)
{
    statusBar()->showMessage(theMessage, 2000);
}

void MainWindow::setFileName(QString theName)
{
    setCurrentFile(theName);
}

void MainWindow::setCurs()
{
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
}

void MainWindow::restoreCurs()
{
    //mainout << "restoreCurs in MW" << endl;

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}