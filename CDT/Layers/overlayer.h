#ifndef OVERLAYER_H
#define OVERLAYER_H

#include"cdtbaselayer.h"
#include "dialogdbconnection.h"

class QWidgetAction;
class QColor;
class QgsFeatureRendererV2;
class CDTProjectTreeItem;
class CDTExtractionLayer;

class OverLayer : public CDTBaseLayer
{
    Q_OBJECT
    Q_CLASSINFO("tableName",tr("overlayer"))
public:
   explicit OverLayer(QUuid uuid,QObject *parent = 0);
    ~OverLayer();

    friend QDataStream &operator<<(QDataStream &out,const OverLayer &overlay);
    friend QDataStream &operator>>(QDataStream &in, OverLayer &overlay);

    QString shapefileID()   const;
    QColor  color()         const;
    QColor  borderColor()   const;
    int     layerTransparency() const;

    void    setRenderer(QgsFeatureRendererV2 *r);
    void    setOriginRenderer();

private:
    static QList<OverLayer *> layers;

signals:
//    void    nameChanged();
    void    colorChanged(QColor);
    void    borderColorChanged(QColor);
    void    layerTransparencyChanged(int);
signals:
    void removeOverlay(OverLayer*);

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
QDataStream &operator<<(QDataStream &out,const OverLayer &overlay);
QDataStream &operator>>(QDataStream &in, OverLayer &overlay);

#endif // OVERLAYER_H
