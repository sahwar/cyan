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

#include <QFileDialog>
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QPluginLoader>
#include <QApplication>
#include <QToolButton>
#include <QMdiSubWindow>
#include <QVBoxLayout>
#include <QTimer>
#include <QStyleFactory>
#include <QMessageBox>
#include <QImage>
#include <QSettings>
#include <QHeaderView>
#include <QKeySequence>
#include <QDebug>

#include "colorrgb.h"
#include "colorcmyk.h"
#include "colorhsv.h"
#include "newmediadialog.h"
#include "convertdialog.h"

#ifdef Q_OS_MAC
#define CYAN_FONT_SIZE 10
#else
#define CYAN_FONT_SIZE 8
#endif

Editor::Editor(QWidget *parent)
    : QMainWindow(parent)
    , mdi(nullptr)
    , mainToolBar(nullptr)
    , viewToolBar(nullptr)
    , colorToolBar(nullptr)
    , brushToolBar(nullptr)
    , mainMenu(nullptr)
    , mainStatusBar(nullptr)
    , newImageAct(nullptr)
    , openImageAct(nullptr)
    , saveImageAct(nullptr)
    , saveProjectAct(nullptr)
    , saveProjectAsAct(nullptr)
    , newLayerAct(nullptr)
    , saveLayerAct(nullptr)
    , quitAct(nullptr)
    , viewDragAct(nullptr)
    , viewMoveAct(nullptr)
    , viewDrawAct(nullptr)
    , aboutImageMagickAct(nullptr)
    , aboutLcmsAct(nullptr)
    , aboutQtAct(nullptr)
    , convertRGBAct(nullptr)
    , convertCMYKAct(nullptr)
    , convertGRAYAct(nullptr)
    , convertAssignAct(nullptr)
    , convertExtractAct(nullptr)
    , fileMenu(nullptr)
    , helpMenu(nullptr)
    , newMenu(nullptr)
    , saveMenu(nullptr)
    , colorMenu(nullptr)
    , colorProfileRGBMenu(nullptr)
    , colorProfileCMYKMenu(nullptr)
    , colorProfileGRAYMenu(nullptr)
    , colorIntentMenu(nullptr)
    , newButton(nullptr)
    , saveButton(nullptr)
    , colorButton(nullptr)
    , layersTree(nullptr)
    , layersDock(nullptr)
    , layersComp(nullptr)
    , layersOpacity(nullptr)
    //, imageInfoTree(nullptr)
    , brushSize(nullptr)
    , colorTriangle(nullptr)
    , colorPicker(nullptr)
{
    setWindowTitle(qApp->applicationName());
    setAttribute(Qt::WA_QuitOnClose);

    qRegisterMetaType<Magick::Image>("Magick::Image");
    qRegisterMetaType<Magick::Drawable>("Magick::Drawable");
    qRegisterMetaType<Magick::Geometry>("Magick::Geometry");

#ifdef CYAN_DEVEL
    QMessageBox::warning(this,
                         tr("Cyan %1").arg(qApp->applicationVersion()),
                         tr("This is a development version of Cyan. "
                            "Functions are missing and some features are probably broken. "
                            "USE AT OWN RISK!"));
#endif

    setupUI();
    loadSettings();

#ifdef QT_DEBUG
    qDebug() << "color profile path" << Common::getColorProfilesPath();
    qDebug() << "rgb color profiles" << Common::getColorProfiles(Magick::sRGBColorspace);
    qDebug() << "cmyk color profiles" << Common::getColorProfiles(Magick::CMYKColorspace);
    qDebug() << "gray color profiles" << Common::getColorProfiles(Magick::GRAYColorspace);
    qDebug() << "q" << Common::supportedQuantumDepth();
    qDebug() << "jpeg" << Common::supportsJpeg();
    qDebug() << "png" << Common::supportsPng();
    qDebug() << "tiff" << Common::supportsTiff();
    qDebug() << "lcms" << Common::supportsLcms();
    qDebug() << "hdri" << Common::supportsHdri();
    qDebug() << "openmp" << Common::supportsOpenMP();
    qDebug() << "bzlib" << Common::supportsBzlib();
    qDebug() << "cairo" << Common::supportsCairo();
    qDebug() << "fontconfig" << Common::supportsFontConfig();
    qDebug() << "freetype" << Common::supportsFreeType();
    qDebug() << "jp2" << Common::supportsJP2();
    qDebug() << "lzma" << Common::supportsLzma();
    qDebug() << "openexr" << Common::supportsOpenExr();
    qDebug() << "pangocairo" << Common::supportsPangoCairo();
    qDebug() << "raw" << Common::supportsRaw();
    qDebug() << "rsvg" << Common::supportsRsvg();
    qDebug() << "webp" << Common::supportsWebp();
    qDebug() << "xml" << Common::supportsXml();
    qDebug() << "zlib" << Common::supportsZlib();
#endif
}

Editor::~Editor()
{
    saveSettings();
}

View *Editor::getCurrentView()
{
    QMdiSubWindow *tab = mdi->currentSubWindow();
    if (!tab) { return nullptr; }
    View *view = dynamic_cast<View*>(tab->widget());
    if (!view) { return nullptr; }
    return view;
}

void Editor::setupStyle()
{
    // style app
    qApp->setStyle(QStyleFactory::create("fusion"));

    // TODO (own theme)
    QIcon::setThemeName("Adwaita");
    setWindowIcon(QIcon::fromTheme("applications-graphics"));

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
                          "QMainWindow, QMenu, QDockWidget, QMenuBar, QDialog,"
                          "QPushButton, QSpinBox, QDoubleSpinBox, QLineEdit, QRadioButton"
                          "{ font-size: %1pt; }"
                          "QToolBar { border-color: none; }").arg(CYAN_FONT_SIZE));
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

    setCentralWidget(mdi);
    setStatusBar(mainStatusBar);

    setMenuBar(mainMenu);

    mainMenu->addMenu(fileMenu);
    mainMenu->addMenu(colorMenu);
    mainMenu->addMenu(helpMenu);

    mainToolBar->addWidget(newButton);
    mainToolBar->addAction(openImageAct);
    mainToolBar->addWidget(saveButton);

    viewToolBar->addAction(viewMoveAct);
    viewToolBar->addAction(viewDragAct);
    viewToolBar->addAction(viewDrawAct);

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
    colorMenu->addMenu(colorIntentMenu);
    colorMenu->addAction(blackPointAct);

    populateColorProfileMenu(colorProfileRGBMenu,
                             Magick::sRGBColorspace);
    populateColorProfileMenu(colorProfileCMYKMenu,
                             Magick::CMYKColorspace);
    populateColorProfileMenu(colorProfileGRAYMenu,
                             Magick::GRAYColorspace);
    populateColorIntentMenu();

    QLabel *brushSizeLabel = new QLabel(this);
    brushSizeLabel->setPixmap(QIcon(":/icons/brushsize.png")
                              .pixmap(32, 32));
    brushToolBar->addWidget(brushSizeLabel);
    brushToolBar->addWidget(brushSize);
    brushSize->setOrientation(brushToolBar->orientation());

    /*imageInfoTree = new QTreeWidget(this);
    imageInfoTree->setHeaderLabels(QStringList()<<"Meta"<<"Value");
    QDockWidget *imageInfoDock = new QDockWidget(this);
    imageInfoDock->setObjectName(QString("imageInfoDock"));
    imageInfoDock->setWindowTitle(tr("Information"));
    imageInfoDock->setWidget(imageInfoTree);
    imageInfoDock->hide();
    addDockWidget(Qt::TopDockWidgetArea, imageInfoDock);*/
}

