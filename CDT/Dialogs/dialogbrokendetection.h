#ifndef DIALOGBROKENDETECTION_H
#define DIALOGBROKENDETECTION_H

#include <QDialog>
//#include <stxxl/vector>
//typedef stxxl::VECTOR_GENERATOR<QPoint>::result vectorQPoint;
class CDTFileSystem;
class QComboBox;
class GDALDataset;
class cv::Mat;
class QLineEdit;
namespace Ui {
class DialogBrokenDetection;
}

class DialogBrokenDetection : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBrokenDetection(
            const QString &inputImage,
            CDTFileSystem* fileSys ,
            QWidget *parent = 0);

    ~DialogBrokenDetection();
    QString name();
    QString shpid();

private:
    Ui::DialogBrokenDetection *ui;

    CDTFileSystem *fileSystem;

    QLineEdit   *newNameEdit;
    QComboBox   *shpFilePath;
    QComboBox   *imageFilePath;
    QComboBox   *fieldComBox;
    QComboBox   *valueComBox;
    QString _newRasterPath;
    QString _shpFilePath;
    QString _imageFilePath;
    QString _fieldName;
    QString _valueName;
    QString _tagImagePath;
    QString labelShpfilePath;
    QString shpID;
    int _width;
    int _height;
    QList<QPoint>  certainPoints;
    GDALDataset * _poTagImageDS;
    GDALDataset * _poImageDS;
    bool polygonize(QString &rasterPath, QString &tagImagePath, QString &polyPath);
    void rasterize();
    bool labelit(QString &fileName, QList<QPoint> &points);
    bool edgeCanny(QString &fileName, cv::Mat &retMat, int &isColor);
    bool getOneFieldValues(const char *pszName,const char *oneField,QStringList &fieldLists);
    bool getFields(const char *pszName,QStringList &fieldLists);
    float calcOrientationHist( const cv::Mat& img, QPoint pt, int radius,
                                      float* hist,int n ,int isSmoothed,
                                      int isWeighted,float weighted_sigma);

private slots:
    void inputShpFilePath();
//    void inputImageFilePath();
    void _changeValueComBox();
    void startDetect();

};

#endif // DIALOGBROKENDETECTION_H
