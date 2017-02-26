#include "detectlayer.h"
#include "stable.h"
#include "cdtprojecttreeitem.h"
#include "qtcolorpicker.h"
#include "cdtfilesystem.h"
#include "mainwindow.h"
#include "cdtprojectlayer.h"
#include "cdtimagelayer.h"
#include "cdtvariantconverter.h"
#include "cdtextractionlayer.h"
#include "dialognewextraction.h"

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include<qgsapplication.h>


QList<DetectLayer *> DetectLayer::layers;

DetectLayer::DetectLayer(QUuid uuid, QObject *parent) :
    CDTBaseLayer(uuid,parent)
{
    layers.push_back(this);

    CDTProjectTreeItem *keyItem =
            new CDTProjectTreeItem(CDTProjectTreeItem::NANHAI,CDTProjectTreeItem::GROUP,QString(),this);
    autoextraction =
            new CDTProjectTreeItem(CDTProjectTreeItem::EXTRACTIONV2_ROOT,CDTProjectTreeItem::VECTOR,tr("Auto Extraction"),this);
    extractionRootItem =
            new CDTProjectTreeItem(CDTProjectTreeItem::EXTRACTIONV2_ROOT,CDTProjectTreeItem::EMPTY,tr("extraction"),this);
    //mergeshp =
            //new CDTProjectTreeItem(CDTProjectTreeItem::NANHAI,CDTProjectTreeItem::EMPTY,tr("mergeshp"),this);

    keyItem->appendRow(autoextraction);
    keyItem->appendRow(extractionRootItem);

    //autoextraction->appendRow(mergeshp);

    setKeyItem(keyItem);

    QAction *actionRename =
            new QAction(QIcon(":/Icons/Rename.png"),tr("Rename Detection"),this);
    QAction *actionExportShapefile =
            new QAction(QIcon(":/Icons/Export.png"),tr("Export Shapefile"),this);
    QAction *actionRemoveDetection =
            new QAction(QIcon(":/Icons/Remove.png"),tr("Remove Detection"),this);

    QAction *actionAddExtractionLayer  =
            new QAction(QIcon(":/Icons/Add.png"),tr("Add Extraction"),this);
    QAction *actionRemoveAllExtractions     =
            new QAction(QIcon(":/Icons/Remove.png"),tr("Remove All Extractions"),this);

    QAction *actionDeleteShp     =
            new QAction(QIcon(":/Icons/Remove.png"),tr("Delete Shp"),this);


    setActions(QList<QList<QAction *> >()
               <<(QList<QAction *>()<<actionRename
                                    <<actionExportShapefile
                                    <<actionRemoveDetection)
               <<(QList<QAction *>()<<actionAddExtractionLayer
                                    <<actionRemoveAllExtractions));

    connect(actionRemoveAllExtractions,SIGNAL(triggered()),SLOT(removeAllExtractionLayers()));
    connect(actionAddExtractionLayer,SIGNAL(triggered()),SLOT(addExtraction()));

    connect(actionRename,SIGNAL(triggered()),SLOT(rename()));
    connect(actionExportShapefile,SIGNAL(triggered()),SLOT(exportShapefile()));
    connect(actionRemoveDetection,SIGNAL(triggered()),SLOT(remove()));
    connect(actionDeleteShp,SIGNAL(triggered()),SLOT(deleteShpFeature()));
    connect(this,SIGNAL(removeDetection(DetectLayer*)),this->parent(),SLOT(removeDetection(DetectLayer*)));
}

DetectLayer::~DetectLayer()
{
    if (id().isNull())
        return;

    QSqlQuery query(QSqlDatabase::database("category"));
    bool ret;
    ret = query.exec("delete from detectionlayer where id = '"+id().toString()+"'");
    if (!ret)
        qWarning()<<"prepare:"<<query.lastError().text();

    layers.removeAll(this);
}

QColor DetectLayer::borderColor() const
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    query.exec("select bordercolor from detectionlayer where id ='" + this->id().toString() +"'");
    query.next();
    return query.value(0).value<QColor>();

}

