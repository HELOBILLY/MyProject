#include "dialogbrokendetection.h"
#include "ui_dialogbrokendetection.h"
#include "stable.h"
#include "cdtfilesystem.h"
//#include "leoanalysisattributes.h"
#include "dialoglabelit.h"
#include <gdal_alg.h>
//#include <stxxl/vector>
//typedef stxxl::VECTOR_GENERATOR<QPoint>::result vectorQPoint;
using namespace cv;

DialogBrokenDetection::DialogBrokenDetection(
        const QString &inputImage,
        CDTFileSystem *fileSys,
        QWidget *parent) :
    fileSystem(fileSys),
    _imageFilePath(inputImage),
    QDialog(parent),
    ui(new Ui::DialogBrokenDetection)
{
    ui->setupUi(this);
    setWindowTitle(tr("Broken Detection"));
    setWindowIcon(QIcon(":/images/broken.ico"));
    QVBoxLayout *mainLayOut = new QVBoxLayout(this);
    QGridLayout *downLayOut = new QGridLayout();

    QGroupBox *inputGroupBox = new QGroupBox(this);
    QGroupBox *chooseGroupBox = new QGroupBox(this);

    QLabel      *shpFileLabel = new QLabel(this);
    shpFileLabel->setText(tr("ShpFile Name:"));
    shpFilePath = new QComboBox(this);
    QPushButton *shpFileButton = new QPushButton(this);
    shpFileButton->setText(tr("Input Path"));

    QLabel *newNameLabel = new QLabel(this);
    newNameLabel->setText(tr("Type a Shpfile Name:"));
    newNameEdit = new QLineEdit(this);

//    QLabel      *imageFileLabel = new QLabel(this);
//    imageFileLabel->setText(tr("Image Name:  "));
//    imageFilePath = new QComboBox(this);
//    QPushButton *imageFileButton = new QPushButton(this);
//    imageFileButton->setText(tr("Input Path"));

    QLabel      *fieldLabel = new QLabel(this);
    fieldLabel->setText(tr("Field Name:"));
    fieldComBox = new QComboBox(this);

    QLabel      *valueLabel = new QLabel(this);
    valueLabel->setText(tr("Value Name:"));
    valueComBox = new QComboBox(this);

    QPushButton *startButton = new QPushButton(this);
    QHBoxLayout *buttonLayOut = new QHBoxLayout();

    startButton->setMaximumSize(shpFileButton->width(),shpFileButton->height());
    startButton->setText(tr("Start Detection"));
    QSpacerItem *buttonSpacer = new QSpacerItem(startButton->width()*3,startButton->height());
    buttonLayOut->addSpacerItem(buttonSpacer);
    buttonLayOut->addWidget(startButton);

    QHBoxLayout *the1stLayOut = new QHBoxLayout();
    QHBoxLayout *the2ndLayOut = new QHBoxLayout();
    QVBoxLayout *group1LayOut = new QVBoxLayout();

    the1stLayOut->addWidget(shpFileLabel,1);
    the1stLayOut->addWidget(shpFilePath,3);
    the1stLayOut->addWidget(shpFileButton,1);

//    the2ndLayOut->addWidget(imageFileLabel,1);
//    the2ndLayOut->addWidget(imageFilePath,3);
//    the2ndLayOut->addWidget(imageFileButton,1);
    the2ndLayOut->addWidget(newNameLabel,1);
    the2ndLayOut->addWidget(newNameEdit,4);
    the2ndLayOut->addStretch(2);

    downLayOut->setColumnStretch(0,10);
    downLayOut->setColumnStretch(1,30);
    downLayOut->setColumnStretch(2,10);

    downLayOut->addWidget(fieldLabel,0,0);
    downLayOut->addWidget(fieldComBox,0,1);
    downLayOut->addWidget(valueLabel,1,0);
    downLayOut->addWidget(valueComBox,1,1);
//    downLayOut->addWidget(startButton,0,2,2,1);

    group1LayOut->addLayout(the1stLayOut);
    group1LayOut->addLayout(the2ndLayOut);

    inputGroupBox->setTitle(tr("Input Files"));
    inputGroupBox->setLayout(group1LayOut);

    chooseGroupBox->setTitle(tr("Choose Items"));
    chooseGroupBox->setLayout(downLayOut);

    mainLayOut->addWidget(inputGroupBox);
    mainLayOut->addWidget(chooseGroupBox);
    mainLayOut->addLayout(buttonLayOut);

    setLayout(mainLayOut);

    connect(shpFileButton,SIGNAL(clicked()),this,SLOT(inputShpFilePath()));
//    connect(imageFileButton,SIGNAL(clicked()),this,SLOT(inputImageFilePath()));
    connect(startButton,SIGNAL(clicked()),this,SLOT(startDetect()));
}