void Editor::setupMenus()
{
    mainMenu = new QMenuBar(this);
    mainMenu->setObjectName(QString("mainMenu"));

    fileMenu = new QMenu(this);
    fileMenu->setTitle(tr("File"));

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
    mainToolBar->setIconSize(QSize(32, 32));

    viewToolBar = new QToolBar(this);
    viewToolBar->setObjectName(QString("viewToolBar"));
    viewToolBar->setWindowTitle(tr("Canvas Tools"));
    viewToolBar->setIconSize(QSize(32, 32));

    colorToolBar = new QToolBar(this);
    colorToolBar->setObjectName(QString("Color Tools"));
    colorToolBar->setWindowTitle(tr("Color Tools"));
    colorToolBar->setIconSize(QSize(32, 32));

    brushToolBar = new QToolBar(this);
    brushToolBar->setObjectName(QString("brushToolBar"));
    brushToolBar->setWindowTitle(tr("Brush Options"));
    brushToolBar->setIconSize(QSize(32, 32));

    addToolBar(Qt::LeftToolBarArea,
               mainToolBar);
    addToolBar(Qt::LeftToolBarArea,
               viewToolBar);
    addToolBar(Qt::LeftToolBarArea,
               colorToolBar);
    addToolBar(Qt::LeftToolBarArea,
               brushToolBar);
}

void Editor::setupWidgets()
{
    mdi = new Mdi(this);
    mdi->setBackground(QBrush(QColor(20, 20, 20)));

    mainStatusBar = new QStatusBar(this);
    mainStatusBar->setObjectName(QString("mainStatusBar"));

    brushSize = new QSlider(this);
    brushSize->setRange(1,256);
    brushSize->setValue(20);
    brushSize->setMaximumWidth(75);
    brushSize->setMaximumHeight(75);
    brushSize->setOrientation(Qt::Horizontal);
}


void Editor::setupActions()
{

    newImageAct = new QAction(this);
    newImageAct->setText(tr("New Image"));

    openImageAct = new QAction(this);
    openImageAct->setText(tr("Open Image"));

    saveImageAct = new QAction(this);
    saveImageAct->setText(tr("Save Image"));

    saveProjectAct = new QAction(this);
    saveProjectAct->setText(tr("Save Project"));

    saveProjectAsAct = new QAction(this);
    saveProjectAsAct->setText(tr("Save Project as ..."));

    newLayerAct = new QAction(this);
    newLayerAct->setText(tr("New Layer"));

    saveLayerAct = new QAction(this);
    saveLayerAct->setText(tr("Save Layer"));

    quitAct = new QAction(this);
    quitAct->setText(tr("Quit"));

    viewMoveAct = new QAction(this);
    viewMoveAct->setText(tr("Move"));
    viewMoveAct->setCheckable(true);
    viewMoveAct->setChecked(false);

    viewDragAct = new QAction(this);
    viewDragAct->setText(tr("Drag"));
    viewDragAct->setCheckable(true);
    viewDragAct->setChecked(true);

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
    newButton->setIconSize(QSize(32, 32));
    newButton->setMenu(newMenu);
    newButton->setPopupMode(QToolButton::InstantPopup);

    saveButton = new QToolButton(this);
    saveButton->setIconSize(QSize(32, 32));
    saveButton->setMenu(saveMenu);
    saveButton->setPopupMode(QToolButton::InstantPopup);

    colorButton = new QToolButton(this);
    colorButton->setIconSize(QSize(32, 32));
    colorButton->setMenu(colorMenu);
    colorButton->setPopupMode(QToolButton::InstantPopup);
}

