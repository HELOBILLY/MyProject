#include "dialogaftermap.h"
#include "ui_dialogaftermap.h"
#include "stable.h"
#include "mainwindow.h"
#include "cdtimagelayer.h"
#include "cdtprojectlayer.h"
#include "detectlayer.h"
#include "dialogdetecthogsvm.h"

DialogAfterMap::DialogAfterMap(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAfterMap)
{
    ui->setupUi(this);
    ui->comboBox_StandTemplate->addItem(tr("A3_Landscape_Blue"));
    ui->comboBox_StandTemplate->addItem(tr("A3_Landscape_Orange"));
    ui->comboBox_StandTemplate->addItem(tr("A3_Portrait_Blue"));
    ui->comboBox_StandTemplate->addItem(tr("A3_Portrait_Orange"));
    ui->comboBox_StandTemplate->addItem(tr("A4_Landscape_Blue"));
    ui->comboBox_StandTemplate->addItem(tr("A4_Landscape_Orange"));
    ui->comboBox_StandTemplate->addItem(tr("A4_Portrait_Blue"));
    ui->comboBox_StandTemplate->addItem(tr("A4_Portrait_Orange"));
}

DialogAfterMap::~DialogAfterMap()
{
    delete ui;
}

void DialogAfterMap::on_radioButton_Current_clicked()
{

}
//Browse new customized map template
void DialogAfterMap::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Map Template File"), "", tr("Map Template Files (*.qpt)"));

    if (!fileName.isEmpty())
        QMessageBox::information(NULL, "Title", fileName, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
}

