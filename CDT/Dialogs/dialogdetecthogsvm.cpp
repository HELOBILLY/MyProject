#include "dialogdetecthogsvm.h"
#include "ui_dialogdetecthogsvm.h"
#include "stable.h"
#include <qtcolorpicker.h>
#include <QHBoxLayout>
#include "cdtfilesystem.h"
#include "leohelper.h"
#include <gdal_alg.h>

DialogDetectHogSVM::DialogDetectHogSVM(
//        QUuid imageID,
        const QString &inputImage,
        CDTFileSystem* fileSys,
        QWidget *parent) :
    QDialog(parent),
    fileSystem(fileSys),
    inputImagePath(inputImage),
    ui(new Ui::DialogDetectHogSVM)
{
    ui->setupUi(this);
    setWindowTitle(tr("New Detection"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *the1stHLayout = new QHBoxLayout();
    QLabel *my1stLabel = new QLabel(this);
    my1stLineEdit = new QLineEdit(this);

    my1stLabel->setText(tr("Detection Name:"));
    the1stHLayout->addWidget(my1stLabel,1);
    the1stHLayout->addWidget(my1stLineEdit,4);
//    the1stHLayout->addStretch(2);

    QHBoxLayout *the2ndHLayout = new QHBoxLayout();
    QLabel *my2ndLabel = new QLabel(this);
    my2ndColorPicker = new QtColorPicker(this);
    my2ndColorPicker->setStandardColors();
    my2ndColorPicker->setCurrentColor(Qt::red);

    my2ndLabel->setText(tr("Border Color"));
    the2ndHLayout->addWidget(my2ndLabel,1);
    the2ndHLayout->addWidget(my2ndColorPicker,4);
//    the2ndHLayout->addStretch(2);

    QHBoxLayout *the3rdHLayout = new QHBoxLayout();
    QLabel *my3rdLabel = new QLabel(this);
    my3rd1stCheckBox = new QCheckBox(this);
    my3rd2ndCheckBox = new QCheckBox(this);
//    my3rd3rdCheckBox = new QCheckBox(this);
    my3rd4thCheckBox = new QCheckBox(this);
    my3rd5CheckBox = new QCheckBox(this);
    my3rd6CheckBox = new QCheckBox(this);

    my3rdLabel->setText(tr("Detect Category:"));
    my3rd1stCheckBox->setText(tr("fish rafts"));
    my3rd2ndCheckBox->setText(tr("hanging nets"));
//    my3rd3rdCheckBox->setText(tr("fish boats"));
    my3rd4thCheckBox->setText(tr("Wharf"));
    my3rd5CheckBox->setText(tr("Embankment"));
    my3rd6CheckBox->setText(tr("Pond"));

    the3rdHLayout->addWidget(my3rdLabel);
    the3rdHLayout->addWidget(my3rd1stCheckBox);
    the3rdHLayout->addWidget(my3rd2ndCheckBox);
//    the3rdHLayout->addWidget(my3rd3rdCheckBox);
    the3rdHLayout->addWidget(my3rd4thCheckBox);
    the3rdHLayout->addWidget(my3rd5CheckBox);
    the3rdHLayout->addWidget(my3rd6CheckBox);

    QHBoxLayout *the4thLayout = new QHBoxLayout();
    startButton = new QPushButton(this);
    okButton = new QPushButton(this);
    cancelButton = new QPushButton(this);

    startButton->setText(tr("Start Detection"));
    okButton->setText(tr("OK"));
    cancelButton->setText(tr("Cancel"));

    the4thLayout->addWidget(startButton,2);
    the4thLayout->addStretch(3);
    the4thLayout->addWidget(okButton,2);
    the4thLayout->addStretch(3);
    the4thLayout->addWidget(cancelButton,2);

    QHBoxLayout *the5thHLayout = new QHBoxLayout();
    QLabel *my5thLabel = new QLabel(this);
    my5thCombox = new QComboBox(this);
    QPushButton *my5thPushButton = new QPushButton(this);
    my5thLabel->setText(tr("Import Model:"));
    my5thPushButton->setText(tr("Import"));
    the5thHLayout->addWidget(my5thLabel,1);
    the5thHLayout->addWidget(my5thCombox,3);
    the5thHLayout->addWidget(my5thPushButton,1);

    QHBoxLayout *the6thHLayout = new QHBoxLayout();
    QLabel *modelSizeLabel = new QLabel(this);
    QSpacerItem *modelSizeSpacerItem = new QSpacerItem(my5thPushButton->width(),my5thPushButton->height());
    modelSizeLineEdit = new QLineEdit(this);
    modelSizeLabel->setText(tr("Model Size:"));
    modelSizeLineEdit->setEnabled(false);
    the6thHLayout->addWidget(modelSizeLabel,1);
    the6thHLayout->addWidget(modelSizeLineEdit,3);
    the6thHLayout->addSpacerItem(modelSizeSpacerItem);

    QHBoxLayout *the7thHLayout = new QHBoxLayout();
    QLabel *detectingLabel = new QLabel(this);
    detectingLabel->setText(tr("Detecting:"));
    progressBar = new QProgressBar(this);
    //progressBar->setMinimum(0);
    //progressBar->setMaximum(100);
    progressBar->setRange(0,6);
    progressBar->setValue(0);
    the7thHLayout->addWidget(detectingLabel,1);
    the7thHLayout->addWidget(progressBar,3);

    QGroupBox *the1stGroupBox = new QGroupBox(this);
    QVBoxLayout *my1stGroupLayout = new QVBoxLayout();
    my1stGroupLayout->addLayout(the5thHLayout);
    my1stGroupLayout->addLayout(the6thHLayout);
    my1stGroupLayout->addLayout(the2ndHLayout);
    my1stGroupLayout->addLayout(the3rdHLayout);
    my1stGroupLayout->addLayout(the1stHLayout);
    my1stGroupLayout->addLayout(the4thLayout);
    my1stGroupLayout->addLayout(the7thHLayout);

    the1stGroupBox->setLayout(my1stGroupLayout);

    mainLayout->addWidget(the1stGroupBox);
//    setLayout(mainLayout);
    okButton->setEnabled(false);
    //time = new QTime;
    //progressTimer = new QTimer();
    //progressTimer->setInterval(1000);

    //qDebug()<<inputImagePath;
    connect(my5thPushButton,SIGNAL(clicked()),this,SLOT(importModel()));
    connect(startButton,SIGNAL(clicked()),this,SLOT(startPredictLabel()));
    //connect(progressTimer,SIGNAL(timeout()),this,SLOT(progressShow()));
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(close()));
    connect(okButton,SIGNAL(clicked()),this,SLOT(accept()));
}

DialogDetectHogSVM::~DialogDetectHogSVM()
{
    delete ui;
}

QColor DialogDetectHogSVM::borderColor() const
{
    return my2ndColorPicker->currentColor();
}

QStringList DialogDetectHogSVM::returnshpIDList()
{
    return shpIDList;
}

QStringList DialogDetectHogSVM::returnshpPathList()
{
    return tagShpfilePath;
}

QStringList DialogDetectHogSVM::returnshpNameList()
{
    return tagShpfileName;
}

bool DialogDetectHogSVM::polygonize(QString &rasterPath, QString &tagImagePath, QString &polyPath,int isDrawing)
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
    char *pszSrcWKT = NULL;
    pszSrcWKT = const_cast<char *>(_poImageDS->GetProjectionRef());

    OGRSpatialReference* pSpecialReference = new OGRSpatialReference(_poImageDS->GetProjectionRef());
    if(strlen(pszSrcWKT)<=0)
    {
        OGRSpatialReference oSRS;
        oSRS.SetUTM(50,true);   //120
        oSRS.SetWellKnownGeogCS("WGS84");
        oSRS.exportToWkt(&pszSrcWKT);
        qDebug()<<"no projection";
        /*void *hTransformArg;
        hTransformArg = GDALCreateGenImgProjTransformer((GDALDatasetH)_poImageDS,pszSrcWKT,NULL,pszSrcWKT,FALSE,0.0,1);*/

        _poImageDS->SetProjection(pszSrcWKT);
//      if (hTransformArg == NULL)
//      {
//          GDALClose((GDALDatasetH) _poImageDS);
//          return -3;
//      }
    }
    //OGRSpatialReference* pSpecialReference = new OGRSpatialReference(_poImageDS->GetProjectionRef());

    double adfGeoTransform[6]/*= {0,1,0,0,0,1}*/;
    _poImageDS->GetGeoTransform(adfGeoTransform);
    poDstDS->SetGeoTransform(adfGeoTransform);

    GDALClose(poDstDS);

    labelit(tagImagePath,isDrawing);

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

//    qDebug()<<"The counts of features is" <<poLayer->GetFeatureCount()<<endl;
//    OGRFeatureDefn *pFeatureDefn = NULL;
//    pFeatureDefn = poLayer->GetLayerDefn();
//    std::string strLayerName = pFeatureDefn->GetName();
//    poLayer->DeleteFeature(0);
//    poLayer->DeleteFeature(1);
//    poLayer->DeleteFeature(2);
//    poLayer->DeleteFeature(3);
//    std::string strSQL = "REPACK " + strLayerName;
//    poDstDataset->ExecuteSQL(strSQL.c_str(),NULL," ");
//    qDebug()<<"The counts of features is" <<poLayer->GetFeatureCount()<<endl;


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

void DialogDetectHogSVM::labelit(QString &atestRasterPath,int isDrawing)
{
    QString svm_resultFileName = "hog_result.txt";
    QStringList svm_result,image_coordinate;
    QFile file(svm_resultFileName);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        svm_result<< line;
    }
    file.close();

    QFile file_1("imageCoordinate.txt");
    file_1.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream in_1(&file_1);
    while(!in_1.atEnd())
    {
        QString line = in_1.readLine();
        image_coordinate<< line;
    }
    file_1.close();

    GDALAllRegister();
    OGRRegisterAll();
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","YES");
    GDALDataset*    _poImageDS;
    _poImageDS = (GDALDataset*)GDALOpen(atestRasterPath.toUtf8().constData(), GA_Update);
    if (_poImageDS == NULL)
    {
        qDebug()<<("Open Image ")+atestRasterPath+tr(" Failed!");
        return;
    }

    int _bandCount= _poImageDS->GetRasterCount();
    int rowsStep,colsStep;

    rowsStep =  _sampleHeight;
    colsStep = _sampleWidth;

    qDebug()<<rowsStep;
    qDebug()<<colsStep;
    int overLap = _sampleHeight/6+1;
    GDALDataType    _dataType   = _poImageDS->GetRasterBand(1)->GetRasterDataType();
    int _dataSize   = GDALGetDataTypeSize(_dataType)/8;

    for(int i=0;i<image_coordinate.size();i++)
    {
        QStringList tempList = image_coordinate.at(i).split("_");
        int x = tempList.at(0).toInt();
        int y = tempList.at(1).toInt();

        int  nYOff = x*rowsStep-x*overLap/*+rowsStep/2*/;
        int  nXOff = y*colsStep-y*overLap/*+colsStep/2*/;

        int xXOff = /*5;*/_sampleHeight;
        int yYOff = /*5;*/_sampleWidth;

        QVector<uchar*> buffer(_bandCount);
        for(int k =0;k<_bandCount;k++)
        {
            buffer[k] = new uchar[xXOff*yYOff*_dataSize];
        }
        if(svm_result.at(i).toInt()==isDrawing)
        {
            for(int pix_i=0;pix_i<xXOff*yYOff;pix_i++)
            {
                for(int bandi = 0;bandi<_bandCount;bandi++)
                {
                    if(bandi==0){
                        buffer[bandi][pix_i]=255;
                    }else if(bandi==1){
                        buffer[bandi][pix_i]=0;
                    }else if(bandi ==2){
                        buffer[bandi][pix_i]=0;
                    }else{
                        buffer[bandi][pix_i]=0;
                    }
                }
            }

            for (int k=0;k<_bandCount;++k)
            {
                GDALRasterBand *poBand =  _poImageDS->GetRasterBand(k+1);
                poBand->RasterIO(GF_Write,nXOff,nYOff,xXOff,yYOff,buffer[k],xXOff,yYOff,_dataType,0,0);
            }
            for (int k=0;k<_bandCount;++k)
                delete [](buffer[k]);
            }
    }
    GDALClose(_poImageDS);
}