DialogBrokenDetection::~DialogBrokenDetection()
{
    delete ui;
}

QString DialogBrokenDetection::name()
{
    return newNameEdit->text();
//    return labelShpfilePath;
}

QString DialogBrokenDetection::shpid()
{
    return shpID;
}

bool DialogBrokenDetection::polygonize(QString &rasterPath, QString &tagImagePath, QString &polyPath)
{
    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","YES");
    GDALDataset*    _poImageDS;
    _poImageDS = (GDALDataset*)GDALOpen(rasterPath.toUtf8().constData(), GA_Update);
    GDALDriver* poDriver = (GDALDriver*)GDALGetDriverByName("GTiff");
    GDALDataset* poDstDS = poDriver->Create(tagImagePath.toUtf8().constData(),
                                            _poImageDS->GetRasterXSize(),
                                            _poImageDS->GetRasterYSize(),
                                            1,GDT_Byte,NULL);

    OGRSpatialReference* pSpecialReference = new OGRSpatialReference(_poImageDS->GetProjectionRef());

    double adfGeoTransform[6];
    poDstDS->SetProjection(_poImageDS->GetProjectionRef());
    _poImageDS->GetGeoTransform(adfGeoTransform);
    poDstDS->SetGeoTransform(adfGeoTransform);

    GDALClose(poDstDS);

    labelit(tagImagePath,certainPoints);

    GDALDataset *poFlagDS = (GDALDataset *)GDALOpen(tagImagePath.toUtf8().constData(),GA_ReadOnly);
    if (poFlagDS == NULL)
    {
        return false;
    }

    OGRSFDriver* poOgrDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
    if (poOgrDriver == NULL)
    {
        GDALClose(poFlagDS);
        return false;
    }

    QFileInfo info(polyPath);
    if (info.exists())
        poOgrDriver->DeleteDataSource(polyPath.toUtf8().constData());

    OGRDataSource* poDstDataset = poOgrDriver->CreateDataSource(polyPath.toUtf8().constData(),0);
    if (poDstDataset == NULL)
    {
        GDALClose(poFlagDS);
        GDALClose(_poImageDS);
        return false;
    }
    const char* layerName = "polygon";
    OGRLayer* poLayer = poDstDataset->CreateLayer(layerName,pSpecialReference,wkbPolygon,0);
    if (poLayer == NULL)
    {
        GDALClose(poFlagDS);
        GDALClose(_poImageDS);
        OGRDataSource::DestroyDataSource( poDstDataset );
        return false;
    }

    OGRFieldDefn ofDef_DN( "Category", OFTInteger );
    if ( (poLayer->CreateField(&ofDef_DN) != OGRERR_NONE) )
    {
        GDALClose(poFlagDS);
        GDALClose(_poImageDS);
        OGRDataSource::DestroyDataSource( poDstDataset );
        return false;
    }


    char** papszOptions = NULL;
    papszOptions = CSLSetNameValue(papszOptions,"8CONNECTED","8");
    GDALRasterBand *poFlagBand = poFlagDS->GetRasterBand(1);
    GDALRasterBand *poMaskBand = poFlagBand->GetMaskBand();
    CPLErr err = GDALPolygonize((GDALRasterBandH)poFlagBand,(GDALRasterBandH)poMaskBand,(OGRLayerH)poLayer,0,papszOptions,0,0);
    if (err != CE_None)
    {
        GDALClose(poFlagDS);
        GDALClose(_poImageDS);
        OGRDataSource::DestroyDataSource( poDstDataset );
        if (pSpecialReference) delete pSpecialReference;
        return false;
    }

    if (pSpecialReference) delete pSpecialReference;
    GDALClose(poFlagDS);
    GDALClose(_poImageDS);
    OGRDataSource::DestroyDataSource( poDstDataset );

    QFile file;
    file.setFileName(tagImagePath);
    file.remove();
    file.close();
    return true;
}

