#include "grabcut_opencv.h"

const Scalar RED = Scalar(0,0,255);
const Scalar PINK = Scalar(230,130,255);
const Scalar BLUE = Scalar(255,0,0);
const Scalar LIGHTBLUE = Scalar(255,255,160);
const Scalar GREEN = Scalar(0,255,0);

const int BGD_KEY = CV_EVENT_FLAG_CTRLKEY;
const int FGD_KEY = CV_EVENT_FLAG_SHIFTKEY;

GCApplication::GCApplication()
{
    //bgdModel = NULL;
    //fgdModel = NULL;
}

GCApplication::~GCApplication()
{

}

void GCApplication::reset()
{
    if( !mask.empty() )
        mask.setTo(Scalar::all(GC_BGD));
    bgdPxls.clear();
    fgdPxls.clear();
    prBgdPxls.clear();
    prFgdPxls.clear();

    isInitialized = false;
    rectState = NOT_SET;    //NOT_SET == 0
    lblsState = NOT_SET;
    prLblsState = NOT_SET;
    iterCount = 0;
}

 void GCApplication::getBinMask( const Mat& comMask, Mat& binMask )
{
    if( comMask.empty() || comMask.type()!=CV_8UC1 )
        CV_Error( CV_StsBadArg, "comMask is empty or has incorrect type (not CV_8UC1)" );
    if( binMask.empty() || binMask.rows!=comMask.rows || binMask.cols!=comMask.cols )
        binMask.create( comMask.size(), CV_8UC1 );
    binMask = comMask & 1;
}

void GCApplication::setImageAndWinName( const Mat& _image, const string& _winName  )
{
    if( _image.empty() || _winName.empty() )
        return;
    image = _image;
    winName = _winName;
    mask.create( image.size(), CV_8UC1);
    reset();
}

void GCApplication::showImage() const
{
    if( image.empty() || winName.empty() )
        return;

    Mat res;
    Mat binMask;
    if( !isInitialized )
        image.copyTo( res );
    else
    {
        getBinMask( mask, binMask );
        image.copyTo( res, binMask );
    }

    vector<Point>::const_iterator it;

    for( it = bgdPxls.begin(); it != bgdPxls.end(); ++it )
        circle( res, *it, radius, BLUE, thickness );
    for( it = fgdPxls.begin(); it != fgdPxls.end(); ++it )
        circle( res, *it, radius, RED, thickness );
    for( it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it )
        circle( res, *it, radius, LIGHTBLUE, thickness );
    for( it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it )
        circle( res, *it, radius, PINK, thickness );

    if( rectState == IN_PROCESS || rectState == SET )
        rectangle( res, Point( rect.x, rect.y ), Point(rect.x + rect.width, rect.y + rect.height ), GREEN, 2);

    imshow( winName, res );
}

void GCApplication::setRectInMask()
{
    assert( !mask.empty() );
    mask.setTo( GC_BGD );   //GC_BGD == 0
    /*rect.x = max(0, rect.x);
    rect.y = max(0, rect.y);
    rect.width = min(rect.width, image->cols-rect.x);
    rect.height = min(rect.height, image->rows-rect.y);*/
    rect.x = 0;
    rect.y = 0;
    rect.width = mask.cols;
    rect.height = mask.rows;
    (mask(rect)).setTo( Scalar(GC_PR_FGD) );
}

void GCApplication::setLblsInMask( int flags, Point p, bool isPr )
{
    vector<Point> *bpxls, *fpxls;
    uchar bvalue, fvalue;
    if( !isPr )
    {
        bpxls = &bgdPxls;
        fpxls = &fgdPxls;
        bvalue = GC_BGD;    //0
        fvalue = GC_FGD;    //1
    }
    else
    {
        bpxls = &prBgdPxls;
        fpxls = &prFgdPxls;
        bvalue = GC_PR_BGD; //2
        fvalue = GC_PR_FGD; //3
    }
    if( flags & CV_EVENT_LBUTTONDOWN )
    {
        fpxls->push_back(p);
        circle( mask, p, radius, fvalue, thickness );
    }
    if( flags & CV_EVENT_RBUTTONDOWN )
    {
        bpxls->push_back(p);
        circle( mask, p, radius, bvalue, thickness );
    }
}

void GCApplication::mouseClick( int event, int x, int y, int flags, void* )
{
    switch( event )
    {
//    case CV_EVENT_LBUTTONDBLCLK:
//    {
//        if(rectState == NOT_SET)
//        {
//            rectState = IN_PROCESS;
//            rect = Rect( Point(rect.x, rect.y), Point(x,y) );
//            setRectInMask();
//            showImage();
//            rectState = SET;
//        }
//    }
//        break;
    case CV_EVENT_LBUTTONDOWN:

        if( rectState == SET )
        {
            setLblsInMask(flags, Point(x,y), false);
            lblsState = SET;
            showImage();
        }
        break;
    case CV_EVENT_RBUTTONDOWN:
        if(rectState == SET)
        {
            setLblsInMask(flags, Point(x,y), false);
            showImage();
            prLblsState = SET;
        }
        break;
    case CV_EVENT_MOUSEMOVE:
        if( rectState == IN_PROCESS )
        {
            rect = Rect( Point(rect.x, rect.y), Point(x,y) );
            showImage();
        }
        else if( lblsState == SET )
        {
            setLblsInMask(flags, Point(x,y), false);
            showImage();
        }
        else if( prLblsState == SET )
        {
            setLblsInMask(flags, Point(x,y), true);
            showImage();
        }
        break;
    }
}

int GCApplication::nextIter()
{
    if( isInitialized )
        grabCut( image, mask, rect, bgdModel, fgdModel, 1 );
    else
    {
        if( rectState != SET )
            return iterCount;

        if( lblsState == SET || prLblsState == SET )
            grabCut( image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_MASK );
        else
            grabCut( image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_RECT );

        isInitialized = true;
    }
    iterCount++;

    bgdPxls.clear();
    fgdPxls.clear();
    prBgdPxls.clear();
    prFgdPxls.clear();

    return iterCount;
}

void GCApplication::on_mouse( int event, int x, int y, int flags, void* param )
{
    GCApplication *gcapp = (GCApplication *)param;
    gcapp->mouseClick( event, x, y,flags, param );
}