void DialogDetectHogSVM::skipUnknownElement()
{
    reader.readNext();
        while (!reader.atEnd()) {
            if (reader.isEndElement()) {
                reader.readNext();
                break;
            }

            if (reader.isStartElement()) {
                skipUnknownElement();
            } else {
                reader.readNext();
            }
        }
}

void DialogDetectHogSVM::readXmlElement()
{
    Q_ASSERT(reader.isStartElement() && reader.name() == "opencv_storage");
    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "sampleHeight") {
                _sampleHeight = reader.readElementText().toInt();
                if (reader.isEndElement()) {
                    reader.readNext();
                }
//                 qDebug()<<_sampleHeight;

            }else if(reader.name()=="sampleWidth"){
                _sampleWidth= reader.readElementText().toInt();
                if (reader.isEndElement()) {
                    reader.readNext();
                }
//                 qDebug()<<_sampleWidth;
            }else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }
}

bool DialogDetectHogSVM::readFile(QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QString message = QString("Cannot read file %1").arg(fileName);
        qDebug()<<message;
        return false;
    }
    reader.setDevice(&file);
    while (!reader.atEnd()) {
        if (reader.isStartElement()) {
            if (reader.name() == "opencv_storage") {
                readXmlElement();
            } else {
                reader.raiseError(tr("Not a valid xml file"));
            }
        } else {
            reader.readNext();
        }
    }
    file.close();
    if (reader.hasError()) {
        QString message = QString("Failed to parse file %1").arg(fileName);
        qDebug()<<message;
        return false;
    } else if (file.error() != QFile::NoError) {
        QString message = QString("Cannot read file %1").arg(fileName);
        qDebug()<<message;
        return false;
    }
    return true;
}

