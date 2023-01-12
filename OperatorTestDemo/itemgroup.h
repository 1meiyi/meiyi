#ifndef ITEMGROUP_H
#define ITEMGROUP_H

#include "editcontroldefine.h"

#include <QSet>

class EditControl;

class ItemGroup
{
public:
    ItemGroup(EditControl* editControl_);

    //添加一个选择
    //item_需要添加的项
    void AddItem(BasicsDrawItem* item_);

    //添加一个集合选择
    //items_需要添加的项集合
    void AddItem(QList<BasicsDrawItem*> items_);

    //移除选择
    //item_需要移除的项
    void RemoveItem(BasicsDrawItem* item_);

    //从基础数据中全部选择
    void AllItem();

    //清空选择
    void ClearItem();

    //选中集合偏移处理
    //offset_偏移量
    void MoveOffsetDispose(QPoint offset_);

    //记录开始移动
    void StartMove();

    //完成移动调整
    void EndMove();

    //撤销移动
    void RevocationMove(const QPoint offsetPoint_);

    //获得图元集合矩形位置大小
    //items_图元集合
    QRectF GetRectF(const QSet<BasicsDrawItem*>& items_) const;

    //获取选中集合引用
    QSet<BasicsDrawItem*>& GetSelectItems();


private:
    //更新选择的线
    void SelectLine();

    //在多选移动集合中输出节点调整拐点
    void OutInflectionPointAdjustment();

    //集合偏移
    //offset_偏移量
    void GroupOffset(const QPoint offset_);


public:
    static int m_moveNumber;    //移动编号，标识每次移动唯一

private:
    QSet<BasicsDrawItem*> m_items;  //选中的图元集合
    QSet<int> m_lines;              //已经选中的线
    QRect m_rect;                   //集合矩形位置大小
    QPoint m_offsetPoint;           //偏移点
    EditControl* m_editControl;     //编辑控制指针
};

#endif // ITEMGROUP_H