void DialogBrokenDetection::rasterize()
{
    _fieldName = fieldComBox->currentText();
    _valueName = valueComBox->currentText();
    _tagImagePath = QDir::currentPath()+ "/" + QUuid::createUuid().toString().mid(2,7)+".tif";

    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","YES");

    OGRDataSource  *poDS;
    GDALDataset *poDS2;
    poDS= OGRSFDriverRegistrar::Open(_shpFilePath.toUtf8().constData(), FALSE );
    poDS2 =(GDALDataset*)(GDALOpen(_imageFilePath.toUtf8().constData(),GA_ReadOnly));

    OGRLayer* poLayer;
    poLayer = poDS->GetLayer(0);

    int m_nImageWidth = poDS2->GetRasterXSize();
    int m_nImageHeight= poDS2->GetRasterYSize();

    QString layName =QString::fromUtf8(poLayer->GetName());
    GDALClose(poDS2);
    OGR_DS_Destroy(poDS);

    QStringList params;
    params = QStringList()
            << "-a"<<_fieldName<<"-ts"
            <<QString::number(m_nImageWidth)<<QString::number(m_nImageHeight)
            <<"-l"<<layName<<_shpFilePath<<_tagImagePath;

    QProcess qgdal_rasterize;
    qgdal_rasterize.start("gdal_rasterize_chinese", params);
    qDebug()<<params;

    if(!qgdal_rasterize.waitForFinished(-1)){
        QMessageBox::critical(NULL,tr("Error"),
                              tr("Rasterize failed!"));
        return;
    }

}

bool DialogBrokenDetection::labelit(QString &fileName, QList<QPoint> &points)
{
    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","YES");
    GDALDataset*    _poImageDS;
    _poImageDS = (GDALDataset*)GDALOpen(fileName.toUtf8().constData(), GA_Update);
    if (_poImageDS == NULL)
    {
        qDebug()<<("Open Image ")+fileName+tr(" Failed!");
        return false;
    }

    int _bandCount= _poImageDS->GetRasterCount();
    GDALDataType    _dataType   = _poImageDS->GetRasterBand(1)->GetRasterDataType();
    int _dataSize   = GDALGetDataTypeSize(_dataType)/8;

    int index=0;

    foreach(QPoint aPoint,points){
        int  nXOff = aPoint.x();
        int  nYOff = aPoint.y();

        int xXOff = 5;
        int yYOff = 5;

        QVector<uchar*> buffer(_bandCount);
        for(int k =0;k<_bandCount;k++)
        {
            buffer[k] = new uchar[xXOff*yYOff*_dataSize];
        }

        for(int pix_i=0;pix_i<xXOff*yYOff;pix_i++)
        {
//                qDebug()<<"ok";
            for(int j =0;j<_bandCount;++j){
                if(j==0){
                    buffer[j][pix_i]=255;
                }else{
                    buffer[j][pix_i]=0;
                }

            }
//            buffer[0][pix_i]=255;
//            buffer[1][pix_i]=0;
//            buffer[2][pix_i]=0;
//            buffer[3][pix_i]=0;
        }

        for (int k=0;k<_bandCount;++k)
        {
            GDALRasterBand *poBand =  _poImageDS->GetRasterBand(k+1);
            poBand->RasterIO(GF_Write,nXOff,nYOff,xXOff,yYOff,buffer[k],xXOff,yYOff,_dataType,0,0);
        }
        for (int k=0;k<_bandCount;++k)
            delete [](buffer[k]);
        index++;


    }
    qDebug()<<index;
    GDALClose(_poImageDS);
    return true;
}

