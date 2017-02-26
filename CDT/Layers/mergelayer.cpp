#include "mergelayer.h"
#include "stable.h"
#include "cdtextractionlayer.h"
#include "cdtprojecttreeitem.h"
#include "qtcolorpicker.h"
#include "cdtfilesystem.h"

#include "mainwindow.h"
#include "cdtprojectlayer.h"
#include "cdtimagelayer.h"
#include "cdtvariantconverter.h"
#include "cdtextractionlayer.h"

QList<MergeLayer *> MergeLayer::layers;

MergeLayer::MergeLayer(QUuid uuid, QObject *parent):
    CDTBaseLayer(uuid,parent)
{
    layers.push_back(this);
    CDTProjectTreeItem *keyItem =
            new CDTProjectTreeItem(CDTProjectTreeItem::NANHAI,CDTProjectTreeItem::VECTOR,QString(),this);

    setKeyItem(keyItem);

    QAction *actionRename =
            new QAction(QIcon(":/Icons/Rename.png"),tr("Rename Merge"),this);
    QAction *actionExportShapefile =
            new QAction(QIcon(":/Icons/Export.png"),tr("Export Shapefile"),this);
    QAction *actionRemoveMerge =
            new QAction(QIcon(":/Icons/Remove.png"),tr("Remove Merge"),this);

    setActions(QList<QList<QAction *> >()
               <<(QList<QAction *>()<<actionRename
                                    <<actionExportShapefile
                                    <<actionRemoveMerge));

    connect(actionRename,SIGNAL(triggered()),SLOT(rename()));
    connect(actionExportShapefile,SIGNAL(triggered()),SLOT(exportShapefile()));
    connect(actionRemoveMerge,SIGNAL(triggered()),SLOT(remove()));
    connect(this,SIGNAL(removeMerge(MergeLayer*)),this->parent(),SLOT(removeMerge(MergeLayer*)));

}

MergeLayer::~MergeLayer()
{
    if (id().isNull())
        return;

    QSqlQuery query(QSqlDatabase::database("category"));
    bool ret;
    ret = query.exec("delete from mergelayer where id = '"+id().toString()+"'");
    if (!ret)
        qWarning()<<"prepare:"<<query.lastError().text();

    layers.removeAll(this);

}

QString MergeLayer::shapefileID() const
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    query.exec("select shapefilePath from mergelayer where id ='" + this->id().toString() +"'");
    query.next();
    return query.value(0).toString();
}

QColor MergeLayer::color() const
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    query.exec("select color from mergelayer where id ='" + this->id().toString() +"'");
    query.next();
    return query.value(0).value<QColor>();
}

QColor MergeLayer::borderColor() const
{
    QSqlDatabase db = QSqlDatabase::database("category");
    QSqlQuery query(db);
    query.exec("select borderColor from mergelayer where id ='" + this->id().toString() +"'");
    query.next();
    return query.value(0).value<QColor>();
}

int MergeLayer::layerTransparency() const
{
    QgsVectorLayer*p = qobject_cast<QgsVectorLayer*>(canvasLayer());
    if (p)
        return p->layerTransparency();
    else
        return -1;
}

void MergeLayer::setRenderer(QgsFeatureRendererV2 *r)
{
    QgsVectorLayer*p = (QgsVectorLayer*)canvasLayer();
    if (p!=NULL)
    {
        p->setRendererV2(r);
    }
}

void MergeLayer::setOriginRenderer()
{
    QgsSimpleFillSymbolLayerV2* symbolLayer = new QgsSimpleFillSymbolLayerV2();
    symbolLayer->setColor(QColor(0,0,0,0));
    symbolLayer->setBorderColor(borderColor());
    QgsFillSymbolV2 *fillSymbol = new QgsFillSymbolV2(QgsSymbolLayerV2List()<<symbolLayer);
    QgsSingleSymbolRendererV2* singleSymbolRenderer = new QgsSingleSymbolRendererV2(fillSymbol);
    this->setRenderer(singleSymbolRenderer);
}

//QList<MergeLayer *> MergeLayer::getLayers()
//{
//    return layers;
//}

void MergeLayer::remove()
{
    emit removeMerge(this);
}

void MergeLayer::setName(const QString &name)
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

