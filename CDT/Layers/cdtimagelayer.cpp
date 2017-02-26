#include "cdtimagelayer.h"
#include "stable.h"
#include <qgsmultibandcolorrenderer.h>
#include <qgscontrastenhancement.h>
#include <qgscontrastenhancementfunction.h>
#include "mainwindow.h"
#include "cdtprojecttreeitem.h"
#include "cdtprojectlayer.h"
#include "cdtextractionlayer.h"
#include "cdtsegmentationlayer.h"
#include "cdtfilesystem.h"
#include "dialognewextraction.h"
#include "dialognewsegmentation.h"
#include "dialogdbconnection.h"
#include "dialogdetecthogsvm.h"
#include "detectlayer.h"
#include "brokendetect.h"
#include "dialogbrokendetection.h"
#include "mergelayer.h"
#include "overlayer.h"

QList<CDTImageLayer *> CDTImageLayer::layers;

CDTImageLayer::CDTImageLayer(QUuid uuid, QObject *parent)
    : CDTBaseLayer(uuid,parent),
      multibandSelectionWidget(NULL),
      enhancementStyle(QgsContrastEnhancement::NoEnhancement)
{
    setKeyItem(new CDTProjectTreeItem(CDTProjectTreeItem::IMAGE,CDTProjectTreeItem::RASTER,QString(),this));

    MainRootB =
                new CDTProjectTreeItem(CDTProjectTreeItem::FISHNET_ROOT,CDTProjectTreeItem::GROUP,tr("After"),this);
    MainRootA =
                new CDTProjectTreeItem(CDTProjectTreeItem::FISHNET_ROOT,CDTProjectTreeItem::GROUP,tr("Before"),this);
    DetectRoot
            = new CDTProjectTreeItem(CDTProjectTreeItem::FISHNET_ROOT,CDTProjectTreeItem::GROUP,tr("Target Detection"),this);
    DetectRoot_2
            = new CDTProjectTreeItem(CDTProjectTreeItem::FISHNET_ROOT,CDTProjectTreeItem::GROUP,tr("Target Detection"),this);
//    RoadDetectRoot
//            = new CDTProjectTreeItem(CDTProjectTreeItem::FISHNET_ROOT,CDTProjectTreeItem::GROUP,tr("Broken Detection"),this);
//    extractionRoot
//            = new CDTProjectTreeItem(CDTProjectTreeItem::EXTRACTION_ROOT,CDTProjectTreeItem::GROUP,tr("Extractions"),this);
    segmentationsRoot
            = new CDTProjectTreeItem(CDTProjectTreeItem::SEGMENTION_ROOT,CDTProjectTreeItem::GROUP,tr("Polygon Detection"),this);
    segmentationsRoot_2
            = new CDTProjectTreeItem(CDTProjectTreeItem::SEGMENTION_ROOT,CDTProjectTreeItem::GROUP,tr("Polygon Detection"),this);

//    standardKeyItem()->appendRow(DetectRoot);
////    standardKeyItem()->appendRow(RoadDetectRoot);
//    standardKeyItem()->appendRow(segmentationsRoot);
    standardKeyItem()->appendRow(MainRootA);
    //standardKeyItem()->appendRow(MainRootB);
    MainRootA->appendRow(DetectRoot);
    MainRootA->appendRow(segmentationsRoot);

    //MainRootB->appendRow(DetectRoot_2);
    //MainRootB->appendRow(segmentationsRoot_2);

//    standardKeyItem()->appendRow(extractionRoot);

    layers.push_back(this);

    connect(this,SIGNAL(removeImageLayer(CDTImageLayer*)),(CDTProjectLayer*)(this->parent()),SLOT(removeImageLayer(CDTImageLayer*)));


    //actions
//    QWidgetAction *actionMultiBandRenderer  = new QWidgetAction(this);

    QAction *actionRename                   = new QAction(QIcon(":/Icons/Rename.png"),tr("Rename Image"),this);
    QAction *actionRemoveImage              = new QAction(QIcon(":/Icons/Remove.png"),tr("Remove Image"),this);
    QAction *actionAddExtractionLayer       = new QAction(QIcon(":/Icons/Add.png"),tr("Add Extraction"),this);
    QAction *actionAddSegmentationLayer     = new QAction(QIcon(":/Icons/Add.png"),tr("Add Segmentation"),this);
    QAction *actionRemoveAllExtractions     = new QAction(QIcon(":/Icons/Remove.png"),tr("Remove All Extractions"),this);
    QAction *actionRemoveAllSegmentations   = new QAction(QIcon(":/Icons/Remove.png"),tr("Remove All Segmentations"),this);
    QAction *actionDetectFishNet            = new QAction(QIcon(":/Icons/Add.png"),tr("Detect Something"),this);
    QAction *actionBrokenDetect             = new QAction(QIcon(":/Icons/Add.png"),tr("Broken Detect"),this);
    setActions(QList<QList<QAction *> >()
               <<(QList<QAction *>()/*<<actionOpacity<<actionMultiBandRenderer*/<<actionRename<<actionRemoveImage)
               <<(QList<QAction *>()<<actionDetectFishNet)
//               <<(QList<QAction *>()<<actionBrokenDetect)
               <<(QList<QAction *>()<<actionAddSegmentationLayer)
               /*<<(QList<QAction *>()<<actionAddExtractionLayer<<actionRemoveAllExtractions)*/);
//               <<(QList<QAction *>()<<actionAddSegmentationLayer<<actionRemoveAllSegmentations)*//*);


//    Multi-band Selection
//    multibandSelectionWidget = new QWidget(NULL);
//    actionMultiBandRenderer->setDefaultWidget(multibandSelectionWidget);

//qDebug()<<this->id();
    connect(actionBrokenDetect,SIGNAL(triggered()),this,SLOT(addBrokenDetection()));
    connect(actionDetectFishNet,SIGNAL(triggered()),this,SLOT(addDetection()));
    connect(actionRename,SIGNAL(triggered()),this,SLOT(rename()));
    connect(actionRemoveImage,SIGNAL(triggered()),this,SLOT(remove()));
    connect(actionAddSegmentationLayer,SIGNAL(triggered()),this,SLOT(addSegmentation()));
    connect(actionRemoveAllSegmentations,SIGNAL(triggered()),this,SLOT(removeAllSegmentationLayers()));
    connect(actionAddExtractionLayer,SIGNAL(triggered()),this,SLOT(addExtraction()));
    connect(actionRemoveAllExtractions,SIGNAL(triggered()),this,SLOT(removeAllExtractionLayers()));
}

