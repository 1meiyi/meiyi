#ifndef DRAWEQUATION_H
#define DRAWEQUATION_H

#include "editcontroldefine.h"
#include "basicsdrawitem.h"

class QGraphicsSvgItem;


class DrawEquation : public BasicsDrawItem
{
public:
    DrawEquation(const StEquationOperator& equationOperator_);

    //设置端口引脚是否被使用
    //index_端口引脚下标, inUse_真为使用否则未使用, type 端口类型
    void SetPortPinInUse(const int index_, const bool inUse_, const LinkPortPinType type_ = LPP_In);

    //根据输入项获取在此对象中的引脚下标，没有返回-1
    virtual int GetPinIndex(const BasicsDrawItem*const item_) const;

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    //获取端口引脚坐标
    //type_ 输入输出类型, index_端口引脚编号
    QPoint GetPortPinCoord(LinkPortPinType type_, const int index_) const;

    //获取名称
    QString GetName() const;

    //设置位置大小
    //rect_矩形信息
    void SetEquationRect(const QRect& rect_);

    //等式操作符相结构
    StEquationOperator GetEquationOperator() const;

    //创建添加等式图元
    //equationOperator_等式操作符项结构, drawItemGroupData_记录已经删除的数据
    static DrawEquation* CreateDrawEquation(const StEquationOperator& equationOperator_, StDrawItemGroupData* drawItemGroupData_ = nullptr);

    //移除图元
    //id_图元ID，不存在不处理。drawItemGroupData_记录已经删除的数据
    static void RemoveEquation(const int id_, StDrawItemGroupData* drawItemGroupData_ = nullptr);


protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    //初始化
    void InitDrawEquation();

    //判断是否按下引脚
    //coord_当前坐标， 如果是返回引脚指针否则返回空
    StPortPin* IsPortPinPressed(const QPoint coord_);

    //重置端口引脚矩形
    void ResetPortPinRect();

    //重置选择框点矩形
    void ResetSelectBOXRect();

    //重置连线位置
    void ResetLinkLineCoord();

private:
    StEquationOperator m_equationOperator;
    StEquationSelectBOX m_selectBoxs[8];              //框点矩形集合, 这里为8个
    StEquationSelectBOX m_selectBox;                  //记录上次选择的框
    QRect m_drawRect;                                 //绘图区域大小
    QRect m_formerRect;                               //之前位置矩形大小
    QPoint m_originalPoint;                           //原始点，这里为等式矩形左上角点，场景坐标
    QPoint m_terminusPoint;                           //终点，这里为等式矩形左上角点，场景坐标
    bool isLeftEquationPressed;                       //左键按下
    bool isCreateLine;                                //这次鼠标按下事件创建了线， 不向基类传递事件
};

#endif // DRAWEQUATION_H
