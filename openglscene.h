

#ifndef OPENGLSCENE_H
#define OPENGLSCENE_H

#include "point3d.h"

#include <QGraphicsScene>
#include <QLabel>
#include <QtGui/QVector3D>
#include <QtGui/QMatrix4x4>
#include <QtGui/QQuaternion>
#include <QPointF>
#include <QTime>
#include <QGLShaderProgram>
#include <QList>

// 通过控制宏定义 QT_NO_CONCURRENT 来禁用多线程技术
#define QT_NO_CONCURRENT

#ifndef QT_NO_CONCURRENT
#include <QFutureWatcher>
#endif

//! ============ Camera Config ============
#define QUATERNION_CAMERA
//! =======================================

QT_BEGIN_NAMESPACE
class Model;
class TrackBall;
class QHBoxLayout;
class QSlider;
QT_END_NAMESPACE

class OpenGLScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit OpenGLScene(QWidget* parent = nullptr);
    ~OpenGLScene();

    void drawBackground(QPainter *painter, const QRectF &rect);

public slots:
    void enableWireframe(bool enabled);
    void setModelColor();
    void setBackgroundColor();
    void openloadModel();
    void loadModel(const QString &filePath);
    void modelLoaded();
    //model control slots
    void translateX(int value);
    void translateY(int value);
    void translateZ(int value);
    void rotateX(int value);
    void rotateY(int value);
    void rotateZ(int value);
    void scale(double value);

protected:
    //重载鼠标移动函数，对模型进行操作
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

private:
    //创建不同类型的对话框
    QDialog *createDialog(const QString &windowTitle) const;
    QSlider *createSlider(int rangeMax, const char *setterSlot);
    QHBoxLayout * createSpinBox(QString label, int rangeFrom, int rangeTo, const char *member);
    QHBoxLayout * createDoubleSpinBox(QString label, double rangeFrom, double rangeTo, double singleStep, const char *member);

    //加载模型 setmodel
    void setModel(Model *model);

    bool m_wireframeEnabled;
    bool m_normalsEnabled;

    //模型与背景颜色变量
    QColor m_modelColor;
    QColor m_backgroundColor;

    //模型对象
    // Model *m_model;
    QList<Model*> m_models;

    //用来展示标签，展示文件名，点、面、线的数量
    QLabel *m_labels[4];

    QGraphicsRectItem *m_lightItem;

#ifndef QT_NO_CONCURRENT
    QFutureWatcher<Model *> m_modelLoader;
#endif

//======= added ==========


#ifdef QUATERNION_CAMERA
    int m_distExp;
    TrackBall *m_trackBall;
#else
    //float m_distance;
    //Point3d m_rotation;
    QMatrix4x4 m_matrixView;
    QMatrix4x4 m_matrixModel;
    QMatrix4x4 m_matrixModelView;    // = matrixView * matrixModel
    QMatrix4x4 m_matrixProjection;

    float m_cameraAngleX;
    float m_cameraAngleY;
    float m_cameraDistance;
#endif

    void initGL();
    void initLights();
    void drawAxis();
    void drawGrid();
    void drawBox();
    void generateTube(QVector3D p1, QVector3D p2, QVector3D p3, float radius = 2, bool saveRearVector = false);
    void drawTube(/*QVector3D p1, QVector3D p2, float radius*/);
    QPointF pixelPosToViewPos(const QPointF& p);

    // Test
    GLuint vboId;
    QVector<QVector3D> m_tubeVertices;
    QVector<int> m_tubeIndices;
    QVector<QVector3D> m_tubeNormals;
    QVector<QVector3D> m_prevFacet;
};

#endif
