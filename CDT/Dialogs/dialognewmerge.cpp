#include "dialognewmerge.h"
#include "ui_dialognewmerge.h"
#include "cdtfilesystem.h"
#include "stable.h"
#include "mergelayer.h"
#include "cdtlayernamevalidator.h"

DialogNewMerge::DialogNewMerge(const QString shpfilePathA, const QString shpfilePathB, CDTFileSystem *fileSys, QWidget *parent) :
    QDialog(parent),
    _shpfilePathA(shpfilePathA),
    _shpfilePathB(shpfilePathB),
    fileSystem(fileSys),
    ui(new Ui::DialogNewMerge)
{
    ui->setupUi(this);
    shapefileTempPath = QDir::tempPath()+"/"+QUuid::createUuid().toString()+".shp";
    ui->colorPicker->setStandardColors();
    ui->borderColorPicker->setStandardColors();

    connect(ui->pushButton,SIGNAL(clicked()),SLOT(onAccepted()));


    int index = MergeLayer::staticMetaObject.indexOfClassInfo("tableName");
//    if (index != -1)
//    {
//        CDTLayerNameValidator *validator = new CDTLayerNameValidator
//                (QSqlDatabase::database("category"),"name",CDTExtractionLayer::staticMetaObject.classInfo(index).value(),QString("imageid='%1'").arg(imageID));
//        ui->lineEditName->setValidator(validator);
//    }

    ui->lineEditName->setText(tr("Untitled"));
    shapefileTempPath = QDir::tempPath()+"/"+QUuid::createUuid().toString()+".shp";
}

DialogNewMerge::~DialogNewMerge()
{
    delete ui;
}

QString DialogNewMerge::name() const
{
    return ui->lineEditName->text();
}

QColor DialogNewMerge::color() const
{
    return ui->colorPicker->currentColor();
}

QColor DialogNewMerge::borderColor() const
{
    return ui->borderColorPicker->currentColor();
}

QString DialogNewMerge::fileID() const
{
    return shapefileID;
}

void DialogNewMerge::onAccepted()
{
    if(_shpfilePathA.isEmpty()){
        return;
    }
    if(_shpfilePathB.isEmpty()){
        return;
    }
    //qDebug()<< "The shpA is "<<_shpfilePathA;
    //qDebug()<< "The shpB is "<<_shpfilePathB;

    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","YES");

    OGRSFDriver* poOgrDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
    if (poOgrDriver == NULL)
    {
        qDebug()<<"Get ESRI SHAPEFILE Driver Failed";
        return ;
    }

    shapefileTempPath = QDir::tempPath()+"/"+DialogNewMerge::name()+".shp";

    OGRDataSource* poDstDatasetA = poOgrDriver->Open(_shpfilePathA.toUtf8().constData(),1);
    OGRDataSource* poDstDatasetB = poOgrDriver->Open(_shpfilePathB.toUtf8().constData(),1);

    QFileInfo info(shapefileTempPath);
    if (info.exists())
        poOgrDriver->DeleteDataSource(shapefileTempPath.toUtf8().constData());

    OGRDataSource* poDstDatasetResult =
                   poOgrDriver->CreateDataSource(shapefileTempPath.toUtf8().constData(),0);
    if (poDstDatasetA == NULL ||poDstDatasetB == NULL ||poDstDatasetResult == NULL)
    {
        qDebug()<<"Open Shapefile Failed!";
        return ;
    }

    OGRLayer *poLayerA = poDstDatasetA->GetLayer(0);
    OGRLayer *poLayerB = poDstDatasetB->GetLayer(0);

    const char* layerName = "polygon";
    OGRLayer* poLayerResult = poDstDatasetResult->CreateLayer(layerName,poLayerA->GetSpatialRef(),wkbMultiPolygon,0);

    char **p = new char *[4];
    p[0] = "SKIP_FAILURES=YES";
    p[1] = "PROMOTE_TO_MULTI=YES";
    p[2] = "INPUT_PREFIX=1";
    p[3] = "METHOD_PREFIX=2";
    poLayerA->Union(poLayerB,poLayerResult,p,NULL,NULL);

    poLayerResult->SyncToDisk();

    OGRDataSource::DestroyDataSource(poDstDatasetA);
    OGRDataSource::DestroyDataSource(poDstDatasetB);
    OGRDataSource::DestroyDataSource(poDstDatasetResult);

    shapefileID = QUuid::createUuid().toString();
    fileSystem->registerFile(shapefileID,shapefileTempPath,QString(),QString()
                             ,CDTFileSystem::getShapefileAffaliated(shapefileTempPath));
    this->accept();
}
