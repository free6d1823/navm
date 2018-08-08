#include "mainwidget.h"

#include <QMouseEvent>

#include <math.h>

#define PI  3.1415967
#define BIRDVIEW_ROTATE_STEP    (PI/180.0)
enum ViewMode {
    VM_NONE = 0,
    VM_BIRDVIEW = 1,    //see from top
    VM_BEHIND = 2,      //see behind car
    VM_AHEAD = 3,       //see in front of car
    VM_FRONT = 4,       //sit inside car see front
    VM_BACK = 5,        // see back
    VM_LEFT = 6,        // see left side
    VM_RIGHT = 7,       //see right side
    VM_SAT = 8,       //satellite patrol
};
ViewMode m_nViewMode = VM_BEHIND;

typedef struct _KeyViewModeMap{
    int keyCode;
    ViewMode mode;
}KeyViewModeMap;
static KeyViewModeMap keyViewModeMap[] = {{Qt::Key_1, VM_BIRDVIEW}, {Qt::Key_2, VM_BEHIND},
                                   {Qt::Key_3, VM_AHEAD}, {Qt::Key_4, VM_FRONT},
                                   {Qt::Key_5, VM_BACK}, {Qt::Key_6, VM_LEFT},
                                   {Qt::Key_7, VM_RIGHT},{Qt::Key_8, VM_SAT}};
#define STATE_RUN   1
#define STATE_STOP  0
void print_mat(const char* name, QMatrix4x4 mat)
{
    printf ("Mat %s =\n", name);
    for (int i=0;i<4;i++) {
        printf("\t%6f, %6f, %6f, %6f\n", mat.row(i).x(),mat.row(i).y(), mat.row(i).z(), mat.row(i).w() );
    }
}
void print_vec(const char* name, QVector3D vec)
{
    printf ("Vec %s =\n", name);
        printf("\t%6f, %6f, %6f\n", vec.x(), vec.y(), vec.z() );
}

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    m_pObject = NULL;
    m_pFloor = NULL;
    m_posLight = QVector3D(10, 10, -10);
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();

    if(m_pObject)
        delete m_pObject;
    if (m_pFloor)
        delete m_pFloor;
    doneCurrent();
}
void MainWidget::InitViewMode()
{
    printf("InitViewMode -- mode %d \n", m_nViewMode);
    if (m_nViewMode == VM_BIRDVIEW){
        m_posCamera = QVector3D(0,10,0);
        m_posCenter = QVector3D(0,0,0);
        m_dirCamera = QVector3D(0,0,1);
        m_matProj.setToIdentity();
        m_matProj.perspective(60.0f, 4.0f/3.0f, 1, 20);



    } else {

        // Reset projection
        m_matProj.setToIdentity();
        m_matProj.perspective(52.0f, 3.0f/3.0f, 1, 20);

        m_posCamera = QVector3D(0,6,-9);
        m_posCenter = QVector3D(0,1,-4);
        m_dirCamera = QVector3D(0,1,0);

    }
    update();
    //print_mat("m_matProjection", m_matProj);
    //print_vec("m_posCamera", m_posCamera);
    //print_vec("m_posCenter", m_posCenter);
    //print_vec("m_dirCamera", m_dirCamera);
}

