#ifndef PARTS_H
#define PARTS_H
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>

class Parts
{
public:
    Parts();
    virtual ~Parts();
    bool init();
    void transform(QMatrix4x4& transform){m_matTransform = transform;}
    void setState(QMatrix4x4& newState){m_matModle = newState;}
    void draw(QOpenGLShaderProgram* program,QMatrix4x4& pojection, QMatrix4x4& view, QVector3D& light );
protected:
    bool loadModelToBuffers(const char* pathModel);
    QOpenGLBuffer m_arrayVerBuf;
    QOpenGLBuffer m_arrayTexBuf;
    QOpenGLBuffer m_arrayNorBuf;
    QOpenGLBuffer m_indexBuf;
    int m_nNumToDraw;
    QMatrix4x4  m_matModle; //state inside car
    QMatrix4x4  m_matTransform; //whole car state


};

class Body : public Parts
{
public:
    Body();
    virtual ~Body();
    bool init();
};
class Wheels : public Parts
{
public:
    Wheels(int front);
    virtual ~Wheels();
    bool init();
    void run(int level);
    void calibrate(float p1, float p2);
    void draw(QOpenGLShaderProgram* program,QMatrix4x4& pojection, QMatrix4x4& view, QVector3D& light );
private:
    int m_nFront;
    QVector3D   m_axis;
    float   m_angle;
    int         m_level; //run speed
};
#endif // PARTS_H
