#ifndef LEOHELPER_H
#define LEOHELPER_H

class cv::Mat;
class LeoHelper
{
public:
    LeoHelper();
    static bool outmat(QString txt, cv::Mat &data);
};

#endif // LEOHELPER_H
