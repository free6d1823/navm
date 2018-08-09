#include "parts.h"
#include "objloader.h"
using namespace std;

Parts::Parts(): m_indexBuf(QOpenGLBuffer::IndexBuffer)
{
    // Generate array buffers
    m_arrayVerBuf.create();
    m_arrayTexBuf.create();
    m_arrayNorBuf.create();

    m_indexBuf.create();
    m_nNumToDraw = 0;
}
Parts::~Parts()
{
    m_arrayVerBuf.destroy();
    m_arrayTexBuf.destroy();
    m_arrayNorBuf.destroy();
    m_indexBuf.destroy();
}

bool Parts::loadModelToBuffers(const char* pathModel)
{
    std::vector<QVector3D> vertices;
    std::vector<QVector2D> uvs;
    std::vector<QVector3D> normals;
    bool res;
    res = loadOBJ(pathModel, vertices, uvs, normals);
    if (!res) {
        perror("Fata error!! Check obj file.\n");
        return false;
    }

    vector<unsigned short> indexed_indices;
    vector<QVector3D> indexed_vertices;
    vector<QVector2D> indexed_uvs;
    vector<QVector3D> indexed_normals;
    indexVBO(vertices, uvs, normals, indexed_indices, indexed_vertices, indexed_uvs, indexed_normals);
    m_nNumToDraw = indexed_indices.size();
#if 0
    float mx, my, mz, nx,ny,nz;
    mx = my = mz = nx = ny = nz = 0;
    int nVert = indexed_vertices.size();
    for(int i=0; i< nVert; i++){
        if (indexed_vertices[i].x() > mx) mx = indexed_vertices[i].x();
        if (indexed_vertices[i].x() < nx) nx = indexed_vertices[i].x();
        if (indexed_vertices[i].y() > my) my = indexed_vertices[i].y();
        if (indexed_vertices[i].y() < ny) ny = indexed_vertices[i].y();
        if (indexed_vertices[i].z() > mz) mz = indexed_vertices[i].z();
        if (indexed_vertices[i].z() < nz) nz = indexed_vertices[i].z();
    }
    printf ("object %d vertics\n", nVert);
    printf("\tx: (%f - %f) center %f\n", nx, mx, (nx + mx)/2);
    printf("\ty: (%f - %f) center %f\n", ny, my, (ny + my)/2);
    printf("\tz: (%f - %f) center %f\n", nz, mz, (nz + mz)/2);
#endif
    m_arrayVerBuf.bind();
    m_arrayVerBuf.allocate(&indexed_vertices[0], indexed_vertices.size() * sizeof(QVector3D));
    m_arrayVerBuf.release();
    m_arrayTexBuf.bind();
    m_arrayTexBuf.allocate(&indexed_uvs[0], indexed_uvs.size() * sizeof(QVector2D));
    m_arrayTexBuf.release();
    m_arrayNorBuf.bind();
    m_arrayNorBuf.allocate(&indexed_normals[0], indexed_normals.size() * sizeof(QVector3D));
    m_arrayNorBuf.release();

    // Transfer index data to VBO 1
    m_indexBuf.bind();
    m_indexBuf.allocate(&indexed_indices[0], indexed_indices.size() * sizeof(GLushort));

    m_matModle.setToIdentity();
}

