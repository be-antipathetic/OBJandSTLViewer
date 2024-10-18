
#include "model.h"
#include <QMessageBox>

Model::Model(const QString &filePath)
    : m_fileName(QFileInfo(filePath).fileName())
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    //根据文件名后缀的不同运行不同的打开函数
    if (filePath.endsWith(".stl", Qt::CaseInsensitive))
    {
        loadStl(file);
    } else if (filePath.endsWith(".obj", Qt::CaseInsensitive))
    {
        loadObj(file);
    }
    //重新设置为单位矩阵（identity matrix）
    m_transform.setToIdentity();
}

Model::~Model()
{

}

void Model::transform(const QMatrix4x4& matrix)
{
    m_transform = matrix;
    if (m_vertices.empty())
        return;


    // 创建一个局部变量 v，用于存储变换后的顶点
    QVector<QVector3D> v;

    //使用 OpenMP 并行化顶点的赋值操作
    #pragma omp parallel for
    v = m_vertices;

    // 获取 v 的起始和结束迭代器
    auto it = v.end(), end = v.begin();

    // 逆向遍历顶点并应用变换矩阵
    while ( it != end )
    {
        // qDebug() << "X: " << ((QVector3D*)it)->x();
        *it = matrix * *it;
        --it;
    }

    // 将变换后的顶点赋值给 m_verticesNew
    m_verticesNew = v;

    // 重新计算所有相关数据，更新
    recomputeAll();
}


// void Model::transform(const QMatrix4x4& matrix)
// {
//     // 累积应用变换矩阵
//     m_transform *= matrix;

//     if (m_vertices.empty())
//         return;

//     // 更新顶点
//     QVector<QVector3D> transformedVertices = m_vertices;
//     for (int i = 0; i < transformedVertices.size(); ++i) {
//         transformedVertices[i] = m_transform * transformedVertices[i];
//     }

//     m_verticesNew = transformedVertices;

//     // 更新包围盒或其他需要重新计算的数据
//     recomputeAll();
// }




//绘制线框、法线
void Model::render(bool wireframe, bool normals)
{
//==================================================================================
     glPushMatrix();  // 保存当前矩阵

    // 应用模型的变换矩阵
    //glMultMatrixf(m_transform.constData());
//==================================================================================
    //启用深度测试，OpenGL 会比较当前要绘制的像素的深度值与已经绘制在相同位置的像素的深度值。
    //如果当前像素的深度值更小（更靠近观察者），那么它就会覆盖掉原来的像素；否则，它就会被丢弃。
    //glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    if (wireframe)
    {
        // 设置顶点指针
        glVertexPointer(3, GL_FLOAT, 0, (float *)m_vertices.data());
        // 以线框模式绘制边
        glDrawElements(GL_LINES, m_edgeIndices.size(), GL_UNSIGNED_INT, m_edgeIndices.data());
    }
    else
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glShadeModel(GL_SMOOTH);

        // 启用法线数组
        glEnableClientState(GL_NORMAL_ARRAY);

        // 设置顶点和法线指针
        glVertexPointer(3, GL_FLOAT, 0, (float *)m_verticesNew.data());
        glNormalPointer(GL_FLOAT, 0, (float *)m_normals.data());
        // 绘制三角形面
        glDrawElements(GL_TRIANGLES, m_vertexIndices.size(), GL_UNSIGNED_INT, m_vertexIndices.data());

        // 禁用法线数组和光照
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);
    }

    // 如果需要显示法线
    if (normals)
    {
        QVector<QVector3D> normals;
        for (int i = 0; i < m_normals.size(); ++i)
            normals << m_verticesNew.at(i) << (m_verticesNew.at(i) + m_normals.at(i) * 0.02f);
        glVertexPointer(3, GL_FLOAT, 0, (float *)normals.data());
        glDrawArrays(GL_LINES, 0, normals.size());
    }

    //禁用顶点数组
    glDisableClientState(GL_VERTEX_ARRAY);

    glPushMatrix();  // 保存当前矩阵
    drawBoundingBox();

    glPopMatrix();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

}



