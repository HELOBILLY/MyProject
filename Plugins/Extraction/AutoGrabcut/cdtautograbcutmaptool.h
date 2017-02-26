#ifndef CDTAUTOGRABCUTMAPTOOL_H
#define CDTAUTOGRABCUTMAPTOOL_H

#include <qgsmaptool.h>

class QgsMapCanvas;
class QgsRubberBand;
class QgsVectorLayer;

class CDTAutoGrabcutMapTool : public QgsMapTool
{
    Q_OBJECT
public:
    friend class AutoGrabcutInterface;
    explicit CDTAutoGrabcutMapTool(QgsMapCanvas *canvas);
    ~CDTAutoGrabcutMapTool();

    virtual void canvasMoveEvent ( QMouseEvent * e );
    virtual void canvasPressEvent( QMouseEvent * e );
signals:

public slots:

private:
    QgsRubberBand   *mRubberBand;
    QString         imagePath;
    QgsVectorLayer  *vectorLayer;

};

#endif // CDTAUTOGRABCUTMAPTOOL_H
