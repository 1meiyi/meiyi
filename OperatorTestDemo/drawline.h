#ifndef DRAWLINE_H
#define DRAWLINE_H

#include "basicsdrawitem.h"

#include <QGraphicsItem>


class DrawLine : public BasicsDrawItem
{
public:
    DrawLine(const StLineSegment& lineSegment_);

    //获得线段所在线ID
    int GetLineID() const;

    virtual QRectF boundingRect() const override;

    //获取画线是的矩形范围大小
    QRectF DrawLineRectF();

    //更新选择整条线，不是拖动选择，而是为了标识鼠标点击线段所在的整条线
    //id_线唯一标识, 若不存在不做任何处理, isSelect_值为真选择，否则不选择
    static void UpdateLineSelect(const int lineID_, bool isSelect_);


    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    //创建线
    //linkLine_需要创建的线链接线结构, drawItemGroupData_记录成功添加的数据
    static void CreateLine(const StLinkLine& linkLine_, StDrawItemGroupData* drawItemGroupData_ = nullptr);

    //移除线
    //lineID_需要移除的线ID, drawItemGroupData_记录成功添加的数据
    static void RemoveLine(const int lineID_, StDrawItemGroupData* drawItemGroupData_ = nullptr);

    //获取链接线结构数据
    //根据整条连线ID构造结构数据，不存在返回默认StLinkLine构造
    static StLinkLine GetLinkLine(const int lineID_);

    //获取选段结构数据信息
    StLineSegment GetLineSegment() const;

protected:


private:
    //初始化
    void InitDrawLine();

private:
    StLineSegment m_lineSegment;
    bool m_isLineSelect;            //线被选中了，点击一个线段整条线都要被选中，只有线才有效
};

#endif // DRAWLINE_H