void Editor::setupColorManagement()
{
    colorPicker = new QtColorPicker(this, -1, true, false);
    colorPicker->setIconSize(QSize(32, 32));
    colorPicker->setStandardColors();

    colorToolBar->addWidget(colorPicker);
    colorToolBar->addWidget(colorButton);

    colorTriangle = new QtColorTriangle(this);
    colorTriangle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QDockWidget *colorDock = new QDockWidget(this);
    colorDock->setWindowIcon(QIcon(":/icons/colors.png"));
    colorDock->setObjectName(QString("colorTriangle"));
    colorDock->setWindowTitle(tr("Triangle"));
    colorDock->setWidget(colorTriangle);
    colorDock->layout()->setContentsMargins(5, 5, 5, 5);
    colorDock->setMinimumHeight(150);
    colorDock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(colorTriangle, SIGNAL(colorChanged(QColor)), colorPicker, SLOT(setCurrentColor(QColor)));
    connect(colorPicker, SIGNAL(colorChanged(QColor)), this, SLOT(handleColorChanged(QColor)));
    connect(colorPicker, SIGNAL(colorChanged(QColor)), colorTriangle, SLOT(setColor(QColor)));

    QDockWidget *colorRGBDock = new QDockWidget(this);
    colorRGBDock->setObjectName(QString("colorRGBDock"));
    colorRGBDock->setWindowTitle(tr("RGB"));
    colorRGBDock->setWindowIcon(QIcon(":/icons/colors.png"));
    //colorRGBDock->setMaximumHeight(100);

    ColorRGB *colorRGB = new ColorRGB(this);
    colorRGBDock->setWidget(colorRGB);
    connect(colorTriangle, SIGNAL(colorChanged(QColor)), colorRGB, SLOT(setColor(QColor)));
    connect(colorRGB, SIGNAL(colorChanged(QColor)), colorTriangle, SLOT(setColor(QColor)));

    QDockWidget *colorCMYKDock = new QDockWidget(this);
    colorCMYKDock->setObjectName(QString("colorCMYKDock"));
    colorCMYKDock->setWindowTitle(tr("CMYK"));
    colorCMYKDock->setWindowIcon(QIcon(":/icons/colors.png"));
    //colorCMYKDock->setMaximumHeight(100);

    ColorCMYK *colorCMYK = new ColorCMYK(this);
    colorCMYKDock->setWidget(colorCMYK);
    connect(colorTriangle, SIGNAL(colorChanged(QColor)), colorCMYK, SLOT(setColor(QColor)));
    connect(colorCMYK, SIGNAL(colorChanged(QColor)), colorTriangle, SLOT(setColor(QColor)));

    QDockWidget *colorHSVDock = new QDockWidget(this);
    colorHSVDock->setObjectName(QString("colorHSVDock"));
    colorHSVDock->setWindowTitle(tr("HSV"));
    colorHSVDock->setWindowIcon(QIcon(":/icons/colors.png"));
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

    connect(quitAct, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(viewMoveAct, SIGNAL(triggered(bool)), this, SLOT(handleSetMoveMode(bool)));
    connect(viewDragAct, SIGNAL(triggered(bool)), this, SLOT(handleSetDragMode(bool)));
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

    connect(brushToolBar, SIGNAL(orientationChanged(Qt::Orientation)), brushSize, SLOT(setOrientation(Qt::Orientation)));
    connect(brushSize, SIGNAL(valueChanged(int)), this, SLOT(handleBrushSize()));
}

void Editor::setupIcons()
{
    newImageAct->setIcon(QIcon(":/icons/image.png"));
    newLayerAct->setIcon(QIcon(":/icons/layer.png"));
    newButton->setIcon(QIcon(":/icons/layer.png"));
    saveButton->setIcon(QIcon(":/icons/save.png"));
    colorButton->setIcon(QIcon(":/icons/colors.png"));
    openImageAct->setIcon(QIcon(":/icons/folder.png"));
    saveImageAct->setIcon(QIcon(":/icons/image.png"));
    saveProjectAct->setIcon(QIcon(":/icons/page_save.png"));
    saveProjectAsAct->setIcon(QIcon(":/icons/page_save.png"));
    saveLayerAct->setIcon(QIcon(":/icons/layer_save.png"));
    quitAct->setIcon(QIcon(":/icons/quit.png"));
    viewMoveAct->setIcon(QIcon(":/icons/move.png"));
    viewDragAct->setIcon(QIcon(":/icons/hand.png"));
    viewDrawAct->setIcon(QIcon(":/icons/paintbrush.png"));
    convertRGBAct->setIcon(QIcon(":/icons/colors.png"));
    convertCMYKAct->setIcon(QIcon(":/icons/colors.png"));
    convertGRAYAct->setIcon(QIcon(":/icons/colors.png"));
    convertAssignAct->setIcon(QIcon(":/icons/colors.png"));
    convertExtractAct->setIcon(QIcon(":/icons/colors.png"));
    colorProfileRGBMenu->setIcon(QIcon(":/icons/colors.png"));
    colorProfileCMYKMenu->setIcon(QIcon(":/icons/colors.png"));
    colorProfileGRAYMenu->setIcon(QIcon(":/icons/colors.png"));
    colorIntentMenu->setIcon(QIcon(":/icons/colors.png"));
    blackPointAct->setIcon(QIcon(":/icons/colors.png"));
}

void Editor::setupShortcuts()
{
    newImageAct->setShortcut(QKeySequence(tr("Ctrl+N")));
    newLayerAct->setShortcut(QKeySequence(tr("Ctrl+L")));
    openImageAct->setShortcut(QKeySequence(tr("Ctrl+O")));
    quitAct->setShortcut(QKeySequence(tr("Ctrl+Q")));
}

void Editor::populateColorProfileMenu(QMenu *menu,
                                      Magick::ColorspaceType colorspace)
{
    menu->clear();
    QMapIterator<QString, QString> i(Common::getColorProfiles(colorspace));
    while (i.hasNext()) {
        i.next();
        QAction *action = new QAction(menu);
        action->setIcon(QIcon(":/icons/colors.png"));
        action->setText(i.key());
        action->setData(i.value());
        action->setCheckable(true);
        menu->addAction(action);
        connect(action,
                SIGNAL(triggered()),
                this,
                SLOT(selectDefaultColorProfile()));
    }
}

void Editor::selectDefaultColorProfile()
{
    QAction *action = dynamic_cast<QAction*>(sender());
    if (!action) { return; }

    QMenu *menu = dynamic_cast<QMenu*>(action->parent());
    if (!menu) { return; }

    for (int i=0;i<menu->actions().size();++i) {
        QAction *otherAction = menu->actions().at(i);
        if (!otherAction) { continue; }
        otherAction->setChecked(false);
    }
    action->setChecked(true);

    QSettings settings;
    settings.beginGroup(QString("color"));
    if (menu->objectName() == QString("colorProfileRGBMenu")) {
        settings.setValue(QString("rgb_profile"),
                          action->data().toString());
    } else if (menu->objectName() == QString("colorProfileCMYKMenu")) {
        settings.setValue(QString("cmyk_profile"),
                          action->data().toString());
    } else if (menu->objectName() == QString("colorProfileGRAYMenu")) {
        settings.setValue(QString("gray_profile"),
                          action->data().toString());
    }
    settings.endGroup();
    settings.sync();
}

void Editor::setDefaultColorProfiles(QMenu *menu)
{
    QSettings settings;
    settings.beginGroup(QString("color"));

    if (menu->actions().size()==0) {
        if (menu->objectName() == QString("colorProfileRGBMenu")) {
            populateColorProfileMenu(menu,
                                     Magick::sRGBColorspace);
        } else if (menu->objectName() == QString("colorProfileCMYKMenu")) {
            populateColorProfileMenu(menu,
                                     Magick::CMYKColorspace);
        } else if (menu->objectName() == QString("colorProfileGRAYMenu")) {
            populateColorProfileMenu(menu,
                                     Magick::GRAYColorspace);
        }
    }

    if (menu->objectName() == QString("colorProfileRGBMenu")) {
        if (settings.value(QString("rgb_profile")).isValid()) {
            setDefaultColorProfileFromFilename(menu,
                                               settings.value(QString("rgb_profile"))
                                               .toString());
        } else {
            setDefaultColorProfileFromTitle(menu,
                                            QString("sRGB (built-in)"));
        }
    } else if (menu->objectName() == QString("colorProfileCMYKMenu")) {
        if (settings.value(QString("cmyk_profile")).isValid()) {
            setDefaultColorProfileFromFilename(menu,
                                               settings.value(QString("cmyk_profile"))
                                               .toString());
        } else {
            setDefaultColorProfileFromTitle(menu,
                                            QString("ISO Coated v2 (built-in)"));
        }
    } else if (menu->objectName() == QString("colorProfileGRAYMenu")) {
        if (settings.value(QString("gray_profile")).isValid()) {
            setDefaultColorProfileFromFilename(menu,
                                               settings.value(QString("gray_profile"))
                                               .toString());
        } else {
            setDefaultColorProfileFromTitle(menu,
                                            QString("ISO Coated v2 - GREY 1c - (built-in)"));
        }
    }

    settings.endGroup();
}

void Editor::setDefaultColorProfileFromFilename(QMenu *menu,
                                                const QString &filename)
{
    for (int i=0;i<menu->actions().size();++i) {
        QAction *action = menu->actions().at(i);
        if (!action) { continue; }
        if (action->data().toString() == filename) {
            action->setChecked(true);
        } else { action->setChecked(false); }
    }
}

void Editor::setDefaultColorProfileFromTitle(QMenu *menu,
                                             const QString &title)
{
    for (int i=0;i<menu->actions().size();++i) {
        QAction *action = menu->actions().at(i);
        if (!action) { continue; }
        if (action->text() == title) {
            action->setChecked(true);
        } else { action->setChecked(false); }
    }
}

const QString Editor::selectedDefaultColorProfile(QMenu *menu)
{
    for (int i=0;i<menu->actions().size();++i) {
        QAction *action = menu->actions().at(i);
        if (!action) { continue; }
        //qDebug() << "ACT" << action->data().toString() << action->isChecked();
        if (action->isChecked()) { return action->data().toString(); }
    }
    return QString();
}

Magick::Blob Editor::selectedDefaultColorProfileData(QMenu *menu)
{
    QString filename;
    for (int i=0;i<menu->actions().size();++i) {
        QAction *action = menu->actions().at(i);
        if (!action) { continue; }
        if (action->isChecked()) { filename =  action->data().toString(); }
    }
    if (!filename.isEmpty()) {
        try {
            Magick::Image image;
            image.read(filename.toStdString());
            Magick::Blob profile;
            image.write(&profile);
            return profile;
        }
        catch(Magick::Error &error_ ) { emit errorMessage(error_.what()); }
        catch(Magick::Warning &warn_ ) { emit warningMessage(warn_.what()); }
    }
    return Magick::Blob();
}

void Editor::populateColorIntentMenu()
{
    QAction *action1 = new QAction(colorIntentMenu);
    action1->setText(tr("Undefined"));
    action1->setData(Common::UndefinedRenderingIntent);
    action1->setCheckable(true);
    connect(action1,
            SIGNAL(triggered()),
            this,
            SLOT(setDefaultColorIntent()));

    QAction *action2 = new QAction(colorIntentMenu);
    action2->setText(tr("Saturation"));
    action2->setData(Common::SaturationRenderingIntent);
    action2->setCheckable(true);
    connect(action2,
            SIGNAL(triggered()),
            this,
            SLOT(setDefaultColorIntent()));

    QAction *action3 = new QAction(colorIntentMenu);
    action3->setText(tr("Perceptual"));
    action3->setData(Common::PerceptualRenderingIntent);
    action3->setCheckable(true);
    connect(action3,
            SIGNAL(triggered()),
            this,
            SLOT(setDefaultColorIntent()));

    QAction *action4 = new QAction(colorIntentMenu);
    action4->setText(tr("Absolute"));
    action4->setData(Common::AbsoluteRenderingIntent);
    action4->setCheckable(true);
    connect(action4,
            SIGNAL(triggered()),
            this,
            SLOT(setDefaultColorIntent()));

    QAction *action5 = new QAction(colorIntentMenu);
    action5->setText(tr("Relative"));
    action5->setData(Common::RelativeRenderingIntent);
    action5->setCheckable(true);
    connect(action5,
            SIGNAL(triggered()),
            this,
            SLOT(setDefaultColorIntent()));

    colorIntentMenu->addAction(action1);
    colorIntentMenu->addAction(action2);
    colorIntentMenu->addAction(action3);
    colorIntentMenu->addAction(action4);
    colorIntentMenu->addAction(action5);
}

void Editor::setDefaultColorIntent()
{
    QAction *action = dynamic_cast<QAction*>(sender());
    if (!action) { return; }

    for (int i=0;i<colorIntentMenu->actions().size();++i) {
        QAction *otherAction = colorIntentMenu->actions().at(i);
        if (!otherAction) { continue; }
        otherAction->setChecked(false);
    }
    action->setChecked(true);

    QSettings settings;
    settings.beginGroup(QString("color"));
    settings.setValue(QString("intent"),
                      action->data().toInt());
    settings.endGroup();
    settings.sync();
}

void Editor::loadDefaultColorIntent()
{
    QSettings settings;
    settings.beginGroup(QString("color"));
    for (int i=0;i<colorIntentMenu->actions().size();++i) {
        QAction *action = colorIntentMenu->actions().at(i);
        if (!action) { continue; }
        if (action->data().toInt() == settings.value(QString("intent"),
                                                     Common::PerceptualRenderingIntent).toInt())
        {
            action->setChecked(true);
        } else { action->setChecked(false); }
    }
    settings.endGroup();
}

void Editor::handleColorConvertRGB(bool ignoreColor, const QString &title)
{
    handleColorConvert(ignoreColor,
                       Magick::sRGBColorspace,
                       title);
}

void Editor::handleColorConvertCMYK(bool ignoreColor, const QString &title)
{
    handleColorConvert(ignoreColor,
                       Magick::CMYKColorspace,
                       title);
}

void Editor::handleColorConvertGRAY(bool ignoreColor, const QString &title)
{
    handleColorConvert(ignoreColor,
                       Magick::GRAYColorspace,
                       title);
}

void Editor::handleColorProfileAssign()
{
    if (!getCurrentView()) { return; }
    switch(getCurrentView()->getCanvas().colorSpace()) {
    case Magick::CMYKColorspace:
        handleColorConvertCMYK(true,
                               tr("Assign Color Profile"));
        break;
    case Magick::GRAYColorspace:
        handleColorConvertGRAY(true,
                               tr("Assign Color Profile"));
        break;
    default:
        handleColorConvertRGB(true,
                              tr("Assign Color Profile"));
    }
}

void Editor::handleColorConvert(bool ignoreColor,
                                Magick::ColorspaceType colorspace,
                                const QString &title)
{
    if (!getCurrentView()) { return; }
    if (getCurrentView()->getCanvas().colorSpace() == colorspace && !ignoreColor) {
        emit statusMessage(tr("Already the requested colorspace"));
        return;
    }
    QString profile;
    switch(colorspace) {
    case Magick::CMYKColorspace:
        profile = selectedDefaultColorProfile(colorProfileCMYKMenu);
        break;
    case Magick::GRAYColorspace:
        profile = selectedDefaultColorProfile(colorProfileGRAYMenu);
        break;
    default:
        profile = selectedDefaultColorProfile(colorProfileRGBMenu);
    }
    ConvertDialog *dialog = new ConvertDialog(this,
                                              title,
                                              profile,
                                              colorspace);
    int ret = dialog->exec();
    if (ret == QDialog::Accepted &&
        !dialog->getProfile().isEmpty())
    {
        qDebug() << "CONVERT USING" << dialog->getProfile();
        Common::Canvas canvas = getCurrentView()->getCanvasProject();
        canvas.image = Common::convertColorspace(canvas.image,
                                                 canvas.profile,
                                                 dialog->getProfile());
        for (int i=0;i<canvas.layers.size();++i) {
            canvas.layers[i].image= Common::convertColorspace(canvas.layers[i].image,
                                                              canvas.profile,
                                                              dialog->getProfile());
        }
        getCurrentView()->updateCanvas(canvas);
        updateTabTitle(getCurrentView());
    }

    QTimer::singleShot(100,
                       dialog,
                       SLOT(deleteLater()));
}

void Editor::saveSettings()
{
    emit statusMessage(tr("Saving settings ..."));
    QSettings settings;

    settings.beginGroup("engine");
    settings.setValue("disk_limit",
                      Common::getDiskResource());
    settings.setValue("memory_limit",
                      Common::getMemoryResource());
    //settings.setValue("thread_limit",
                      //Magick::ResourceLimits::thread());
    settings.endGroup();

    settings.beginGroup("gui");
    settings.setValue("editor_state",
                      saveState());
    settings.setValue("editor_geometry",
                      saveGeometry());
    settings.setValue("editor_maximized",
                      isMaximized());
    /*settings.setValue("infotree_header_state",
                      imageInfoTree->header()->saveState());
    settings.setValue("infotree_header_geometry",
                      imageInfoTree->header()->saveGeometry());*/
    settings.endGroup();

    settings.beginGroup(QString("color"));
    settings.setValue(QString("rgb_profile"),
                      selectedDefaultColorProfile(colorProfileRGBMenu));
    settings.setValue(QString("cmyk_profile"),
                      selectedDefaultColorProfile(colorProfileCMYKMenu));
    settings.setValue(QString("gray_profile"),
                      selectedDefaultColorProfile(colorProfileGRAYMenu));
    settings.setValue(QString("blackpoint"),
                      blackPointAct->isChecked());
    settings.endGroup();
}

void Editor::loadSettings()
{
    emit statusMessage(tr("Loading settings ..."));
    QSettings settings;

    settings.beginGroup("engine");
    Common::setDiskResource(settings
                            .value("disk_limit", 0).toInt());
    Common::setMemoryResource(settings
                              .value("memory_limit", 8).toInt());
    //Common::setThreadResources(settings
                               //.value("thread_limit", 0).toInt());
    settings.endGroup();

    settings.beginGroup("gui");
    if (settings.value("editor_state").isValid()) {
        restoreState(settings
                     .value("editor_state").toByteArray());
    }
    if (settings.value("editor_geometry").isValid()) {
        restoreGeometry(settings
                        .value("editor_geometry").toByteArray());
    }
    /*if (settings.value("infotree_header_state").isValid()) {
        imageInfoTree->header()->restoreState(settings
                                              .value("infotree_header_state").toByteArray());
    }
    if (settings.value("infotree_header_geometry").isValid()) {
        imageInfoTree->header()->restoreGeometry(settings
                                                 .value("infotree_header_geometry").toByteArray());
    }*/
    if (settings.value("editor_maximized").toBool()) { showMaximized(); }
    settings.endGroup();

    brushSize->setOrientation(brushToolBar->orientation());

    emit statusMessage(tr("Engine disk cache limit: %1 GB")
                       .arg(Common::getDiskResource()));
    emit statusMessage(tr("Engine memory limit: %1 GB")
                       .arg(Common::getMemoryResource()));

    setDefaultColorProfiles(colorProfileRGBMenu);
    setDefaultColorProfiles(colorProfileCMYKMenu);
    setDefaultColorProfiles(colorProfileGRAYMenu);
    loadDefaultColorIntent();
    settings.beginGroup(QString("color"));
    blackPointAct->setChecked(settings.value(QString("blackpoint"),
                                             true)
                              .toBool());
    settings.endGroup();
}

void Editor::loadProject(const QString &filename)
{
    if (filename.isEmpty()) { return; }
    QFileInfo fileInfo(filename);
    if (fileInfo.suffix().toLower() != "miff") { return; }
    if (Common::isValidCanvas(filename)) {
        Common::Canvas canvas = Common::readCanvas(filename);
        newTab(canvas);
    }
}

void Editor::saveProject(const QString &filename)
{
    if (filename.isEmpty() || !getCurrentView()) { return; }
    qDebug() << "save project" << filename;

    if (Common::writeCanvas(getCurrentView()->getCanvasProject(), filename)) {
        /*Common::Canvas canvas = Common::readCanvas(filename);
        qDebug() << "====> canvas" << canvas.label << canvas.layers.size();
        newTab(canvas);
        //view->loadCanvas(canvas);
        mdi->tileSubWindows();*/
    } else {
        QMessageBox::warning(this,
                             tr("Cyan Error"),
                             tr("Failed to save the project!"));
    }
}

void Editor::saveImage(QString filename)
{
    qDebug() << "save image" << filename;
}

void Editor::loadImage(QString filename)
{
    if (filename.isEmpty()) { return; }
    if (Common::isValidCanvas(filename)) {
        emit statusMessage(tr("Loading canvas %1").arg(filename));
        loadProject(filename);
        emit statusMessage(tr("Done"));
    } else {
        qDebug() << "HAS LAYERS?" << Common::hasLayers(filename);
        emit statusMessage(tr("Loading image %1").arg(filename));
        readImage(filename);
        emit statusMessage(tr("Done"));
    }
}

void Editor::readImage(QString filename)
{
    if (filename.isEmpty()) { return; }
    Magick::Image image;
    image.quiet(false);

    try { image.read(filename.toStdString()); }
    catch(Magick::Error &error_ ) {
        emit errorMessage(error_.what());
        return;
    }
    catch(Magick::Warning &warn_ ) { emit warningMessage(warn_.what()); }

    try {
        image.magick("MIFF");
        image.fileName(filename.toStdString());
        if (image.label().empty()) {
            QFileInfo fileInfo(filename);
            image.label(fileInfo.baseName().toStdString());
        }

        if (image.iccColorProfile().length()==0) {
            QString defPro;
            switch(image.colorSpace()) {
            case Magick::CMYKColorspace:
                defPro = selectedDefaultColorProfile(colorProfileCMYKMenu);
                break;
            case Magick::GRAYColorspace:
                defPro = selectedDefaultColorProfile(colorProfileGRAYMenu);
                break;
            default:
                defPro = selectedDefaultColorProfile(colorProfileRGBMenu);
            }
            ConvertDialog *dialog = new ConvertDialog(this,
                                                      tr("Assign Color Profile"),
                                                      defPro,
                                                      image.colorSpace());
            int ret = dialog->exec();
            if (ret == QDialog::Accepted &&
                !dialog->getProfile().isEmpty())
            {
                Magick::Blob profile;
                Magick::Image input;
                input.read(dialog->getProfile().toStdString());
                input.write(&profile);
                image = Common::convertColorspace(image,
                                                  Magick::Blob(),
                                                  profile);
                if (image.columns()>0 &&
                    image.rows()>0 &&
                    image.iccColorProfile().length()>0)
                {
                    newTab(image);
                }
            }
            QTimer::singleShot(100,
                               dialog,
                               SLOT(deleteLater()));
        } else {
            if (image.columns()>0 &&
                image.rows()>0)
            {
                newTab(image);
            }
        }
    }
    catch(Magick::Error &error_ ) { emit errorMessage(error_.what()); }
    catch(Magick::Warning &warn_ ) { emit warningMessage(warn_.what()); }
}

void Editor::saveProjectDialog()
{
    if (!getCurrentView()) { return; }
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save Project"),
                                                    QDir::homePath(),
                                                    tr("Project Files (*.miff)"));
    if (!filename.endsWith(".miff",
                           Qt::CaseInsensitive)) { filename.append(".miff"); }
    saveProject(filename);
}

