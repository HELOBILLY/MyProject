#ifndef CDTIMAGELAYER_H
#define CDTIMAGELAYER_H

#include "cdtbaselayer.h"

class QAction;
class BrokenDetect;
class DetectLayer;
class CDTProjectTreeItem;
class CDTExtractionLayer;
class CDTSegmentationLayer;
class CategoryInformation;
class MergeLayer;
class OverLayer;
enum QgsContrastEnhancement::ContrastEnhancementAlgorithm;

typedef QList<CategoryInformation> CDTCategoryInformationList;

class CDTImageLayer:public CDTBaseLayer
{
    Q_OBJECT
    Q_CLASSINFO("CDTImageLayer","Image")
    Q_CLASSINFO("tableName","imagelayer")
    Q_PROPERTY(QString Source READ path DESIGNABLE true USER true)
public:
    explicit CDTImageLayer(QUuid uuid, QObject *parent = 0);
    ~CDTImageLayer();

    friend QDataStream &operator<<(QDataStream &out, const CDTImageLayer &image);
    friend QDataStream &operator>>(QDataStream &in, CDTImageLayer &image);
    friend class CDTSegmentationLayer;
//    friend class DetectLayer;

    void initLayer(const QString& name,const QString& path);
    void setCategoryInfo(const CDTCategoryInformationList& info);
    QString path()const;
    int bandCount()const;    

    static QList<CDTImageLayer *> getLayers();
    static CDTImageLayer *getLayer(const QUuid& id);
    QVector<MergeLayer *>           GetMergeLayers();
    QVector<OverLayer *>            GetOverLayers();
    QVector<BrokenDetect *>         GetBrokenDetectionLayers();
    QVector<DetectLayer *>          GetDetectionLayers();
    QVector<CDTExtractionLayer *>   GetExtractionLayers();
    QVector<CDTSegmentationLayer *> GetSegmentationLyers();


    CDTProjectTreeItem *GetSegmentationsRoot();//before
    CDTProjectTreeItem *GetSegmentationsRoot_2();//after
    CDTProjectTreeItem *GetExtractionRoot();//interactive

    CDTProjectTreeItem *GetMainRootB();//after
    CDTProjectTreeItem *GetMainRootA();//before
    CDTProjectTreeItem *GetDetectRoot_2();//after
    CDTProjectTreeItem *GetDetectRoot();//before
    CDTProjectTreeItem *GetRoadDetectRoot();
signals:
    void removeImageLayer(CDTImageLayer*);

public slots:
//    void setRenderer();
    void addBrokenDetection();
    void addDetection();
    void addExtraction();
    void addSegmentation();    
    void remove();
    void removeExtraction(CDTExtractionLayer*);
    void removeAllExtractionLayers();
    void removeSegmentation(CDTSegmentationLayer*);
    void removeDetection(BrokenDetect*);
    void removeDetection(DetectLayer*);
//    void removeMerge(MergeLayer*);
    void removeAllSegmentationLayers();
    void setLayerOpacity(int opacity);

    void redBandChanged(int bandIDFrom0);
    void greenBandChanged(int bandIDFrom0);
    void blueBandChanged(int bandIDFrom0);
    void onEnhancementChanged(int enhancementStyle);
private:
    void addBrokenDetection(BrokenDetect *detection);
    void addDetection(DetectLayer * detection);
    void addExtraction(CDTExtractionLayer* extraction);
    void addSegmentation(CDTSegmentationLayer* segmentation);

    void updateRenderer();

private:
    QVector<MergeLayer *>           merges;
    QVector<OverLayer *>            overlays;
    QVector<BrokenDetect *>         brokenDetection;
    QVector<DetectLayer *>          detections;
    QVector<CDTExtractionLayer *>   extractions;
    QVector<CDTSegmentationLayer *> segmentations;

    CDTProjectTreeItem *segmentationsRoot;
    CDTProjectTreeItem *segmentationsRoot_2;
    CDTProjectTreeItem *extractionRoot;

    CDTProjectTreeItem *MainRootB;
    CDTProjectTreeItem *MainRootA;
    CDTProjectTreeItem *DetectRoot_2;
    CDTProjectTreeItem *DetectRoot;
    CDTProjectTreeItem *RoadDetectRoot;

    QWidget *multibandSelectionWidget;
    int rBandID,gBandID,bBandID;
    QgsContrastEnhancement::ContrastEnhancementAlgorithm enhancementStyle;

    static QList<CDTImageLayer *> layers;
};

QDataStream &operator<<(QDataStream &out, const CDTImageLayer &image);
QDataStream &operator>>(QDataStream &in, CDTImageLayer &image);


class CategoryInformation
{
public:
    CategoryInformation(QUuid uuid=QUuid(),QString name=QString(),QColor clr=QColor())
        :id(uuid),categoryName(name),color(clr){}
    QUuid id;
    QString categoryName;
    QColor color;
};

QDataStream &operator <<(QDataStream &out,const CategoryInformation &categoryInformation);
QDataStream &operator >>(QDataStream &in, CategoryInformation &categoryInformation);
#endif // CDTIMAGELAYER_H
