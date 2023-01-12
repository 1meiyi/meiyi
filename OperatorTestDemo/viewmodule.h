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
    QPoint m_original;              //��ѡ������ʼ��
    QPoint m_viewportOffset;        //�ӿ�ƫ������
    EditControl* m_editControl;
    bool m_isDirectionPress;        //�����Ƿ���
    bool m_isLeftPress;
};

#endif // VIEWMODULE_H
