#ifndef TRACKBALL_H
#define TRACKBALL_H

#include <QtWidgets>

#include <QtGui/qvector3d.h>
#include <QtGui/qquaternion.h>

class TrackBall
{
public:
    TrackBall();
    //带参数的构造函数，初始化角速度和旋转轴
    TrackBall(float angularVelocity, const QVector3D& axis);

    // coordinates in [-1,1]x[-1,1]
    //开始一次旋转操作
    void push(const QPointF& p, const QQuaternion &transformation);

    //更新旋转操作
    void move(const QPointF& p, const QQuaternion &transformation);

    //结束一次旋转操作
    void release(const QPointF& p, const QQuaternion &transformation);

    //启动定时器
    void start();

    //关闭定时器
    void stop();

    //返回当前的旋转四元数
    QQuaternion rotation() const;

private:
    //旋转状态。
    QQuaternion m_rotation;

    //旋转轴
    QVector3D m_axis;

    //角速度
    float m_angularVelocity;

    //保存上一次的位置
    QPointF m_lastPos;

    //保存上一次的时间
    QTime m_lastTime;

    //指示是否暂停
    bool m_paused;

    //指示是否按下
    bool m_pressed;
};

#endif
