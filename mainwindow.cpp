#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("STL and OBJ Viewer");

    QOpenGLWidget *glWidget = new QOpenGLWidget();
    ui->view->setViewport(glWidget);

    glScene = new OpenGLScene(this);  // 初始化 glScene
    ui->view->setScene(glScene);

    ui->view->resize(800, 600);
    ui->view->move(10, 10);

    statusLabel = new QLabel(ui->statusbar);
    statusLabel->setText("Default model");
    ui->statusbar->addWidget(statusLabel);


    connectSignalsSlots();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//连接所有的信号和槽
void MainWindow::connectSignalsSlots()
{

    //读取指定路径模型文件，加载模型。 设置模型颜色与背景颜色
    QObject::connect(ui->actionOpen, &QAction::triggered, glScene, &OpenGLScene::openloadModel);
    QObject::connect(ui->actionClearModels,&QAction::triggered,glScene,&OpenGLScene::clearModels);
    QObject::connect(ui->actionsetBackgroundColor,&QAction::triggered,glScene,&OpenGLScene::setBackgroundColor);
    QObject::connect(ui->actionsetModelColor,&QAction::triggered,glScene,&OpenGLScene::setModelColor);
    QObject::connect(ui->actionBoundingBox,&QAction::triggered,glScene,&OpenGLScene::setBoundingBox);
    //QObject::connect(ui->checkbox_wireframe,&QCheckBox::toggled,glScene,&OpenGLScene::enableWireframe);

}
