#ifndef VIEWMODULE_H
#define VIEWMODULE_H

#include <QGraphicsView>


class EditControl;

class ViewModule : public QGraphicsView
{
public:
    ViewModule(EditControl* editControl_);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    QWidget* m_rubberBand;
    QPoint m_original;              //框选矩形起始点
    QPoint m_viewportOffset;        //视口偏移坐标
    EditControl* m_editControl;
    bool m_isDirectionPress;        //方向是否按下
    bool m_isLeftPress;
};

#endif // VIEWMODULE_H
