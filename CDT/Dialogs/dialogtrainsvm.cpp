#include "dialogtrainsvm.h"
#include "ui_dialogtrainsvm.h"
#include "stable.h"
#include "leohelper.h"
#include "dialoglabelit.h"

DialogTrainSVM::DialogTrainSVM(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTrainSVM)
{
    ui->setupUi(this);
    setWindowTitle(tr("Train SVM Model"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    QMenuBar *hogMenuBar = new QMenuBar(this);
    hogMenuBar->setGeometry(QRect(0, 0, this->width(), 24));

    QMenu *fileMenu = hogMenuBar->addMenu(QIcon(":/Icons/file_1.ico"),tr("&File"));
    actionInputTrainFolder = new QAction( QIcon(":/Icons/multiple_input.ico"),
                                          tr("&Import Train Folder"),this);
    fileMenu->addAction(actionInputTrainFolder);

    QMenu *modelMenu = hogMenuBar->addMenu(QIcon(":/Icons/model.ico"),tr("&SVM Models"));
    actionTrainSVM = new QAction(QIcon(":/Icons/train.ico"),tr("&Train SVM Model"),this);
    actionSaveModel = new QAction(QIcon(":/Icons/save.ico"),tr("&Save Model"),this);
    modelMenu->addAction(actionTrainSVM);
    modelMenu->addSeparator();
    modelMenu->addAction(actionSaveModel);

    layout->setMenuBar(hogMenuBar);
    connect(actionTrainSVM,SIGNAL(triggered()),this,SLOT(trainSVMmodel()));
    connect(actionInputTrainFolder,SIGNAL(triggered()),this,SLOT(inputTainFolders()));
    connect(ui->trainFolderComboBox,SIGNAL(currentIndexChanged(QString)),
            this,SLOT(changeCurrentClass()));
    connect(actionSaveModel,SIGNAL(triggered()),this,SLOT(saveModel()));

    classNameTransform.insert(tr("yuPai"),("1"));
    classNameTransform.insert(tr("diaoWang"),("2"));
    classNameTransform.insert(tr("chuanZhi"),("3"));
    classNameTransform.insert(tr("maTou"),("4"));
    classNameTransform.insert(tr("diAn"),("5"));
    classNameTransform.insert(tr("weiTang"),("6"));
    classNameStringList<<tr("yuPai")<<tr("diaoWang")<<tr("chuanZhi")<<tr("maTou")<<tr("diAn")<<tr("weiTang");
}

DialogTrainSVM::~DialogTrainSVM()
{
    delete ui;
}

void DialogTrainSVM::changeSaveXML(QString &xmlName)
{
    QFile file(xmlName);
    file.open(QFile::ReadOnly);
    QDomDocument m_doc;
    m_doc.setContent(&file);
    file.close();

    QDomElement root = m_doc.documentElement();
    if(root.tagName()!= "opencv_storage")
    {
        qDebug()<<"not opencv_storage";
        return ;
    }

    QDomElement element_sampleHeight = m_doc.createElement(tr("sampleHeight"));
    QDomElement element_sampleWidth = m_doc.createElement(tr("sampleWidth"));
    QDomText text;
    text = m_doc.createTextNode(QString::number(_sampleHeight));
    element_sampleHeight.appendChild(text);
    text.clear();
    text = m_doc.createTextNode(QString::number(_sampleWidth));
    element_sampleWidth.appendChild(text);

    root.appendChild(element_sampleHeight);
    root.appendChild(element_sampleWidth);

    m_doc.normalize();

    QFile filexml(xmlName);
    if( !filexml.open( QFile::WriteOnly | QFile::Truncate) ){
         return;}
    QTextStream ts(&filexml);
    ts.reset();
    ts.setCodec("UTF-8");
    m_doc.save(ts, 4,QDomNode::EncodingFromTextStream);
    filexml.close();
    return;
}

bool DialogTrainSVM::deleteSomethingInXML(QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite|QIODevice::Text)){
       qDebug()<<"Open XML Failed";
        return false;
    }
    QTextStream in(&file);
    QStringList allLine;
    while(!in.atEnd()){
        QString line = in.readLine();
        if(line.contains("&#xd;")){
            int j = line.indexOf("&#xd;");
            line.replace(j,5," ");
        }
        allLine<<line;
    }
    file.close();
    file.setFileName(fileName);
    file.open(QIODevice::ReadWrite|QIODevice::Truncate);
    QTextStream out(&file);
    for(int i =0;i<allLine.size();i++)
    {
        out<<allLine.at(i);
        out<<endl;
    }
    file.close();
    return true;
}