void Editor::saveImageDialog()
{
    qDebug() << "save image dialog";
}

void Editor::loadImageDialog()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open Media"),
                                                    QDir::homePath(),
                                                    tr("Media Files (%1)")
                                                    .arg(Common::supportedReadFormats()));
    if (!filename.isEmpty()) { loadImage(filename); }
}

void Editor::newImageDialog()
{
    NewMediaDialog *dialog = new NewMediaDialog(this);
    int res =  dialog->exec();
    if (res == QDialog::Accepted) { newTab(dialog->getImage()); }
    QTimer::singleShot(100,
                       dialog,
                       SLOT(deleteLater()));
}

void Editor::newLayerDialog()
{
    if (!getCurrentView()) { return; }
    NewMediaDialog *dialog = new NewMediaDialog(this,
                                                tr("New Layer"),
                                                Common::newLayerDialogType,
                                                getCurrentView()->getCanvas().colorSpace(),
                                                getCurrentView()->getCanvasSize());
    int res =  dialog->exec();
    if (res == QDialog::Accepted) {
        getCurrentView()->addLayer(dialog->getImage());
    }

    QTimer::singleShot(100,
                       dialog,
                       SLOT(deleteLater()));
}

void Editor::handleNewImage(Magick::Image image)
{
    if (image.columns()>0 && image.rows()>0) { newTab(image); }
}