void DetectLayer::setRenderer(QgsFeatureRendererV2 *r)
{
    QgsVectorLayer*p = qobject_cast<QgsVectorLayer*>(canvasLayer());
    if (p)
        p->setRendererV2(r);
}

void DetectLayer::setOriginRenderer()
{
    QgsSimpleFillSymbolLayerV2* symbolLayer = new QgsSimpleFillSymbolLayerV2();
    symbolLayer->setColor(QColor(0,0,0,0));
    symbolLayer->setBorderColor(borderColor()/*Qt::red*/);
    QgsFillSymbolV2 *fillSymbol = new QgsFillSymbolV2(QgsSymbolLayerV2List()<<symbolLayer);
    QgsSingleSymbolRendererV2* singleSymbolRenderer = new QgsSingleSymbolRendererV2(fillSymbol);
    this->setRenderer(singleSymbolRenderer);
}

QString DetectLayer::shapefilePath() const
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    query.exec("select shapefilePath from detectionlayer where id ='" + this->id().toString() +"'");
    query.next();
    return query.value(0).toString();
}

void DetectLayer::addExtraction(CDTExtractionLayer *extraction)
{
    extractions.push_back(extraction);
    emit layerChanged();
}

void DetectLayer::remove()
{
    emit removeDetection(this);
}

void DetectLayer::setName(const QString &name)
{
    if (this->name() == name)
        return;

    if (name.isEmpty())
        return;

    bool ret = false;
    QSqlQuery query(QSqlDatabase::database("category"));
    ret = query.prepare(QString("UPDATE %1 set name = ? where id =?").arg(tableName()));
    if (ret==false) return;
    query.bindValue(0,name);
    query.bindValue(1,this->id().toString());
    ret = query.exec();
    if (ret==false) return;

    keyItem()->setText(name);
    emit nameChanged(name);
}

void DetectLayer::rename()
{
    bool ok;
    QString text = QInputDialog::getText(
                NULL, tr("Input New Name"),
                tr("Rename:"), QLineEdit::Normal,
                this->name(), &ok);
    if (ok && !text.isEmpty())
        setName(text);
}

