#include "cdtgrabcutmaptool.h"
#include <QtCore>
#include <QLabel>
#include <QImage>
#include <QScrollArea>
#include <qgsgeometry.h>
#include <qgsrubberband.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgis.h>
#include <QMouseEvent>
#include <gdal_priv.h>
#include <opencv2/opencv.hpp>

typedef struct tagVERTEX2D
{
    double x;
    double y;
}VERTEX2D;

QgsPolygon grabcut(const QgsPolygon &polygon,QString imagePath)
{
    //init
    GDALAllRegister();
    GDALDataset *poSrcDS = (GDALDataset *)GDALOpen(imagePath.toUtf8().constData(),GA_ReadOnly);
    if (poSrcDS == NULL)
    {
        qWarning()<<QObject::tr("Open Image File: %1 failed!").arg(imagePath);
        return QgsPolygon();
    }
    //int _bandCount = poSrcDS->GetRasterCount();
    double ImageWidth = poSrcDS->GetRasterXSize();
    double ImageHeight = poSrcDS->GetRasterYSize();

    double padfTransform[6] = {0,1,0,0,0,1};
    double padfTransform_i[6];
    poSrcDS->GetGeoTransform(padfTransform);
    double pStdGeoTransform[6] = {0,1,0,0,0,1};
    bool isTrans = false;
    for (int i=0;i<6;++i)
    {
        if (fabs(padfTransform[i]-pStdGeoTransform[i])>0.001)
        {
            isTrans = true;
            break;
        }
    }
    if (!isTrans)
    {
        padfTransform[5] = -1.0;
    }
    GDALInvGeoTransform(padfTransform,padfTransform_i);

    //transform
    std::vector<VERTEX2D> vecInputPoints;
    QVector<QgsPoint> originalPoints = polygon[0];
    foreach (QgsPoint pt, originalPoints) {
        VERTEX2D newPt;
        GDALApplyGeoTransform(padfTransform_i,pt.x(),pt.y(),&newPt.x,&newPt.y);
        vecInputPoints.push_back(newPt);
    }

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::min();
    double maxY = std::numeric_limits<double>::min();

    //The coordinate value of the image range
    double OriginalMinX = padfTransform[0]+0*padfTransform[1]+0*padfTransform[2];
    double OriginalMinY = padfTransform[3]+0*padfTransform[4]+0*padfTransform[5];
    double OriginalMaxX = padfTransform[0]+ImageWidth*padfTransform[1]+ImageHeight*padfTransform[2];
    double OriginalMaxY = padfTransform[3]+ImageWidth*padfTransform[4]+ImageHeight*padfTransform[5];
    //qDebug()<<OriginalMinX<<OriginalMinY<<OriginalMaxX<<OriginalMaxY;

    for (size_t i=0;i<vecInputPoints.size();++i)
    {
        if (vecInputPoints[i].x<minX)
            minX = vecInputPoints[i].x;
        if (vecInputPoints[i].x>maxX)
            maxX = vecInputPoints[i].x;
        if (vecInputPoints[i].y<minY)
            minY = vecInputPoints[i].y;
        if (vecInputPoints[i].y>maxY)
            maxY = vecInputPoints[i].y;
    }

    /*if(minX < 0 || maxX > ImageWidth || minY < 0 || maxY > ImageHeight)
    {
        qWarning()<<QObject::tr("The range of the selected point is out of bounds");
        return QgsPolygon();
    }

    if(minX-0.5 < 1e-5)
        minX = 0.5;
    if(maxX-2047.5 > 1e-5)
        maxX = 2047.5;
    if(minY-0.5 < 1e-5)
        minY = 0.5;
    if(maxY-2047.5 > 1e-5)
        maxY = 2047.5;*/

    //Limit the coordinate range
    if(minX+OriginalMinX < 1e-5)
        minX = -OriginalMinX;
    if(maxX-OriginalMaxX > 1e-5)
        maxX = OriginalMaxX;
    if(minY-OriginalMinY < 1e-5)
        minY = OriginalMinY;
    if(maxY+OriginalMaxY > 1e-5)
        maxY = -OriginalMaxY;

    int rec_nXOff = (int)minX;
    int rec_nYOff = (int)minY;
    int rec_nWidth  = (int)maxX-rec_nXOff+1;
    int rec_nHeight = (int)maxY-rec_nYOff+1;

    //int nXOff = rec_nXOff - rec_nWidth/12;
    //int nYOff = rec_nYOff - rec_nHeight/12;
    //int nWidth = rec_nWidth + rec_nWidth/6;
    //int nHeight = rec_nHeight + rec_nHeight/6;

    /*if(nXOff<0)
    {
        nXOff=0;
    }
    if(nYOff<0)
    {
        nYOff=0;
    }*/

    /*QVector<double> minBandVal_texture(_bandCount),maxBandVal_texture(_bandCount);
    double dfMin,dfMax,dfMean,dfStdDev;
    for (int k=0;k<3;++k)
    {
        GDALRasterBand *poBand =  poSrcDS->GetRasterBand(k+1);
        poBand->GetStatistics ( FALSE,  TRUE, &dfMin,  &dfMax,  &dfMean,  &dfStdDev);
        if(dfMin>=0&&dfMax<=255)
        {
            minBandVal_texture[k] = 0;
            maxBandVal_texture[k] = 255;
        }else{
            //        minBandVal_texture[k]=dfMean-2*dfStdDev;
            //        maxBandVal_texture[k]=dfMean+2*dfStdDev;
            int *anHistogram=new int[int(dfMax-dfMin+1)];
            poBand->GetHistogram( dfMin-0.5, dfMax+0.5, dfMax-dfMin+1, anHistogram, FALSE, FALSE, GDALDummyProgress, NULL );

            float nLeft = 5,nRight = 5;
            float histRatioL=nLeft*1.0/100;
            float histRatioH=nRight*1.0/100;

            int lowDes = (int) (histRatioL * ImageWidth*ImageHeight);
            int highDes = (int)(histRatioH * ImageWidth*ImageHeight);
            int lh[2] = {0, 255};
            int h,sum;
            for (h = 0, sum = 0; h < dfMax-dfMin+1; ++h)
            {
                sum += anHistogram[h];
                if (sum >= lowDes)
                {
                    lh[0] = h+dfMin;
                    break;
                }
            }
            for (h = dfMax-dfMin+1 - 1, sum = 0; h >= 0; --h)
            {
                sum += anHistogram[h];
                if (sum >= highDes)
                {
                    lh[1] = h+dfMin;
                    break;
                }
            }

            delete []anHistogram;

            qDebug()<<lh[0];
            qDebug()<<lh[1];
            minBandVal_texture[k] = lh[0];
            maxBandVal_texture[k] = lh[1];
        }

    }*/

    GCApplication gcapp;
    gcapp.radius = 3;

    string filename = imagePath.toStdString();
    Mat SrcImage = imread(filename,1);
    if( SrcImage.empty() )
    {
        cout<< "\n Durn, couldn't read image filename " << filename << endl;
        return QgsPolygon();
    }

    Mat image(SrcImage,Rect(rec_nXOff,rec_nYOff,rec_nWidth,rec_nHeight));

    const string winName = "image";
    cvNamedWindow( winName.c_str(), CV_WINDOW_AUTOSIZE );
    cvSetMouseCallback( winName.c_str(), GCApplication::on_mouse, &gcapp );

    gcapp.setImageAndWinName( image, winName );
    if (gcapp.rectState == gcapp.NOT_SET)
    {
        gcapp.rectState = gcapp.IN_PROCESS;
        gcapp.rect = Rect(0,0,image.cols,image.rows);
        gcapp.setRectInMask();
        gcapp.showImage();
        gcapp.rectState = gcapp.SET;
    }
    //gcapp.showImage();

    for(;;)
    {
        int c = cvWaitKey(0);
        bool isBreak = false;
        switch( (char) c )
        {
        case '\x0d':
            isBreak = true;
            break;
        case '+':
            if(gcapp.radius<0 || gcapp.radius >10)
            {
                cout <<"The radius is error!"<< endl;
                return QgsPolygon();
            }
            else
                gcapp.radius++;
            break;
        case '-':
            if(gcapp.radius<0 || gcapp.radius >10)
            {
                cout <<"The radius is error!"<< endl;
                return QgsPolygon();
            }
            else
                gcapp.radius--;
            break;
        case 'r':
            cout << endl;
            gcapp.reset();
            gcapp.showImage();
            break;
        case 'n':
            int iterCount = gcapp.getIterCount();
            cout << "<" << iterCount << "... ";
            int newIterCount = gcapp.nextIter();
            if( newIterCount > iterCount )
            {
                gcapp.showImage();
                cout << iterCount << ">" << endl;
            }
            else
                cout << "rect must be determined>" << endl;
            break;
        }

        if (isBreak)
            break;
    }
    cvDestroyAllWindows();

    /*cv::Mat  bgdModel, fgdModel;
    cv::Mat  image = cv::Mat::zeros(nHeight,nWidth,CV_8UC3);
    std::vector<uchar> vecImageData(nWidth*nHeight);

        for (int k=0;k<3;++k)
        {
            const double pixelmax = minBandVal_texture[k];
            const double pixelmin = maxBandVal_texture[k];
            const double ak = (double)255 / (double)(pixelmax - pixelmin);
            const double bk =-(double)255 * (double)pixelmin/(double)(pixelmax - pixelmin);
            poSrcDS->GetRasterBand(k+1)->RasterIO(GF_Read,nXOff,nYOff,nWidth,nHeight,&vecImageData[0],nWidth,nHeight,GDT_Byte,0,0);

            for(int i=0;i<nHeight;i++)
            {
                for(int j=0;j<nWidth;j++)
                {
                     image.at<cv::Vec3b>(i,j)[k]= ak*vecImageData[i*nWidth+j]+bk+0.5;
                }
            }

        }

    cv::Rect rect;
    rect.x = rec_nWidth/12;
    rect.y = rec_nHeight/12;
    rect.width = rec_nWidth;
    rect.height = rec_nHeight;

    const cv::Scalar GREEN = cv::Scalar(0,255,0);
    cv::Mat mask = cv::Mat::zeros(nHeight,nWidth,CV_8U);
    mask.setTo(0);

    std::vector<cv::Point> pContourInputPoints;
    std::vector< std::vector<cv::Point> > pContour;

    for (size_t i=0;i<vecInputPoints.size();++i)
    {
       cv::Point acvPoint;
       acvPoint.x=vecInputPoints[i].x -nXOff;
       acvPoint.y=vecInputPoints[i].y -nYOff;
       pContourInputPoints.push_back(acvPoint);
    }
    pContour.push_back(pContourInputPoints);

    cv::drawContours(mask,pContour,0,cv::Scalar(3),CV_FILLED);

    cv::grabCut(image,mask,rect,bgdModel,fgdModel,3,cv::GC_INIT_WITH_MASK);*/

    const cv::Scalar GREEN = cv::Scalar(0,255,0);
    cv::Mat res,binMask;
    cv::rectangle(image,gcapp.rect,GREEN,2);

    cv::imwrite("image.tif",image);
    binMask.create( gcapp.mask.size(), CV_8UC1 );
    binMask = gcapp.mask & 1;
    image.copyTo( res, binMask );
    cv::imwrite("res.tif",res);

    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binMask,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);

    std::vector<VERTEX2D> vecAllPoints;

    QgsPolygon exportPolygon;
    QgsPolyline polyline;

    //Find the largest contour
    int maxSize = 0 ;
    foreach(std::vector<cv::Point>  contour,contours)
    {
        if(contour.size()>maxSize)
        {
            maxSize = contour.size();
        }
    }

    foreach(std::vector<cv::Point>  contour,contours)
    {
        vecAllPoints.clear();
        if((!contour.empty())&&(contour.size()==maxSize))
        {
            for(int i =0;i<contour.size();i++)
            {
                VERTEX2D Pt;
                Pt.x = contour.at(i).x;
                Pt.y = contour.at(i).y;
                vecAllPoints.push_back(Pt);
            }

            for (int i=0;i<vecAllPoints.size();++i)
            {
                vecAllPoints[i].x += rec_nXOff;
                vecAllPoints[i].y += rec_nYOff;
            }

            for (int i = 0;i < vecAllPoints.size();i++)
            {
                QgsPoint ptTemp;
                ptTemp.set(
                        padfTransform[0] + padfTransform[1]*vecAllPoints[i].x
                        + padfTransform[2]*vecAllPoints[i].y,
                        padfTransform[3] + padfTransform[4]*vecAllPoints[i].x
                        + padfTransform[5]*vecAllPoints[i].y);
                polyline.push_back(ptTemp);
            }
        }
    } 

    exportPolygon.push_back(polyline);
    qDebug()<< "test error" <<"2" <<endl;
    return exportPolygon;
}

