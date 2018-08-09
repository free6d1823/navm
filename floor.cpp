#include "floor.h"
#include "videosource.h"

#ifdef FILE_SIM
VideoSource* m_pVs = NULL;
#else
CameraSource* m_pVs = NULL;
#endif
Floor::Floor()
    : m_indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();
    m_texture = NULL;
    // Generate 2 VBOs
    m_arrayBuf.create();
    m_indexBuf.create();

#ifdef FILE_SIM
    m_pVs = new VideoSource();
#else
    m_pVs = new CameraSource();
#endif

    init();
}

Floor::~Floor()
{
    m_arrayBuf.destroy();
    m_indexBuf.destroy();
    delete m_texture;
printf("Floor::~Floor() delete m_pVs=%p\n", m_pVs)   ;
    if (m_pVs){
        delete m_pVs;
  //      m_pVs = NULL;
    }
}

bool Floor::initShaders()
{
    // Compile vertex shader
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
    {
        return Return_Error("addShaderFromSourceFile Vertex failed!\n");
    }
    // Compile fragment shader
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
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

    return true;
}

bool Floor::initTextures()
{
 //   m_pImage = new QImage(m_pData, 1024, 1024, QImage::Format_RGBA8888);

    // Load cube.png image
//    m_texture = new QOpenGLTexture(QImage(":/chessboard.png").mirrored());
    unsigned char * pData = m_pVs->GetFrameData();
//    if (!pData)
//        return false;
    m_texture = new QOpenGLTexture(QImage(pData, m_pVs->Width(),
                                          m_pVs->Height(), QImage::Format_RGBA8888).mirrored());

    // Set nearest filtering mode for texture minification
    m_texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    m_texture->setWrapMode(QOpenGLTexture::Repeat);

    return true;
}
void Floor::init()
{
    // For cube we would need only 8 vertices but we have to
    // duplicate vertex for each face because texture coordinate
    // is different.
    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-10, 0, 10), QVector2D(0.0f, 0.0f)},  // v0 -- v1
        {QVector3D( 10, 0, 10), QVector2D(1.0f, 0.0f)},  //
        {QVector3D(-10, 0, -10), QVector2D(0.0f, 1.0f)}, // v2 -- v3
        {QVector3D( 10, 0, -10), QVector2D(1.0f, 1.0f)},  //

    };

    // Indices for drawing cube faces using triangle strips.
    // Triangle strips can be connected by duplicating indices
    // between the strips. If connecting strips have opposite
    // vertex order then last index of the first strip and first
    // index of the second strip needs to be duplicated. If
    // connecting strips have same vertex order then only last
    // index of the first strip needs to be duplicated.
    GLushort indices[] = {
         0,  1,  2,  1,  3, 2
    };


    // Transfer vertex data to VBO 0
    m_arrayBuf.bind();
    m_arrayBuf.allocate(vertices, 4 * sizeof(VertexData));

    // Transfer index data to VBO 1
    m_indexBuf.bind();
    m_indexBuf.allocate(indices, 6 * sizeof(GLushort));

    initShaders();
    initTextures();

    m_matModle.setToIdentity();
}

/* \brief update basic matrics
 * \param pojection
 * \param view
 * \param light
 */
void Floor::update(QMatrix4x4& pojection, QMatrix4x4& view, QVector3D& light)
{
    m_matProjection = pojection*view;
}


void Floor::draw(bool bReload )
{
    /*
    if(m_texture){
        delete m_texture;
    }
    if (!initTextures())
        return;
*/
    if (bReload) {
        unsigned char * pData = m_pVs->GetFrameData();
        m_texture->destroy();
        m_texture->create();
        m_texture->setData(QImage(pData, m_pVs->Width(),
                      m_pVs->Height(), QImage::Format_RGBA8888).mirrored());
    }
    m_program.bind();

    m_texture->bind();
    m_program.setUniformValue("mvp_matrix", m_matProjection * m_matModle);

    m_program.setUniformValue("texture", 0);

    // Tell OpenGL which VBOs to use
    m_arrayBuf.bind();
    m_indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = m_program.attributeLocation("a_position");
    m_program.enableAttributeArray(vertexLocation);
    m_program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = m_program.attributeLocation("a_texcoord");
    m_program.enableAttributeArray(texcoordLocation);
    m_program.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    m_texture->release();
    m_arrayBuf.release();
    m_indexBuf.release();
    m_program.release();
}