void DialogTrainSVM::inputTainFolders()
{    ui->trainFolderComboBox->clear();
     _trainSamplesNum=0;
     QString path = QFileDialog::getExistingDirectory(NULL, tr("Choose the folder"),"",QFileDialog::ShowDirsOnly);
     if(path.isEmpty()){
         return;
     }

     QDir *dir=new QDir(path);
     QStringList filter;
     QFileInfoList fileList = dir->entryInfoList();
     QStringList classFileName;
     foreach(QFileInfo fileInfo,fileList){
         if(fileInfo.isDir()&&(!fileInfo.fileName().contains("."))){
             classFileName<<fileInfo.fileName();
         }
     }
     ui->trainFolderComboBox->insertItems(0,classNameStringList);
 ///All records are saved in classNameFileInfoList
     foreach(QString tempClass , classFileName){
         QString tempPath = path+"\\"+tempClass;
         qDebug()<<"The path is "<<tempPath;
         QDir *dir=new QDir(tempPath);
         filter<<"*.png"<<"*.xpm"<<"*.jpg"<<"*.img"<<"*.tif"<<"*.bmp";
         dir->setNameFilters(filter);
         QFileInfoList fileList = dir->entryInfoList();
         _trainSamplesNum+=fileList.size();
         classNameFileInfoList.insert(tempClass,fileList);
     }
     ///obtain the size of a train sample
     QString aFilePath;
     aFilePath=classNameFileInfoList.value(classFileName.at(0)).at(0).absoluteFilePath();
     cv::Mat src = cv::imread(aFilePath.toLocal8Bit().constData(), 1);
      _sampleHeight = src.rows;
      _sampleWidth = src.cols;
//     _isInitiazationTrainSamples++;

     QStandardItemModel *treeModelObject = new QStandardItemModel(this);
     QList<QStandardItem *> *treeObject = new QList<QStandardItem *>;

     QString currentClass = classFileName.at(0);
     foreach(QFileInfo fileInfo,classNameFileInfoList.value(currentClass)){
         QStandardItem *tempObject = new QStandardItem(fileInfo.fileName());
         treeObject->append(tempObject);
     }
     foreach(QStandardItem *tempObject,*treeObject){
             treeModelObject->appendRow(tempObject);
     }
     treeModelObject->setHorizontalHeaderLabels(QStringList()<<
                                                (QString::number(classNameFileInfoList.value(currentClass).size()))+tr(" Images"));
 //    ui->trainImageTreeView->header();
     ui->trainImageTreeView->setModel(treeModelObject);

}

void DialogTrainSVM::changeCurrentClass()
{
    QStandardItemModel *treeModelObject = new QStandardItemModel(this);
    QList<QStandardItem *> *treeObject = new QList<QStandardItem *>;

    QString currentClass;
    currentClass = classNameTransform.value(ui->trainFolderComboBox->currentText());
    foreach(QFileInfo fileInfo,classNameFileInfoList.value(currentClass)){
        QStandardItem *tempObject = new QStandardItem(fileInfo.fileName());

        treeObject->append(tempObject);
    }
    foreach(QStandardItem *tempObject,*treeObject){
            treeModelObject->appendRow(tempObject);
    }
    treeModelObject->setHorizontalHeaderLabels(QStringList()<<
                                               (QString::number(classNameFileInfoList.value(currentClass).size()))+tr(" Images"));
//    ui->trainImageTreeView->header();
    ui->trainImageTreeView->setModel(treeModelObject);
}