CDTGrabcutMapTool::CDTGrabcutMapTool(QgsMapCanvas *canvas) :
    QgsMapTool(canvas),
    mRubberBand(NULL),
    vectorLayer(NULL)
{
    mCursor = Qt::ArrowCursor;
}

CDTGrabcutMapTool::~CDTGrabcutMapTool()
{
    delete mRubberBand;
}

void CDTGrabcutMapTool::canvasMoveEvent(QMouseEvent *e)
{
    if ( mRubberBand == NULL )
    {
        return;
    }
    if ( mRubberBand->numberOfVertices() > 0 )
    {
        mRubberBand->removeLastPoint( 0 );
        mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    }
}

void CDTGrabcutMapTool::canvasPressEvent(QMouseEvent *e)
{
    if ( mRubberBand == NULL )
    {
        mRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
        mRubberBand->setBorderColor(QColor(Qt::red));
    }
    if ( e->button() == Qt::LeftButton )
    {
        mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    }
    else if ( e->button() == Qt::RightButton )
    {
        if ( mRubberBand->numberOfVertices() > 2 )
        {
            QgsGeometry* polygonGeom = mRubberBand->asGeometry();
            QgsPolygon polygon = polygonGeom->asPolygon();
            delete polygonGeom;
            mRubberBand->reset( QGis::Polygon );
            delete mRubberBand;
            mRubberBand = 0;

            qDebug()<< "test error" <<"1" <<endl;
            QgsPolygon snakePolygon = grabcut(polygon,imagePath);
            qDebug()<< "test error" <<"3" <<endl;
            QgsGeometry* newPolygonGeom = QgsGeometry::fromPolygon(snakePolygon);
            QgsFeature f(vectorLayer->pendingFields(),0);
            f.setGeometry(newPolygonGeom);
            vectorLayer->beginEditCommand( "Grabcut" );
            qDebug()<<vectorLayer->addFeature(f);
            vectorLayer->endEditCommand();
            canvas()->refresh();
            qDebug()<< "Test error" << "4"<< endl;
        }
        else{
            mRubberBand->reset( QGis::Polygon );
            delete mRubberBand;
            mRubberBand = 0;
        }
    }
}
