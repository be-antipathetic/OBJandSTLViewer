#ifndef POINT3D_H
#define POINT3D_H

#include "math.h"
#include <qglobal.h>

struct Point3d
{
    float x, y, z;

    //初始化列表,用来直接创造点
    Point3d()
        : x(0), y(0), z(0)
    {
        //构造函数
    }

    Point3d(float x_, float y_, float z_)
        : x(x_), y(y_), z(z_)
    {
       //构造函数
    }

    //重载操作运算符
    Point3d operator+(const Point3d &p) const
    {
        //返回两个点的和
        return Point3d(*this) += p;
    }

    Point3d operator-(const Point3d &p) const
    {
        //返回两个点的差
        return Point3d(*this) -= p;
    }

    Point3d operator*(float f) const
    {
        //返回两个点的乘积
        return Point3d(*this) *= f;
    }


    Point3d &operator+=(const Point3d &p)
    {
        //将另一个 Point3d 加到当前对象
        x += p.x;
        y += p.y;
        z += p.z;
        return *this;
    }

    Point3d &operator-=(const Point3d &p)
    {
        x -= p.x;
        y -= p.y;
        z -= p.z;
        return *this;
    }

    Point3d &operator*=(float f)
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    Point3d normalize() const
    {
        float r = 1. / sqrt(x * x + y * y + z * z);
        return Point3d(x * r, y * r, z * r);
    }
    float &operator[](unsigned int index) {
        Q_ASSERT(index < 3);
        return (&x)[index];
    }

    const float &operator[](unsigned int index) const {
        Q_ASSERT(index < 3);
        return (&x)[index];
    }
};

inline float dot(const Point3d &a, const Point3d &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Point3d cross(const Point3d &a, const Point3d &b)
{
    return Point3d(a.y * b.z - a.z * b.y,
                   a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x);
}



#endif // POINT3D_H
