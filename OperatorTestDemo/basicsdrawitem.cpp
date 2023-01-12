#include "basicsdrawitem.h"
#include "editcontrol.h"
#include "scenemodule.h"
#include "drawline.h"
#include "itemgroup.h"

#include <QGraphicsSceneMouseEvent>
#include <QMap>

EditControl* BasicsDrawItem::m_editControl;     //编辑控制指针

BasicsDrawItem::BasicsDrawItem(const DrawItemType drawItemType_, const int id_, QGraphicsItem* parent_):
    m_id(id_),
    m_drawItemType(drawItemType_),
    QGraphicsItem(parent_)
{
    m_moveNumberX = 1;   //1没有有意义，只要和ItemGroup的m_moveNumberX初始值不同就行
    m_moveNumberY = 1;   //1没有有意义，只要和ItemGroup的m_moveNumberY初始值不同就行
    m_isLeftPressed = false;
    m_isMoveImmediately = false;
    m_isFlectionPoint = false;
    m_itemGroup = nullptr;
}

QRectF BasicsDrawItem::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

int BasicsDrawItem::type() const
{
    return m_drawItemType;
}

int BasicsDrawItem::GetID() const
{
    return m_id;
}

int BasicsDrawItem::GetPortPinIndex(const BasicsDrawItem * const item_) const
{
    for(int i = 0; i < 2; ++i)
        for(auto link : i ? m_inputLinks : m_outputLinks)
            if(link.item == item_)
                return link.pinIndex;

    return -1;
}

bool BasicsDrawItem::IsSelect() const
{
    return m_itemGroup;
}

bool BasicsDrawItem::IsFlectionPoint() const
{
    return m_drawItemType > DrawItemAnnotation && m_isFlectionPoint;
}

bool BasicsDrawItem::IsTraversal() const
{
    return m_isTraversal;
}

bool BasicsDrawItem::IsAllowOverlap() const
{
    return m_editControl->IsAreaAllowOverlap(QRect(x(), y(), m_width, m_height), this);
}

void BasicsDrawItem::SetEditControl(EditControl *editControl_)
{
    m_editControl = editControl_;
}

void BasicsDrawItem::AddGroup(ItemGroup* itemGroup_)
{
    m_itemGroup = itemGroup_;
}

void BasicsDrawItem::RemoveGroup()
{
    m_itemGroup = nullptr;
}


QList<StDrawItemLink>& BasicsDrawItem::GetDrawItemLink(const LinkPortPinType type_)
{
    if(type_ & LPP_In)
        return m_inputLinks;

    return m_outputLinks;
}

BasicsDrawItem *BasicsDrawItem::GetDrawItemLinkFirst(const LinkPortPinType type_)
{
    if(type_ & LPP_In)
        return m_inputLinks[0].item;

    return m_outputLinks[0].item;
}

void BasicsDrawItem::SetMoveImmediately(const bool isMove)
{
    m_isMoveImmediately = isMove;
    m_isTraversal = !isMove;
}

MobileJudgmentType BasicsDrawItem::GetMobileJudgment(const LinkPortPinType type_)
{
    m_isTraversal = true;//已经遍历

    if(m_drawItemType == DrawItemAnnotation)//注释图元
        return MJ_XYEnabled;

    //自己调用
    if(type_ == LPP_Nullify){
        if(m_drawItemType  < DrawItemAnnotation)//等式
            return MJ_XYEnabled;

        return (MobileJudgmentType)(GetDrawItemLinkFirst(LPP_In)->GetMobileJudgment(LPP_In) &
                   GetDrawItemLinkFirst(LPP_Out)->GetMobileJudgment(LPP_Out));
    }
    //别图元调用
    if(m_drawItemType  < DrawItemAnnotation)//等式
        return m_isMoveImmediately ? MJ_XYEnabled : MJ_Nullify;

    //线 有且只有一个输入一个输出
    BasicsDrawItem* item = GetDrawItemLinkFirst(type_);

    if(m_isMoveImmediately)//是此次移动集合中判断的item
        return item->GetMobileJudgment(type_);

    MobileJudgmentType isMobile = MJ_Nullify;
    if(m_drawItemType & HLINE){//横线
        isMobile = MJ_XEnabled;
        if(item->m_drawItemType > DrawItemAnnotation)
            isMobile = (MobileJudgmentType)(isMobile | MJ_YEnabled);
    }
    else{//纵线
        isMobile = MJ_YEnabled;
        if(item->m_drawItemType > DrawItemAnnotation)
            isMobile = (MobileJudgmentType)(isMobile | MJ_XEnabled);
    }

    return isMobile;
}

