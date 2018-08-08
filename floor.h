#ifndef FLOOR_H
#define FLOOR_H

#include "basicobject.h"

class Floor : public BasicObject
{
public:
    Floor();
    virtual ~Floor();
    /* \brief render floor image
     * \param bReload set true to reload texture
     */

    void draw(bool bReload = false);
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
    void init();
    bool initShaders();
    bool initTextures();

private:
    QOpenGLBuffer m_arrayBuf;
    QOpenGLBuffer m_indexBuf;

};
#endif // FLOOR_H
