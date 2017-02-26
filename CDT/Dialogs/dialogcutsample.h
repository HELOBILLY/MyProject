#ifndef DIALOGCUTSAMPLE_H
#define DIALOGCUTSAMPLE_H

#include <QDialog>
#include <QProgressBar>

namespace Ui {
class DialogCutSample;
}

class DialogCutSample : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCutSample(QWidget *parent = 0);
    ~DialogCutSample();

    QString InputImagePath() const;
    QString OutputImagePath() const;
    QString SizeX() const;
    QString SizeY() const;
    QString OverlayX() const;
    QString OverlayY() const;
    int InputSizeX;
    int InputSizeY;
    int InputOverlayX;
    int InputOverlayY;
    int count;

    QString dir;
    QString path;

private slots:
    void on_pushButtonInput_clicked();
    void on_pushButtonOutput_clicked();
    void on_pushButton_OK_clicked();
    void on_pushButton_Cancel_clicked();

private:

    Ui::DialogCutSample *ui;
};

#endif // DIALOGCUTSAMPLE_H
