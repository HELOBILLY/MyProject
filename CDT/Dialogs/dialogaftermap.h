#ifndef DIALOGAFTERMAP_H
#define DIALOGAFTERMAP_H

#include <QDialog>
#include <QString>

namespace Ui {
class DialogAfterMap;
}

class DialogAfterMap : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAfterMap(QWidget *parent = 0);
    ~DialogAfterMap();
    QString inputImagePath;

private slots:
    void on_radioButton_Current_clicked();

    void on_pushButton_clicked();

    void on_pushButton_OutPut_PDF_clicked();

    void on_pushButton_OutPut_Img_clicked();

private:
    Ui::DialogAfterMap *ui;
};
#endif // DIALOGAFTERMAP_H