void DialogTrainSVM::trainSVMmodel()
{
    if(classNameFileInfoList.isEmpty()){
         QMessageBox::critical(this,tr("Error"),tr("Please Set the Train Samples!"));
         return;
    }
    int index = 0;
    cv::Mat data, responses;
    responses = cv::Mat::zeros( _trainSamplesNum, 1, CV_32FC1 );
    QMap<QString,QFileInfoList>::const_iterator i = classNameFileInfoList.constBegin();
    while (i != classNameFileInfoList.constEnd()) {
       foreach(QFileInfo tempFileInfo,i.value()){
//           QStringList tempFilePath01 = tempFileInfo.absoluteFilePath().split("/");
//           QString tempFilePath = tempFilePath01.join("\\\\");
//           qDebug()<<tempFilePath;
           cv::Mat src_1 = cv::imread(tempFileInfo.absoluteFilePath().toLocal8Bit().constData()
                                    , 1);
           cv::Mat src;
           cvtColor( src_1, src, CV_BGR2GRAY );
           if(src.empty()){
               break;
               QMessageBox::critical(this,"Error","Occur errors when reading"+tempFileInfo.absoluteFilePath());
           }
//           qDebug()<<src.rows;
//           qDebug()<<src.cols;
           cv::HOGDescriptor *hog = new cv::HOGDescriptor(
                       /*win Size 64*64*/    cvSize(src.rows,src.cols)
                       /*block Size*/       ,cvSize(src.rows/2,src.cols/2)
                       /*block stride Size*/,cvSize(src.rows/4,src.rows/4)
                       /*cell Size*/        ,cvSize(src.rows/4,src.rows/4)
                       /*bin Size*/         , 9);
           QVector<float>Qdescriptors;
           std::vector<float>descriptors;
           hog->compute(src, descriptors,cv::Size(1,1), cv::Size(0,0));
           Qdescriptors = QVector<float>::fromStdVector(descriptors);

           if (index==0)
           {
               qDebug()<<_trainSamplesNum;
               _sampleHeight = src.rows;
               _sampleWidth  = src.cols;
               data = cv::Mat::zeros(_trainSamplesNum,Qdescriptors.size(),CV_32FC1);
           }
           for(int j =0;j<Qdescriptors.size();j++)
           {
               data.at<float>(index,j)=Qdescriptors.at(j);
           }
//           qDebug()<<Qdescriptors.size();
           responses.at<float>(index,0)=i.key().toInt();
           index++;
       }
        ++i;
    }
    LeoHelper::outmat("hog_train.txt",data);
//    LeoAnalysisAttributes::outmat("hog_train.txt",data);
//    LeoAnalysisAttributes::outmat("response.txt",responses);
        qDebug()<<index;
     _trainModelName =
              QDir::currentPath()+ "/" + QUuid::createUuid().toString().mid(2,7)+".xml";
     cv::SVM classifier;
     classifier.train_auto(data,responses,cv::Mat(),cv::Mat(),cv::SVMParams());
     classifier.save(_trainModelName.toLocal8Bit().constData());

     changeSaveXML(_trainModelName);
     deleteSomethingInXML(_trainModelName);
//     QFile file(_trainModelName);
//     file.open(QFile::WriteOnly | QIODevice::Text | QIODevice::Append);
//     QXmlStreamWriter xmlWriter(&file);
//     xmlWriter.setAutoFormatting(true);
//     xmlWriter.writeStartElement("opencv_storage");
//     xmlWriter.writeTextElement("sampleHeight",QString::number(_sampleHeight));
//     xmlWriter.writeTextElement("sampleWidth",QString::number(_sampleWidth));
//     xmlWriter.writeEndElement();
//     file.close();
     qDebug()<<"Model OK";
     QMessageBox::information(this,tr("Information"),tr("Train Model Complete!"));
}

void DialogTrainSVM::saveModel()
{
    if(_trainModelName.isEmpty()){
        QMessageBox::critical(this,tr("Error"),tr("Please Train a SVM Model!"));
        return;
    }
    DialogLabelit *dialogSaveModel = new DialogLabelit(this);
    QString postFix = " *.xml ";
    dialogSaveModel->setPostFix(postFix);
    QString _trainModelSaveName;
    if(dialogSaveModel->exec()==DialogLabelit::Accepted)
    {
         _trainModelSaveName = dialogSaveModel->name();
    }
    QFile file;
    file.copy(_trainModelName,_trainModelSaveName);
    file.setFileName(_trainModelName);
    file.remove();
    file.close();
}
