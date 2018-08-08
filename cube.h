#ifndef CUBE_H
#define CUBE_H

#include "basicobject.h"



class Cube : public BasicObject
{
public:
    Cube();
    virtual ~Cube();
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
private:
    QOpenGLBuffer m_arrayBuf;
    QOpenGLBuffer m_indexBuf;

};


#endif // CUBE_H
