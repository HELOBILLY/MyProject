#ifndef BROKENDETECT_H
#define BROKENDETECT_H
#include "cdtbaselayer.h"
#include "dialogdbconnection.h"

class QWidgetAction;
class QColor;
class QgsFeatureRendererV2;
class CDTProjectTreeItem;

class BrokenDetect : public CDTBaseLayer
{
    Q_OBJECT
    Q_CLASSINFO("tableName",tr("detectionlayer"))
public:
    explicit BrokenDetect(QUuid uuid,QObject *parent = 0);
    ~BrokenDetect();

//    friend class BrokenDetect;
    friend QDataStream &operator<<(QDataStream &out,const BrokenDetect &detection);
    friend QDataStream &operator>>(QDataStream &in, BrokenDetect &detection);

    QColor  borderColor()const;

    void setRenderer(QgsFeatureRendererV2 *r);
    void setOriginRenderer();

    QString shapefilePath() const;

private:
    static QList<BrokenDetect *> layers;


signals:
    void layerTransparencyChanged(int);
    void borderColorChanged(QColor);

    void removeDetection(BrokenDetect*);

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

};
QDataStream &operator<<(QDataStream &out,const BrokenDetect &detection);
QDataStream &operator>>(QDataStream &in, BrokenDetect &detection);
#endif // BROKENDETECT_H
