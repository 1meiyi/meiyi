#include "itemgroup.h"

#include "editcontrol.h"
#include "scenemodule.h"
#include "basicsdrawitem.h"
#include "drawline.h"

int ItemGroup::m_moveNumber;    //移动编号，标识每次移动唯一

ItemGroup::ItemGroup(EditControl* editControl_):
    m_editControl(editControl_)
{

}


void ItemGroup::AddItem(BasicsDrawItem* item_)
{
    if(item_ == nullptr || m_items.contains(item_))
        return;

    m_items.insert(item_);
    item_->AddGroup(this);
    item_->update();
    m_rect = GetRectF(m_items).toRect();//重置矩形大小
    SelectLine();//更新线选择
}

void ItemGroup::AddItem(QList<BasicsDrawItem*> items_)
{
    for(auto item : items_)
        if(!m_items.contains(item)){
            m_items.insert(item);
            item->AddGroup(this);
            item->update();
        }

    m_rect = GetRectF(m_items).toRect();//重置矩形大小
    SelectLine();//更新线选择
}

void ItemGroup::RemoveItem(BasicsDrawItem* item_)
{
    if(item_ == nullptr || !m_items.contains(item_))
        return;

    m_items.remove(item_);
    item_->RemoveGroup();
    item_->update();

    m_rect = GetRectF(m_items).toRect();//重置矩形大小
    SelectLine();//更新线选择
}

void ItemGroup::AllItem()
{
    for(auto item : m_editControl->GetBasicsItems())
        if(!m_items.contains(item)){
            m_items.insert(item);
            item->AddGroup(this);
            item->update();
        }

    m_rect = GetRectF(m_items).toRect();//重置矩形大小
    SelectLine();//更新线选择
}

void ItemGroup::ClearItem()
{
    for(auto item : m_items){
        item->RemoveGroup();
        item->update();
    }

    m_items.clear();
    m_rect = GetRectF(m_items).toRect();//重置矩形大小
    SelectLine();//更新线选择
}

void ItemGroup::MoveOffsetDispose(QPoint offset_)
{
    //防止左上越界
    const QPoint pos = m_rect.topLeft();
    if(pos.x() + offset_.x() < 0)
        offset_.setX(offset_.x() - (pos.x() + offset_.x()));

    if(pos.y() + offset_.y() < 0)
        offset_.setY(offset_.y() - (pos.y() + offset_.y()));

    if(!offset_.x() && !offset_.y())
        return;//没有偏移量了

    for(auto item : m_items)
        item->SetMoveImmediately(true);

    MobileJudgmentType isMobile = MJ_XYEnabled;//初始化X/Y都可以移动

    for(auto item : m_items){
        if(item->IsTraversal() || item->type() < DrawItemAnnotation)//
            continue;//已经遍历过,除了线的图元横轴和纵轴都可以移动

        isMobile = (MobileJudgmentType)(isMobile & item->GetMobileJudgment());

        if(isMobile == MJ_Nullify)
            break;//没有移动量了
    }

    if(!(isMobile & MJ_XEnabled))
        offset_.setX(0);

    if(!(isMobile & MJ_YEnabled))
        offset_.setY(0);

    if(!offset_.x() && !offset_.y()){//没有偏移量了
        for(auto item : m_items)
            item->SetMoveImmediately(false);

        return;
    }

    GroupOffset(offset_);//集合偏移

    for(auto item : m_items)
        item->SetMoveImmediately(false);

    OutInflectionPointAdjustment();//拐点调整

    m_offsetPoint += offset_;
    m_rect.moveTo(pos + offset_);
    m_editControl->m_pSceneModule->update();
}

void ItemGroup::StartMove()//记录开始移动
{
    m_offsetPoint = QPoint(0, 0);
}