bool DialogBrokenDetection::edgeCanny(QString &fileName, cv::Mat &retMat, int &isColor)
{
    int kernel_size = 3;
    int lowThreshold=10;
    int ratio = 3;
    Mat src, src_gray;
    Mat dst, detected_edges;

    src = imread(fileName.toLocal8Bit().constData());

    if( !src.data )
      { qDebug()<<tr("Error read image");
        return false; }
    dst.create( src.size(), src.type() );
    cvtColor( src, src_gray, CV_BGR2GRAY );
    blur( src_gray, detected_edges, Size(3,3) );

    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

    dst = Scalar::all(0);

    src.copyTo( dst, detected_edges);
//    dst.copyTo(returnMat);

    if(isColor){

        dst.copyTo(retMat);
    }else{
        detected_edges.copyTo(retMat);

    }

//    QString edgeImageGray = "edgeMatGray.tif";
//    QString edgeImageColor = "edgeMatCol.tif";
//    cv::imwrite(edgeImageGray.toLocal8Bit().constData(),detected_edges);
//    cv::imwrite(edgeImageColor.toLocal8Bit().constData(),dst);
    return true;
}

bool DialogBrokenDetection::getOneFieldValues(const char *pszName, const char *oneField, QStringList &fieldLists)
{
    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","YES");
    OGRSFDriver* poOgrDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
    if (poOgrDriver == NULL)
    {
        qDebug()<<"Get ESRI SHAPEFILE Driver Failed";
        return false;
    }
    OGRDataSource* poDstDataset = poOgrDriver->Open(pszName,1);
    if (poDstDataset == NULL)
    {
        qDebug()<<"Open Shapefile "<<pszName<<" Failed!";
        return false;
    }
    OGRLayer *poLayer = poDstDataset->GetLayer(0);
    OGRFeature *poFeature;
    poLayer->ResetReading();

    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
        //OGRField pValue;
        fieldLists<<QString::number(poFeature->GetFieldAsInteger(oneField));
    }
    OGRDataSource::DestroyDataSource(poDstDataset);
    OGRCleanupAll();
    return true;
}

bool DialogBrokenDetection::getFields(const char *pszName, QStringList &fieldLists)
{
    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","YES");
    OGRSFDriver* poOgrDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
    if (poOgrDriver == NULL)
    {
        qDebug()<<"Get ESRI SHAPEFILE Driver Failed";
        return false;
    }
    OGRDataSource* poDstDataset = poOgrDriver->Open(pszName,1);
    if (poDstDataset == NULL)
    {
        qDebug()<<"Open Shapefile "<<pszName<<" Failed!";
        return false;
    }
    OGRLayer *poLayer = poDstDataset->GetLayer(0);
    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

    int fieldCount = poFDefn->GetFieldCount();

    for(int i =0;i<fieldCount;i++){
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
        fieldLists<<poFieldDefn->GetNameRef();
    }
    return true;
}