void DetectLayer::deleteShpFeature()
{
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer*>(canvasLayer());

    QString tempShpPath = layer->name()+".shp";
    QgsVectorLayer *newLayer = new QgsVectorLayer(/*shpPath*/tempShpPath,QFileInfo(/*shpPath*/tempShpPath).completeBaseName(),"ogr");
    if (!newLayer->isValid())
    {
        QMessageBox::critical(NULL,tr("Error"),tr("Open shapefile ")+tempShpPath+tr(" failed!"));
        delete newLayer;
        return;
    }
    //qDebug()<<"The message is"<<tempShpPath<<newLayer->name();

    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");

    OGRSFDriver* poOgrDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
    if (poOgrDriver == NULL)
    {
        qDebug()<<"Get ESRI SHAPEFILE Driver Failed";
        return ;
    }

    OGRDataSource* poDstDataset = poOgrDriver->Open(tempShpPath.toUtf8().constData(),1);
    if (poDstDataset == NULL)
    {
        qDebug()<<"Open Shapefile Failed!";
        return ;
    }

    OGRLayer *poLayer = poDstDataset->GetLayer(0);
    poLayer->ResetReading();

    if(!poLayer->TestCapability("OLCDeleteFeature"))
    {
        qDebug()<<"can not delete features!";
    }

    OGRFeatureDefn *pFeatureDefn = NULL;
    pFeatureDefn = poLayer->GetLayerDefn();
    std::string strLayerName = pFeatureDefn->GetName();
    poLayer->DeleteFeature(0);
    poLayer->DeleteFeature(1);
    poLayer->DeleteFeature(2);
    poLayer->DeleteFeature(3);
    std::string strSQL = "REPACK " + strLayerName;
    poDstDataset->ExecuteSQL(strSQL.c_str(),NULL," ");

//    while((poFeature = poLayer->GetNextFeature()) != NULL)
//    {
//        poLayer->DeleteFeature(0);
//        poLayer->DeleteFeature(1);
//        poLayer->DeleteFeature(2);
//        poLayer->DeleteFeature(3);
//        poLayer->DeleteFeature(4);
//    }
//    poLayer->DeleteFeature(0);
//    QString sql = "REPACK "+QString::fromUtf8(poLayer->GetName());
//    poDstDataset->ExecuteSQL(sql.toUtf8().constData(),NULL,"");
    OGRDataSource::DestroyDataSource(poDstDataset);
    OGRCleanupAll();
    emit layerChanged();
}
void DetectLayer::initLayer(const QString &name,
                            const QString &shpPath,
                            CDTDatabaseConnInfo url,
                            const QColor &color)
{
    QString tempShpPath;
    this->fileSystem()->getFile(shpPath,tempShpPath);
//    qDebug()<<"im "+this->id();
    QgsVectorLayer *newLayer = new QgsVectorLayer(/*shpPath*/tempShpPath,QFileInfo(/*shpPath*/tempShpPath).completeBaseName(),"ogr");
    if (!newLayer->isValid())
    {
        QMessageBox::critical(NULL,tr("Error"),tr("Open shapefile ")+tempShpPath+tr(" failed!"));
        delete newLayer;
        return;
    }

    //qDebug()<<"123456"<<tempShpPath<<shpPath<<newLayer->name();

    setCanvasLayer(newLayer);
    autoextraction->setMapLayer(newLayer);
    connect(newLayer,SIGNAL(layerTransparencyChanged(int)),this,SIGNAL(layerTransparencyChanged(int)));

    keyItem()->setText(name);

    QSqlQuery query(QSqlDatabase::database("category"));
    bool ret ;
    ret = query.prepare("insert into detectionlayer VALUES(?,?,?,?,?,?)");
    if (ret==false)
    {
        logger()->error("Init detectionLayer Fialed!");
        delete newLayer;
        return;
    }

    query.bindValue(0,id().toString());
    query.bindValue(1,name);
    query.bindValue(2,shpPath);
//    query.bindValue(3,mkPath);
//    query.bindValue(4,method);
//    query.bindValue(5,dataToVariant(params));

    query.bindValue(3,dataToVariant(url));
    query.bindValue(4,color);
    query.bindValue(5,((CDTImageLayer*)parent())->id().toString());
    ret = query.exec();
    if (ret==false)
    {
        logger()->error("Init detectionLayer Fialed!");
        delete newLayer;
        return;
    }

    //dynamic properties
//    foreach (QString key, params.keys()) {
//        this->setProperty((QString("   ")+key).toLocal8Bit().constData(),params.value(key.toLocal8Bit().constData()));
//    }

    QList<QPair<QLabel*,QWidget*>> widgets;
    //Widgets for context menu
    QtColorPicker *borderColorPicker = new QtColorPicker(NULL);
    borderColorPicker->setStandardColors();           //show standard colors
    borderColorPicker->setToolTip(tr("Border color"));
    connect(borderColorPicker,SIGNAL(colorChanged(QColor)),SLOT(setBorderColor(QColor)));     //change colors
    connect(this,SIGNAL(borderColorChanged(QColor)),borderColorPicker,SLOT(setCurrentColor(QColor)));
    connect(this,SIGNAL(destroyed()),borderColorPicker,SLOT(deleteLater()));
    widgets.append(qMakePair(new QLabel(tr("Border color")),(QWidget*)borderColorPicker));


    QSlider *sliderTransparency = new QSlider(Qt::Horizontal,NULL);
    sliderTransparency->setMinimum(0);
    sliderTransparency->setMaximum(100);
    sliderTransparency->setToolTip(tr("Transparency"));
    connect(sliderTransparency,SIGNAL(valueChanged(int)),SLOT(setLayerTransparency(int)));
    connect(this,SIGNAL(layerTransparencyChanged(int)),sliderTransparency,SLOT(setValue(int)));
    connect(this,SIGNAL(destroyed()),sliderTransparency,SLOT(deleteLater()));
    widgets.append(qMakePair(new QLabel(tr("Transparency")),(QWidget*)sliderTransparency));
    setWidgetActions(widgets);

    setOriginRenderer();

    emit nameChanged(name);
    emit borderColorChanged(color);
    emit appendLayers(QList<QgsMapLayer*>()<<canvasLayer());
    emit layerChanged();
}