CDTImageLayer::~CDTImageLayer()
{
    if (id().isNull())
        return;

    QSqlQuery query(QSqlDatabase::database("category"));
    bool ret;
    ret = query.exec("delete from imagelayer where id = '"+id().toString()+"'");
    if (!ret)
        qWarning()<<"prepare:"<<query.lastError().text();
    ret = query.exec("delete from category where imageID = '"+id().toString()+"'");
    if (!ret)
        qWarning()<<"prepare:"<<query.lastError().text();

    query.exec("select id from image_validation_samples where imageID = '"+id().toString()+"'");
    QStringList list;
    while (query.next())
    {
        list.push_back(query.value(0).toString());
    }
    foreach (QString string, list) {
        ret = query.exec("delete from point_category where validationid = '"+string+"'");
        if (!ret)
            qWarning()<<"prepare:"<<query.lastError().text();
    }
    ret = query.exec("delete from image_validation_samples where imageID = '"+id().toString()+"'");
    if (!ret)
        qWarning()<<"prepare:"<<query.lastError().text();
    layers.removeAll(this);
}

void CDTImageLayer::initLayer(const QString &name, const QString &path)
{
    QgsRasterLayer *newCanvasLayer = new QgsRasterLayer(path,QFileInfo(path).completeBaseName());
    if (!newCanvasLayer->isValid())
    {
        QMessageBox::critical(NULL,tr("Error"),tr("Open image ")+path+tr(" failed!"));
        delete newCanvasLayer;
        return;
    }

    keyItem()->setText(name);
    keyItem()->setToolTip(path);

    setCanvasLayer(newCanvasLayer);

    QSqlDatabase db = QSqlDatabase::database("category");
    if (db.isValid()==false)
    {
        QMessageBox::critical(NULL,tr("Error"),tr("database is invalid"));
        return ;
    }
    QSqlQuery query(db);
    bool ret ;
    ret = query.prepare("insert into imagelayer VALUES(?,?,?,?)");
    if (ret == false)
    {
        QMessageBox::critical(NULL,tr("Error"),tr("insert image layer failed!\nerror:")+query.lastError().text());
        return ;
    }
    query.bindValue(0,id().toString());
    query.bindValue(1,name);
    query.bindValue(2,path);
    query.bindValue(3,((CDTProjectLayer*)parent())->id().toString());
    query.exec();

    QList<QPair<QLabel*,QWidget*>> widgets;
    //Opacity slider
    QSlider *slider = new QSlider();
    slider->setOrientation(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(100);
    slider->setValue(100);
    connect(slider,SIGNAL(valueChanged(int)),SLOT(setLayerOpacity(int)));
    connect(this,SIGNAL(destroyed()),slider,SLOT(deleteLater()));
    widgets<<qMakePair(new QLabel(tr("Opacity")),(QWidget*)slider);

    //Enhancement
    QComboBox *comboEnhancement = new QComboBox();
    comboEnhancement->addItems(QStringList()
                               <<tr("No Enhancement")
                               <<tr("Stretch To Min/Max")
                               <<tr("Stretch And Clip To Min/Max")
                               <<tr("Clip To Min/Max"));
    comboEnhancement->setCurrentIndex(1);
    connect(comboEnhancement,SIGNAL(currentIndexChanged(int)),SLOT(onEnhancementChanged(int)));
    widgets<<qMakePair(new QLabel(tr("Enhancement")),(QWidget*)comboEnhancement);

    //Init multiband selection widget
    if (newCanvasLayer->bandCount()>=3)
    {
        QStringList bandTexts = QStringList()<<tr("Red")<<tr("Green")<<tr("Blue");
        QList<QColor> bandColors = QList<QColor>()<<QColor(Qt::red)<<QColor(Qt::green)<<QColor(Qt::blue);
        QStringList bandNames;
        for(int i=0;i<newCanvasLayer->bandCount();++i)
            bandNames<<tr("Band %1").arg(i+1);

        QList<QComboBox*> comboList;
        const int rgbCount = 3;
        for(int i=0;i<rgbCount;++i)
        {
            QLabel *label = new QLabel(bandTexts[i]);
            QPalette palette = label->palette();
            palette.setColor(QPalette::WindowText,bandColors[i]);
            label->setPalette(palette);
            QComboBox *combo = new QComboBox();
            combo->addItems(bandNames);
            combo->setCurrentIndex(i);
            comboList << combo;
            widgets<<qMakePair(label,(QWidget*)combo);
        }

        rBandID = 1;gBandID = 2;bBandID = 3;

        connect(comboList[0],SIGNAL(currentIndexChanged(int)),SLOT(redBandChanged(int)));
        connect(comboList[1],SIGNAL(currentIndexChanged(int)),SLOT(greenBandChanged(int)));
        connect(comboList[2],SIGNAL(currentIndexChanged(int)),SLOT(blueBandChanged(int)));
    }

    setWidgetActions(widgets);

    newCanvasLayer->setContrastEnhancement(QgsContrastEnhancement::StretchToMinimumMaximum,QgsRaster::ContrastEnhancementCumulativeCut,QgsRectangle(),0);

    emit appendLayers(QList<QgsMapLayer*>()<<canvasLayer());
    emit layerChanged();
}

void CDTImageLayer::setCategoryInfo(const CDTCategoryInformationList &info)
{
    QSqlDatabase db = QSqlDatabase::database("category");
    if (db.isValid()==false)
    {
        QMessageBox::critical(NULL,tr("Error"),tr("database is invalid"));
        return ;
    }
    QSqlQuery query(db);
    bool ret ;

    ret = query.prepare("insert into category VALUES(?,?,?,?)");
    if (ret == false)
    {
        QMessageBox::critical(NULL,tr("Error"),tr("insert data failed!\nerror:")+query.lastError().text());
        return ;
    }

    foreach (CategoryInformation inf, info) {
        query.bindValue(0,inf.id.toString());
        query.bindValue(1,inf.categoryName);
        query.bindValue(2,inf.color);
        query.bindValue(3,id().toString());
        query.exec();
    }
}

QString CDTImageLayer::path() const
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    query.exec("select path from imageLayer where id ='" + this->id().toString() +"'");
    query.next();
    return query.value(0).toString();
}