//读取obj文件
void Model::loadObj(QFile &file)
{
    // 1e9 = 1*10^9 = 1,000,000,000
    //设置初始边界值
    QVector3D boundsMin( 1e9, 1e9, 1e9);
    QVector3D boundsMax(-1e9,-1e9,-1e9);

    QTextStream in(&file);
    while (!in.atEnd())
    {
        //按行读取
        QString input = in.readLine();
        // 跳过空行和注释行。# 代表注释行
        if (input.isEmpty() || input[0] == '#')
            //跳出本次循环
            continue;

        //读取 input 字符串的数据
        QTextStream ts(&input);
        QString id;
        // 读取 input 中的第一个单词，并将其存储在 id 中, >> 在QtextStream用于提取字符串
        ts >> id;
        //---------------  v = List of vertices with (x,y,z[,w]) corrdinates. -----------------
        //处理顶点定义
        if (id == "v")
        {
            QVector3D p;
            for (int i = 0; i < 3; ++i)
            {
                //当读取到顶点定义 (v) 时，将顶点的三个坐标值存储到 QVector3D 对象 p 中
                //分三次，分别为x,y,z坐标
                ts >> p[i];
                //更新 boundsMin 和 boundsMax 以计算模型的边界
                boundsMin[i] = qMin(boundsMin[i], p[i]);
                boundsMax[i] = qMax(boundsMax[i], p[i]);
            }
            //将解析到的顶点添加到 m_vertices 中
            //append 等价于 <<
            m_vertices.append(p);
        }
        //--------------- f = Face definitions -----------------
        //处理面定义
        else if (id == "f" || id == "fo")
        {
            //变长数组容器，容器初始容量为 4 个元素
            QVarLengthArray<int, 4> p;


            while (!ts.atEnd())
            {
                QString vertex;
                ts >> vertex;

                //读取顶点索引,按 '/' 分割成列表提取第一个值也就是顶点索引
                const int vertexIndex = vertex.split('/').value(0).toInt();
                if (vertexIndex)
                {
                    //面有时候使用负值索引，负值代表从下向上索引，这里全转换为正值索引
                    p.append((vertexIndex > 0) ? (vertexIndex - 1) : (m_vertices.size() + vertexIndex));
                }
            }
            //qDebug() << QString("p.size(%1) vs. vertexIndices.size(%2)").arg(p.size()).arg(m_vertexIndices.size());
            //处理边索引
            for (int i = 0; i < p.size(); ++i)
            {
                const int edgeA = p[i];
                const int edgeB = p[(i + 1) % p.size()];
                //qDebug() << QString("edgeA(%1/%2), edgeB(%3/%4)").arg(QString::number(edgeA), QString::number(i), QString::number(edgeB), QString::number((i + 1) % p.size())) ;

                if (edgeA < edgeB)
                {
                    m_edgeIndices << edgeA << edgeB;
                    //qDebug() << "Added : edgeA : " << edgeA << " / edgeB : " << edgeB ;
                }
            }

            // append vertex / texture-coordinate / normal
            for (int i = 0; i < 3; ++i)
                m_vertexIndices.append(p[i]);

            // 如果是四边形，处理额外的三角形
            if (p.size() == 4)
                for (int i = 0; i < 3; ++i)
                    m_vertexIndices << p[(i + 2) % 4];
        }
    }

    //读取obj文件完毕，输出模型的大小和边界
//    qDebug() << QString("size(%1), max-x(%2), min-x(%3), max-y(%4), min-y(%5), max-z(%6), min-z(%7)")
//                    .arg(QString::number(m_vertices.size()),
//                         QString::number(boundsMax.x()),
//                         QString::number(boundsMin.x()),
//                         QString::number(boundsMax.y()),
//                         QString::number(boundsMin.y()),
//                         QString::number(boundsMax.z()),
//                         QString::number(boundsMin.z()));
    const QVector3D bounds = boundsMax - boundsMin;
    const qreal scale = 1 / qMax(bounds.x(), qMax(bounds.y(), bounds.z()));
    qDebug() << QString("scale(%1) = 1 / %2")
                    .arg(QString::number(scale), QString::number(qMax(bounds.x(), qMax(bounds.y(), bounds.z()))));
    //    for (int i = 0; i < m_vertices.size(); ++i) {
    //        //the way to place the model by mutiplying the ratio.
    //        float ratio = 0.f;
    //        m_vertices[i] = (m_vertices[i] - (boundsMin + bounds * ratio)) * scale;
    //    }

    m_verticesNew = m_vertices;
    recomputeAll();
}

