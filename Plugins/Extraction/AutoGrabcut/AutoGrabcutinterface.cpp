#include "AutoGrabcutinterface.h"
#include "cdtautograbcutmaptool.h"
#include <qgsvectorlayer.h>

AutoGrabcutInterface::AutoGrabcutInterface(QObject *parent)
:CDTExtractionInterface(parent)
{

}

QString AutoGrabcutInterface::methodName() const
{
    return tr("AutoGrabcut");
}

QString AutoGrabcutInterface::description() const
{
    return tr("AutoGrabcut active contour");
}

QgsMapTool *AutoGrabcutInterface::mapTool(QgsMapCanvas *canvas, QString imagePath, QgsVectorLayer *vectorLayer)
{
    CDTAutoGrabcutMapTool *autograbcutMapTool = new CDTAutoGrabcutMapTool(canvas);
    autograbcutMapTool->imagePath = imagePath;
    autograbcutMapTool->vectorLayer = vectorLayer;
    return autograbcutMapTool;
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(AutoGrabcut, AutoGrabcutInterface)
#endif

