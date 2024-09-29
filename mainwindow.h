#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "openglscene.h"
#include "model.h"
#include "point3d.h"
#include "trackball.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    //连接信号和槽
    void     connectSignalsSlots();

    //label显示状态信息
    QLabel* statusLabel;

    //显示openglwidget窗口
    OpenGLScene* glScene;

    //设置界面对话框
    //settingdialog *setdlg;


};
#endif // MAINWINDOW_H
