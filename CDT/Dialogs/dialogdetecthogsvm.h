#ifndef DIALOGDETECTHOGSVM_H
#define DIALOGDETECTHOGSVM_H

#include <QDialog>
#include <QProgressBar>
#include <QTime>
#include <QTimer>
#include <QDebug>

class QCheckBox;
class QLineEdit;
class QComboBox;
class QColor;
class QtColorPicker;
class CDTFileSystem;
class QXmlStreamReader;
struct QUuid;

namespace Ui {
class DialogDetectHogSVM;
}

class DialogDetectHogSVM : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDetectHogSVM(
            const QString &inputImage,
            CDTFileSystem* fileSys ,
            QWidget *parent = 0);
    ~DialogDetectHogSVM();

    QColor  borderColor() const;
    QStringList returnshpIDList();
    QStringList returnshpPathList();
    QStringList returnshpNameList();

private:
    QStringList tagShpfilePath,shpIDList,tagShpfileName;
    QList<bool> categoryIsChosen;
    Ui::DialogDetectHogSVM *ui;
    QLineEdit *modelSizeLineEdit;
    QCheckBox *my3rd1stCheckBox;
    QCheckBox *my3rd2ndCheckBox;
    QCheckBox *my3rd3rdCheckBox;
    QCheckBox *my3rd4thCheckBox;
    QCheckBox *my3rd5CheckBox;
    QCheckBox *my3rd6CheckBox;
    QLineEdit *my1stLineEdit;
    QComboBox *my5thCombox;
    QtColorPicker *my2ndColorPicker;
    QPushButton *startButton ;
    QProgressBar *progressBar;
    QPushButton *okButton ;
    QPushButton *cancelButton ;
    QString inputImagePath;
    CDTFileSystem *fileSystem;
    QXmlStreamReader reader;
    //QTime *time;
    //QTimer *progressTimer;

    bool isFinished;
    int _sampleHeight,_sampleWidth;
    QString  _importModelName;

    bool polygonize(QString &rasterPath, QString &tagImagePath, QString &polyPath, int isDrawing);
    void labelit(QString &atestRasterPath, int isDrawing);
    void skipUnknownElement();
    void readXmlElement();
    bool readFile(QString &fileName);
private slots:
    void importModel();
    void startPredictLabel();
    //void progressShow();
};

#endif // DIALOGDETECTHOGSVM_H