void BasicsDrawItem::MobileAdjustment(const QPoint offset_)
{
    setPos(pos().toPoint() + offset_);
    for(int i = 0; i < 2; ++i)
        for(auto link : i ? m_inputLinks : m_outputLinks)
            link.item->PassiveMovementAdjustment(offset_, i ? LPP_Out : LPP_In);
}

void BasicsDrawItem::PassiveMovementAdjustment(QPoint offset_, const LinkPortPinType type_)
{
    if(m_drawItemType <= DrawItemAnnotation || m_isMoveImmediately)//只有线才能被动调整, 本次已经移动了，不再调整
        return;

    if(offset_.x() && m_drawItemType & HLINE){//X有效 且此对象是横线
            int tempWidth = m_width + offset_.x() * (type_ & LPP_In ? -1 : 1) * (m_drawItemType == HLINE ? 1 : -1);//长度调整
            m_drawItemType = (DrawItemType)(tempWidth < 0 ? m_drawItemType ^ DrawItem_REV : m_drawItemType);//类型调整
            m_width = tempWidth < 0 ? -tempWidth : tempWidth;

            if(type_ & LPP_In)//输入端口调整需要改变坐标
                setX(x() + offset_.x());

            offset_.rx() = 0;//X轴方向偏移量已经抵消
            m_moveNumberX = ItemGroup::m_moveNumber;//更新本次移动编号，防止重复移动
    }

    if(offset_.y() && m_drawItemType & ZLINE){//Y有效 且此对象是纵线
            int tempHeight = m_height + offset_.y() * (type_ & LPP_In ? -1 : 1) * (m_drawItemType == ZLINE ? 1 : -1);//长度调整
            m_drawItemType = (DrawItemType)(tempHeight < 0 ? m_drawItemType ^ DrawItem_REV : m_drawItemType);//类型调整
            m_height = tempHeight < 0 ? -tempHeight : tempHeight;

            if(type_ & LPP_In)//输入端口调整需要改变坐标
                setY(y() + offset_.y());

            offset_.ry() = 0;//Y轴方向偏移量已经抵消
            m_moveNumberY = ItemGroup::m_moveNumber;//更新本次移动编号，防止重复移动
    }

    if(m_moveNumberX != ItemGroup::m_moveNumber && offset_.x()){
        GetDrawItemLinkFirst(type_ & LPP_In ? LPP_Out : LPP_In)->PassiveMovementAdjustment(QPoint(offset_.x(), 0), type_);
        m_moveNumberX = ItemGroup::m_moveNumber;//更新本次移动编号，防止重复移动
        setX(x() + offset_.x());
    }

    if(m_moveNumberY != ItemGroup::m_moveNumber && offset_.y()){
        GetDrawItemLinkFirst(type_ & LPP_In ? LPP_Out : LPP_In)->PassiveMovementAdjustment(QPoint(0, offset_.y()), type_);
        m_moveNumberY = ItemGroup::m_moveNumber;//更新本次移动编号，防止重复移动
        setY(y() + offset_.y());
    }
}

void BasicsDrawItem::InsertLinkItem(BasicsDrawItem *item_, const LinkPortPinType type_, const int index_)
{
    (type_ & LPP_In ? m_inputLinks : m_outputLinks) << StDrawItemLink(item_, index_);
}

void BasicsDrawItem::RemoveLinkItem(const BasicsDrawItem * const item_, const LinkPortPinType type_)
{
    auto& links = type_ & LPP_In ? m_inputLinks : m_outputLinks;
    for(auto first = links.begin(), end = links.end(); first != end; ++first)
        if((*first).item == item_){
            links.erase(first);
            break;
        }
}