void ItemGroup::EndMove()//完成移动调整
{
    if(!m_offsetPoint.x() && !m_offsetPoint.y())
        return;//没有移动

    for(auto item : m_items)
        if(!item->IsAllowOverlap()){
            RevocationMove(-m_offsetPoint);
            m_rect.moveTo(m_rect.topLeft() - m_offsetPoint);
            m_editControl->m_pSceneModule->update();
            return;//有重叠已经撤销移动
        }

    //加入撤销
    StItemGroupMoveData* itemGroupMoveData = new StItemGroupMoveData;
    QList<int> itemIDs;
    for(auto& item : m_items)
        itemGroupMoveData->itemGroupIDs << item->GetID();

    itemGroupMoveData->frontOffset = -m_offsetPoint;
    itemGroupMoveData->nextOffset = m_offsetPoint;
    m_editControl->PushOperation(itemGroupMoveData, m_editControl->m_throwOperationFunction);
    m_offsetPoint = QPoint(0, 0);
    m_editControl->m_pSceneModule->CrossBorderRevise(m_rect);
}

void ItemGroup::RevocationMove(const QPoint offsetPoint_)//撤销移动
{
    for(auto first : m_items)
        first->SetMoveImmediately(true);//此次移动集合中的item

    GroupOffset(offsetPoint_);//集合偏移

    for(auto first : m_items)
        first->SetMoveImmediately(false);

    OutInflectionPointAdjustment();//拐点调整
}

QRectF ItemGroup::GetRectF(const QSet<BasicsDrawItem*>& items_) const
{
    QRect rect(0, 0, 0, 0);
    auto first = items_.begin();
    auto end = items_.end();

    //添加第一个矩形
    if(first != end){
        rect = (*first)->boundingRect().toRect();
        rect.moveTo((*first)->pos().toPoint());
    }

    //如果后续还有矩形，那么遍历比较，把越界的新矩形边界设置为新边界
    for(; first != end; ++first){
        auto boundingRect = (*first)->boundingRect();
        boundingRect.moveTo((*first)->pos().toPoint());

        if(boundingRect.x() < rect.x())
            rect.setX(boundingRect.x());

        if(boundingRect.y() < rect.y())
            rect.setY(boundingRect.y());

        if(boundingRect.right() > rect.right())
            rect.setRight(boundingRect.right());

        if(boundingRect.bottom() > rect.bottom())
            rect.setBottom(boundingRect.bottom());
    }

    return rect;
}

QSet<BasicsDrawItem *>& ItemGroup::GetSelectItems()
{
    return m_items;
}

void ItemGroup::SelectLine()
{
    QSet<int> selectLine;
    for(auto item : m_items)
        if(item->type() > DrawItemAnnotation){//线
            int lineID = static_cast<DrawLine*>(item)->GetLineID();
                selectLine.insert(lineID);//在同一条线的线段有相同线ID
        }

    //添加新选则的线
    for(auto lineID : selectLine)
        if(!m_lines.contains(lineID)){
            DrawLine::UpdateLineSelect(lineID, true);
            m_lines.insert(lineID);
        }


    //清除已经没有选择的线
    for(auto first = m_lines.begin(); first != m_lines.end();){
        if(!selectLine.contains(*first)){
            DrawLine::UpdateLineSelect(*first, false);
            first = m_lines.erase(first);

            if(first == m_lines.end())
                break;

           continue;
        }

        ++first;
    }

    m_editControl->m_pSceneModule->update();
}

void ItemGroup::OutInflectionPointAdjustment()//在多选移动集合中输出节点调整拐点
{
    QSet<BasicsDrawItem*> adjustmentItems;//需要调整的项
    for(auto item : m_items){
        auto adjustmentTarget = item;
        if(adjustmentTarget->type() < DrawItemAnnotation){//操作符等式
            adjustmentItems.insert(adjustmentTarget);

            for(auto& link : item->GetDrawItemLink(LPP_In)){
                adjustmentTarget = link.item;
                for(; adjustmentTarget->type() > DrawItemAnnotation;
                    adjustmentTarget = adjustmentTarget->GetDrawItemLinkFirst(LPP_In));

                adjustmentItems.insert(adjustmentTarget);
            }
        }
        else if(adjustmentTarget->type() > DrawItemAnnotation){//线
            for(; adjustmentTarget->type()  > DrawItemAnnotation;
                adjustmentTarget = adjustmentTarget->GetDrawItemLinkFirst(LPP_In));

            adjustmentItems.insert(adjustmentTarget);
        }
    }

    for(auto targetItem : adjustmentItems)
        targetItem->OutInflectionPointAdjustment();
}

void ItemGroup::GroupOffset(const QPoint offset_)
{
     ++m_moveNumber;
    for(auto first : m_items)
        first->MobileAdjustment(offset_);
}



















