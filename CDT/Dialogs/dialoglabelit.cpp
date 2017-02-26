#include "dialoglabelit.h"
#include "ui_dialoglabelit.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>

DialogLabelit::DialogLabelit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLabelit)
{
    index = 0;
    ui->setupUi(this);
    setWindowTitle(tr("Export Location"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    QHBoxLayout *downLayOut = new QHBoxLayout(this);
    QComboBox *pathComboBox = new QComboBox(this);
    QPushButton *outputButton = new QPushButton(this);
    outputButton->setText(tr("&Export Path"));

    QLabel *tittleLabel = new QLabel(this);
    tittleLabel->setMaximumSize(this->width(),outputButton->height());
    tittleLabel->setFrameStyle(/*QFrame::Panel |*/ QFrame::Sunken);
    tittleLabel->setText(tr("Please Set the Export File Path:"));
    tittleLabel->setFont(QFont("Consolas", 10, QFont::Bold));
    tittleLabel->setAlignment(Qt::AlignCenter);

    downLayOut->addWidget(pathComboBox);
    downLayOut->addWidget(outputButton);
    downLayOut->setStretch(0,3);
    downLayOut->setStretch(1,1);

    mainLayout->addWidget(tittleLabel);
    mainLayout->addLayout(downLayOut);

    mainLayout->setStretch(0,1);
    mainLayout->setStretch(1,10);

    setLayout(mainLayout);
    connect(outputButton,SIGNAL(clicked()),this,SLOT(outPutPath()));
}

DialogLabelit::~DialogLabelit()
{
    delete ui;
}

QString DialogLabelit::name()
{
    return Path;
}

void DialogLabelit::setPostFix(QString &post)
{
    postFix = post;
    index++;
}

void DialogLabelit::outPutPath()
{
    if(index ==0)
    {
        Path = QFileDialog::getSaveFileName(NULL,tr("Export"),QString(), "Raster(*.tif *.png *.xpm *.jpg *.img)");
        this->accept();
    }else{
        Path = QFileDialog::getSaveFileName(NULL,tr("Export"),QString(), "Raster("+postFix+")");
        this->accept();
    }
}