void Editor::handleError(const QString &message)
{
    qWarning() << "error" << message;
    mainStatusBar->showMessage(message, 6000);
    QMessageBox::warning(this,
                         tr("Cyan Error"),
                         message);
}

void Editor::handleWarning(const QString &message)
{
    qWarning() << "warning" << message;
    mainStatusBar->showMessage(message);
    QMessageBox::warning(this,
                         tr("Cyan Warning"),
                         message);
}

void Editor::handleStatus(const QString &message)
{
    qWarning() << "status" << message;
    mainStatusBar->showMessage(message,
                               6000);
}

void Editor::newTab(Common::Canvas canvas)
{
    qDebug() << "new tab from canvas/project";
    QMdiSubWindow *tab = new QMdiSubWindow(mdi);
    tab->setAttribute(Qt::WA_DeleteOnClose);

    View *view = new View(tab);

    /*connect(view, SIGNAL(selectedLayer(int)), this, SLOT(handleLayerSelected(int)));
    connect(view, SIGNAL(errorMessage(QString)), this, SLOT(handleError(QString)));
    connect(view, SIGNAL(statusMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(view, SIGNAL(warningMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(view, SIGNAL(viewClosed()), this, SLOT(handleViewClosed()));
    connect(view, SIGNAL(updatedLayers()), this, SLOT(handleLayersUpdated()));
    connect(view, SIGNAL(switchMoveTool()), this, SLOT(handleSwitchMoveTool()));
    connect(view, SIGNAL(updatedBrushStroke(int)), this, SLOT(handleUpdateBrushSize(int)));
    connect(view, SIGNAL(openImages(QList<QUrl>)), this, SLOT(handleOpenImages(QList<QUrl>)));
    connect(layersTree, SIGNAL(moveLayerEvent(QKeyEvent*)), view, SLOT(moveLayerEvent(QKeyEvent*)));*/
    connectView(view);

    view->setCanvasSpecsFromImage(canvas.image);
    view->setLayersFromCanvas(canvas);
    view->setFit(true);
    view->setBrushColor(colorPicker->currentColor());

    tab->setWidget(view);
    tab->showMaximized();

    /*if (viewMoveAct->isChecked()) {
        view->setInteractiveMode(View::IteractiveMoveMode);
    } else if (viewDragAct->isChecked()) {
        view->setInteractiveMode(View::InteractiveDragMode);
    } else if (viewDrawAct->isChecked()) {
        view->setInteractiveMode(View::InteractiveDrawMode);
    }*/
    setViewTool(view);
    updateTabTitle(view);
    handleTabActivated(tab);
}