int CDTImageLayer::bandCount() const
{
    QgsRasterLayer* layer = (QgsRasterLayer*)canvasLayer();
    if (layer==NULL) return 0;
    return layer->bandCount();
}

QList<CDTImageLayer *> CDTImageLayer::getLayers()
{
    return layers;
}

CDTImageLayer *CDTImageLayer::getLayer(const QUuid &id)
{
    foreach (CDTImageLayer *layer, layers) {
        if (id == layer->id())
            return layer;
    }
    return NULL;
}

QVector<MergeLayer *> CDTImageLayer::GetMergeLayers()
{
    return merges;
}

QVector<OverLayer *> CDTImageLayer::GetOverLayers()
{
    return overlays;
}
QVector<BrokenDetect *> CDTImageLayer::GetBrokenDetectionLayers()
{
    return brokenDetection;
}

QVector<DetectLayer *> CDTImageLayer::GetDetectionLayers()
{
    return detections;
}

QVector<CDTExtractionLayer *> CDTImageLayer::GetExtractionLayers()
{
    return extractions;
}

QVector<CDTSegmentationLayer *> CDTImageLayer::GetSegmentationLyers()
{
    return segmentations;
}

CDTProjectTreeItem *CDTImageLayer::GetSegmentationsRoot()
{
    return segmentationsRoot;
}

