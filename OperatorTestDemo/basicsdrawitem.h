/*****************************************************************
**基础绘图类
**1统一基类管理，移动/连线调整， 选择
**
**
**
** Author：laichao
** Date：2022-03-28
******************************************************************/

#ifndef BASICSDRAWITEM_H
#define BASICSDRAWITEM_H

#include "editcontroldefine.h"
#include <QGraphicsItem>

class EditControl;
class ItemGroup;
class SceneModule;
class QGraphicsSceneMouseEvent;

class BasicsDrawItem : public QObject, public QGraphicsItem
{
    Q_PROPERTY(QPoint point READ GetPoint)
    Q_OBJECT
public:
    BasicsDrawItem(const DrawItemType drawItemType_, const int id_, QGraphicsItem* parent_ = nullptr);

    virtual QRectF boundingRect() const override;

    //得到该对象类型
    virtual int type() const override;

    //获取唯一标识ID
    int GetID() const;

    //获取端口引脚下标编号
    //item_链接的对象， 没有返回-1
    int GetPortPinIndex(const BasicsDrawItem*const item_) const;

    //是否被选中
    bool IsSelect() const;

    //是否为拐点线
    bool IsFlectionPoint() const;

    //是否已经在判断移动中遍历
    bool IsTraversal() const;

    //判断当前等式是否在此位置合理，是否可以重叠，线可以重叠
    bool IsAllowOverlap() const;

    //添加编辑控制指针
    static void SetEditControl(EditControl* editControl_);

    //添加入集合选中
    //itemGroup_ 集合指针
    void AddGroup(ItemGroup* itemGroup_);

    //移除选中集合
    void RemoveGroup();

    //得到绘图项链接
    //type_ 判断是输出还是输出
    QList<StDrawItemLink>& GetDrawItemLink(const LinkPortPinType type_);

    //得到绘图项链接第一个, 不判断容器是否为空
    //type_ 判断是输出还是输出
    BasicsDrawItem* GetDrawItemLinkFirst(const LinkPortPinType type_);

    //设置和取消立马移动状态/同时初始化遍历标识
    void SetMoveImmediately(const bool isMove);

    //[1] 判断能否可以移动
    //type_调用方式, LPP_Nullify自己调
    MobileJudgmentType GetMobileJudgment(const LinkPortPinType type_ = LPP_Nullify);

    //[2] 先移动，再告诉与之有关联的图元他们自己对应调整自己。
    void MobileAdjustment(const QPoint offset_);

    //[3] 被动移动调整, 只有线才可以被动调整
    //offset_偏移量, type_调用者是输入方还是输出方
    void PassiveMovementAdjustment(QPoint offset_, const LinkPortPinType type_);

    //插入链接项SceneModule* m_sceneModule;     //场景模块
    //item_需要链接的对象, type_链接类型, index_链接的引脚下标
    void InsertLinkItem(BasicsDrawItem* item_, const LinkPortPinType type_, const int index_);

    //移除链接项
    //item_需要移除的对象, type_链接类型
    void RemoveLinkItem(const BasicsDrawItem*const item_, const LinkPortPinType type_);


    //输出线拐点重叠调整
    void OutInflectionPointAdjustment();

    //拐点调整
    void flectionPointAdjustment(QList<BasicsDrawItem*>& items_);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPoint GetPoint() const;

protected:
    QList<StDrawItemLink> m_inputLinks;    //绘图项链接
    QList<StDrawItemLink> m_outputLinks;   //绘图项链接
    static EditControl* m_editControl;     //编辑控制指针
    ItemGroup* m_itemGroup;                //图元集合指针
    int m_width;                           //图元宽度
    int m_height;                          //图元高度
    int m_id;                              //图元唯一编号

private:
    int m_moveNumberX;              //移动编号，没有移动都有一个编号
    int m_moveNumberY;              //移动编号，没有移动都有一个编号
    DrawItemType m_drawItemType;    //绘图项类型
    QPointF m_mouseFromItemPoint;   //鼠标点击图元，相对于图元坐标
    bool m_isLeftPressed;           //左键已经按下
    bool m_isMoved;                 //已经移动
    bool m_isMoveImmediately;       //现在立马移动的图元
    bool m_isTraversal;             //设置是否已经遍历
    bool m_isFlectionPoint;         //拐点表示，只有线才有用
};

#endif // BASICSDRAWITEM_H
