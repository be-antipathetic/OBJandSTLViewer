#ifndef MODEL_H
#define MODEL_H


#include <QString>
#include <QVector3D>
#include <QVector>
#include <QMatrix4x4>
#include <QFile>
#include <math.h>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QVarLengthArray>
#include <QtOpenGL>
#include <QDebug>

class Model
{
public:
    Model();
    //重载 Model 带文件路径的构造函数，用于从文件加载模型。
    Model(const QString &filePath);
    ~Model();

    //渲染模型，可以选择是否显示线框、法线和G代码运动/线条
    void render(bool wireframe = false, bool normals = false);

    //绘制模型的包围盒
    void drawBoundingBox();
    //模型包围盒真值,为真值则绘制模型的包围盒
    bool m_ifBoundingBox;

    //应用变换矩阵到模型
    //void transform(QMatrix4x4 matrix);
    void transform(const QMatrix4x4 &matrix); //多模型累计变换矩阵

    //获取模型文件名
    QString fileName() const { return m_fileName; }
    //获取点、面、边，内联函数。编译器在调用该函数时，可以直接将函数体的代码插入到调用点，避免函数调用的开销。特别适合短小且频繁调用的函数
    int faces() const { return m_vertexIndices.size() / 3; }
    int edges() const { return m_edgeIndices.size() / 2; }
    int points() const { return m_vertices.size(); }

    //模型的尺寸、中心、最小和最大边界
    QVector3D m_size;
    QVector3D m_center;
    QVector3D m_min;
    QVector3D m_max;


private:

    QString m_fileName;
    //模型的顶点，法线，边、面索引
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_verticesNew;
    QVector<QVector3D> m_normals;
    QVector<int> m_edgeIndices;
    QVector<int> m_vertexIndices;



    //QMatrix4x4 m_transform 在 3D 图形和建模中通常用于描述和执行几何变换，如平移、旋转、缩放和透视投影
    QMatrix4x4 m_transform;


    //加载 obj 模型和 stl 模型
    void loadObj(QFile &file);
    void loadStl(QFile &file);

    //计算模型的边
    void computeEdges();
    //重新计算所有相关数据
    void recomputeAll();
};

#endif // MODEL_H
