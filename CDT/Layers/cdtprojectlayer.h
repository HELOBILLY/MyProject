#ifndef CDTPROJECT_H
#define CDTPROJECT_H

#include "cdtbaselayer.h"

class QAction;
class CDTImageLayer;
class CDTPixelChangeLayer;
class CDTVectorChangeLayer;
class CDTFileSystem;

class CDTProjectLayer: public CDTBaseLayer
{
    Q_OBJECT    
    Q_CLASSINFO("CDTProjectLayer",tr("Project"))
    Q_CLASSINFO("tableName",tr("project"))
public:
    explicit CDTProjectLayer(QUuid uuid,QObject *parent = 0);
    ~CDTProjectLayer();

    friend QDataStream &operator <<(QDataStream &out,const CDTProjectLayer &project);
    friend QDataStream &operator >>(QDataStream &in, CDTProjectLayer &project);
    friend class CDTProjectTreeModel;
    friend class CDTBaseLayer;
    friend class CDTSegmentationLayer;
    friend class DetectLayer;

    void    initLayer(QString name);
    bool    isCDEnabled(QUuid projectID);
    QVector<CDTImageLayer *>    GetimagesLayers();

public slots:
    void addImageLayer();
    void addImageLayer(CDTImageLayer *image);
    void removeImageLayer(CDTImageLayer *image);
    void removeAllImageLayers();


    void addPBCDBinaryLayer();
    void addPBCDBinaryLayer(QByteArray result);
    void removePixelChangeLayer(CDTPixelChangeLayer *layer);
    void removeAllPixelChangeLayers();


    void addVectorChangeDetectionLayer();
    void addVectorChangeLayer(CDTVectorChangeLayer *layer);
    void removeVectorChangeLayer(CDTVectorChangeLayer *layer);
    void removeAllVectorChangeLayers();

private:
    bool    isFileExsit;
    QVector<CDTImageLayer *>    images;
    QVector<CDTPixelChangeLayer *>   pixelChanges;
    QVector<CDTVectorChangeLayer *>   vectorChanges;
    CDTFileSystem *fileSystem;

    CDTProjectTreeItem *imagesRoot;
    CDTProjectTreeItem *pixelChangesRoot;
    CDTProjectTreeItem *vectorChangesRoot;

};

#endif // CDTPROJECT_H
