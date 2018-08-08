#include <QApplication>
#include <QLabel>
#include <QSurfaceFormat>

#include "mainwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    a.setApplicationName("AroundViewMornitoring System");
    a.setApplicationVersion("0.01");
    MainWidget widget;
    widget.show();

    return a.exec();
}