CDTProjectTreeItem *CDTImageLayer::GetSegmentationsRoot_2()
{
    return segmentationsRoot_2;
}

CDTProjectTreeItem *CDTImageLayer::GetExtractionRoot()
{
    return extractionRoot;
}

CDTProjectTreeItem *CDTImageLayer::GetMainRootB()
{
    return MainRootB;
}

CDTProjectTreeItem *CDTImageLayer::GetMainRootA()
{
    return MainRootA;
}

CDTProjectTreeItem *CDTImageLayer::GetDetectRoot_2()
{
    return DetectRoot_2;
}

CDTProjectTreeItem *CDTImageLayer::GetDetectRoot()
{
    return DetectRoot;
}

CDTProjectTreeItem *CDTImageLayer::GetRoadDetectRoot()
{
    return RoadDetectRoot;
}

void CDTImageLayer::addBrokenDetection()
{
    DialogBrokenDetection *dlg = new DialogBrokenDetection(this->path(),this->fileSystem());
    if(dlg->exec()==DialogBrokenDetection::Accepted)
    {
        BrokenDetect *detection = new BrokenDetect(QUuid::createUuid(),this);
        detection->initLayer(dlg->shpid(),dlg->name(),
                             CDTDatabaseConnInfo(),Qt::red);
        RoadDetectRoot->appendRow(detection->standardKeyItem());
        addBrokenDetection(detection);
    }
    delete dlg;
}

void CDTImageLayer::addDetection()
{
    DialogDetectHogSVM *dlg = new DialogDetectHogSVM(/*this->id(),*/this->path(),this->fileSystem());
    if(dlg->exec()==DialogDetectHogSVM::Accepted){
        QStringList shpIDList = dlg->returnshpIDList();
//        QStringList shpPathList = dlg->returnshpPathList();
        QStringList shpNameList = dlg->returnshpNameList();
        int index = 0;
        foreach(QString tempShpID,shpIDList){
            DetectLayer *detection = new DetectLayer(QUuid::createUuid(),this);
            if(index == 0)
            {
                detection->initLayer(shpNameList.at(index),tempShpID,
                                     CDTDatabaseConnInfo(),dlg->borderColor());
            }else if(index == 1)
            {
                detection->initLayer(shpNameList.at(index),tempShpID,
                                     CDTDatabaseConnInfo(),Qt::green);
            }else if(index == 2)
            {
                detection->initLayer(shpNameList.at(index),tempShpID,
                                     CDTDatabaseConnInfo(),Qt::blue);
            }else if(index == 3)
            {
                detection->initLayer(shpNameList.at(index),tempShpID,
                                     CDTDatabaseConnInfo(),Qt::cyan);
            }else if(index == 4)
            {
                detection->initLayer(shpNameList.at(index),tempShpID,
                                     CDTDatabaseConnInfo(),Qt::yellow);
            }

            DetectRoot->appendRow(detection->standardKeyItem());
            addDetection(detection);
            index++;
        }
        delete dlg;
    }

}

