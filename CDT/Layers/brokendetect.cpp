#include "brokendetect.h"
#include "stable.h"
#include "cdtprojecttreeitem.h"
#include "qtcolorpicker.h"
#include "cdtfilesystem.h"
#include "mainwindow.h"
#include "cdtprojectlayer.h"
#include "cdtimagelayer.h"
#include "cdtvariantconverter.h"

QList<BrokenDetect *> BrokenDetect::layers;

BrokenDetect::BrokenDetect(QUuid uuid, QObject *parent) :
    CDTBaseLayer(uuid,parent)
{
    layers.push_back(this);
    CDTProjectTreeItem *keyItem =
            new CDTProjectTreeItem(CDTProjectTreeItem::NANHAI,CDTProjectTreeItem::VECTOR,QString(),this);

    setKeyItem(keyItem);

    QAction *actionRename =
            new QAction(QIcon(":/Icons/Rename.png"),tr("Rename Detection"),this);
    QAction *actionExportShapefile =
            new QAction(QIcon(":/Icons/Export.png"),tr("Export Shapefile"),this);
    QAction *actionRemoveDetection =
            new QAction(QIcon(":/Icons/Remove.png"),tr("Remove Detection"),this);

    setActions(QList<QList<QAction *> >()
               <<(QList<QAction *>()<<actionRename
                                    <<actionExportShapefile
                                    <<actionRemoveDetection));

    connect(actionRename,SIGNAL(triggered()),SLOT(rename()));
    connect(actionExportShapefile,SIGNAL(triggered()),SLOT(exportShapefile()));
    connect(actionRemoveDetection,SIGNAL(triggered()),SLOT(remove()));
    connect(this,SIGNAL(removeDetection(BrokenDetect*)),this->parent(),SLOT(removeDetection(BrokenDetect*)));
}

BrokenDetect::~BrokenDetect()
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

QColor BrokenDetect::borderColor() const
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    query.exec("select bordercolor from detectionlayer where id ='" + this->id().toString() +"'");
    query.next();
    return query.value(0).value<QColor>();
}

void BrokenDetect::setRenderer(QgsFeatureRendererV2 *r)
{
    QgsVectorLayer*p = qobject_cast<QgsVectorLayer*>(canvasLayer());
    if (p)
        p->setRendererV2(r);
}

void BrokenDetect::setOriginRenderer()
{
    QgsSimpleFillSymbolLayerV2* symbolLayer = new QgsSimpleFillSymbolLayerV2();
    symbolLayer->setColor(QColor(0,0,0,0));
    symbolLayer->setBorderColor(borderColor()/*Qt::red*/);
    QgsFillSymbolV2 *fillSymbol = new QgsFillSymbolV2(QgsSymbolLayerV2List()<<symbolLayer);
    QgsSingleSymbolRendererV2* singleSymbolRenderer = new QgsSingleSymbolRendererV2(fillSymbol);
    this->setRenderer(singleSymbolRenderer);
}

QString BrokenDetect::shapefilePath() const
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    query.exec("select shapefilePath from detectionlayer where id ='" + this->id().toString() +"'");
    query.next();
    return query.value(0).toString();
}

void BrokenDetect::remove()
{
    emit removeDetection(this);
}

void BrokenDetect::setName(const QString &name)
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

void BrokenDetect::rename()
{
    bool ok;
    QString text = QInputDialog::getText(
                NULL, tr("Input New Name"),
                tr("Rename:"), QLineEdit::Normal,
                this->name(), &ok);
    if (ok && !text.isEmpty())
        setName(text);
}

void BrokenDetect::initLayer(const QString &name, const QString &shpPath, CDTDatabaseConnInfo url, const QColor &color)
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

    setCanvasLayer(newLayer);
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
    borderColorPicker->setStandardColors();
    borderColorPicker->setToolTip(tr("Border color"));
    connect(borderColorPicker,SIGNAL(colorChanged(QColor)),SLOT(setBorderColor(QColor)));
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

void BrokenDetect::setBorderColor(const QColor &clr)
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

void BrokenDetect::setLayerTransparency(int transparency)
{
    QgsVectorLayer*p = qobject_cast<QgsVectorLayer*>(canvasLayer());
    if (p)
    {
        p->setLayerTransparency(transparency);
        canvas()->refresh();
    }
}

void BrokenDetect::exportShapefile()
{
    QString id = shapefilePath();
    fileSystem()->exportFiles(id);
}


QDataStream &operator<<(QDataStream &out, const BrokenDetect &detection)
{
    QSqlQuery query(QSqlDatabase::database("category"));
    query.exec("select * from detectionlayer where id ='" + detection.id().toString() +"'");
    query.next();

    out <<detection.id()         //id
       <<query.value(1).toString() //name
      <<query.value(2).toString() //shapfile
    <<query.value(3)//dbUrl
    <<query.value(4);//Border Color

    return out;
}


QDataStream &operator>>(QDataStream &in, BrokenDetect &detection)
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

    return in;
}