void DialogDetectHogSVM::importModel()
{
    _importModelName = QFileDialog::getOpenFileName(this,
                                                    "Open SVM Model",".",
                                                    "Model (*.xml )");
    qDebug()<<_importModelName;
    my5thCombox->insertItem(0,_importModelName);
    my5thCombox->insertItem(0,_importModelName);
    readFile(_importModelName);
    QString modelSize = QString::number(_sampleHeight)+" * "+QString::number(_sampleWidth);
    modelSizeLineEdit->setText(modelSize);
}

void DialogDetectHogSVM::startPredictLabel()
{
    //time->start();
    //progressTimer->start(0);
    progressBar->setValue(0);
    if(_importModelName.isEmpty()){
        QMessageBox::critical(this,tr("Warning"),tr("Please import the modle of SVM!"));
        return;
    }
    if(my1stLineEdit->text().isEmpty()){
        QMessageBox::critical(this,tr("Warning"),tr("Please set the output name of shpfiles!"));
        return;
    }
    if(!my3rd1stCheckBox->isChecked()&&!my3rd2ndCheckBox->isChecked()
            /*&&!my3rd3rdCheckBox->isChecked()*/&&!my3rd4thCheckBox->isChecked()
            &&!my3rd5CheckBox->isChecked()&&!my3rd6CheckBox->isChecked()){
        QMessageBox::critical(this,tr("Warning"),tr("Please set the output catrgory!"));
        return;
    }
    readFile(_importModelName);
    progressBar->setValue(1);
    //***********************manage test image*******************************//
    if(my3rd1stCheckBox->isChecked()){
        categoryIsChosen<<1;
    }else{
        categoryIsChosen<<0;
    }
    if(my3rd2ndCheckBox->isChecked()){
        categoryIsChosen<<1;
    }else{
        categoryIsChosen<<0;
    }
//    if(my3rd3rdCheckBox->isChecked()){
//        categoryIsChosen<<1;
//    }else{
//        categoryIsChosen<<0;
//    }
    if(my3rd4thCheckBox->isChecked()){
        categoryIsChosen<<1;
    }else{
        categoryIsChosen<<0;
    }
    if(my3rd5CheckBox->isChecked()){
        categoryIsChosen<<1;
    }else{
        categoryIsChosen<<0;
    }
    if(my3rd6CheckBox->isChecked()){
        categoryIsChosen<<1;
    }else{
        categoryIsChosen<<0;
    }
    progressBar->setValue(2);

    int rowsStep,colsStep;
    rowsStep = _sampleHeight;
    colsStep = _sampleWidth;

    int overLap = _sampleHeight/6+1;
    cv::Mat testSrc_1 = cv::imread(inputImagePath.toLocal8Bit().constData(), 1);
    cv::Mat testSrc;
    cvtColor( testSrc_1, testSrc, CV_BGR2GRAY );
    int height = (testSrc.rows-overLap)/(rowsStep-overLap);
    int width = (testSrc.cols-overLap)/(colsStep-overLap);
    //   qDebug()<< testSrc.channels();
    cv::Mat data_test;
    int n=0;
    QStringList imageCoodinate;
    for(int i=0;i<height;i++)
     {
         for(int j=0;j<width;j++)
            {
                cv::Mat tempMat = cv::Mat::zeros(rowsStep,colsStep,CV_8U);
                for(int rowi = 0;rowi<rowsStep;rowi++)
                    {
                      uchar* pRow = testSrc.ptr<uchar>(i*rowsStep-i*overLap+rowi);
                      uchar* tempRow = tempMat.ptr<uchar>(rowi);

                      for(int coli = 0;coli<colsStep;coli++)
                           {
                             tempRow[coli]=pRow[j*colsStep-j*overLap+coli];
                            }
                        }

//                     QString tempFileName;
//                     tempFileName="img_"+QString::number(i)+"_"+QString::number(j)+".tif";
//                     cv::imwrite(tempFileName.toStdString(),tempMat);
                     imageCoodinate<<QString::number(i)+"_"+QString::number(j);
                     cv::HOGDescriptor *hog = new cv::HOGDescriptor(
                                /*win Size 64*64*/    cvSize(rowsStep,colsStep)
                                /*block Size*/       ,cvSize(rowsStep/2,colsStep/2)
                                /*block stride Size*/,cvSize(rowsStep/4,colsStep/4)
                                /*cell Size*/        ,cvSize(rowsStep/4,colsStep/4)
                                /*bin Size*/         , 9);
                      QVector<float>Qdescriptors;
                      std::vector<float>descriptors;
                      hog->compute(tempMat, descriptors,cv::Size(1,1), cv::Size(0,0));
                      Qdescriptors = QVector<float>::fromStdVector(descriptors);
                      if (n == 0)
                      {
                          data_test = cv::Mat::zeros(height*width,Qdescriptors.size(),CV_32FC1);
//                              qDebug()<<tempMat.channels();
//                              qDebug()<<Qdescriptors.size();
                       }
                       for(int k =0;k<Qdescriptors.size();k++)
                       {
                           data_test.at<float>(n,k)=Qdescriptors.at(k);
                        }
//                             qDebug()<<n;
                          n++;
                    }
     }

  LeoHelper::outmat("hog_test.txt",data_test);
  progressBar->setValue(3);

  QFile file("imageCoordinate.txt");
  file.open(QIODevice::WriteOnly|QIODevice::Text);
  QTextStream out(&file);
  for(int i =0;i<imageCoodinate.size();i++)
  {
  out<<imageCoodinate.at(i);
  out<<endl;
  }
  file.close();
  progressBar->setValue(4);

  cv::Mat result(data_test.rows,1,CV_32FC1);
//  cv::FileStorage modelFile(_importModelName.toLocal8Bit().constData(),cv::FileStorage::READ);
  cv::SVM classifier;
  classifier.load(_importModelName.toLocal8Bit().constData());
  classifier.predict(data_test,result);

  QString svm_resultFileName = "hog_result.txt";
  LeoHelper::outmat(svm_resultFileName,result);
  qDebug()<<"SVM ok";
  progressBar->setValue(5);
  //qDebug()<<"size= "<<categoryIsChosen.size();

  for(int i =0;i<categoryIsChosen.size();i++){
      //progressBar->setValue(i);
      if(categoryIsChosen.at(i)){
          int isDrawing = i+1;
          QString shpfilePath;
          QString tagImagePath = QDir::tempPath()+"/"+QUuid::createUuid().toString()+".tif";
          if(isDrawing == 1){
              shpfilePath = QString(tr("fish rafts"))+".shp";
          }else if(isDrawing ==2){
              shpfilePath = QString(tr("hanging nets"))+".shp";
          }else if(isDrawing ==3){
              shpfilePath = QString(tr("Wharf"))+".shp";
          }else if(isDrawing == 4){
              shpfilePath = QString(tr("Embankment"))+".shp";
          }else if(isDrawing ==5){
              shpfilePath = QString(tr("Pond"))+".shp";
          }
          //QString shpfilePath = QDir::tempPath()+"/"+QUuid::createUuid().toString()+".shp";
          QFile file;
          file.setFileName(inputImagePath);
          file.copy(tagImagePath);
          file.close();
//          labelit(tagImagePath,isDrawing);
          polygonize(inputImagePath,tagImagePath,shpfilePath,isDrawing);

          QString name;
          if(isDrawing == 1){
              name = QString(tr("fish rafts"));//_importModelName.mid(_importModelName.lastIndexOf("/")+1,_importModelName.lastIndexOf("."))+"_"+my1stLineEdit->text()+"_"+tr("fish rafts");
          }else if(isDrawing ==2){
              name = QString(tr("hanging nets"));//_importModelName.mid(_importModelName.lastIndexOf("/")+1,_importModelName.lastIndexOf("."))+"_"+my1stLineEdit->text()+"_"+tr("hanging nets");
          }/*else if(isDrawing ==3){
              name = _importModelName.mid(_importModelName.lastIndexOf("/")+1,_importModelName.lastIndexOf("."))+"_"+my1stLineEdit->text()+"_"+tr("fish boats");
          }*/else if(isDrawing ==3){
              name = QString(tr("Wharf"));//_importModelName.mid(_importModelName.lastIndexOf("/")+1,_importModelName.lastIndexOf("."))+"_"+my1stLineEdit->text()+"_"+tr("Wharf");
          }else if(isDrawing == 4){
              name = QString(tr("Embankment"));//_importModelName.mid(_importModelName.lastIndexOf("/")+1,_importModelName.lastIndexOf("."))+"_"+my1stLineEdit->text()+"_"+tr("Embankment");
          }else if(isDrawing ==5){
              name = QString(tr("Pond"));//_importModelName.mid(_importModelName.lastIndexOf("/")+1,_importModelName.lastIndexOf("."))+"_"+my1stLineEdit->text()+"_"+tr("Pond");
          }
          tagShpfileName<<name;
          name.clear();
          QString shpID = QUuid::createUuid().toString();
          shpIDList<<shpID;
          tagShpfilePath<<shpfilePath;

          fileSystem->registerFile(shpID,shpfilePath,QString(),QString(),
                                   CDTFileSystem::getShapefileAffaliated(shpfilePath));
          qDebug()<<CDTFileSystem::getShapefileAffaliated(shpfilePath);
      }

  }

  progressBar->setValue(6);
  okButton->setEnabled(true);
  QMessageBox::information(this,tr("Information"),tr("Predict Label Complete!"));
}

