#include "leohelper.h"
#include "stable.h"

LeoHelper::LeoHelper()
{
}

bool LeoHelper::outmat(QString txt, cv::Mat &data)
{
    QFile file(QDir::currentPath()+ "/"+txt);
    if (!file.open(QFile::WriteOnly | QFile::Text)){
        qDebug()<<"Error: read txt failed";
        return false;
    }
    QTextStream out(&file);
    for(int i=0;i<data.rows;i++)
    {
        float *pRow = data.ptr<float>(i);
        for(int j=0;j<data.cols;j++){
            out<<QString::number(pRow[j]);
            out<<"       ";
        }
        out<<endl;
    }
    return true;
}