void Editor::newTab(Magick::Image image, QSize geo)
{
    qDebug() << "new tab from image";
    //if (!image.isValid()) { return; }
    /*if ((image.columns()==0 || image.rows() == 0) && geo.width()==0) {
        emit statusMessage(tr("Invalid image/size"));
        return new View(this);
    }*/

    QMdiSubWindow *tab = new QMdiSubWindow(mdi);
    tab->setAttribute(Qt::WA_DeleteOnClose);

    View *view = new View(tab);

    /*connect(view, SIGNAL(selectedLayer(int)), this, SLOT(handleLayerSelected(int)));
    connect(view, SIGNAL(errorMessage(QString)), this, SLOT(handleError(QString)));
    connect(view, SIGNAL(statusMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(view, SIGNAL(warningMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(view, SIGNAL(viewClosed()), this, SLOT(handleViewClosed()));
    connect(view, SIGNAL(updatedLayers()), this, SLOT(handleLayersUpdated()));
    connect(view, SIGNAL(switchMoveTool()), this, SLOT(handleSwitchMoveTool()));
    connect(view, SIGNAL(updatedBrushStroke(int)), this, SLOT(handleUpdateBrushSize(int)));
    connect(view, SIGNAL(openImages(QList<QUrl>)), this, SLOT(handleOpenImages(QList<QUrl>)));
    connect(layersTree, SIGNAL(moveLayerEvent(QKeyEvent*)), view, SLOT(moveLayerEvent(QKeyEvent*)));*/
    connectView(view);

    if (geo.width()>0) {
        view->setupCanvas(geo.width(), geo.height());
    } else if (image.columns()>0) {
        view->setCanvasSpecsFromImage(image);
    }
    if (geo.width()>0) { view->addLayer(view->getCanvas()); }
    else { view->addLayer(image); }
    view->setFit(true);
    view->setBrushColor(colorPicker->currentColor());

    tab->setWidget(view);
    tab->showMaximized();

    /*if (viewMoveAct->isChecked()) {
        view->setInteractiveMode(View::IteractiveMoveMode);
    } else if (viewDragAct->isChecked()) {
        view->setInteractiveMode(View::InteractiveDragMode);
    } else if (viewDrawAct->isChecked()) {
        view->setInteractiveMode(View::InteractiveDrawMode);
    }*/
    setViewTool(view);
    updateTabTitle(view);
    handleTabActivated(tab);
    //return view;
}