float DialogBrokenDetection::calcOrientationHist(const cv::Mat &img, QPoint pt, int radius, float *hist, int n, int isSmoothed, int isWeighted, float weighted_sigma)
{
    //radius  should be based on Pt half-centric square side length
        int i, j, k, len = (radius*2+1)*(radius*2+1);
        //Center-weighted using a Gaussian function
        float expf_scale = -1.f/(2.f * weighted_sigma * weighted_sigma);
        //Why add 4 it is to give temporary histogram open extra four storage locations,
        //Used to store temphist [-1], temphist [-2], temphist [n], temphist [n + 1] of
        //Why add n it, this n positions are reserved temphist [0 ... n-1] of
        //Why do len * 4, 4 len array location which is left to the
        //length of the X, Y, W and the direction of the Ori
        cv::AutoBuffer<float> buf(len*4 + n+4);
        //X is the transverse gradient, Y is a longitudinal gradient,
        //Mag is gradient magnitude = sqrt (X ^ 2 + Y ^ 2),
        //Ori is the gradient direction = arctan (Y / X)
        float *X = buf, *Y = X + len, *Mag = X, *Ori = Y + len, *W = Ori + len;
        float* temphist = W + len + 2;//Plus 2 is used to store temphist [-1], temphist [-2]

        //Temporary histogram cleared
        for( i = 0; i < n; i++ )
            temphist[i] = 0.f;

        //Down, left to right scan seek horizontal,
        //vertical gradient values and corresponding weights from the
        for( i = -radius, k = 0; i <= radius; i++ )
        {
            int y = pt.y() + i;//The first point of the original image img pt.y + i row
            if( y <= 0 || y >= img.rows - 1 )//Border checks
                continue;
            for( j = -radius; j <= radius; j++ )
            {
                int x = pt.x() + j;//The first point of the original image img pt.x + j column
                if( x <= 0 || x >= img.cols - 1 )//Border checks
                    continue;
                //Transverse gradient
                float dx = (float)(img.at<uchar>(y, x+1) - img.at<uchar>(y, x-1));
                //Longitudinal gradient
                float dy = (float)(img.at<uchar>(y-1, x) - img.at<uchar>(y+1, x));
                //Save longitudinal and transverse gradient gradientSave longitudinal and transverse gradient gradient
                X[k] = dx; Y[k] = dy;
                //Calculating a weighted array
                if(isWeighted)
                    W[k] = (i*i + j*j)*expf_scale;
                else
                    W[k] = 1.f; //If you do not weighted, the right point on the weight of each statistic is the same
                k++;
            }
        }
        //Copy the actual statistics point to len, since the rectangular local neighborhood may exceed the image boundary,
        len = k;//So the actual number of points is always less than or equal (radius * 2 + 1) * (radius * 2 + 1)

        //Calculated gradient magnitude at a specified pixel in the neighborhood, and the right to re-gradient direction
        exp(W, W, len); //Weights
        fastAtan2(Y, X, Ori, len, true);//Gradient direction
        magnitude(X, Y, Mag, len);//Gradient magnitude

        //Fill temporary histogram, the horizontal axis is the direction of the gradient angle [0,360), bin width of n / 360;
        //Right vertical axis is multiplied by the corresponding weight gradient magnitude
        for( k = 0; k < len; k++ )
        {
            int bin = cvRound((n/360.f)*Ori[k]);//K-th angle is obtained Ori [k] of bin index number
            if( bin >= n )
                bin -= n;
            if( bin < 0 )
                bin += n;
            temphist[bin] += W[k]*Mag[k];
        }

        if(isSmoothed)
        {
            // Histogram smoothing, smoothing into the output histogram array
            temphist[-1] = temphist[n-1];
            temphist[-2] = temphist[n-2];
            temphist[n] = temphist[0];
            temphist[n+1] = temphist[1];
            for( i = 0; i < n; i++ )
            {
                hist[i] = (temphist[i-2] + temphist[i+2])*(1.f/16.f) +
                    (temphist[i-1] + temphist[i+1])*(4.f/16.f) +
                    temphist[i]*(6.f/16.f);
            }
        }
        else  //Not smooth histogram
        {
            for( i = 0; i < n; i++ )
            {
                hist[i] = temphist[i];
            }
        }

        //Maximum gradient histogram
        float maxval = hist[0];
        for( i = 1; i < n; i++ )
            maxval = std::max(maxval, hist[i]);

        //cal direction uncertain
        float sum=0,certain;
        for(int i = 0;i<n;i++)
        {
            if(hist[i]==maxval)
            {

            }else{
                sum+=hist[i];
            }

        }
        certain = maxval-sum/(n-1);

        return certain;
}

