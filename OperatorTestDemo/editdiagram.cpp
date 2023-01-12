#include "editdiagram.h"
#include "ui_editdiagram.h"

#include "editcontrol.h"

#include <QPushButton>
#include <QPainter>
#include <QStandardPaths>

EditDiagram::EditDiagram(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EditDiagram)
{
    ui->setupUi(this);

    m_editControl = new EditControl;
    ui->viewLaoyt->addWidget(m_editControl->GetViewPWindget());

    connect(m_editControl, &EditControl::SignalErrorThrow, [=](const QString& error_){
       ui->PoPError->append(error_);
    });

    connect(ui->removeItem, &QPushButton::clicked, m_editControl, &EditControl::RemoveCurrentSelectGroup);//移除测试图元
    connect(ui->front, &QPushButton::clicked, m_editControl, &EditControl::FrontStep);//移除测试图元
    connect(ui->next, &QPushButton::clicked, m_editControl, &EditControl::NextStep);//移除测试图元
    connect(ui->dynamicTest, &QPushButton::clicked, m_editControl, &EditControl::DynamicTesting);//移除测试图元

    connect(ui->printscreen, &QPushButton::clicked, [=](){
        //截图保存
        auto imageRect =  m_editControl->PrintscreenRectF();
        int brandHeight = 200;
        QImage image(imageRect.width(), imageRect.height() + brandHeight, QImage::Format_ARGB32);
        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing);

        //截图
        m_editControl->Printscreen(&painter, QRectF(0, 0, image.width(), image.height()), imageRect);

        //附加绘制
        painter.fillRect(0, imageRect.height(), imageRect.width(), brandHeight, QColor(50, 50, 50));
        QFont font;
        font.setBold(true);
        font.setPixelSize(brandHeight / 4);
        painter.drawText(0, imageRect.height() + brandHeight * 0.8, "Hello World");
        QString desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        image.save(desktop_path + "/Image.png", "PNG");
    });


}

EditDiagram::~EditDiagram()
{
    delete ui;
}