void CDTImageLayer::addExtraction()
{
//    DialogNewExtraction *dlg = new DialogNewExtraction(this->id(),this->path(),this->fileSystem());
//    if(dlg->exec()==DialogNewExtraction::Accepted)
//    {
//        CDTExtractionLayer *extraction = new CDTExtractionLayer(QUuid::createUuid(),this);
//        extraction->initLayer(dlg->name(),dlg->fileID(),dlg->color(),dlg->borderColor()/*,dlg->opacity()*/);
//        extractionRoot->appendRow(extraction->standardKeyItem());
//        addExtraction(extraction);
//    }

//    delete dlg;
}

void CDTImageLayer::addSegmentation()
{
    DialogNewSegmentation* dlg = new DialogNewSegmentation(this->id(),this->path(),this->fileSystem());
    if(dlg->exec()==DialogNewSegmentation::Accepted)
    {
        CDTSegmentationLayer *segmentation = new CDTSegmentationLayer(QUuid::createUuid(),this);
        segmentation->initLayer(
                    dlg->name(),dlg->shapefileID(),dlg->markfileID(),
                    dlg->method(),dlg->params(),/*dlg->databaseConnInfo()*/CDTDatabaseConnInfo(),dlg->borderColor());
        segmentationsRoot->appendRow(segmentation->standardKeyItem());
        addSegmentation(segmentation);
    }
    delete dlg;
}

void CDTImageLayer::remove()
{
    emit removeLayer(QList<QgsMapLayer*>()<<canvasLayer());
    emit removeImageLayer(this);
}

void CDTImageLayer::removeExtraction(CDTExtractionLayer *ext)
{    
    int index = extractions.indexOf(ext);
    if (index>=0)
    {
        QStandardItem* keyItem = ext->standardKeyItem();
        keyItem->parent()->removeRow(keyItem->index().row());
        extractions.remove(index);
        emit removeLayer(QList<QgsMapLayer*>()<<ext->canvasLayer());
        fileSystem()->removeFile(ext->shapefileID());
        emit layerChanged();
    }
}

void CDTImageLayer::removeAllExtractionLayers()
{
    foreach (CDTExtractionLayer* ext, extractions) {        
        QStandardItem* keyItem = ext->standardKeyItem();
        keyItem->parent()->removeRow(keyItem->index().row());
        emit removeLayer(QList<QgsMapLayer*>()<<ext->canvasLayer());
        fileSystem()->removeFile(ext->shapefileID());
        delete ext;
    }
    extractions.clear();
    emit layerChanged();
}

void CDTImageLayer::removeSegmentation(CDTSegmentationLayer *sgmt)
{
    int index = segmentations.indexOf(sgmt);
    if (index>=0)
    {        
        QStandardItem* keyItem = sgmt->standardKeyItem();
        keyItem->parent()->removeRow(keyItem->index().row());
        segmentations.remove(index);
        emit removeLayer(QList<QgsMapLayer*>()<<sgmt->canvasLayer());
        fileSystem()->removeFile(sgmt->shapefilePath());
        fileSystem()->removeFile(sgmt->markfilePath());
        emit layerChanged();
    }
}

void CDTImageLayer::removeDetection(BrokenDetect *bedemt)
{
    int index = brokenDetection.indexOf(bedemt);
    if(index>=0)
    {
        QStandardItem *keyItem = bedemt->standardKeyItem();
        keyItem->parent()->removeRow(keyItem->index().row());
        brokenDetection.remove(index);
        emit removeLayer(QList<QgsMapLayer*>()<<bedemt->canvasLayer());
        fileSystem()->removeFile(bedemt->shapefilePath());
        emit layerChanged();
    }
}

