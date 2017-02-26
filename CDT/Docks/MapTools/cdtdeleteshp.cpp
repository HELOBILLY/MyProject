#include "cdtdeleteshp.h"
#include "stable.h"
#include "cdtimagelayer.h"
#include "mainwindow.h"
#include "cdtcategorytoolbar.h"
#include "cdtfilesystem.h"

#include <QMessageBox>
#include <QList>
#include <qgis.h>

qgis_devMapToolIdentifyAction::qgis_devMapToolIdentifyAction(QgsMapCanvas *canvas)
    : QgsMapToolIdentify(canvas)
{

}

qgis_devMapToolIdentifyAction::~qgis_devMapToolIdentifyAction()
{

}

void qgis_devMapToolIdentifyAction::canvasReleaseEvent(QMouseEvent *e)
{
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer*>(canvas());
    IdentifyMode mode = QgsMapToolIdentify::LayerSelection;
    QList<IdentifyResult> results = QgsMapToolIdentify::identify( e->x(), e->y(), mode );

    if ( results.isEmpty() )
    {
        QMessageBox::warning(NULL, "warning", "No features at this position found.",
                             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
    }
    else
    {
        IdentifyResult feature = results.at( 0 );
        QString title = feature.mLayer->name();
        QString content = feature.mFeature.attribute(0).toString();
        QgsGeometry *geom = feature.mFeature.geometry();
        //scal or pan

        mHighlight = new QgsHighlight(canvas(),geom,layer);
        mHighlight->setColor(QColor(255,0,0,255));
        mHighlight->setBuffer(0.5);
        mHighlight->show();

        QMessageBox::information(NULL,title,content,
                                 QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
        emit deleteHighlight();
    }
}

void qgis_devMapToolIdentifyAction::deleteHighlight()
{
    if ( mHighlight )
    {
        mHighlight->hide();
        delete mHighlight;
    }

}

