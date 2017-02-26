#ifndef CDTGRABCUTMAPTOOL_H
#define CDTGRABCUTMAPTOOL_H

#include <qgsmaptool.h>
#include <grabcut_opencv.h>

class QgsMapCanvas;
class QgsRubberBand;
class QgsVectorLayer;

class CDTGrabcutMapTool : public QgsMapTool
{
    Q_OBJECT
public:
    friend class GrabcutInterface;
    explicit CDTGrabcutMapTool(QgsMapCanvas *canvas);
    ~CDTGrabcutMapTool();

    virtual void canvasMoveEvent ( QMouseEvent * e );
    virtual void canvasPressEvent( QMouseEvent * e );
signals:

public slots:

private:
    QgsRubberBand   *mRubberBand;
    QString         imagePath;
    QgsVectorLayer  *vectorLayer;

};

#endif // CDTGRABCUTMAPTOOL_H
