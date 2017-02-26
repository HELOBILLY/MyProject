//#ifndef GRABCUT_OPENCV_H
//#define GRABCUT_OPENCV_H
#pragma once
#include <QtCore>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <cdtgrabcutmaptool.h>

using namespace std;
using namespace cv;

class GCApplication
{
public:
    GCApplication();
    ~GCApplication();

public:
    enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
    int radius;
    static const int thickness = -1;

    void reset();
    void setImageAndWinName( const Mat& _image, const string& _winName );
    void showImage() const;
    void mouseClick( int event, int x, int y, int flags, void* param );
    int nextIter();
    int getIterCount() const { return iterCount; }

    static void getBinMask( const Mat& comMask, Mat& binMask );
    static void on_mouse( int event, int x, int y, int flags, void* param);

    Mat mask;
    Mat bgdModel, fgdModel;
    Rect rect;

    void setRectInMask();
    void setLblsInMask( int flags, Point p, bool isPr );

    string winName;
    Mat image;

    uchar rectState, lblsState, prLblsState;
    bool isInitialized;

    vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
    int iterCount;
};

//#endif // GRABCUT_OPENCV_H
