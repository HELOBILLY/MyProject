#ifndef CDTEXTRACTIONLAYER_H
#define CDTEXTRACTIONLAYER_H

#include "cdtbaselayer.h"

class QColor;
class QAction;
class QWidgetAction;
class QgsFeatureRendererV2;
class MergeLayer;
class OverLayer;

class CDTExtractionLayer : public CDTBaseLayer
{
    Q_OBJECT    
    Q_CLASSINFO("CDTExtractionLayer","Extraction")
    Q_CLASSINFO("tableName","extractionlayer")

//    Q_PROPERTY(QString Name READ name WRITE setName DESIGNABLE true USER true)
    Q_PROPERTY(QColor Color READ color WRITE setColor DESIGNABLE true USER true)
    Q_PROPERTY(QColor Border_Color READ borderColor WRITE setBorderColor DESIGNABLE true USER true)

public:    
    explicit CDTExtractionLayer(QUuid uuid,QObject *parent=0);
    ~CDTExtractionLayer();

    friend QDataStream &operator<<(QDataStream &out,const CDTExtractionLayer &extraction);
    friend QDataStream &operator>>(QDataStream &in, CDTExtractionLayer &extraction);

    QString shapefileID()   const;
    QColor  color()         const;
    QColor  borderColor()   const;
    int     layerTransparency() const;

    void    setRenderer(QgsFeatureRendererV2 *r);
    void    setOriginRenderer();

    static QList<CDTExtractionLayer *>  getLayers();
    static CDTExtractionLayer *         getLayer(QUuid id);

public slots:
//    void    onContextMenuRequest(QWidget *parent);
    void    setColor(const QColor &clr);
    void    setBorderColor(const QColor &clr);
    void    setLayerTransparency(const int &transparency);
    void    initLayer(const QString& name,
            const QString &shpID,
            const QColor &color,
            const QColor &borderColor, const QString imageID);
    void    remove();
    void    exportShapefile();
public slots:
    void addMerge();
    void addOverlay();
    void removeMerge(MergeLayer*);
    void removeOverlay(OverLayer*);
    void removeAllMerges();
    void removeAllOverlay();

signals:
    void    nameChanged();
    void    colorChanged(QColor);
    void    borderColorChanged(QColor);
    void    layerTransparencyChanged(int);
    void    removeExtraction(CDTExtractionLayer*);

private:
//    QWidgetAction   *actionChangeParams;
//    QAction         *actionRename;
//    QAction         *actionExportShapefile;
//    QAction         *actionRemoveExtraction;    
    static QList<CDTExtractionLayer *> layers;

    QVector<MergeLayer *> merges;
    QVector<OverLayer *>  overlays;

    CDTProjectTreeItem* mergeRootItem;
    CDTProjectTreeItem* mergeShpItem;

    void addMerge(MergeLayer* merge);
    void addOverlay(OverLayer* overlay);

};

QDataStream &operator<<(QDataStream &out,const CDTExtractionLayer &extraction);
QDataStream &operator>>(QDataStream &in, CDTExtractionLayer &extraction);

#endif // CDTEXTRACTIONLAYER_H