void DialogBrokenDetection::inputShpFilePath()
{
    disconnect(fieldComBox,SIGNAL(currentIndexChanged(QString)));
    shpFilePath->removeItem(0);
    fieldComBox->removeItem(0);
    valueComBox->removeItem(0);
    shpFilePath->clear();
    fieldComBox->clear();
    valueComBox->clear();
    _shpFilePath = QFileDialog::getOpenFileName(this,
                                                tr("Open ShpFile"),".",
                                                tr("Image (*.shp)"));
    shpFilePath->insertItem(0,_shpFilePath);
    QStringList tempFieldsList,allFieldValues;
    QSet<QString> fieldValuesSet;
    QStringList fieldValuesList;
    getFields(_shpFilePath.toUtf8().constData(),tempFieldsList);
    fieldComBox->insertItems(0,tempFieldsList);

    getOneFieldValues(_shpFilePath.toUtf8().constData()
                                             ,fieldComBox->currentText().toUtf8().constData()
                                             ,allFieldValues);
    foreach(QString tempStr,allFieldValues){
        fieldValuesSet<<tempStr;
    }
    foreach(QString tempStr,fieldValuesSet){
        fieldValuesList<<tempStr;
    }
    valueComBox->insertItems(0,fieldValuesList);

    connect(fieldComBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(_changeValueComBox()));
}

//void DialogBrokenDetection::inputImageFilePath()
//{
//   _imageFilePath =QFileDialog::getOpenFileName(this,
//                                                tr("Open Images"),".",
//                                                tr("Images(*.png *.xpm *.jpg *.img *.tif)"));
//   imageFilePath->insertItem(0,_imageFilePath);

//}

void DialogBrokenDetection::_changeValueComBox()
{
    QStringList allFieldValues;
    QSet<QString> fieldValuesSet;
    QStringList fieldValuesList;
    valueComBox->removeItem(0);
    valueComBox->clear();
    getOneFieldValues(_shpFilePath.toUtf8().constData()
                                             ,fieldComBox->currentText().toUtf8().constData()
                                             ,allFieldValues);
    foreach(QString tempStr,allFieldValues){
        fieldValuesSet<<tempStr;
    }
    foreach(QString tempStr,fieldValuesSet){
        fieldValuesList<<tempStr;
    }
    valueComBox->insertItems(0,fieldValuesList);
}