void Model::loadStl(QFile &file)
{
    QTextStream stream(&file);
    const QString &head = stream.readLine();
    if (head.left(6) == "solid " && head.size() < 80)	// ASCII format
    {
        //        name = head.right(head.size() - 6).toStdString();
        QString word;
        stream >> word;
        for(; word == "facet" ; stream >> word)
        {
            stream >> word;	// normal x y z
            QVector3D n;
            stream >> n[0] >> n[1] >> n[2];
            n.normalize();

            stream >> word >> word;	// outer loop
            stream >> word;
            size_t startIndex = m_vertices.size();
            for(; word != "endloop" ; stream >> word)
            {
                QVector3D v; //vertex x y z
                stream >> v[0] >> v[1] >> v[2];
                m_vertices.push_back(v);
                //                qDebug() << "==== outer loop ===";
            }

            for(size_t i = startIndex + 2 ; i < m_vertices.size() ; ++i)
            {
                m_vertexIndices.push_back(startIndex);
                m_vertexIndices.push_back(i - 1);
                m_vertexIndices.push_back(i);

            //读取 stl 文件完毕，输出模型的大小和边界
            //             qDebug() << QString("edgeA(%1), edgeB(%2)").arg(QString::number(startIndex), QString::number(i)) ;

                //                if (startIndex < (i-1))
                m_edgeIndices << (startIndex) << (i - 1) << i;
            }
            stream >> word;	// endfacet
        }
    } else
    {
        file.setTextModeEnabled(false);
        file.seek(80);
        quint32 triangleCount;
        file.read((char*)&triangleCount, sizeof(triangleCount));
        m_vertices.reserve(triangleCount * 3U);
        m_normals.reserve(triangleCount * 3U);
        m_vertexIndices.reserve(triangleCount * 3U);
        for(size_t i = 0 ; i < triangleCount ; ++i)
        {
            QVector3D n, a, b, c;
#define READ_VECTOR(v)\
            do {\
                    float f;\
                    file.read((char*)&f, sizeof(float));	v[0] = f;\
                    file.read((char*)&f, sizeof(float));	v[1] = f;\
                    file.read((char*)&f, sizeof(float));	v[2] = f;\
            } while(false)

            READ_VECTOR(n);
            READ_VECTOR(a);
            READ_VECTOR(b);
            READ_VECTOR(c);

            m_vertexIndices.push_back(m_vertices.size());
            m_vertexIndices.push_back(m_vertices.size() + 1);
            m_vertexIndices.push_back(m_vertices.size() + 2);
            m_vertices.push_back(a);
            m_vertices.push_back(b);
            m_vertices.push_back(c);

            quint16 attribute_byte_count;
            file.read((char*)&attribute_byte_count, sizeof(attribute_byte_count));
        }
    }
    m_verticesNew = m_vertices;
    recomputeAll();
}


void Model::computeEdges()
{

}

//绘制包围盒，程序参考：http://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
void Model::drawBoundingBox()
{
    //绘制模型的包围盒

    glColor3f(1.0f, 0.647f, 0.0f);

    //draw front plane
    glBegin(GL_LINE_LOOP);
    glVertex3f(m_min.x(), m_max.y(), m_max.z());
    glVertex3f(m_min.x(), m_min.y(), m_max.z());
    glVertex3f(m_max.x(), m_min.y(), m_max.z());
    glVertex3f(m_max.x(), m_max.y(), m_max.z());
    glEnd();

    //draw rear plane
    glBegin(GL_LINE_LOOP);
    glVertex3f(m_min.x(), m_max.y(), m_min.z());
    glVertex3f(m_min.x(), m_min.y(), m_min.z());
    glVertex3f(m_max.x(), m_min.y(), m_min.z());
    glVertex3f(m_max.x(), m_max.y(), m_min.z());
    glEnd();


    //draw horizontal lines
    glBegin(GL_LINES);
    glVertex3f(m_min.x(), m_max.y(), m_min.z());
    glVertex3f(m_min.x(), m_max.y(), m_max.z());
    glVertex3f(m_max.x(), m_max.y(), m_min.z());
    glVertex3f(m_max.x(), m_max.y(), m_max.z());

    glVertex3f(m_min.x(), m_min.y(), m_min.z());
    glVertex3f(m_min.x(), m_min.y(), m_max.z());
    glVertex3f(m_max.x(), m_min.y(), m_min.z());
    glVertex3f(m_max.x(), m_min.y(), m_max.z());

    //开始绘制包围盒内部虚线

    //利用包围盒中心点 m_center 作为起点绘制




    glEnd();

}