void MergeLayer::rename()
{
    bool ok;
    QString text = QInputDialog::getText(
                NULL, tr("Input New Name"),
                tr("Rename:"), QLineEdit::Normal,
                this->name(), &ok);
    if (ok && !text.isEmpty())
        setName(text);
}

void MergeLayer::initLayer(const QString &name, const QString &shpID, const QColor &color, const QColor &borderColor)
{
    QString tempShpPath;
    this->fileSystem()->getFile(shpID,tempShpPath);
    qDebug()<<tempShpPath;
    QgsVectorLayer *newLayer = new QgsVectorLayer(tempShpPath,QFileInfo(tempShpPath).completeBaseName(),"ogr");
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
    ret = query.prepare("insert into mergelayer VALUES(?,?,?,?,?)");
    if (ret==false)
    {
        qDebug()<<"Prepare 'insert into mergelayer failed!'";
        return;
    }
    query.addBindValue(id().toString());
    query.addBindValue(name);
    query.addBindValue(shpID);
    query.addBindValue(color);
    query.addBindValue(borderColor);
//    query.addBindValue(((CDTExtractionLayer*)parent())->id().toString());
    ret = query.exec();
    if (ret==false)
    {
        qDebug()<<"insert into mergelayer failed!";
        return;
    }

    QList<QPair<QLabel*,QWidget*>> widgets;
//    QtColorPicker *colorPicker = new QtColorPicker();
//    colorPicker->setStandardColors();
//    colorPicker->setCurrentColor(color);
//    connect(colorPicker,SIGNAL(colorChanged(QColor)),SLOT(setColor(QColor)));
//    connect(this,SIGNAL(colorChanged(QColor)),colorPicker,SLOT(setCurrentColor(QColor)));
//    widgets.append(qMakePair(new QLabel(tr("Color")),(QWidget*)colorPicker));

    QtColorPicker *borderColorPicker = new QtColorPicker();
    borderColorPicker->setStandardColors();
    borderColorPicker->setCurrentColor(borderColor);
    connect(borderColorPicker,SIGNAL(colorChanged(QColor)),SLOT(setBorderColor(QColor)));
    connect(this,SIGNAL(borderColorChanged(QColor)),borderColorPicker,SLOT(setCurrentColor(QColor)));
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

    setWidgetActions(widgets);

    setOriginRenderer();

    emit appendLayers(QList<QgsMapLayer*>()<<canvasLayer());
    emit layerChanged();
}

void MergeLayer::setBorderColor(const QColor &clr)
{
    if (this->borderColor() == clr)
        return;
    QSqlQuery query(QSqlDatabase::database("category"));
    query.prepare("UPDATE mergelayer set bordercolor = ? where id =?");
    query.bindValue(0,clr);
    query.bindValue(1,this->id().toString());
    query.exec();

    setOriginRenderer();
    this->canvas()->refresh();

    emit borderColorChanged(clr);
    emit layerChanged();
}

void MergeLayer::setLayerTransparency(int transparency)
{
    QgsVectorLayer*p = qobject_cast<QgsVectorLayer*>(canvasLayer());
    if (p)
    {
        p->setLayerTransparency(transparency);
        canvas()->refresh();
    }
}

void MergeLayer::exportShapefile()
{
    QString id = shapefileID();
    fileSystem()->exportFiles(id);
}


QDataStream &operator<<(QDataStream &out, const MergeLayer &merge)
{
    QSqlQuery query(QSqlDatabase::database("category"));
    query.exec("select * from mergelayer where id ='" + merge.id().toString() +"'");
    query.next();

    out<<merge.id()//id
      <<query.value(1).toString()       //name
     <<query.value(2).toString()        //shapefile
    <<query.value(3).value<QColor>()    //color
    <<query.value(4).value<QColor>();    //border color
//    <<-1        //deprecated
//    <<query.value(5).toString();
    return out;
}


QDataStream &operator>>(QDataStream &in, MergeLayer &merge)
{
    QUuid id;
    QString name,shp;
    QColor color,borderColor;

    in>>id>>name>>shp>>color>>borderColor;
    merge.setID(id);
    merge.initLayer(name,shp,color,borderColor);
    return in;
}
