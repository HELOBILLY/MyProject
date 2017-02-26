#ifndef DETECTLAYER_H
#define DETECTLAYER_H

#include "cdtbaselayer.h"
#include "dialogdbconnection.h"

class QWidgetAction;
class QColor;
class QgsFeatureRendererV2;
class CDTProjectTreeItem;
class CDTExtractionLayer;

class DetectLayer : public CDTBaseLayer
{
    Q_OBJECT
    Q_CLASSINFO("tableName",tr("detectionlayer"))
public:
    explicit DetectLayer(QUuid uuid,QObject *parent = 0);
    ~DetectLayer();

//    friend class BrokenDetect;
    friend QDataStream &operator<<(QDataStream &out,const DetectLayer &detection);
    friend QDataStream &operator>>(QDataStream &in, DetectLayer &detection);

    QColor  borderColor()const;

    void setRenderer(QgsFeatureRendererV2 *r);
    void setOriginRenderer();

    QString shapefilePath() const;
    QString myLayerBaseName;

private:
    static QList<DetectLayer *> layers;

    QVector<CDTExtractionLayer *> extractions;

    CDTProjectTreeItem* extractionRootItem;
    CDTProjectTreeItem* autoextraction;
    //CDTProjectTreeItem* mergeshp;

    void addExtraction(CDTExtractionLayer* extraction);

signals:
    void layerTransparencyChanged(int);
    void borderColorChanged(QColor);

    void removeDetection(DetectLayer*);

public slots:
    void remove();
    void setName(const QString &name);
    void rename();
    void initLayer(const QString& name,
                   const QString &shpPath,
                   CDTDatabaseConnInfo url,
                   const QColor &color);
    void setBorderColor(const QColor &clr);
    void setLayerTransparency(int transparency);

    void exportShapefile();

    void addExtraction();
    void removeExtraction(CDTExtractionLayer*ext);
    void removeAllExtractionLayers();
    void deleteShpFeature();

};
QDataStream &operator<<(QDataStream &out,const DetectLayer &detection);
QDataStream &operator>>(QDataStream &in, DetectLayer &detection);
#endif // DETECTLAYER_H
