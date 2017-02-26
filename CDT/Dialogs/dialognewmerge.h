#ifndef DIALOGNEWMERGE_H
#define DIALOGNEWMERGE_H

#include <QDialog>
class CDTFileSystem;

namespace Ui {
class DialogNewMerge;
}

class DialogNewMerge : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewMerge(const QString shpfilePathA,
                            const QString shpfilePathB,
                            CDTFileSystem* fileSys,
                            QWidget *parent = 0);
    ~DialogNewMerge();

    QString name()          const;
    QColor  color()         const;
    QColor  borderColor()   const;
    QString fileID()        const;
private slots:
    void onAccepted();
private:
    Ui::DialogNewMerge *ui;
private:
    QString _shpfilePathA;
    QString _shpfilePathB;
    CDTFileSystem *fileSystem;

    QString shapefileTempPath;
    QString shapefileID;
};

#endif // DIALOGNEWMERGE_H
