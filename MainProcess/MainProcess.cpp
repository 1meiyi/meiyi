#include "MainProcess.h"
#include "ui_MainProcess.h"
#include <QWindow>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include "globalmessagedefinition.h"

MainProcess::MainProcess(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainProcess)
    ,m_process(nullptr)
{
    ui->setupUi(this);
    m_chidMainWindow = nullptr;

    connect(ui->close, &QPushButton::clicked, [=](){
        if(m_process)
            Clole();
        else
            Open();
    });
}

MainProcess::~MainProcess()
{
    delete ui;
}

void MainProcess::Open()
{
    if(m_process == nullptr){
        QString cmd = "OperatorTestDemo";
        QStringList argList;
        argList << QString::number(this->winId());
        m_process = new QProcess(this);
        connect(m_process, &QProcess::readyReadStandardError, this, &MainProcess::slot_createWaitingBar);
        m_process->start(cmd, argList);
    }
}

void MainProcess::Clole()
{
    if(m_chidMainWindow){
        ui->mainLayout->removeWidget(m_chidMainWindow);
        delete m_chidMainWindow;
        m_chidMainWindow = nullptr;
    }

    if(m_process){
        m_process->close();
        delete m_process;
        m_process = nullptr;
    }
}

void MainProcess::slot_createWaitingBar()
{
    GlobalMessageDefinition::StWindowsEmbedded windowsEmbedded;
    QByteArray buffer = m_process->readAllStandardError();


    if(!GlobalMessageDefinition::JsonToStruct(windowsEmbedded, buffer))
        return;

    auto first = windowsEmbedded.windowGroup.begin();

    QWindow *childWin = QWindow::fromWinId(first.key());
       if(childWin){
           m_chidMainWindow = QWidget::createWindowContainer(childWin);
           ui->mainLayout->addWidget(m_chidMainWindow);
       }

}

