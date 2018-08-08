#ifndef CAR_H
#define CAR_H
#include "basicobject.h"
#include "parts.h"

class Car : public BasicObject
{
public:
    Car();
    virtual ~Car();
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
    void calibrate(float p1, float p2);
    int run(int level);
protected:
    bool init();
private:
    Wheels* m_pFront;
    Wheels* m_pRear;
    Body* m_pBody;

    QVector3D   m_light;
 //   QMatrix4x4   m_matProjection;
 //   QMatrix4x4   m_matModle;
    QMatrix4x4   m_matView;
};

#endif // CAR_H
