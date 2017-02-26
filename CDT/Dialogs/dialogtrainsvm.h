#ifndef DIALOGTRAINSVM_H
#define DIALOGTRAINSVM_H

#include <QDialog>

namespace Ui {
class DialogTrainSVM;
}

class DialogTrainSVM : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTrainSVM(QWidget *parent = 0);
    ~DialogTrainSVM();

    QAction *actionInputTrainFolder;
    QAction *actionTrainSVM;
    QAction *actionSaveModel;
private:
    Ui::DialogTrainSVM *ui;

    ///All train records are saved in classNameFileInfoList
    QMap<QString,QFileInfoList> classNameFileInfoList;
    QMap<QString,QString> classNameTransform;
    QStringList classNameStringList;

    QString _trainModelName;
    int _trainSamplesNum;
    int _sampleHeight,_sampleWidth;

    void changeSaveXML(QString &xmlName);
    bool deleteSomethingInXML(QString &fileName);

private slots:
    void inputTainFolders();
    void changeCurrentClass();
    void trainSVMmodel();
    void saveModel();
};

#endif // DIALOGTRAINSVM_H
