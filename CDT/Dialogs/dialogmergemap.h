#ifndef DIALOGMERGEMAP_H
#define DIALOGMERGEMAP_H

#include <QDialog>
class CDTFileSystem;

namespace Ui {
class DialogMergeMap;
}

class DialogMergeMap : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMergeMap(const QString shpfilePathA,
                            const QString shpfilePathB,
                            CDTFileSystem* fileSys,
                            QWidget *parent = 0);
    ~DialogMergeMap();
    QString name()          const;
    QColor  color()         const;
    QColor  borderColor()   const;
    QString fileID()        const;
private slots:
    void InputShpfile();
    void onAccepted();

private:
    QString ShpPath;
    QString _shpfilePathA;
    QString _shpfilePathB;
    CDTFileSystem *fileSystem;

    QString shapefileTempPath;
    QString shapefileID;
private:
    Ui::DialogMergeMap *ui;
};
#endif // DIALOGMERGEMAP_H
