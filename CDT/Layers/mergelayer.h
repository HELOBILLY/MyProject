#ifndef MERGELAYER_H
#define MERGELAYER_H

#include"cdtbaselayer.h"
#include "dialogdbconnection.h"

class QWidgetAction;
class QColor;
class QgsFeatureRendererV2;
class CDTProjectTreeItem;
class CDTExtractionLayer;

class MergeLayer : public CDTBaseLayer
{
    Q_OBJECT
    Q_CLASSINFO("tableName",tr("mergelayer"))
public:
   explicit MergeLayer(QUuid uuid,QObject *parent = 0);
    ~MergeLayer();

    friend QDataStream &operator<<(QDataStream &out,const MergeLayer &merge);
    friend QDataStream &operator>>(QDataStream &in, MergeLayer &merge);

    QString shapefileID()   const;
    QColor  color()         const;
    QColor  borderColor()   const;
    int     layerTransparency() const;

    void    setRenderer(QgsFeatureRendererV2 *r);
    void    setOriginRenderer();

//    static QList<MergeLayer *>  getLayers();
//    static MergeLayer *         getLayer(QUuid id);
private:
    static QList<MergeLayer *> layers;

signals:
//    void    nameChanged();
    void    colorChanged(QColor);
    void    borderColorChanged(QColor);
    void    layerTransparencyChanged(int);
signals:
    void removeMerge(MergeLayer*);

public slots:
    void remove();
    void setName(const QString &name);
    void rename();
    void initLayer(const QString& name,
                   const QString &shpID,
                   const QColor &color,
                   const QColor &borderColor);
    void setBorderColor(const QColor &clr);
    void setLayerTransparency(int transparency);

    void exportShapefile();
};
QDataStream &operator<<(QDataStream &out,const MergeLayer &merge);
QDataStream &operator>>(QDataStream &in, MergeLayer &merge);

#endif // MERGELAYER_H