void DialogBrokenDetection::startDetect()
{
    if(_shpFilePath.isEmpty())
    {
        QMessageBox::critical(this,tr("Error"),tr("Please Input a ShpFile!"));
        return;
    }
    if(_imageFilePath.isEmpty())
    {
        QMessageBox::critical(this,tr("Error"),tr("Please Input a Image!"));
        return;
    }
    if(newNameEdit->text().isEmpty())
    {
        QMessageBox::critical(this,tr("Error"),tr("Please Type a Name of Shpfile!"));
        return;
    }
    /************rasterize************/
    rasterize();
//    _fieldName = fieldComBox->currentText();
//    _valueName = valueComBox->currentText();
//    _tagImagePath = QDir::currentPath()+ "/" + "18f639d.tif";
    /************rasterize************/

    /************Canny************/
    cv::Mat src,edgeMat;
    int isCanny =0;
    if(isCanny){
        int isCol = 0;
        edgeCanny(_imageFilePath,edgeMat,isCol);
    }else{
        src=cv::imread(_imageFilePath.toLocal8Bit().constData());
        cv::GaussianBlur( src, src, cv::Size(3,3), 0, 0, 1);
        cvtColor( src, edgeMat, CV_BGR2GRAY );
    }

//    cv::imshow("w",edgeMat);
    /************Canny************/

    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","YES");

    _poTagImageDS = (GDALDataset*)GDALOpen(_tagImagePath.toUtf8().constData(),GA_ReadOnly);
    _poImageDS = (GDALDataset*)GDALOpen(_imageFilePath.toUtf8().constData(),GA_ReadOnly);

    _width      = _poImageDS->GetRasterXSize();
    _height     = _poImageDS->GetRasterYSize();

    GDALRasterBand* poFlagBand = _poTagImageDS->GetRasterBand(1);
    QList<QPoint> points;
    if(poFlagBand==NULL)
    {
        QMessageBox::critical(this,tr("Error"),"No Tag Image!");
        return;
    }
//    qDebug()<<_valueName;
    int* buffer_tag = new int[_width];

        for(int i=0;i<_height;++i)
        {
            poFlagBand->RasterIO(GF_Read,0,i,_width,1,buffer_tag,_width,1,GDT_Int32,0,0);
            for(int j=0;j<_width;++j)
               {
                    if(buffer_tag[j]==_valueName.toInt())
                    {
                        points.push_back(QPoint(j,i));
//                        qDebug()<<QString::number(j)+"     "+QString::number(i);
                    }else{
                        continue;
                    }
                }
        }
        GDALClose(_poImageDS);
        GDALClose(_poTagImageDS);
//        qDebug()<<QString::number(points.size());
//        foreach(QPoint tempPoint,points){
//            qDebug()<<QString::number(tempPoint.x())+"     "+QString::number(tempPoint.y());
//        }
        QList<int> bins;
        bins<<-180<<-140<<-100<<-60<<-20<<20<<60<<100<<140<<180;

        QList<float> certain;
        int templet_size = 25;
        int index = 0;
        foreach(QPoint tempPoint,points){
            int row = tempPoint.y()-templet_size/2;
            int col = tempPoint.x()-templet_size/2;
            if((row>0)&&(row<=edgeMat.rows)&&(col>0)&&col<edgeMat.cols)
            {
                cv::Mat tempMat;
                tempMat.create(cv::Size(templet_size,templet_size),/*edgeMat.type()*/CV_8U);
                for(int i=0;i<templet_size;i++)
                   {
                    uchar* pRow = edgeMat.ptr<uchar>(row+i);
                    uchar* tempRow = tempMat.ptr<uchar>(i);

                    for(int j=0;j<templet_size;j++)
                    {
                        tempRow[j]=pRow[col+j];
                    }

                }
                QPoint center(templet_size/2,templet_size/2);
                int radius = templet_size/2;//
                int  bins_count = 9;//bin
                float sigma = radius*0.5f;//Half of the standard weighting function, set radius Statistical Area
                bool isSmoothed = false; //
                bool isWeighted = false; //
                cv::Mat originHist = cv::Mat::zeros(bins_count,1,CV_32FC1);
                float*  oh = (float*)originHist.data;
                float tempCertain =
                calcOrientationHist(tempMat,center,
                                    radius,oh,bins_count,
                                    isSmoothed,isWeighted,sigma);

                if(tempCertain<800){
                    certain.append(tempCertain);
                    certainPoints.push_back(tempPoint);
                }

//                QString tempFileName;
//                tempFileName="img_"+QString::number(index)+".tif";
//                cv::imwrite(tempFileName.toStdString(),tempMat);

                index++;
            }else{}
        }
        qDebug()<<index;
        qDebug()<<certain.size();
        QString labelImagePath;
        labelImagePath = QDir::tempPath()+"/"+QUuid::createUuid().toString()+".tif";
        labelShpfilePath = QDir::tempPath()+"/"+QUuid::createUuid().toString()+".shp";

//        DialogLabelit *dialogLabel = new DialogLabelit(this);
//        if(dialogLabel->exec()==DialogLabelit::Accepted){

//            outPutPath =dialogLabel->name();
//        }
//        QString outPutPath;
//        outPutPath = QDir::currentPath()+ "/label.tif";
//        QFile file;
//        file.setFileName(labelImagePath);
//        file.remove();
//        file.close();
//        file.setFileName(_imageFilePath);
//        file.copy(labelImagePath);
//        file.close();
        polygonize(_imageFilePath,labelImagePath,labelShpfilePath);
        qDebug()<<"ok polygonize";
//        labelit(labelImagePath,certainPoints);
//        _newRasterPath =labelImagePath;

        QFile file;
        file.setFileName(_tagImagePath);
        file.remove();
        file.close();

        shpID = QUuid::createUuid().toString();

        fileSystem->registerFile(shpID,labelShpfilePath,QString(),QString(),
                                 CDTFileSystem::getShapefileAffaliated(labelShpfilePath));
        //qDebug()<<CDTFileSystem::getShapefileAffaliated(labelShpfilePath);

//        LeoAnalysisAttributes::imwriteXML(QString("out.xml"),certainPoints);
        this->accept();

}
