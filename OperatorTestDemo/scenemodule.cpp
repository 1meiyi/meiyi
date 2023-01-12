#include "scenemodule.h"
#include "editcontrol.h"
#include "itemgroup.h"

#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QDebug>

SceneModule::SceneModule(EditControl* editControl_, QObject *parent):
    QGraphicsScene(parent),
  m_editControl(editControl_)
{

}


void SceneModule::CrossBorderRevise(const QRect& rect_)
{
    if(rect_.right() > width())
        setSceneRect(0, 0, rect_.right() + 100, height());


    if(rect_.bottom() > height())
        setSceneRect(0, 0, width(), rect_.bottom() + 100);
}

QPoint SceneModule::GetMusePoint() const
{
    return m_mousePoint;
}


void SceneModule::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    auto config = m_editControl->Config();
    painter->fillRect(0, 0, width(), height(), config.sceneBackColor);
}

void SceneModule::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    painter->save();

    const auto& tempLinkLine = m_editControl->GetTempLinkLine();
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter->setPen(pen);

    for(auto first = tempLinkLine.line.begin(), end = tempLinkLine.line.end(); first != end; ++first){
        painter->setPen(first->isFullLine ? Qt::SolidLine : Qt::DotLine);
        painter->drawLine(first->sPoint, first->ePoint);
    }

    painter->restore();
}

void SceneModule::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);

    auto editState = m_editControl->GetEditState();
    auto& tempLinkLine = m_editControl->GetTempLinkLine();

    //Ctrl已经按下 或 进入了端口引脚中 或 没有开始链接
    if(editState & CtrlPressed || editState & EnterPortPin || !(editState & StartLink))
        return;

    //添加了填充线状态,在连线时鼠标当前指引线和引脚不是同轴方向，需要增加一条线来填充
    if(editState & AddFillLine){
        m_editControl->UpdateEditState(AddFillLine, true);
        tempLinkLine.line.pop_back();
    }

    auto rFirst = tempLinkLine.line.rbegin();
    auto rNext = rFirst + 1;
    rFirst->isFullLine = false;
    rNext->isFullLine = false;

    if(rFirst->type == HLINE){
        rFirst->ePoint = event->scenePos().toPoint();
        rFirst->sPoint.setY(rFirst->ePoint.y());
        rNext->ePoint.setY(rFirst->sPoint.y());
    }
    else if(rFirst->type == ZLINE){
        rFirst->ePoint = event->scenePos().toPoint();
        rFirst->sPoint.setX(rFirst->ePoint.x());
        rNext->ePoint.setX(rFirst->sPoint.x());
    }

    m_editControl->UpdateEditState(SamePointDrawLine, true);
    update();
}

void SceneModule::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePoint = event->scenePos().toPoint();
    QGraphicsScene::mousePressEvent(event);
    
    if(event->button() == Qt::RightButton){
        m_editControl->ClearTempLinkLine();
        return update();
    }

    auto editState = m_editControl->GetEditState();
    //在Ctrl没有按下 且 上次图元没有按下 左键按下
    if(!(editState & CtrlPressed) && !(editState & PressedDrewItem) && event->button() == Qt::LeftButton)
        m_editControl->m_pItemGroup->ClearItem();//清空多选

    //Ctrl已经按下 或 进入了端口引脚中 或 没有开始链接 或 没有左键按下
    if(editState & CtrlPressed || editState & EnterPortPin || editState & PortPinOutNotEdit ||
        !(editState & StartLink) || event->button() != Qt::LeftButton)
        return;

    auto& tempLinkLine = m_editControl->GetTempLinkLine();
    if(event->scenePos().toPoint() == tempLinkLine.line.last().sPoint){//新建节点和上次为同一点
        m_editControl->UpdateEditState(SamePointDrawLine);
        return;
    }

    tempLinkLine.line[tempLinkLine.line.size() - 2].isFullLine = true;
    tempLinkLine.line << StLineSegment("", "", tempLinkLine.line.last().ePoint, tempLinkLine.line.last().ePoint,
                                          tempLinkLine.line.last().type == HLINE ? ZLINE : HLINE);

    update();
}

void SceneModule::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    if (event->button() == Qt::RightButton && !(m_editControl->GetEditState() & PressedDrewItem))
        m_editControl->PopContextMenu(DrawItem_Nullify);
}

void SceneModule::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    m_editControl->ClearTempLinkLine();
    m_editControl->UpdateEditState(DragAddEquation);//拖拽添加等式状态
}

void SceneModule::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    m_editControl->UpdateEditState(DragAddEquation, true);//清楚 拖拽添加等式状态
}

void SceneModule::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    //m_editControl->AddTestEquation(event->scenePos().toPoint());
    //m_editControl->UpdateEditState(DragAddEquation);//清楚 拖拽添加等式状态
}

void SceneModule::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    static int number;
    QString operatorName = QString("Operator") + QString::number(++number);
    m_editControl->AddTestEquation(operatorName, event->scenePos().toPoint());
    m_editControl->UpdateEditState(DragAddEquation, true);//清楚 拖拽添加等式状态

    QMessageBox::warning(nullptr, "dropEvent", "dropEvent");
}