// void Model::drawBoundingBox()
// {
//     // 获取模型的中心点坐标
//     QVector3D center = (m_max + m_min) / 2;

//     // 获取包围盒的最小和最大边界
//     QVector3D minBound = m_min;
//     QVector3D maxBound = m_max;

//     // 禁用光照
//     glDisable(GL_LIGHTING);

//     // 使用橙色绘制包围盒
//     glColor3f(1.0f, 0.647f, 0.0f);

//     // 设置虚线模式
//     glEnable(GL_LINE_STIPPLE);
//     glLineStipple(1, 0x00FF); // 1表示每个点重复1次，0x00FF表示虚线模式

//     // 将包围盒分割成 n * n * n 个小块
//     int n = 10; // 分成10个小块
//     float dx = (maxBound.x() - minBound.x()) / n;
//     float dy = (maxBound.y() - minBound.y()) / n;
//     float dz = (maxBound.z() - minBound.z()) / n;

//     // 遍历每个小块并绘制包围盒
//     for (int i = 0; i < n; ++i)
//     {
//         for (int j = 0; j < n; ++j)
//         {
//             for (int k = 0; k < n; ++k)
//             {
//                 // 当前小块的最小和最大边界
//                 QVector3D minCube = minBound + QVector3D(i * dx, j * dy, k * dz);
//                 QVector3D maxCube = minCube + QVector3D(dx, dy, dz);

//                 // 绘制小块的边缘
//                 glBegin(GL_LINES);

//                 // 绘制底面
//                 glVertex3f(minCube.x() - center.x(), minCube.y() - center.y(), minCube.z() - center.z());
//                 glVertex3f(maxCube.x() - center.x(), minCube.y() - center.y(), minCube.z() - center.z());

//                 glVertex3f(maxCube.x() - center.x(), minCube.y() - center.y(), minCube.z() - center.z());
//                 glVertex3f(maxCube.x() - center.x(), maxCube.y() - center.y(), minCube.z() - center.z());

//                 glVertex3f(maxCube.x() - center.x(), maxCube.y() - center.y(), minCube.z() - center.z());
//                 glVertex3f(minCube.x() - center.x(), maxCube.y() - center.y(), minCube.z() - center.z());

//                 glVertex3f(minCube.x() - center.x(), maxCube.y() - center.y(), minCube.z() - center.z());
//                 glVertex3f(minCube.x() - center.x(), minCube.y() - center.y(), minCube.z() - center.z());

//                 // 绘制顶面
//                 glVertex3f(minCube.x() - center.x(), minCube.y() - center.y(), maxCube.z() - center.z());
//                 glVertex3f(maxCube.x() - center.x(), minCube.y() - center.y(), maxCube.z() - center.z());

//                 glVertex3f(maxCube.x() - center.x(), minCube.y() - center.y(), maxCube.z() - center.z());
//                 glVertex3f(maxCube.x() - center.x(), maxCube.y() - center.y(), maxCube.z() - center.z());

//                 glVertex3f(maxCube.x() - center.x(), maxCube.y() - center.y(), maxCube.z() - center.z());
//                 glVertex3f(minCube.x() - center.x(), maxCube.y() - center.y(), maxCube.z() - center.z());

//                 glVertex3f(minCube.x() - center.x(), maxCube.y() - center.y(), maxCube.z() - center.z());
//                 glVertex3f(minCube.x() - center.x(), minCube.y() - center.y(), maxCube.z() - center.z());

//                 // 连接底面和顶面
//                 glVertex3f(minCube.x() - center.x(), minCube.y() - center.y(), minCube.z() - center.z());
//                 glVertex3f(minCube.x() - center.x(), minCube.y() - center.y(), maxCube.z() - center.z());

//                 glVertex3f(maxCube.x() - center.x(), minCube.y() - center.y(), minCube.z() - center.z());
//                 glVertex3f(maxCube.x() - center.x(), minCube.y() - center.y(), maxCube.z() - center.z());