void Parts::draw(QOpenGLShaderProgram* program,QMatrix4x4& pojection, QMatrix4x4& view, QVector3D& light )
{
    QMatrix4x4 model = m_matTransform* m_matModle;

    program->bind();
    program->setUniformValue("MVP", pojection * view * model);
    program->setUniformValue("V", view);
    program->setUniformValue("M", model);

    m_arrayVerBuf.bind();
    // Offset for position
    int id = program->attributeLocation("vertexPosition_modelspace");
    program->enableAttributeArray(id);
    program->setAttributeBuffer(id, GL_FLOAT, 0, 3, sizeof(QVector3D));
    m_arrayVerBuf.release();

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    m_arrayTexBuf.bind();
    id = program->attributeLocation("vertexUV");
    program->enableAttributeArray(id);
    program->setAttributeBuffer(id, GL_FLOAT, 0, 2, sizeof(QVector2D));

    m_arrayNorBuf.bind();
    id = program->attributeLocation("vertexNormal_modelspace");
    program->enableAttributeArray(id);
    program->setAttributeBuffer(id, GL_FLOAT, 0, 3, sizeof(QVector3D));
    m_arrayNorBuf.release();

    m_indexBuf.bind();

    glDrawElements(GL_TRIANGLES, m_nNumToDraw, GL_UNSIGNED_SHORT, 0);

    m_indexBuf.release();
    program->release();
}
///////////////////////////////////////////////////////////////////////////////////////
Body::Body() :Parts()
{
}
Body::~Body()
{
}
bool Body::init()
{
    return loadModelToBuffers("/home/cj/workspace/navm/porch_body.obj");
}
///////////////////////////////////////////////////////////////////////////////////////
Wheels::Wheels(int front) :Parts()
{
    m_nFront = front;   //1 for front wheels
    m_level = 0;
    m_angle = 0;
}
Wheels::~Wheels()
{
}
bool Wheels::init()
{
    if (m_nFront){
//        m_axis = QVector3D(-0.000818, (0.605885-0.277211), -0.768252);//y-- makes lower z--make front
        m_axis = QVector3D(-0.000818, 0.327574, -1.203252);
        return loadModelToBuffers("/home/cj/workspace/navm/porch_frontwheels.obj");
    }
 //   m_axis = QVector3D(0.001061, (0.605885-0.277211), 0.742935);
    m_axis = QVector3D(0.001061, 0.327574, 1.156935);

    return loadModelToBuffers("/home/cj/workspace/navm/porch_rearwheels.obj");
}
float detP1 = 0;
float detP2 = 0;
void Wheels::calibrate(float p1, float p2)
{
    m_axis.setY(m_axis.y()+p1);
    m_axis.setZ(m_axis.z()+p2);
    printf("wheel %d current: y=%f, z=%f\n", m_nFront, m_axis.y(),m_axis.z());
}

void Wheels::draw(QOpenGLShaderProgram* program,QMatrix4x4& pojection, QMatrix4x4& view, QVector3D& light )
{
    QMatrix4x4 model;
    if (m_level > 0){
        m_angle += m_level*2;
        if (m_angle > 360.0)
            m_angle -= 360.0;
        QMatrix4x4 rot;
        QMatrix4x4 t1,t2;
        t1.translate(0,-m_axis.y(),-m_axis.z());
        rot.rotate(m_angle, QVector3D(1,0,0));
        t2.translate(0, m_axis.y(), m_axis.z());
        model = m_matTransform* t2*rot*t1*m_matModle;
    }else
        model = m_matTransform* m_matModle;

    program->bind();
    program->setUniformValue("MVP", pojection * view * model);
    program->setUniformValue("V", view);
    program->setUniformValue("M", model);

    m_arrayVerBuf.bind();
    // Offset for position
    int id = program->attributeLocation("vertexPosition_modelspace");
    program->enableAttributeArray(id);
    program->setAttributeBuffer(id, GL_FLOAT, 0, 3, sizeof(QVector3D));
    m_arrayVerBuf.release();

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    m_arrayTexBuf.bind();
    id = program->attributeLocation("vertexUV");
    program->enableAttributeArray(id);
    program->setAttributeBuffer(id, GL_FLOAT, 0, 2, sizeof(QVector2D));

    m_arrayNorBuf.bind();
    id = program->attributeLocation("vertexNormal_modelspace");
    program->enableAttributeArray(id);
    program->setAttributeBuffer(id, GL_FLOAT, 0, 3, sizeof(QVector3D));
    m_arrayNorBuf.release();

    m_indexBuf.bind();

    glDrawElements(GL_TRIANGLES, m_nNumToDraw, GL_UNSIGNED_SHORT, 0);

    m_indexBuf.release();
    program->release();
}
void Wheels::run(int level)
{
    m_level = level;

}
