#ifndef SCENEMODULE_H
#define SCENEMODULE_H

#include "editcontroldefine.h"

#include <QGraphicsScene>

class EditControl;

class SceneModule : public QGraphicsScene
{
public:
    SceneModule(EditControl* editControl_, QObject *parent = nullptr);

    //越界矫正
    void CrossBorderRevise(const QRect& rect_);

    //获取最近一次记录的鼠标点
    QPoint GetMusePoint() const;

protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
    virtual void drawForeground(QPainter *painter, const QRectF &rect) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event) override;


private:
    EditControl* m_editControl; //编辑控制指针
    QPoint m_mousePoint;        //记录最小点击的鼠标点
};



#endif // SCENEMODULE_H
