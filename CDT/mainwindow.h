#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "recentfilesupervisor.h"
#include "log4qt/logger.h"

namespace Ui {
class MainWindow;
}
class QToolButton;
class QLineEdit;
class QModelIndex;
class QTreeView;
struct QUuid;

class QgsMapCanvas;
class QgsScaleComboBox;

class CDTDockWidget;
class CDTProjectWidget;
class CDTTrainingSampleDockWidget;
class CDTValidationSampleDockWidget;
class CDTCategoryDockWidget;
class CDTExtractionDockWidget;
class CDTLayerInfoWidget;
class CDTUndoWidget;
class CDTAttributeDockWidget;
class CDTPlot2DDockWidget;
class CDTTaskDockWidget;
class CDTProjectLayer;
class DialogConsole;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
    friend class RecentFileSupervisor;
    friend class CDTProjectWidget;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();       

private:
    void initIconSize();
    void initActions();
    void initMenuBar();
    void initToolBar();
    void initStatusBar();
    void initDockWidgets();
    void initConsole();    

    void registerDocks(Qt::DockWidgetArea area, CDTDockWidget* dock);

public:
    static MainWindow                   *getMainWindow();
    static QTreeView                    *getProjectTreeView();
    static CDTTrainingSampleDockWidget  *getTrainingSampleDockWidget();
    static CDTAttributeDockWidget       *getAttributesDockWidget();
    static CDTPlot2DDockWidget          *getPlot2DDockWidget();
    static CDTExtractionDockWidget      *getExtractionDockWidget();
    static CDTUndoWidget                *getUndoWidget();
    static CDTLayerInfoWidget           *getLayerInfoWidget();
    static CDTTaskDockWidget            *getTaskDockWIdget();
    static CDTProjectWidget             *getCurrentProjectWidget();
    static QgsMapCanvas                 *getCurrentMapCanvas();

    static QUuid getCurrentProjectID();
    static CDTProjectLayer* GetCurrentProject();
    static QSize getIconSize();

signals:
    void loadSetting();
    void updateSetting();
public slots:
    void onCurrentTabChanged(int i);
    void showMouseCoordinate(const QgsPoint & p);
    void showScale( double theScale );
    void userCenter();
    void userScale();

private slots:
    void onActionNew();
    void onActionOpen();
    void onActionSave();
    void onActionSaveAll();
    void onActionSaveAs();
    void onRecentFileTriggered();

    void on_treeViewObjects_customContextMenuRequested(const QPoint &pos);
    void on_treeViewObjects_clicked(const QModelIndex &index);

    void updateTaskDock();
    void clearAllDocks();

    void on_actionTrainSVM_triggered();

    //void on_action_PreDisasterMap_triggered();

    void on_action_AfterDisasterMap_triggered();

    //void on_action_MergeMap_triggered();

    void on_actionSelectSample_triggered();

protected:
    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);
    void closeEvent(QCloseEvent *e);

private:
    Ui::MainWindow *ui;

    CDTAttributeDockWidget          *dockWidgetAttributes;
    CDTPlot2DDockWidget             *dockWidgetPlot2D;
    CDTTrainingSampleDockWidget     *dockWidgetTrainingSample;
    CDTValidationSampleDockWidget   *dockWidgetValidationSample;
    CDTCategoryDockWidget           *dockWidgetCategory;
    CDTExtractionDockWidget         *dockWidgetExtraction;
    CDTUndoWidget                   *dockWidgetUndo;
    CDTLayerInfoWidget              *dockWidgetLayerInfo;
    CDTTaskDockWidget               *dockWidgetTask;

    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSaveAll;
    QAction *actionSaveAs;
    QAction *actionConsole;

    QMenu *menuFile;
    QMenu *menuRecent;

    QLineEdit *lineEditCoord;
    QgsScaleComboBox *scaleEdit;

    QSize iconSize;

    RecentFileSupervisor *supervisor;
    int recentFileCount;
    QToolButton* recentFileToolButton;
    QStringList recentFilePaths;

    QList<CDTDockWidget*> docks;
    static MainWindow* mainWindow;
    static bool isLocked;
};

#endif // MAINWINDOW_H
