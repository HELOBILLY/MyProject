#ifndef DIALOGTHEMEMAP_H
#define DIALOGTHEMEMAP_H

#include <QDialog>
#include <QString>

namespace Ui {
class DialogThemeMap;
}

class DialogThemeMap : public QDialog
{
    Q_OBJECT

public:
    explicit DialogThemeMap(QWidget *parent = 0);
    ~DialogThemeMap();
    QString inputImagePath;

private slots:
    void on_radioButton_Current_clicked();

    void on_pushButton_clicked();

    void on_pushButton_OutPut_PDF_clicked();

    void on_pushButton_Output_Img_clicked();

private:
    Ui::DialogThemeMap *ui;
};

#endif // DIALOGTHEMEMAP_H
