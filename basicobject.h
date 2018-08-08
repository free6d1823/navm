#ifndef BASICOBJECT_H
#define BASICOBJECT_H
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>


struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};

bool Return_Error(const char* reason);

class BasicObject : protected QOpenGLFunctions
{
public:
    BasicObject(){}
    virtual ~BasicObject(){}
    void draw();
    /* \brief update basic matrics
     * \param pojection
     * \param view
     * \param light
     */
    void update(QMatrix4x4& pojection, QMatrix4x4& view, QVector3D& light);

    /* \brief apply transform matrix to object
     * \param transform
     */
    void transform(QMatrix4x4& transform);

protected:
    bool initShaders();
    bool initTextures();
    void init();
    QOpenGLShaderProgram m_program;
    QOpenGLTexture *m_texture;


    QMatrix4x4   m_matProjection;
    QMatrix4x4   m_matModle;
};
#endif // BASICOBJECT_H
