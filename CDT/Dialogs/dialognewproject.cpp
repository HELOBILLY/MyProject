#include "dialognewproject.h"
#include "ui_dialognewproject.h"
#include "stable.h"

DialogNewProject::DialogNewProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewProject)
{
    ui->setupUi(this);
}

DialogNewProject::~DialogNewProject()
{
    delete ui;
}

QString DialogNewProject::projectName() const
{
    return ui->lineEditName->text();
}

QString DialogNewProject::projectPath() const
{
    return ui->lineEditPath->text();
}

void DialogNewProject::on_pushButton_clicked()
{    
    QSettings setting("WHU","CDTStudio");//company and project
    setting.beginGroup("Project");
    QString filepath = setting.value("lastDir",".").toString();
    QString path = QFileDialog::getSaveFileName(this,tr("Create project file"),filepath,"*.cdtpro");

    if (path.isEmpty())
        return;
    QFileInfo fileinfo(path);
    ui->lineEditPath->setText(path);    
    ui->lineEditName->setText(fileinfo.baseName());
    setting.setValue("lastDir",fileinfo.absolutePath());
    setting.endGroup();
}
