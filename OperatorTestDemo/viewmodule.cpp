#include "viewmodule.h"
#include "editcontrol.h"
#include "itemgroup.h"


#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>

ViewModule::ViewModule(EditControl* editControl_):
    m_editControl(editControl_)
{
    setMouseTracking(true);
    setAcceptDrops(true);

    m_rubberBand = new QWidget(this);
    m_rubberBand->hide();
    m_isLeftPress = false;
    m_isDirectionPress = false;
    m_rubberBand->setStyleSheet(QString("border:2px dotted %1;")
        .arg(m_editControl->Config().BOXSelectColor.name()));
}

void ViewModule::keyPressEvent(QKeyEvent *event)
{
    if(!event->isAutoRepeat() && 
        event->key() == Qt::Key_Control && 
        !(m_editControl->GetEditState() & (SpacePressed | StartLink))){
        m_editControl->UpdateEditState(CtrlPressed);
    }
    else if (!event->isAutoRepeat() && event->key() == Qt::Key_Space &&
        !(m_editControl->GetEditState() & (CtrlPressed | StartLink)) &&
        !m_isLeftPress) {
        m_editControl->UpdateEditState(SpacePressed);//空格键按下状态
        setDragMode(QGraphicsView::ScrollHandDrag);//可以拖动场景
    }
    else if(!event->isAutoRepeat() && event->key() == Qt::Key_Delete){
        m_editControl->RemoveCurrentSelectGroup();
    }
    else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_Right || event->key() == Qt::Key_Down) {
        QPoint offset(0, 0);

        if (event->key() == Qt::Key_Left)
            offset.rx() -= 1;

        if (event->key() == Qt::Key_Up)
            offset.ry() -= 1;

        if (event->key() == Qt::Key_Right)
            offset.rx() += 1;

        if (event->key() == Qt::Key_Down)
            offset.ry() += 1;

        if (!m_isDirectionPress) {
            m_isDirectionPress = true;
            m_editControl->m_pItemGroup->StartMove();
       }

        m_editControl->m_pItemGroup->MoveOffsetDispose(offset);
        return;
    }

   QGraphicsView::keyPressEvent(event);
}

void ViewModule::keyReleaseEvent(QKeyEvent *event)
{
    if(!event->isAutoRepeat() && event->key() == Qt::Key_Control){
        m_editControl->UpdateEditState(CtrlPressed, true);
    }
    else if (!event->isAutoRepeat() && event->key() == Qt::Key_Space) {
        m_editControl->UpdateEditState(SpacePressed, true);//取消空格键按下
        setDragMode(QGraphicsView::NoDrag);//取消拖动场景
    }
    else if(!event->isAutoRepeat() && event->key() == Qt::Key_A && m_editControl->GetEditState() & CtrlPressed){
        m_editControl->m_pItemGroup->AllItem();//全选
    }
    else if (!event->isAutoRepeat() && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_Right || event->key() == Qt::Key_Down)) {
        if (m_isDirectionPress) {
            m_isDirectionPress = false;
            m_editControl->m_pItemGroup->EndMove();
        }
    }

    QGraphicsView::keyReleaseEvent(event);
}

void ViewModule::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    if(!(m_editControl->GetEditState() & CtrlPressed) &&            //ctrl没有按下 多选
       !(m_editControl->GetEditState() & SpacePressed) &&           //空格没有按下 场景拖动
       !(m_editControl->GetEditState() & PressedDrewItem) &&        //没有开始链接
        event->button() == Qt::LeftButton &&                        //左键按下
       scene()->items(mapToScene(event->pos())).isEmpty()){         //按下区域没用图元

        m_original = event->pos();
        m_isLeftPress = true;
        m_rubberBand->setGeometry(m_original.x(), m_original.y(), 0, 0);
        m_rubberBand->show();
    }

    if (event->button() == Qt::LeftButton && m_editControl->GetEditState() & SpacePressed) {//开始拖动场景
        m_viewportOffset = event->pos();
    }
}

void ViewModule::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);

    if(m_isLeftPress){//规划选框
        m_rubberBand->setGeometry(QRect(m_original, event->pos()).normalized());
        m_editControl->BOXSelect(mapToScene(m_rubberBand->geometry()));
    }
}

void ViewModule::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    if(event->button() == Qt::LeftButton){
        m_rubberBand->hide();
        m_isLeftPress = false;
    }

    m_editControl->UpdateEditState(PressedDrewItem, true);//清除图元按下
}

void ViewModule::wheelEvent(QWheelEvent *event)
{
    if(!(m_editControl->GetEditState() & CtrlPressed))
        return;

    double sizeRatio = matrix().m11();
    // 获取当前鼠标相对于view的位置;
    QPointF cursorPoint = event->pos();
    // 获取当前鼠标相对于scene的位置;
    QPointF scenePos = mapToScene(QPoint(cursorPoint.x(), cursorPoint.y()));

    if(event->delta() > 0 && sizeRatio < 10)
        scale(1.2, 1.2);
    else if(event->delta() < 0 && sizeRatio > 0.1)
        scale(1 / 1.2, 1 / 1.2);

    // 将scene坐标转换为放大缩小后的坐标;
    QPointF viewPoint = matrix().map(scenePos);
    // 通过滚动条控制view放大缩小后的展示scene的位置;
    horizontalScrollBar()->setValue(int(viewPoint.x() - cursorPoint.x()));
    verticalScrollBar()->setValue(int(viewPoint.y() - cursorPoint.y()));

}
