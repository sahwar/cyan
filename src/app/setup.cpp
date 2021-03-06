/*
# Copyright Ole-André Rodlie.
#
# ole.andre.rodlie@gmail.com
#
# This software is governed by the CeCILL license under French law and
# abiding by the rules of distribution of free software. You can use,
# modify and / or redistribute the software under the terms of the CeCILL
# license as circulated by CEA, CNRS and INRIA at the following URL
# "https://www.cecill.info".
#
# As a counterpart to the access to the source code and rights to
# modify and redistribute granted by the license, users are provided only
# with a limited warranty and the software's author, the holder of the
# economic rights and the subsequent licensors have only limited
# liability.
#
# In this respect, the user's attention is drawn to the associated risks
# with loading, using, modifying and / or developing or reproducing the
# software by the user in light of its specific status of free software,
# that can mean that it is complicated to manipulate, and that also
# so that it is for developers and experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to test and test the software's suitability
# Requirements in the conditions of their systems
# data to be ensured and, more generally, to use and operate
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license and that you accept its terms.
*/

#include "editor.h"
#include <QStyleFactory>
#include <QApplication>
#include <QVBoxLayout>

#include "colorrgb.h"
#include "colorcmyk.h"
#include "colorhsv.h"

void Editor::setupStyle()
{
    // style app
    qApp->setStyle(QStyleFactory::create("fusion"));

    QIcon::setThemeName("Cyan");
    setWindowIcon(QIcon::fromTheme("cyan"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53,53,53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(15,15,15));
    palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    palette.setColor(QPalette::Link, Qt::white);
    palette.setColor(QPalette::LinkVisited, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Highlight, QColor(196,110,33));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    qApp->setPalette(palette);

    setStyleSheet(QString("QMenu::separator { background-color: rgb(53, 53, 53); height: 1px; }"
                          /*"QMainWindow, QMenu, QDockWidget, QMenuBar, QDialog,"
                          "QPushButton, QSpinBox, QDoubleSpinBox, QLineEdit, QRadioButton"
                          "{ font-size: %1pt; }"*/
                          "QToolBar { border-color: none; }")/*.arg(CYAN_FONT_SIZE)*/);
}

void Editor::setupUI()
{
    setupStyle();
    setupWidgets();
    setupMenus();
    setupToolbars();
    setupActions();
    setupButtons();
    setupColorManagement();
    setupImageLayers();
    setupConnections();
    setupIcons();
    setupShortcuts();
    setupOptions();

    setCentralWidget(mdi);
    setStatusBar(mainStatusBar);

    setMenuBar(mainMenu);

    mainMenu->addMenu(fileMenu);
    mainMenu->addMenu(colorMenu);
    mainMenu->addMenu(optMenu);
    mainMenu->addMenu(helpMenu);

    mainToolBar->addWidget(newButton);
    mainToolBar->addAction(openImageAct);
    mainToolBar->addWidget(saveButton);
    mainToolBar->addAction(viewMoveAct);
    mainToolBar->addAction(viewDrawAct);

    fileMenu->addAction(newImageAct);
    fileMenu->addAction(newLayerAct);
    fileMenu->addSeparator();
    fileMenu->addAction(openImageAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveProjectAct);
    fileMenu->addAction(saveProjectAsAct);
    fileMenu->addAction(saveImageAct);
    fileMenu->addAction(saveLayerAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    helpMenu->addAction(aboutImageMagickAct);
    helpMenu->addAction(aboutLcmsAct);
    helpMenu->addAction(aboutQtAct);

    newMenu->addAction(newImageAct);
    newMenu->addAction(newLayerAct);

    saveMenu->addAction(saveProjectAct);
    saveMenu->addAction(saveProjectAsAct);
    saveMenu->addAction(saveImageAct);
    saveMenu->addAction(saveLayerAct);

    colorMenu->addAction(convertRGBAct);
    colorMenu->addAction(convertCMYKAct);
    colorMenu->addAction(convertGRAYAct);
    colorMenu->addSeparator();
    colorMenu->addAction(convertAssignAct);
    colorMenu->addAction(convertExtractAct);
    colorMenu->addSeparator();
    colorMenu->addMenu(colorProfileRGBMenu);
    colorMenu->addMenu(colorProfileCMYKMenu);
    colorMenu->addMenu(colorProfileGRAYMenu);
    colorMenu->addSeparator();
    colorMenu->addMenu(colorIntentMenu);
    colorMenu->addAction(blackPointAct);

    viewMoveAct->setChecked(true);

    populateColorProfileMenu(colorProfileRGBMenu,
                             Magick::sRGBColorspace);
    populateColorProfileMenu(colorProfileCMYKMenu,
                             Magick::CMYKColorspace);
    populateColorProfileMenu(colorProfileGRAYMenu,
                             Magick::GRAYColorspace);
    populateColorIntentMenu();

    QLabel *brushSizeLabel = new QLabel(this);
    brushSizeLabel->setPixmap(QIcon(":/icons/brushsize.png")
                              .pixmap(24, 24));


    QWidget *brushWidget = new QWidget(this);
    QVBoxLayout *brushLayout = new QVBoxLayout(brushWidget);

    QWidget *brushSizeWidget = new QWidget(this);
    QHBoxLayout *brushSizeLayout = new QHBoxLayout(brushSizeWidget);

    brushSizeLayout->addWidget(brushSizeLabel);
    brushSizeLayout->addWidget(brushSize);
    brushLayout->addWidget(brushSizeWidget);
    brushLayout->addStretch();

    brushDock = new QDockWidget(this);
    brushDock->setWindowTitle(tr("Brush"));
    brushDock->setObjectName(QString("brushDock"));
    brushDock->setWidget(brushWidget);
    addDockWidget(Qt::RightDockWidgetArea,
                  brushDock);
}

void Editor::setupMenus()
{
    mainMenu = new QMenuBar(this);
    mainMenu->setObjectName(QString("mainMenu"));

    fileMenu = new QMenu(this);
    fileMenu->setTitle(tr("File"));

    optMenu = new QMenu(this);
    optMenu->setTitle(tr("Options"));

    helpMenu = new QMenu(this);
    helpMenu->setTitle(tr("Help"));

    newMenu = new QMenu(this);
    newMenu->setTitle(tr("New"));

    saveMenu = new QMenu(this);
    saveMenu->setTitle(tr("Save"));

    colorMenu = new QMenu(this);
    colorMenu->setTitle(tr("Color"));

    colorProfileRGBMenu = new QMenu(this);
    colorProfileRGBMenu->setTitle(tr("Default RGB profile"));
    colorProfileRGBMenu->setObjectName(QString("colorProfileRGBMenu"));

    colorProfileCMYKMenu = new QMenu(this);
    colorProfileCMYKMenu->setTitle(tr("Default CMYK profile"));
    colorProfileCMYKMenu->setObjectName(QString("colorProfileCMYKMenu"));

    colorProfileGRAYMenu = new QMenu(this);
    colorProfileGRAYMenu->setTitle(tr("Default GRAY profile"));
    colorProfileGRAYMenu->setObjectName(QString("colorProfileGRAYMenu"));

    colorIntentMenu = new QMenu(this);
    colorIntentMenu->setTitle(tr("Rendering Intent"));
}

void Editor::setupToolbars()
{
    mainToolBar = new QToolBar(this);
    mainToolBar->setObjectName(QString("mainToolBar"));
    mainToolBar->setWindowTitle(tr("Main"));


    addToolBar(Qt::LeftToolBarArea,
               mainToolBar);
}

void Editor::setupWidgets()
{
    mdi = new Mdi(this);
    mdi->setBackground(QBrush(QColor(20, 20, 20)));
    //mdi->setViewMode(QMdiArea::TabbedView);

    mainStatusBar = new QStatusBar(this);
    mainStatusBar->setObjectName(QString("mainStatusBar"));

    brushSize = new QSlider(this);
    brushSize->setRange(1,256);
    brushSize->setValue(20);
    brushSize->setOrientation(Qt::Horizontal);
}


void Editor::setupActions()
{

    newImageAct = new QAction(this);
    newImageAct->setText(tr("New image"));

    openImageAct = new QAction(this);
    openImageAct->setText(tr("Open"));

    saveImageAct = new QAction(this);
    saveImageAct->setText(tr("Save image"));

    saveProjectAct = new QAction(this);
    saveProjectAct->setText(tr("Save project"));

    saveProjectAsAct = new QAction(this);
    saveProjectAsAct->setText(tr("Save project as ..."));
    saveProjectAsAct->setDisabled(true);

    newLayerAct = new QAction(this);
    newLayerAct->setText(tr("New layer"));

    saveLayerAct = new QAction(this);
    saveLayerAct->setText(tr("Save layer as ..."));

    quitAct = new QAction(this);
    quitAct->setText(tr("Quit"));

    viewMoveAct = new QAction(this);
    viewMoveAct->setText(tr("Move"));
    viewMoveAct->setCheckable(true);
    viewMoveAct->setChecked(false);

    viewDrawAct = new QAction(this);
    viewDrawAct->setText("Draw");
    viewDrawAct->setCheckable(true);
    viewDrawAct->setChecked(false);

    aboutImageMagickAct = new QAction(this);
    aboutImageMagickAct->setText(tr("About ImageMagick"));

    aboutLcmsAct = new QAction(this);
    aboutLcmsAct->setText(tr("About Little CMS"));

    aboutQtAct = new QAction(this);
    aboutQtAct->setText(tr("About Qt"));

    convertRGBAct = new QAction(this);
    convertRGBAct->setText(tr("Convert to RGB"));

    convertCMYKAct = new QAction(this);
    convertCMYKAct->setText(tr("Convert to CMYK"));

    convertGRAYAct = new QAction(this);
    convertGRAYAct->setText(tr("Convert to GRAY"));

    convertAssignAct = new QAction(this);
    convertAssignAct->setText(tr("Assign color profile"));

    convertExtractAct = new QAction(this);
    convertExtractAct->setText(tr("Extract color profile"));

    blackPointAct = new QAction(this);
    blackPointAct->setText(tr("Black point compensation"));
    blackPointAct->setCheckable(true);
}

void Editor::setupButtons()
{
    newButton = new QToolButton(this);
   // newButton->setIconSize(QSize(32, 32));
    newButton->setMenu(newMenu);
    newButton->setPopupMode(QToolButton::InstantPopup);
    newButton->setText(tr("New"));
    newButton->setToolTip(tr("New"));
    //newButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    saveButton = new QToolButton(this);
    //saveButton->setIconSize(QSize(32, 32));
    saveButton->setMenu(saveMenu);
    saveButton->setPopupMode(QToolButton::InstantPopup);
    saveButton->setText(tr("Save"));
    saveButton->setToolTip(tr("Save"));
    //saveButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

void Editor::setupColorManagement()
{
    colorPicker = new QtColorPicker(this, -1, true, false);
    colorPicker->setIconSize(QSize(120,22));
    colorPicker->setFlat(true);
    colorPicker->setContentsMargins(0,0,0,0);
    colorPicker->setFixedSize(colorPicker->iconSize());
    colorPicker->setStyleSheet(QString("margin:0;padding:0;"));
    colorPicker->setStandardColors();
    colorPicker->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mainStatusBar->addPermanentWidget(colorPicker);

    colorTriangle = new QtColorTriangle(this);
    colorTriangle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QDockWidget *colorDock = new QDockWidget(this);
    colorDock->setWindowIcon(QIcon(":/icons/colors.png"));
    colorDock->setObjectName(QString("colorTriangle"));
    colorDock->setWindowTitle(tr("Triangle"));
    colorDock->setWidget(colorTriangle);
    //colorDock->layout()->setContentsMargins(5, 5, 5, 5);
    colorDock->setMinimumHeight(150);
    colorDock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(colorTriangle, SIGNAL(colorChanged(QColor)), colorPicker, SLOT(setCurrentColor(QColor)));
    connect(colorPicker, SIGNAL(colorChanged(QColor)), this, SLOT(handleColorChanged(QColor)));
    connect(colorPicker, SIGNAL(colorChanged(QColor)), colorTriangle, SLOT(setColor(QColor)));

    QIcon colorsIcon = QIcon::fromTheme("smartart_change_color_gallery");

    QDockWidget *colorRGBDock = new QDockWidget(this);
    colorRGBDock->setObjectName(QString("colorRGBDock"));
    colorRGBDock->setWindowTitle(tr("RGB"));
    colorRGBDock->setWindowIcon(colorsIcon);
    //colorRGBDock->setMaximumHeight(100);

    ColorRGB *colorRGB = new ColorRGB(this);
    colorRGBDock->setWidget(colorRGB);
    connect(colorTriangle, SIGNAL(colorChanged(QColor)), colorRGB, SLOT(setColor(QColor)));
    connect(colorRGB, SIGNAL(colorChanged(QColor)), colorTriangle, SLOT(setColor(QColor)));

    QDockWidget *colorCMYKDock = new QDockWidget(this);
    colorCMYKDock->setObjectName(QString("colorCMYKDock"));
    colorCMYKDock->setWindowTitle(tr("CMYK"));
    colorCMYKDock->setWindowIcon(colorsIcon);
    //colorCMYKDock->setMaximumHeight(100);

    ColorCMYK *colorCMYK = new ColorCMYK(this);
    colorCMYKDock->setWidget(colorCMYK);
    connect(colorTriangle, SIGNAL(colorChanged(QColor)), colorCMYK, SLOT(setColor(QColor)));
    connect(colorCMYK, SIGNAL(colorChanged(QColor)), colorTriangle, SLOT(setColor(QColor)));

    QDockWidget *colorHSVDock = new QDockWidget(this);
    colorHSVDock->setObjectName(QString("colorHSVDock"));
    colorHSVDock->setWindowTitle(tr("HSV"));
    colorHSVDock->setWindowIcon(colorsIcon);
    //colorHSVDock->setMaximumHeight(100);

    ColorHSV *colorHSV = new ColorHSV(this);
    colorHSVDock->setWidget(colorHSV);
    connect(colorTriangle, SIGNAL(colorChanged(QColor)), colorHSV, SLOT(setColor(QColor)));
    connect(colorHSV, SIGNAL(colorChanged(QColor)), colorTriangle, SLOT(setColor(QColor)));

    colorTriangle->setColor(QColor(Qt::black));

    addDockWidget(Qt::RightDockWidgetArea,
                  colorDock);
    addDockWidget(Qt::RightDockWidgetArea,
                  colorHSVDock);
    addDockWidget(Qt::RightDockWidgetArea,
                  colorRGBDock);
    addDockWidget(Qt::RightDockWidgetArea,
                  colorCMYKDock);
}

void Editor::setupImageLayers()
{
    layersOpacity = new QSlider(this);
    layersOpacity->setRange(0,100);
    layersOpacity->setValue(100);
    layersOpacity->setOrientation(Qt::Horizontal);
    connect(layersOpacity, SIGNAL(sliderReleased()), this, SLOT(handleLayersOpacity()));
    connect(layersOpacity, SIGNAL(sliderMoved(int)), this, SLOT(handleLayersOpacity()));


    layersTree = new LayerTree(this);
    layersTree->setExpandsOnDoubleClick(true);
    connect(layersTree, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(handleLayerActivated(QTreeWidgetItem*,int)));
    connect(layersTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(handleLayerActivated(QTreeWidgetItem*,int)));
    connect(layersTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(handleLayerDoubleclicked(QTreeWidgetItem*,int)));
    connect(layersTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(handleLayerActivated(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), layersTree, SLOT(handleTabActivated(QMdiSubWindow*)));
    connect(mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(handleTabActivated(QMdiSubWindow*)));

    layersComp = new QComboBox(this);
    populateCompBox();
    connect(layersComp, SIGNAL(currentIndexChanged(QString)), this, SLOT(handleLayerCompChanged(QString)));

    layersDock = new QDockWidget(this);
    layersDock->setObjectName(QString("layersDock"));
    layersDock->setWindowTitle(tr("Layers"));
   // layersDock->setFeatures(QDockWidget::DockWidgetMovable/*|QDockWidget::DockWidgetClosable*/);

    QWidget *layersContainer = new QWidget(this);
    QVBoxLayout *layersContainerLayout = new QVBoxLayout(layersContainer);
    layersContainerLayout->setContentsMargins(5, 5, 5, 0);

    layersContainerLayout->addWidget(layersComp);
    layersContainerLayout->addWidget(layersOpacity);
    layersContainerLayout->addWidget(layersTree);
    layersDock->setWidget(layersContainer);

    addDockWidget(Qt::LeftDockWidgetArea, layersDock);
}

void Editor::setupConnections()
{

    //connect(&common, SIGNAL(errorMessage(QString)), this, SLOT(handleError(QString)));
    //connect(&common, SIGNAL(warningMessage(QString)), this, SLOT(handleWarning(QString)));

    connect(newImageAct, SIGNAL(triggered(bool)), this, SLOT(newImageDialog()));
    connect(newLayerAct, SIGNAL(triggered(bool)), this, SLOT(newLayerDialog()));
    connect(openImageAct, SIGNAL(triggered(bool)), this, SLOT(loadImageDialog()));
    connect(saveProjectAct, SIGNAL(triggered(bool)), this, SLOT(saveProjectDialog()));
    connect(saveImageAct, SIGNAL(triggered(bool)), this, SLOT(saveImageDialog()));
    connect(saveLayerAct, SIGNAL(triggered(bool)), this, SLOT(saveLayerDialog()));

    connect(quitAct, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(viewMoveAct, SIGNAL(triggered(bool)), this, SLOT(handleSetMoveMode(bool)));
    connect(viewDrawAct, SIGNAL(triggered(bool)), this, SLOT(handleSetDrawMode(bool)));

    connect(aboutImageMagickAct, SIGNAL(triggered()), this, SLOT(aboutImageMagick()));
    connect(aboutLcmsAct, SIGNAL(triggered()), this, SLOT(aboutLcms()));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    connect(convertRGBAct, SIGNAL(triggered()), this, SLOT(handleColorConvertRGB()));
    connect(convertCMYKAct, SIGNAL(triggered()), this, SLOT(handleColorConvertCMYK()));
    connect(convertGRAYAct, SIGNAL(triggered()), this, SLOT(handleColorConvertGRAY()));
    connect(convertAssignAct, SIGNAL(triggered()), this, SLOT(handleColorProfileAssign()));

    connect(this, SIGNAL(statusMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(this, SIGNAL(warningMessage(QString)), this, SLOT(handleWarning(QString)));
    connect(this, SIGNAL(errorMessage(QString)), this, SLOT(handleError(QString)));
    connect(mdi, SIGNAL(openImages(QList<QUrl>)), this, SLOT(handleOpenImages(QList<QUrl>)));

    connect(layersTree, SIGNAL(selectedLayer(int)), this, SLOT(handleLayerTreeSelectedLayer(int)));
    connect(layersTree, SIGNAL(layerVisibilityChanged(int,bool)), this, SLOT(handleLayerVisibility(int,bool)));
    connect(layersTree, SIGNAL(layerLabelChanged(int,QString)), this, SLOT(handleLayerLabel(int,QString)));

    connect(brushSize, SIGNAL(valueChanged(int)), this, SLOT(handleBrushSize()));
}

void Editor::setupIcons()
{
    newImageAct->setIcon(QIcon::fromTheme("document-new"));
    newLayerAct->setIcon(QIcon::fromTheme("document-new"));
    newButton->setIcon(QIcon::fromTheme("document-new"));

    openImageAct->setIcon(QIcon::fromTheme("document-open"));
    saveButton->setIcon(QIcon::fromTheme("document-save"));
    saveImageAct->setIcon(QIcon::fromTheme("document-save"));
    saveProjectAct->setIcon(QIcon::fromTheme("document-save"));
    saveLayerAct->setIcon(QIcon::fromTheme("document-save"));
    saveProjectAsAct->setIcon(QIcon::fromTheme("document-save-as"));
    quitAct->setIcon(QIcon::fromTheme("application-exit"));

    viewMoveAct->setIcon(QIcon::fromTheme("transform_move"));
    viewDrawAct->setIcon(QIcon::fromTheme("paintbrush"));

    QIcon colorsIcon = QIcon::fromTheme("smartart_change_color_gallery");
    QIcon colorWheelIcon = QIcon::fromTheme("color_wheel");

    convertRGBAct->setIcon(QIcon::fromTheme("convert_gray_to_color"));
    convertCMYKAct->setIcon(QIcon::fromTheme("convert_gray_to_color"));
    convertGRAYAct->setIcon(QIcon::fromTheme("convert_color_to_gray"));
    convertAssignAct->setIcon(colorsIcon);
    convertExtractAct->setIcon(colorsIcon);
    colorProfileRGBMenu->setIcon(colorWheelIcon);
    colorProfileCMYKMenu->setIcon(colorWheelIcon);
    colorProfileGRAYMenu->setIcon(colorWheelIcon);
    colorIntentMenu->setIcon(QIcon::fromTheme("monitor_window_flow"));
    blackPointAct->setIcon(colorWheelIcon);

    aboutQtAct->setIcon(QIcon::fromTheme("help-about"));
    aboutLcmsAct->setIcon(QIcon::fromTheme("help-about"));
    aboutImageMagickAct->setIcon(QIcon::fromTheme("help-about"));
}

void Editor::setupShortcuts()
{
    newImageAct->setShortcut(QKeySequence(tr("Ctrl+N")));
    newLayerAct->setShortcut(QKeySequence(tr("Ctrl+L")));
    openImageAct->setShortcut(QKeySequence(tr("Ctrl+O")));
    quitAct->setShortcut(QKeySequence(tr("Ctrl+Q")));
}

void Editor::setupOptions()
{

}
