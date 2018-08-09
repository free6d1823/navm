#include "car.h"
#include "objloader.h"
#include <vector>

using namespace std;

///////////////////////////////////////////////////////////////////////////
Car::Car()
{

    initializeOpenGLFunctions();
    m_texture = NULL;

    m_pBody = new Body();
    m_pFront = new Wheels(1);
    m_pRear = new Wheels(0);
    // Initializes cube geometry and transfers it to VBOs
    init();
}

Car::~Car()
{
    if(m_pBody) delete m_pBody;
    if(m_pFront) delete m_pFront;
    if(m_pRear) delete m_pRear;

    delete m_texture;
}

bool Car::init()
{

    // Compile vertex shader
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vcarshader.glsl"))
    {
        return Return_Error("addShaderFromSourceFile Vertex failed!\n");
    }
    // Compile fragment shader
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fcarshader.glsl"))
    {
        return Return_Error("addShaderFromSourceFile Fragment failed!\n");
    }

    // Link shader pipeline
    if (!m_program.link())
    {
        return Return_Error("OpenGLShaderProgram link failed!\n");
    }

    // Bind shader pipeline for use
    if (!m_program.bind())
    {
        return Return_Error("OpenGLShaderProgram bind failed!\n");
    }
//    m_texture = new QOpenGLTexture(QImage(":/cube.png").mirrored());
    m_texture = new QOpenGLTexture(QImage(":/skin.png").mirrored());
    if(!m_texture){
        return Return_Error("QOpenGLTexture init failed!\n");
    }
    m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_texture->setWrapMode(QOpenGLTexture::Repeat);

    m_pBody->init();
    m_pFront->init();
    m_pRear->init();
    //move up
    QMatrix4x4 state;

    state.translate(0,0.605885,0);//initial car position
    m_pFront->setState(state);
    m_pRear->setState(state);

    state.rotate(180,QVector3D(0,1,0));
    m_pBody->setState(state);

}
void Car::calibrate(float p1, float p2)
{
    m_pFront->calibrate(p1, p2);
    m_pRear->calibrate(p1, p2);
}
/* \brief update basic matrics
 * \param pojection
 * \param view
 * \param light
 */
void Car::update(QMatrix4x4& pojection, QMatrix4x4& view, QVector3D& light)
{
    m_matProjection = pojection;
    m_matView = view;
    m_light = light;
}

/* \brief apply transform matrix to object
 * \param transform
 */
void Car::transform(QMatrix4x4& transform)
{
    m_matModle = transform;
    m_pBody->transform(transform);
    m_pFront->transform(transform);
    m_pRear->transform(transform);
}
int Car::run(int level)
{
    m_pRear->run(level);
    m_pFront->run(level);
}

void Car::draw()
{
    m_program.bind();
    m_texture->bind();
    m_program.setUniformValue("myTextureSampler", 0);
    m_program.setUniformValue("LightPosition_worldspace", m_light);
    m_program.release();

    m_pBody->draw(&m_program, m_matProjection, m_matView, m_light);
    m_pFront->draw(&m_program, m_matProjection, m_matView, m_light);
    m_pRear->draw(&m_program, m_matProjection, m_matView, m_light);

    m_texture->release();
}