//                 glVertex3f(maxCube.x() - center.x(), maxCube.y() - center.y(), minCube.z() - center.z());
//                 glVertex3f(maxCube.x() - center.x(), maxCube.y() - center.y(), maxCube.z() - center.z());

//                 glVertex3f(minCube.x() - center.x(), maxCube.y() - center.y(), minCube.z() - center.z());
//                 glVertex3f(minCube.x() - center.x(), maxCube.y() - center.y(), maxCube.z() - center.z());

//                 glEnd();
//             }
//         }
//     }

//     // 禁用虚线模式
//     glDisable(GL_LINE_STIPPLE);

//     // 重新启用光照
//     glEnable(GL_LIGHTING);
// }


void Model::recomputeAll()
{
    // qDebug() << Q_FUNC_INFO;

    //calculate normals of each face
    int size = m_verticesNew.size();
    m_normals.resize(size);
    for (int i = 0; i < m_vertexIndices.size(); i += 3)
    {
        const QVector3D a = m_verticesNew.at(m_vertexIndices.at(i));
        const QVector3D b = m_verticesNew.at(m_vertexIndices.at(i+1));
        const QVector3D c = m_verticesNew.at(m_vertexIndices.at(i+2));

        const QVector3D normal = QVector3D::crossProduct(b - a, c - a).normalized();

        for (int j = 0; j < 3; ++j)
            m_normals[m_vertexIndices.at(i + j)] += normal;
    }

    /* //debug output
    qDebug() << "=========== Output =============";
    for (int i = 0; i < m_verticesNew.size(); ++i) {
        qDebug() << QString("x(%1), y(%2), z(%3)").arg(m_verticesNew.at(i).x()).arg(m_verticesNew.at(i).y()).arg(m_verticesNew.at(i).z());
    }

    for (int i = 0; i< m_vertexIndices.size(); ++i) {
        qDebug() << QString("index %1").arg(m_vertexIndices.at(i));
    }

    for (int i = 0; i < m_normals.size(); ++i ) {
        qDebug() << QString("x(%1), y(%2), z(%3)").arg(m_normals.at(i).x()).arg(m_normals.at(i).y()).arg(m_normals.at(i).z());
    }
    qDebug() << "========================";
    */
    //recompute normals and bounding volume
    GLfloat minX, maxX,
        minY, maxY,
        minZ, maxZ;
    minX = maxX = m_verticesNew[0].x();
    minY = maxY = m_verticesNew[0].y();
    minZ = maxZ = m_verticesNew[0].z();

    for (int i = 0; i < size; ++i)
    {
        //compute normals
        m_normals[i] = m_normals[i].normalized();
        //calculate values of maximum and minimum
        if (m_verticesNew[i].x() < minX) minX = m_verticesNew[i].x();
        if (m_verticesNew[i].x() > maxX) maxX = m_verticesNew[i].x();
        if (m_verticesNew[i].y() < minY) minY = m_verticesNew[i].y();
        if (m_verticesNew[i].y() > maxY) maxY = m_verticesNew[i].y();
        if (m_verticesNew[i].z() < minZ) minZ = m_verticesNew[i].z();
        if (m_verticesNew[i].z() > maxZ) maxZ = m_verticesNew[i].z();
    }

    m_size   = QVector3D(maxX-minX, maxY-minY, maxZ-minZ);
    m_center = QVector3D((minX+maxX)/2, (minY+maxY)/2, (minZ+maxZ)/2);

    m_min = QVector3D(minX, minY, minZ);
    m_max = QVector3D(maxX, maxY, maxZ);

    qDebug() << QString("MIN : x(%1), y(%2), z(%3)").arg(m_min.x()).arg(m_min.y()).arg(m_min.z());
    qDebug() << QString("MAX : x(%1), y(%2), z(%3)").arg(m_max.x()).arg(m_max.y()).arg(m_max.z());
    qDebug() << QString("SIZE : x(%1), y(%2), z(%3)").arg(m_size.x()).arg(m_size.y()).arg(m_size.z());
    qDebug() << QString("center : x(%1), y(%2), z(%3)").arg(m_center.x()).arg(m_center.y()).arg(m_center.z());

    //QMatrix4x4 center(1,1,1,1), scale(1,1,1,1);
    //m_transform.translate(QMatrix4x4(1,1,1) * center)
}
