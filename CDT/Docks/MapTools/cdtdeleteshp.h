#ifndef CDTDELETESHP_H
#define CDTDELETESHP_H

#include <QObject>
#include <QMouseEvent>
#include <qgsmaptoolidentify.h>
#include <qgshighlight.h>

class qgis_devMapToolIdentifyAction : public QgsMapToolIdentify
{
    Q_OBJECT

public:
    qgis_devMapToolIdentifyAction(QgsMapCanvas *canvas);
    ~qgis_devMapToolIdentifyAction();

    //! 重写鼠标键释放事件
    virtual void canvasReleaseEvent(QMouseEvent *e);
    QgsHighlight *mHighlight;

public slots:
    void deleteHighlight();
};
#endif // CDTDELETESHP_H