void CDTImageLayer::removeDetection(DetectLayer *decmt)
{
    int index = detections.indexOf(decmt);
    if(index>=0)
    {
        QStandardItem *keyItem = decmt->standardKeyItem();
        keyItem->parent()->removeRow(keyItem->index().row());
        detections.remove(index);
        emit removeLayer(QList<QgsMapLayer*>()<<decmt->canvasLayer());
        fileSystem()->removeFile(decmt->shapefilePath());
        emit layerChanged();
    }
}

//void CDTImageLayer::removeMerge(MergeLayer *mgl)
//{
//    int index = merges.indexOf(mgl);
//    if(index>=0)
//    {
//        QStandardItem *keyItem = mgl->standardKeyItem();
//        keyItem->parent()->removeRow(keyItem->index().row());
//        merges.remove(index);
//        emit removeLayer(QList<QgsMapLayer*>()<<mgl->canvasLayer());
//        fileSystem()->removeFile(mgl->shapefileID());
//        emit layerChanged();
//    }

//}

void CDTImageLayer::removeAllSegmentationLayers()
{
    foreach (CDTSegmentationLayer* sgmt, segmentations) {        
        QStandardItem* keyItem = sgmt->standardKeyItem();
        keyItem->parent()->removeRow(keyItem->index().row());
        emit removeLayer(QList<QgsMapLayer*>()<<sgmt->canvasLayer());
        fileSystem()->removeFile(sgmt->shapefilePath());
        fileSystem()->removeFile(sgmt->markfilePath());
        delete sgmt;
    }
    segmentations.clear();
    emit layerChanged();
}

void CDTImageLayer::setLayerOpacity(int opacity)
{
    QgsRasterLayer *rasterLayer =  qobject_cast<QgsRasterLayer*>(canvasLayer());
    if (rasterLayer)
    {
        rasterLayer->renderer()->setOpacity(opacity/100.);
        canvas()->refresh();
    }
}

void CDTImageLayer::redBandChanged(int bandIDFrom0)
{
    rBandID = bandIDFrom0 + 1;
    updateRenderer();
}

void CDTImageLayer::greenBandChanged(int bandIDFrom0)
{
    gBandID = bandIDFrom0 + 1;
    updateRenderer();
}

void CDTImageLayer::blueBandChanged(int bandIDFrom0)
{
    bBandID = bandIDFrom0 + 1;
    updateRenderer();
}

void CDTImageLayer::onEnhancementChanged(int enhancementStyle)
{
    this->enhancementStyle = (QgsContrastEnhancement::ContrastEnhancementAlgorithm)enhancementStyle;
    updateRenderer();
}

void CDTImageLayer::addBrokenDetection(BrokenDetect *detection)
{
    brokenDetection.push_back(detection);
    emit layerChanged();
}

void CDTImageLayer::addDetection(DetectLayer *detection)
{
    detections.push_back(detection);
    emit layerChanged();
}

void CDTImageLayer::addExtraction(CDTExtractionLayer *extraction)
{
    extractions.push_back(extraction);
    emit layerChanged();
}


void CDTImageLayer::addSegmentation(CDTSegmentationLayer *segmentation)
{
    segmentations.push_back(segmentation);
    emit layerChanged();
}

void CDTImageLayer::updateRenderer()
{    
    QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>(this->canvasLayer());

    if (layer->bandCount()>=3)
    {
        //Band
        QgsRasterRenderer *renderer =
                new QgsMultiBandColorRenderer(layer->renderer(),rBandID,gBandID,bBandID);
        layer->setRenderer(renderer);
    }

    //Contrast
    layer->setContrastEnhancement(enhancementStyle,QgsRaster::ContrastEnhancementCumulativeCut,QgsRectangle(),0);
    MainWindow::getCurrentMapCanvas()->refresh();
}

