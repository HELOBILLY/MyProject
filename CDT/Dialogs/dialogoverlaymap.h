#ifndef DIALOGOVERLAYMAP_H
#define DIALOGOVERLAYMAP_H

#include <QDialog>
#include <QString>

namespace Ui {
class DialogOverlayMap;
}

class DialogOverlayMap : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOverlayMap(QWidget *parent = 0);
    ~DialogOverlayMap();
    QString inputImagePath;

private slots:
    void on_radioButton_Current_clicked();

    void on_pushButton_clicked();

    void on_pushButton_OutPut_PDF_clicked();

    void on_pushButton_OutPut_Img_clicked();

private:
    Ui::DialogOverlayMap *ui;
};
#endif // DIALOGOVERLAYMAP_H
