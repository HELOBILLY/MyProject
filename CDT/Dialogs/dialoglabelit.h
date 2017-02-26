#ifndef DIALOGLABELIT_H
#define DIALOGLABELIT_H

#include <QDialog>

namespace Ui {
class DialogLabelit;
}

class DialogLabelit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLabelit(QWidget *parent = 0);
    ~DialogLabelit();
    QString  name();
    void setPostFix(QString &post);

private:
    Ui::DialogLabelit *ui;
    QString Path;
    QString postFix;
    int index;


private slots:
    void outPutPath();

};

#endif // DIALOGLABELIT_H