QDataStream &operator<<(QDataStream &out, const CDTImageLayer &image)
{
    out<<image.id()<<image.path()<<image.name();

    out<<image.segmentations.size();
    for (int i=0;i<image.segmentations.size();++i)
        out<<*(image.segmentations[i]);

    out<<image.detections.size();
    for(int i=0;i<image.detections.size();++i)
        out<<*(image.detections[i]);

    out<<image.brokenDetection.size();
    for(int i =0;i<image.brokenDetection.size();i++)
        out<<*(image.brokenDetection[i]);

//    out<<image.extractions.size();
//    for (int i=0;i<image.extractions.size();++i)
//        out<<*(image.extractions[i]);

    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    bool ret;
    ret = query.exec("select id,name,color from category where imageID = '" + image.id() + "'");
    if (!ret) qDebug()<<query.lastError();

    CDTCategoryInformationList categoryInfo;
    while(query.next())
    {
        categoryInfo.push_back(CategoryInformation
            (query.value(0).toString(),query.value(1).toString(),query.value(2).value<QColor>()));
    }
    out<<categoryInfo;


    query.exec(QString("select id,name,pointset_name from image_validation_samples where imageid = '%1'").arg(image.id()));
    QList<QVariantList> validationPoints;
    while(query.next())
    {
        QVariantList record;
        for (int i=0;i<query.record().count();++i)
            record<<query.value(i);
        validationPoints.push_back(record);
    }
    out<<validationPoints;

    foreach (QVariantList list, validationPoints) {
        QString validationID = list[0].toString();
        query.exec(QString("select id,categoryid from point_category where validationid = '%1'").arg(validationID));
        QMap<int,QString> point_category;
        while(query.next())
        {
            point_category.insert(query.value(0).toInt(),query.value(1).toString());
        }
        out<<point_category;
        qDebug()<<point_category;
    }
    return out ;
}


QDataStream &operator>>(QDataStream &in, CDTImageLayer &image)
{
    QUuid id;
    in>>id;
    image.setID(id);

    QString name,path;
    in>>path;
    in>>name;
    image.initLayer(name,path);

    int count;
    in>>count;
    for (int i=0;i<count;++i)
    {
        CDTSegmentationLayer* segmentation = new CDTSegmentationLayer(QUuid(),&image);
        in>>(*segmentation);
        image.segmentationsRoot->appendRow(segmentation->standardKeyItem());
        image.segmentations.push_back(segmentation);
    }
    in>>count;
    for(int i=0;i<count;++i)
    {
        DetectLayer *detection = new DetectLayer(QUuid(),&image);
        in>>(*detection);
        image.DetectRoot->appendRow(detection->standardKeyItem());
        image.detections.push_back(detection);
    }
    in>>count;
    for(int i=0;i<count;++i)
    {
        BrokenDetect *brokenDetection = new BrokenDetect(QUuid(),&image);
        in>>(*brokenDetection);
        image.RoadDetectRoot->appendRow(brokenDetection->standardKeyItem());
        image.brokenDetection.push_back(brokenDetection);
    }
//    in>>count;
//    for (int i=0;i<count;++i)
//    {
//        CDTExtractionLayer* extraction = new CDTExtractionLayer(QUuid(),&image);
//        in>>(*extraction);
//        image.extractionRoot->appendRow(extraction->standardKeyItem());
//        image.extractions.push_back(extraction);
//    }

    CDTCategoryInformationList info;
    in>>info;
    image.setCategoryInfo(info);

    QList<QVariantList> validationPoints;
    in>>validationPoints;
    QSqlQuery query(QSqlDatabase::database("category"));
    query.prepare(QString("insert into image_validation_samples values(?,?,?,?)"));
    foreach (QVariantList record, validationPoints) {
        query.bindValue(0,record[0]);
        query.bindValue(1,record[1]);
        query.bindValue(2,image.id().toString());
        query.bindValue(3,record[2]);
        query.exec();
    }

    for (int i=0;i<validationPoints.size();++i)
    {
        QMap<int,QString> point_category;
        in >>point_category;
        query.prepare(QString("insert into point_category values(?,?,?)"));
        foreach (int id, point_category.keys()) {
            query.bindValue(0,id);
            query.bindValue(1,point_category.value(id));
            query.bindValue(2,validationPoints[i][0].toString());
            query.exec();
        }
    }

    return in;
}

QDataStream &operator <<(QDataStream &out,const CategoryInformation &categoryInformation)
{
    out<<categoryInformation.id<<categoryInformation.categoryName<<categoryInformation.color;
    return out;
}

QDataStream &operator >>(QDataStream &in, CategoryInformation &categoryInformation)
{
    in>>categoryInformation.id>>categoryInformation.categoryName>>categoryInformation.color;
    return in;
}