void Editor::connectView(View *view)
{
    if (!view) { return; }
    qDebug() << "connect new view";
    connect(view, SIGNAL(selectedLayer(int)), this, SLOT(handleLayerSelected(int)));
    connect(view, SIGNAL(errorMessage(QString)), this, SLOT(handleError(QString)));
    connect(view, SIGNAL(statusMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(view, SIGNAL(warningMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(view, SIGNAL(viewClosed()), this, SLOT(handleViewClosed()));
    connect(view, SIGNAL(updatedLayers()), this, SLOT(handleLayersUpdated()));
    connect(view, SIGNAL(switchMoveTool()), this, SLOT(handleSwitchMoveTool()));
    connect(view, SIGNAL(updatedBrushStroke(int)), this, SLOT(handleUpdateBrushSize(int)));
    connect(view, SIGNAL(openImages(QList<QUrl>)), this, SLOT(handleOpenImages(QList<QUrl>)));
    connect(layersTree, SIGNAL(moveLayerEvent(QKeyEvent*)), view, SLOT(moveLayerEvent(QKeyEvent*)));
}

void Editor::setViewTool(View *view)
{
    if (!view) { return; }
    qDebug() << "set view tool";
    if (viewMoveAct->isChecked()) {
        view->setInteractiveMode(View::IteractiveMoveMode);
    } else if (viewDragAct->isChecked()) {
        view->setInteractiveMode(View::InteractiveDragMode);
    } else if (viewDrawAct->isChecked()) {
        view->setInteractiveMode(View::InteractiveDrawMode);
    }
}

// TODO
/*void Editor::newTabFromLayer(View *parentView,
                             int layerID)
{
    //qDebug() << "new tab from layer" << layerID;
    return;


    if (!parentView) { return; }
    Common::Layer layer  = parentView->getLayer(layerID);
    QMdiSubWindow *tab = new QMdiSubWindow(mdi);
    View *view = new View(this, layerID);

    connect(view, SIGNAL(selectedLayer(int)), this, SLOT(handleLayerSelected(int)));
    connect(view, SIGNAL(errorMessage(QString)), this, SLOT(handleError(QString)));
    connect(view, SIGNAL(statusMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(view, SIGNAL(warningMessage(QString)), this, SLOT(handleStatus(QString)));
    connect(view, SIGNAL(viewClosed()), this, SLOT(handleViewClosed()));
    connect(view, SIGNAL(updatedLayers()), this, SLOT(handleLayersUpdated()));
    //connect(view, SIGNAL(updatedCanvas(View::Canvas,int)), parentView, SLOT(setLayerFromCanvas(View::Canvas,int)));
    connect(view, SIGNAL(switchMoveTool()), this, SLOT(handleSwitchMoveTool()));
    connect(view, SIGNAL(updateBrushSize(bool)), this, SLOT(handleUpdateBrushSize(bool)));

    tab->setWidget(view);
    tab->setWindowTitle(QString::fromStdString(layer.image.fileName()).split("/").takeLast());
    tab->setAttribute(Qt::WA_DeleteOnClose);

    //tab->showNormal();
    tab->showMaximized();
    //mdi->tileSubWindows();

    view->setCanvasSpecsFromImage(layer.image);
    view->addLayer(layer.image);
    view->setFit(true);
    if (viewMoveAct->isChecked()) {
        view->setInteractiveMode(View::IteractiveMoveMode);
    } else if (viewDragAct->isChecked()) {
        view->setInteractiveMode(View::InteractiveDragMode);
    } else if (viewDrawAct->isChecked()) {
        view->setInteractiveMode(View::InteractiveDrawMode);
    }
    updateTabTitle(view);
    //parseImageInfo(view->getCanvas());
}*/

void Editor::handleLayerCompChanged(const QString &comp)
{
    if (comp.isEmpty()) { return; }
    LayerTreeItem *layer = dynamic_cast<LayerTreeItem*>(layersTree->currentItem());
    QMdiSubWindow *tab = mdi->currentSubWindow();
    if (!layer || !tab) { return; }
    View *view = qobject_cast<View*>(tab->widget());
    if (!view) { return; }
    QMapIterator<Magick::CompositeOperator, QString> i(Common::compositeModes());
    while (i.hasNext()) {
        i.next();
        if (i.value() == comp) {
            layer->setComposite(i.key());
            view->setLayerComposite(layer->getLayerID(), i.key());
            break;
        }
    }
}

void Editor::populateCompBox()
{
    layersComp->clear();

    /*
    gimp modes in cyan:
    normal - ok
    dissolve - ok
    lighten only - ok
    screen - ok
    dodge (color) - ok
    addition (plus) - ok
    darken only - ok
    multiply - ok
    burn (color) - ok
    overlay - ok
    soft light - ok
    hard light - ok
    difference - ok
    substract (minus src) - ok
    grain * - https://www.imagemagick.org/discourse-server/viewtopic.php?t=25085
    divide (src) - ok
    hue - ok
    saturation - ok
    color - ??? https://docs.gimp.org/en/gimp-concepts-layer-modes.html
    value - ??? https://docs.gimp.org/en/gimp-concepts-layer-modes.html
    */

    layersComp->addItem(Common::compositeModes()[MagickCore::OverCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::DissolveCompositeOp]);

    layersComp->insertSeparator(layersComp->count());

    layersComp->addItem(Common::compositeModes()[MagickCore::PlusCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::MultiplyCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::OverlayCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::ScreenCompositeOp]);

    layersComp->insertSeparator(layersComp->count());

    layersComp->addItem(Common::compositeModes()[MagickCore::ColorDodgeCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::LinearDodgeCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::ColorBurnCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::LinearBurnCompositeOp]);

    layersComp->insertSeparator(layersComp->count());

    layersComp->addItem(Common::compositeModes()[MagickCore::LightenCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::DarkenCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::LightenIntensityCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::DarkenIntensityCompositeOp]);

    layersComp->insertSeparator(layersComp->count());

    layersComp->addItem(Common::compositeModes()[MagickCore::SoftLightCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::HardLightCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::VividLightCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::PegtopLightCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::PinLightCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::LinearLightCompositeOp]);

    layersComp->insertSeparator(layersComp->count());

    layersComp->addItem(Common::compositeModes()[MagickCore::DifferenceCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::MinusSrcCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::DivideSrcCompositeOp]);

    layersComp->insertSeparator(layersComp->count());

    layersComp->addItem(Common::compositeModes()[MagickCore::HueCompositeOp]);
    layersComp->addItem(Common::compositeModes()[MagickCore::SaturateCompositeOp]);

    layersComp->insertSeparator(layersComp->count());

    QMapIterator<Magick::CompositeOperator, QString> i(Common::compositeModes());
    while (i.hasNext()) {
        i.next();
        if (layersComp->findText(i.value(),
                                  Qt::MatchExactly |
                                  Qt::MatchCaseSensitive)==-1)
        {
            layersComp->addItem(i.value());
        }
    }

    layersComp->setCurrentIndex(layersComp->findText(QString("Normal")));
}

void Editor::handleLayerActivated(QTreeWidgetItem *item, int col)
{
    Q_UNUSED(col)
    handleLayerActivated(item, item);
}

void Editor::handleLayerActivated(QTreeWidgetItem *item, QTreeWidgetItem *old)
{
    Q_UNUSED(old)
    LayerTreeItem *layer = dynamic_cast<LayerTreeItem*>(item);
    if (!layer) { return; }
    layersComp->setCurrentIndex(layersComp->findText(Common::compositeModes()[layer->getComposite()]));
    //qDebug() << "???" << layer->getOpacity() << layer->getOpacity()*100;
    layersOpacity->setValue(qRound(layer->getOpacity()*100));
}

// TODO
void Editor::handleLayerDoubleclicked(QTreeWidgetItem *item, int col)
{
    qDebug() << "layer tree item double clicked";
    Q_UNUSED(col)
    LayerTreeItem *layer = dynamic_cast<LayerTreeItem*>(item);
    View *view = qobject_cast<View*>(getCurrentView());
    if (!layer || !view) { return; }
    //newTabFromLayer(view, layer->getLayerID());
}

void Editor::handleLayerSelected(int layer)
{
    for (int i=0;i<layersTree->topLevelItemCount();++i) {
        LayerTreeItem *item = dynamic_cast<LayerTreeItem*>(layersTree->topLevelItem(i));
        if (!item) { continue; }
        if (layer == item->getLayerID()) {
            layersTree->setCurrentItem(item);
            handleLayerActivated(item, 0);
            return;
        }
    }
}

void Editor::handleLayersOpacity()
{
    double value = layersOpacity->value();
    LayerTreeItem *layer = dynamic_cast<LayerTreeItem*>(layersTree->currentItem());
    if (!layer) { return; }

    qDebug() << "set layer opacity" << value << value/100;
    QMdiSubWindow *tab = mdi->currentSubWindow();
    if (!layer || !tab) { return; }
    View *view = qobject_cast<View*>(tab->widget());
    if (!view) { return; }
    layer->setOpacity(value/100);
    view->setLayerOpacity(layer->getLayerID(), value/100);
}

void Editor::handleViewClosed()
{
    qDebug() << "view closed";
    layersTree->clear();
    layersTree->handleTabActivated(mdi->currentSubWindow());
}

void Editor::handleLayersUpdated()
{
    layersTree->handleTabActivated(mdi->currentSubWindow());
}

void Editor::handleSetDragMode(bool triggered)
{
    qDebug() << "set drag mode" << triggered;
    if (!triggered) {
        viewDragAct->setChecked(true);
        return;
    }
    handleSwitchMoveTool(View::InteractiveDragMode);
}

void Editor::handleSetMoveMode(bool triggered)
{
    qDebug() << "set move mode" << triggered;
    if (!triggered) {
        viewMoveAct->setChecked(true);
        return;
    }
    handleSwitchMoveTool(View::IteractiveMoveMode);
}

void Editor::handleTabActivated(QMdiSubWindow *tab)
{
    qDebug() << "handle tab activated";
    if (!tab) { return; }
    View *view = dynamic_cast<View*>(tab->widget());
    if (!view) { return; }

    if (viewDragAct->isChecked()) {
        view->setDragMode(QGraphicsView::ScrollHandDrag);
        view->setInteractive(false);
    } else {
        view->setDragMode(QGraphicsView::NoDrag);
        view->setInteractive(true);
    }
    updateTabTitle();
    handleBrushSize();
}

void Editor::updateTabTitle(View *view)
{
    if (!view) { view = qobject_cast<View*>(getCurrentView()); }
    if (!view) { return; }
    QString title = Common::canvasWindowTitle(view->getCanvas());
    qDebug() << "update canvas title" << title;
    view->setWindowTitle(title);
}

void Editor::handleSwitchMoveTool(View::InteractiveMode tool)
{
    qDebug() << "handle switch view tool" << tool;
    View::InteractiveMode mode = View::InteractiveNoMode;
    if (tool == View::InteractiveNoMode) {
        if (viewMoveAct->isChecked()) {
            // drag
            viewMoveAct->setChecked(false);
            viewDragAct->setChecked(true);
            viewDrawAct->setChecked(false);
            mode = View::InteractiveDragMode;
        } else if (viewDragAct->isChecked()) {
            // draw
            viewMoveAct->setChecked(false);
            viewDragAct->setChecked(false);
            viewDrawAct->setChecked(true);
            mode = View::InteractiveDrawMode;
        } else if (viewDrawAct->isChecked()) {
            // move
            viewMoveAct->setChecked(true);
            viewDragAct->setChecked(false);
            viewDrawAct->setChecked(false);
            mode = View::IteractiveMoveMode;
        }
    } else {
        mode = tool;
        switch(mode) {
        case View::InteractiveDragMode:
            viewMoveAct->setChecked(false);
            viewDragAct->setChecked(true);
            viewDrawAct->setChecked(false);
            break;
        case View::InteractiveDrawMode:
            viewMoveAct->setChecked(false);
            viewDragAct->setChecked(false);
            viewDrawAct->setChecked(true);
            break;
        case View::IteractiveMoveMode:
            viewMoveAct->setChecked(true);
            viewDragAct->setChecked(false);
            viewDrawAct->setChecked(false);
            break;
        default:;
        }
    }
    if (!getCurrentView()) { return; }
    getCurrentView()->setInteractiveMode(mode);
}

void Editor::handleSetDrawMode(bool triggered)
{
    qDebug() << "set draw mode" << triggered;
    if (!triggered) {
        viewDrawAct->setChecked(true);
        return;
    }
    handleSwitchMoveTool(View::InteractiveDrawMode);
}

void Editor::handleBrushSize()
{
    qDebug() << "handle update brush size";
    QList<QMdiSubWindow*> list = mdi->subWindowList();
    for (int i=0;i<list.size();++i) {
        QMdiSubWindow *window = dynamic_cast<QMdiSubWindow*>(list.at(i));
        if (!window) { return; }
        View *view = dynamic_cast<View*>(window->widget());
        if (!view) { return; }
        view->setBrushStroke(brushSize->value());
    }
}

void Editor::handleUpdateBrushSize(int stroke)
{
    qDebug() << "update brush size" << stroke;
    brushSize->setValue(stroke);
}

void Editor::handleOpenImages(const QList<QUrl> urls)
{
    qDebug() << "open images" << urls;
    if (urls.size()==0) { return; }

    for (int i=0;i<urls.size();++i) {
        readImage(urls.at(i).toLocalFile());
    }
    if (urls.size()>1) {
        mdi->tileSubWindows();
    }
}

void Editor::aboutImageMagick()
{
    QMessageBox box(this);
    box.setWindowTitle(tr("About ImageMagick"));

    Magick::Image logo;
    logo.read("logo:");
    logo.scale(Magick::Geometry(256, 256));
    logo.magick("BMP");
    Magick::Blob pix;
    logo.write(&pix);
    box.setIconPixmap(QPixmap::fromImage(QImage::fromData(reinterpret_cast<uchar*>(const_cast<void*>(pix.data())),
                                                          static_cast<int>(pix.length()))));

    QString about;
    about.append(QString("<h3>%1 %2%3</h3>")
                 .arg(MagickPackageName)
                 .arg(MagickLibVersionText)
                 .arg(MagickLibAddendum));
    about.append(QString("<p><a href=\"https://imagemagick.org\">ImageMagick®</a> is used to read, create, save, edit, compose, and  convert bitmap images.</p><p>ImageMagick is distributed under the following <a href=\"https://www.imagemagick.org/script/license.php\">license</a>.</p>"));
    about.append(QString("<p>%1</p>").arg(MagickCopyright));

#ifdef QT_DEBUG
    about.append(QString("<p><strong>Features</strong>:<br><br>%1 %2</p>").arg(MagickQuantumDepth).arg(MagickCore::GetMagickFeatures()));
    about.append(QString("<p><strong>Delegates</strong>:<br><br>%1</p>").arg(MagickCore::GetMagickDelegates()));
    about.append(QString("<p><strong>Disk Limit</strong>: %1<br>").arg(Common::humanFileSize(Magick::ResourceLimits::disk())));
    about.append(QString("<strong>Area Limit</strong>: %1<br>").arg(Common::humanFileSize(Magick::ResourceLimits::area(),false, true)));
    about.append(QString("<strong>Map Limit</strong>: %1<br>").arg(Common::humanFileSize(Magick::ResourceLimits::map())));
    about.append(QString("<strong>Memory Limit</strong>: %1<br>").arg(Common::humanFileSize(Magick::ResourceLimits::memory())));
    about.append(QString("<strong>Width Limit</strong>: %1<br>").arg(Common::humanFileSize(Magick::ResourceLimits::width(), true)));
    about.append(QString("<strong>Height Limit</strong>: %1<br>").arg(Common::humanFileSize(Magick::ResourceLimits::height(), true)));
    about.append(QString("<strong>Thread Limit</strong>: %1</p>").arg(Magick::ResourceLimits::thread()));
#endif

    box.setText(about);
    box.exec();
}

void Editor::aboutLcms()
{
    QMessageBox box(this);
    box.setWindowTitle(tr("About Little CMS"));
    box.setIconPixmap(QPixmap::fromImage(QImage(":/icons/lcms_logo.png")));

    QString about;
    about.append(QString("<h3>Little CMS %1</h3>")
                 .arg(QString::number(LCMS_VERSION)
                      .insert(1,".")
                      .remove(2,1)
                      .remove(3,1)));
    about.append(QString("<p><a href=\"http://www.littlecms.com/\">Little CMS</a> is an small-footprint color management engine, with special focus on accuracy and performance.</p><p>Little CMS is distributed under the following <a href=\"https://opensource.org/licenses/mit-license.php\">license</a>.</p>"));
    about.append(QString("<p>Copyright &copy; 2018 Marti Maria Saguer.<br>All rights reserved.</p>"));
    box.setText(about);
    box.exec();
}

void Editor::handleColorChanged(const QColor &color)
{
    qDebug() << "brush color changed" << color;
    QList<QMdiSubWindow*> list = mdi->subWindowList();
    for (int i=0;i<list.size();++i) {
        QMdiSubWindow *window = dynamic_cast<QMdiSubWindow*>(list.at(i));
        if (!window) { return; }
        View *view = dynamic_cast<View*>(window->widget());
        if (!view) { return; }
        view->setBrushColor(color);
    }
}

void Editor::handleLayerTreeSelectedLayer(int id)
{
    qDebug() << "set selected layer" << id;
    if (!getCurrentView()) { return; }
    getCurrentView()->setSelectedLayer(id);
}