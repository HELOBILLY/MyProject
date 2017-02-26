#include "dialogcutsample.h"
#include "ui_dialogcutsample.h"
#include "stable.h"
#include <QMessageBox>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

DialogCutSample::DialogCutSample(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCutSample)
{
    ui->setupUi(this);
    setWindowTitle(tr("Select Sample"));
    ui->progressBar->setValue(0);
}

DialogCutSample::~DialogCutSample()
{
    delete ui;
}

QString DialogCutSample::InputImagePath() const
{
    return ui->comboBoxInput->itemText(ui->comboBoxInput->currentIndex());
}

QString DialogCutSample::OutputImagePath() const
{
    return ui->comboBoxOutput->itemText(ui->comboBoxOutput->currentIndex());
}

QString DialogCutSample::SizeX() const
{
    return ui->lineEdit_SizeX->text();
}

QString DialogCutSample::SizeY() const
{
    return ui->lineEdit_SizeY->text();
}

QString DialogCutSample::OverlayX() const
{
    return ui->lineEdit_OverlayX->text();
}

QString DialogCutSample::OverlayY() const
{
    return ui->lineEdit_overlayY->text();
}

void DialogCutSample::on_pushButtonInput_clicked()
{
    dir = QFileDialog::getOpenFileName(this,tr("Open image"),".","Images (*.png *.xpm *.jpg *.img *.tif)");
    if(dir.isEmpty())
    {
        QMessageBox::critical(this,tr("Error"),tr("Please Load Image!"));
        return;
    }

    ui->comboBoxInput->insertItem(0,dir);
    ui->comboBoxInput->setCurrentIndex(0);
}

void DialogCutSample::on_pushButtonOutput_clicked()
{
    path = QFileDialog::getExistingDirectory(NULL, tr("Choose the folder"),"",QFileDialog::ShowDirsOnly);
    if(path.isEmpty())
    {
        QMessageBox::critical(this,tr("Error"),tr("Please Load Folder!"));
        return;
    }
    path.replace("\\","/");

    ui->comboBoxOutput->insertItem(0,path);
    ui->comboBoxOutput->setCurrentIndex(0);
}

void DialogCutSample::on_pushButton_OK_clicked()
{
    string filename = dir.toStdString();
    Mat image = imread(filename);
    if (image.empty())
    {
        QMessageBox::critical(this,tr("Error"),tr("Please Load Image!"));
        return;
    }

    //Set Output Path
    QFileInfo fileinfo(dir);
    QString name = fileinfo.baseName();
    QString tempPath = path+'/'+name;

    int x = image.cols;
    int y = image.rows;

    QString sizeX = ui->lineEdit_SizeX->text();
    InputSizeX = sizeX.toInt();
    QString sizeY = ui->lineEdit_SizeY->text();
    InputSizeY = sizeY.toInt();
    QString overlayX = ui->lineEdit_OverlayX->text();
    InputOverlayX = overlayX.toInt();
    QString overlayY = ui->lineEdit_overlayY->text();
    InputOverlayY = overlayY.toInt();

    if (InputSizeX <= 0 || InputSizeY <= 0 || InputSizeX > x || InputSizeY > y)
    {
        QMessageBox::critical(this,tr("Error"),tr("Parameter error!"));
        return;
    }

    if (InputOverlayX < 0 || InputOverlayY < 0 || InputOverlayX > InputSizeX || InputOverlayY > InputSizeY)
    {
        QMessageBox::critical(this,tr("Error"),tr("Parameter error!"));
        return;
    }

    int intervalX = InputSizeX - InputOverlayX;
    int intervalY = InputSizeY - InputOverlayY;

    int Xcount = x/intervalX;
    int Ycount = y/intervalY;
    count = Xcount * Ycount;
    ui->progressBar->setRange(0,count);
    int progressNumber = 0;

    for (int i = 0; i < image.rows; i += intervalX)
    {
        int m = i/intervalX;
        for (int j = 0; j < image.cols; j += intervalY)
        {
            int n = j/intervalY;
            progressNumber++;

            char strM[10];
            char strN[10];
            itoa(m,strM,10);
            itoa(n,strN,10);
            string ImgPath = tempPath.toStdString() + "_" +strM + "_" + strN +".tif";

            if ((InputSizeX + i < y) && (InputSizeY + j < x))
            {
                Mat Sample = image(Rect(j, i, InputSizeY, InputSizeX));
                imwrite(ImgPath,Sample);
            }

            ui->progressBar->setValue(progressNumber);
        }
    }
    QMessageBox::information(this,tr("Information"),tr("Select Sample has completed!"));
}

void DialogCutSample::on_pushButton_Cancel_clicked()
{
    this->close();
}