bool MainWidget::onBirdViewKeyEvent(int key)
{
    double rotateSpeed = BIRDVIEW_ROTATE_STEP;
    float ca = (float) cos(rotateSpeed);
    float sa = (float) sin(rotateSpeed);

    switch (key) {
    case Qt::Key_Up:
        m_posCamera.setY(m_posCamera.y() - 0.05f );
        break;
    case Qt::Key_Down:
        m_posCamera.setY(m_posCamera.y() + 0.05f );
        break;
    case Qt::Key_Left:
        m_dirCamera.setX(m_dirCamera.x() * ca - m_dirCamera.z()*sa);
        m_dirCamera.setZ(m_dirCamera.x() * sa + m_dirCamera.z()*ca);
        break;
    case Qt::Key_Right:
        m_dirCamera.setX(m_dirCamera.x() * ca + m_dirCamera.z()*sa);
        m_dirCamera.setZ(-m_dirCamera.x() * sa + m_dirCamera.z()*ca);
        break;
    default:
        return false;
    }
    return true;
}
bool MainWidget::onBehindKeyEvent(int key)
{
    double rotateSpeed = BIRDVIEW_ROTATE_STEP;
    float ca = (float) cos(rotateSpeed);
    float sa = (float) sin(rotateSpeed);
    float t1, t2;
    switch (key) {
    case Qt::Key_Up:
        m_posCamera.setY(m_posCamera.y() + 0.05f );
        break;
    case Qt::Key_Down:
        m_posCamera.setY(m_posCamera.y() - 0.05f );
        break;
    case Qt::Key_Left:
    {
        t1 = m_posCamera.x() * ca + m_posCamera.z()*sa;
        t2 = -m_posCamera.x() * sa + m_posCamera.z()*ca;
        m_posCamera.setX(t1);
        m_posCamera.setZ(t2);
        t1 = m_posCenter.x() * ca + m_posCenter.z()*sa;
        t2 = -m_posCenter.x() * sa + m_posCenter.z()*ca;
        m_posCenter.setX(t1);
        m_posCenter.setZ(t2);
    }
        break;
    case Qt::Key_Right:
    {
        t1 = m_posCamera.x() * ca - m_posCamera.z()*sa;
        t2 = m_posCamera.x() * sa + m_posCamera.z()*ca;
        m_posCamera.setX(t1);
        m_posCamera.setZ(t2);
        t1 = m_posCenter.x() * ca - m_posCenter.z()*sa;
        t2 = m_posCenter.x() * sa + m_posCenter.z()*ca;
        m_posCenter.setX(t1);
        m_posCenter.setZ(t2);
    }
        break;
        ///
    case Qt::Key_M:
        m_pObject->calibrate(-0.0001,0);
        break;
    case Qt::Key_I:
        m_pObject->calibrate(0.0001,0);
        break;
    case Qt::Key_L:
        m_pObject->calibrate(0, 0.0001f);
        break;
    case Qt::Key_J:
        m_pObject->calibrate(0, -0.0001f);
        break;
    default:
        return false;
    }
    return true;
}
bool MainWidget::checkObjectEvent(int key)
{
    switch(key) {
    case Qt::Key_A://object left
        m_matObject.rotate(5, QVector3D(0,1,0));
         break;
    case Qt::Key_D://right
        m_matObject.rotate(-5, QVector3D(0,1,0));

         break;
    case Qt::Key_W://up
        m_matObject.translate(0,0,0.1);
        break;
    case Qt::Key_X://down
        m_matObject.translate(0,0, -0.1);
         break;
    case Qt::Key_S://stop
        m_nRunLevel++;
        m_nRunLevel &= 0x07;
        m_pObject->run(m_nRunLevel);
    default:
        return false;
    }
    m_pObject->transform(m_matObject);
    return true;
}
void MainWidget::keyPressEvent(QKeyEvent *ev)
{
    size_t i;
    if (ev->key() == Qt::Key_T){
        //stop or start video
        timer.isActive()?timer.stop():timer.start(33,this);
        return;
    }
    for (i=0; i< sizeof(keyViewModeMap)/sizeof(keyViewModeMap[0]); i++)
    {
        if (ev->key() == keyViewModeMap[i].keyCode) {
            m_nViewMode = keyViewModeMap[i].mode;
            InitViewMode();
            return;
        }
    }
    bool isEvent;
    isEvent = checkObjectEvent(ev->key());
    if(! isEvent) {
        switch (m_nViewMode) {
        case VM_BIRDVIEW:
            isEvent = onBirdViewKeyEvent(ev->key());
            break;
        case VM_BEHIND:
        default:
            isEvent = onBehindKeyEvent(ev->key());
            break;
        }
    }
    if(isEvent)
        update();
}
void MainWidget::mousePressEvent(QMouseEvent *e)
{
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
}

void MainWidget::timerEvent(QTimerEvent *)
{
    update();
}

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    m_pObject = new Car;
    m_pFloor = new Floor;

}

void MainWidget::resizeGL(int w, int h)
{
    InitViewMode();
}


void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    QMatrix4x4 view;
    view.lookAt(m_posCamera, m_posCenter, m_dirCamera);

    m_pObject->transform(m_matObject);
    m_pObject->update(m_matProj, view, m_posLight);
    m_pObject->draw();

    m_pFloor->update(m_matProj, view, m_posLight);
    m_pFloor->draw(timer.isActive());
}