void DetectLayer::setBorderColor(const QColor &clr)
{
    if (this->borderColor() == clr)
        return;
    QSqlQuery query(QSqlDatabase::database("category"));
    query.prepare("UPDATE detectionlayer set bordercolor = ? where id =?");
    query.bindValue(0,clr);
    query.bindValue(1,this->id().toString());
    query.exec();

    setOriginRenderer();
    canvas()->refresh();
    emit borderColorChanged(clr);
    emit layerChanged();
}

void DetectLayer::setLayerTransparency(int transparency)
{
    QgsVectorLayer*p = qobject_cast<QgsVectorLayer*>(canvasLayer());
    if (p)
    {
        p->setLayerTransparency(transparency);
        canvas()->refresh();
    }
}

void DetectLayer::exportShapefile()
{
    QString id = shapefilePath();
    fileSystem()->exportFiles(id);
}

void DetectLayer::addExtraction()
{
    QSqlQuery query(QSqlDatabase::database("category"));
    query.exec("select imageID from detectionlayer where id ='" + this->id().toString() +"'");
    query.next();
    QString imageID = query.value(0).toString();
    query.exec("select path from imagelayer where id ='" + imageID +"'");
    query.next();
    QString imagePath = query.value(0).toString();

    DialogNewExtraction *dlg = new DialogNewExtraction(imageID,imagePath,this->fileSystem());
    if(dlg->exec()==DialogNewExtraction::Accepted)
    {
        QSqlQuery query(QSqlDatabase::database("category"));
        query.exec("select imageID from detectionlayer where id ='" + this->id().toString() +"'");
        query.next();
        QString imageID = query.value(0).toString();
        CDTExtractionLayer *extraction = new CDTExtractionLayer(QUuid::createUuid(),this);
        extraction->initLayer(dlg->name(),dlg->fileID(),dlg->color(),dlg->borderColor(),imageID/*,dlg->opacity()*/);
        extractionRootItem->appendRow(extraction->standardKeyItem());
        addExtraction(extraction);
    }

    delete dlg;
}

void DetectLayer::removeExtraction(CDTExtractionLayer *ext)
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

void DetectLayer::removeAllExtractionLayers()
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


QDataStream &operator<<(QDataStream &out, const DetectLayer &detection)
{
    QSqlQuery query(QSqlDatabase::database("category"));
    query.exec("select * from detectionlayer where id ='" + detection.id().toString() +"'");
    query.next();

    out <<detection.id()         //id
       <<query.value(1).toString() //name
      <<query.value(2).toString() //shapfile
    <<query.value(3)//dbUrl
    <<query.value(4);//Border Color

    out<<detection.extractions.size();
    for (int i=0;i<detection.extractions.size();++i)
        out<<*(detection.extractions[i]);

    return out;
}


QDataStream &operator>>(QDataStream &in, DetectLayer &detection)
{
    QUuid id;
    in>>id;
    detection.setID(id);

    QString name,shp;
    in>>name>>shp;
    QVariant temp;
    in>>temp;
    CDTDatabaseConnInfo url = variantToData<CDTDatabaseConnInfo>(temp);
    in>>temp;
    QColor color = temp.value<QColor>();

    detection.initLayer(name,shp,url,color);

    int count;
    in>>count;
    for (int i=0;i<count;++i)
    {
        CDTExtractionLayer* extraction = new CDTExtractionLayer(QUuid(),&detection);
        in>>(*extraction);
        detection.extractionRootItem->appendRow(extraction->standardKeyItem());

        detection.extractions.push_back(extraction);
    }

    return in;
}