void BasicsDrawItem::OutInflectionPointAdjustment()
{
    if(m_drawItemType > DrawItemAnnotation || m_outputLinks.isEmpty())
        return;

    //找出同一个输出端口的线集合
    QMap<int, QList<BasicsDrawItem*>> outLines;
    for(auto& link : m_outputLinks){
        outLines[link.pinIndex] << link.item;
        //从线的头部开始，把所有有拐点的线取消拐点
        for(auto item = link.item; item->m_drawItemType > DrawItemAnnotation;item = item->GetDrawItemLinkFirst(LPP_Out))
            if(item->m_isFlectionPoint){
                item->m_isFlectionPoint = false;
                item->update();
            }
    }

    for(auto& lines : outLines){
        QList<BasicsDrawItem*> justLines;//正向
        QList<BasicsDrawItem*> reverseLines;//逆向
        BasicsDrawItem* startLine = nullptr;
        for(auto line : lines){
            startLine = line;
            if(line->m_drawItemType & DrawItem_REV)//逆向
                reverseLines << line;
            else//正向
                justLines << line;
        }

        if(justLines.size() > 1)//只有同方向上的线大于一才会有分叉
            flectionPointAdjustment(justLines);

        if(reverseLines.size() > 1)
            flectionPointAdjustment(reverseLines);

        if(justLines.size() && reverseLines.size()){
            startLine->m_isFlectionPoint = true;
            startLine->update();
        }
    }
}

void BasicsDrawItem::flectionPointAdjustment(QList<BasicsDrawItem*>& items_)
{
    QMap<int, QList<BasicsDrawItem*>> equilongLines;//将相同长度聚集在一起

    for(auto item : items_){
        int length = item->m_drawItemType & HLINE ? item->m_width : item->m_height;
        equilongLines[length] << item;
    }

    for(auto first = equilongLines.begin(), end = equilongLines.end(); first != end; ++first){
        QList<BasicsDrawItem*> justLines;//正向
        QList<BasicsDrawItem*> reverseLines;//逆向
        BasicsDrawItem* outLine = nullptr;
        for(auto line : *first){
            outLine = line->GetDrawItemLinkFirst(LPP_Out);
            if(outLine->m_drawItemType & DrawItem_REV)//逆向
                reverseLines << outLine;
            else//正向
                justLines << outLine;
        }

        if(justLines.size() > 1)//只有同方向上的线大于一才递归
            flectionPointAdjustment(justLines);

        if(reverseLines.size() > 1)
            flectionPointAdjustment(reverseLines);

        if(first + 1 != end || (justLines.size() && reverseLines.size())){
            outLine->m_isFlectionPoint = true;
            outLine->update();
        }
    }
}

void BasicsDrawItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    auto editState = m_editControl->GetEditState();
    if(editState & StartLink)
        return;

    if(event->button() == Qt::LeftButton){
        m_isLeftPressed = true;
        m_isMoved = false;
        m_mouseFromItemPoint = event->pos();

        if(m_itemGroup)
            m_itemGroup->StartMove();

        m_editControl->UpdateEditState(PressedDrewItem);
    }
    else if (event->button() == Qt::RightButton) {
        if (!m_itemGroup) {//没有选中
            if (!(m_editControl->GetEditState() & CtrlPressed))
                m_editControl->m_pItemGroup->ClearItem();//不是多选状态清楚以前的

            m_editControl->m_pItemGroup->AddItem(this);
            m_itemGroup->StartMove();
        }

        m_editControl->UpdateEditState(PressedDrewItem);
    }
}

void BasicsDrawItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    auto editState = m_editControl->GetEditState();
    if(editState & StartLink)
        return;

    if(m_isLeftPressed){
        if(!m_itemGroup){//没有选中
            if(!(m_editControl->GetEditState() & CtrlPressed))
                m_editControl->m_pItemGroup->ClearItem();//不是多选状态清楚以前的

            m_editControl->m_pItemGroup->AddItem(this);
            m_itemGroup->StartMove();
        }

        //在对象集合中 且 不处于框选放大状态
        if(m_itemGroup && !(editState & Box_State))
            m_itemGroup->MoveOffsetDispose((event->scenePos() - m_mouseFromItemPoint - pos()).toPoint());

        m_isMoved = true;
    }
}

void BasicsDrawItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    auto editState = m_editControl->GetEditState();
    if(editState & StartLink)
        return;

    if(event->button() == Qt::LeftButton){
        if(!m_isMoved){

            if(!(m_editControl->GetEditState() & CtrlPressed))
                m_editControl->m_pItemGroup->ClearItem();//不是多选状态清楚以前的

            m_editControl->m_pItemGroup->AddItem(this);
        }

        if(m_itemGroup)
            m_itemGroup->EndMove();

        m_isLeftPressed = false;
    }

    if (event->button() == Qt::RightButton)
        m_editControl->PopContextMenu(m_drawItemType);
}

QPoint BasicsDrawItem::GetPoint() const
{
    return pos().toPoint();
}


