void DialogAfterMap::on_pushButton_OutPut_PDF_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Wirte to PDF File"),
                               "/home/untitled.pdf",
                               tr("PDF Files (*.pdf)"));
    if (!fileName.isEmpty())
        QMessageBox::information(NULL, "Title", fileName, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    //get the layers
    //by before disaster and after disaster
    //by target

    //auto layers = MainWindow::getCurrentMapCanvas()->layers();
    //QList<QgsMapCanvasLayer> mapLayers;

    //QUuid pID=MainWindow::getCurrentProjectID();

    CDTProjectLayer *ProjLayer=MainWindow::GetCurrentProject();

    QVector<CDTImageLayer *> pImagelayers=ProjLayer->GetimagesLayers();

    //each imagelayer has different layers,such as detection,extract,merge
    //for disaster before, find the target detected layer.
    foreach (CDTImageLayer *layer, pImagelayers)
    {
        QString layername=layer->name();
//        QMessageBox::information(NULL, "LayerName", layername, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        //get the detectlayers of different target
        QVector<DetectLayer*> pDetectionLayers=layer->GetDetectionLayers();
        foreach (DetectLayer *dlayer, pDetectionLayers) {
            QString dlayername=dlayer->name();
            QgsMapLayer *qgsdetectlayer=dlayer->canvasLayer();
            //QString qgsdlayername=qgsdetectlayer->name();
//          QMessageBox::information(NULL, "DetectionLayerName", dlayername+qgsdlayername, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            //if the selected output target is found, then construct the map


        }

       // mapLayers<<QgsMapCanvasLayer(layer);
    }
/*
    foreach (QgsMapLayer *layer, layers) {
        QString layername=layer->name();
        //QString layerid=layer->id();
        QMessageBox::information(NULL, "LayerName", layername, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        mapLayers<<QgsMapCanvasLayer(layer);
    }
    MainWindow::getCurrentMapCanvas()->setLayerSet(mapLayers);
    MainWindow::getCurrentMapCanvas()->refresh();
*/
    // "theMapCanvas" used to find this canonical instance later
 /*     mMapCanvas = new QgsMapCanvas( centralWidget, "theMapCanvas" );
      mMapCanvas->setWhatsThis( tr( "Map canvas. This is where raster and vector "
                                    "layers are displayed when added to the map" ) );
      mMapCanvas = new QgsMapCanvas();
      mMapCanvas->freeze();

      // set canvas color right away
      int myRed = settings.value( "/qgis/default_canvas_color_red", 255 ).toInt();
      int myGreen = settings.value( "/qgis/default_canvas_color_green", 255 ).toInt();
      int myBlue = settings.value( "/qgis/default_canvas_color_blue", 255 ).toInt();
      mMapCanvas->setCanvasColor( QColor( myRed, myGreen, myBlue ) );
*/
//     QgsMapRenderer * 	mapRenderer;
//      QgsMapSettings mapSettings;

//      mapSettings.setExtent( extent() );
//      mapSettings.setOutputSize( outputSize() );
//      mapSettings.setOutputDpi( !qgsDoubleNear( outputDpi(), 0 ) ? outputDpi() : qt_defaultDpiX() );
//      mapSettings.setLayers( layerSet() );
//      mapSettings.setCrsTransformEnabled( hasCrsTransformEnabled() );
//      mapSettings.setDestinationCrs( destinationCrs() );
//      mapSettings.setMapUnits( mapUnits() );

//      QStringList slayers;
//      QgsComposition *mComposition = new QgsComposition( mQgis->mapCanvas()->mapSettings() );
//      QgisApp::instance()->mapCanvas()->mapSettings();

//      mapSettings.setLayers(slyers);
//   // QgsComposition  qgsmap(mapRenderer);
//      QDomDocument templateDoc;
//      if ( templateDoc.setContent( &templateFile ) )
//      {
//          mComposition->loadFromTemplate( templateDoc, nullptr, false, newComposer );
//          c->setUpdatesEnabled( true );
//      }
//    QPainter;


      bool newComposer;
      newComposer=true;
      QSettings settings;
      QString openFileDir = settings.value( "UI/lastComposerTemplateDir", QDir::homePath() ).toString();
      QString openFileString = QFileDialog::getOpenFileName( nullptr, tr( "Load template" ), openFileDir, "*.qpt" );

       if ( openFileString.isEmpty() )
        {
          return; //canceled by the user
        }

       QFile templateFile( openFileString );
       if ( !templateFile.open( QIODevice::ReadOnly ) )
        {
          QMessageBox::warning( this, tr( "Read error" ), tr( "Error, could not read file" ) );
          return;
        }
       QgsMapSettings mapSettings;

       mapSettings=MainWindow::getCurrentMapCanvas()->mapSettings();
       QgsRectangle extent=mapSettings.extent();
       //QgsCoordinateReferenceSystem CRS = mapSettings
       //mapSettings.getOutputSize();
       //mapSettings.getOutputDpi();
       //mapSettings.getLayers();
       //bool bCrsed=mapSettings.hasCrsTransformEnabled();
       //mapSettings.getDestinationCrs();
       //mapSettings.getMapUnits();

       //bool bvalid=mapSettings.hasValidSettings();

       QStringList slayers=mapSettings.layers();
       //int nl=layers.count();
       foreach (QString layer, slayers)
       {
           QString layername=layer;
           //QString layerid=layer->id();
//           QMessageBox::information(NULL, "LayerName", layername, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
       }
        QgsComposition* comp = new QgsComposition(MainWindow::getCurrentMapCanvas()->mapSettings());

        //comp = mComposition;
        if ( comp )
        {
          QDomDocument templateDoc;
          if ( templateDoc.setContent( &templateFile ) )
          {
            comp->loadFromTemplate( templateDoc, nullptr, false, newComposer );
          }
        }
        //save as pdf
        QString outputFileName = fileName;//"d:/nn.pdf";
        bool exportOk = comp->exportAsPDF( outputFileName );
        if ( !exportOk )
        {
          QMessageBox::warning( this, tr( "Atlas processing error" ),
                                  QString( tr( "Error creating %1." ) ).arg( outputFileName ),
                                  QMessageBox::Ok,
                                  QMessageBox::Ok );
            return;
        }else
        {
          QMessageBox::information(this,tr("Information"),tr("Thememap Completed!"));
        }

}


void DialogAfterMap::on_pushButton_OutPut_Img_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Write to Image File"),
                               "/home/untitled.jpg",
                               tr("Images (*.jpg *.xpm *.png)"));
    if (!fileName.isEmpty())
        QMessageBox::information(NULL, "Title", fileName, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
}
