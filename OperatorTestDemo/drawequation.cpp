#include "drawequation.h"
#include "editcontrol.h"
#include "drawline.h"
#include "scenemodule.h"
#include "itemgroup.h"

#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QDebug>

DrawEquation::DrawEquation(const StEquationOperator& equationOperator_):
    BasicsDrawItem(equationOperator_.type, equationOperator_.id),
    m_equationOperator(equationOperator_)
{
    InitDrawEquation();//初始化
    setAcceptHoverEvents(true);
    isLeftEquationPressed = false;
    isCreateLine = false;
}

void DrawEquation::SetPortPinInUse(const int index_, const bool inUse_, const LinkPortPinType type_)
{
    for(auto& link : m_equationOperator.portPins)
        if(link.index == index_ && link.type & type_)
            link.inUse = inUse_;
}

int DrawEquation::GetPinIndex(const BasicsDrawItem*const item_) const
{
    for(int i = 0; i < 2; ++i)
        for(auto link : i ? m_inputLinks : m_outputLinks)
            if(link.item == item_)
                return link.pinIndex;

    return  -1;
}

QRectF DrawEquation::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

void DrawEquation::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->save();

   
    auto& config = m_editControl->Config();
    const int pPinSize = config.portPinSize;//端口引脚正方形范围边长
    painter->setRenderHints(QPainter::Antialiasing);
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter->setPen(pen);

    for(auto& pin : m_equationOperator.portPins){
        painter->setPen(pin.type & LPP_In ? Qt::black : config.outPortPinColor);

        if(pin.type & LPP_Left || pin.type & LPP_Right)
            painter->drawLine(pin.rect.x(), pin.rect.y() + pPinSize / 2, pin.rect.right(), pin.rect.y() + pPinSize / 2);
        else if(pin.type & LPP_Top || pin.type & LPP_Buttom)
            painter->drawLine(pin.rect.x() + pPinSize / 2, pin.rect.y(), pin.rect.x() + pPinSize / 2, pin.rect.bottom());
    }

    painter->setPen(Qt::black);
    painter->drawRect(m_drawRect);
    m_editControl->GetSvgRenderer(m_equationOperator.name)->render(painter, m_drawRect);

    //画文本
    //QFont font("Microsoft YaHei");//微软雅黑
    //font.setBold(true);
    //font.setPixelSize(11);
    //painter->setPen(config.nameColor);
    //painter->setFont(font);
    //QFontMetrics fontMetrics(font);
    //QRect nFontRect = fontMetrics.boundingRect(m_equationOperator.name);
    //painter->drawText(QRect(pPinSize + m_drawRect.width() / 2 - nFontRect.width() / 2,
    //                        m_drawRect.height() / 2 + nFontRect.height() / 5,
    //                        m_drawRect.width(), m_drawRect.height()), 0, m_equationOperator.name);

    for(int i = 0; IsSelect() && i < 8; ++i)
        painter->fillRect(m_selectBoxs[i].rect, Qt::black);

    painter->restore();
}

QPoint DrawEquation::GetPortPinCoord(LinkPortPinType type_, const int index_) const
{
    QPoint point;
    const int portPinSize = m_editControl->Config().portPinSize;
    type_ = (LinkPortPinType)(type_ & (LPP_In | LPP_Out));//只保留输入输出类型

    for(auto& pin : m_equationOperator.portPins)
        if(pin.type & type_ && pin.index == index_){
            if(pin.type & LPP_Left)
                point = QPoint(pin.rect.x(), pin.rect.y() + portPinSize / 2);
            else if(pin.type & LPP_Top)
                point = QPoint(pin.rect.x() + portPinSize / 2, pin.rect.y());
            else if(pin.type & LPP_Right)
                point = QPoint(pin.rect.right(), pin.rect.y() + portPinSize / 2);
            else if(pin.type & LPP_Buttom)
                point = QPoint(pin.rect.x() + portPinSize / 2, pin.rect.bottom());

            point += pos().toPoint();
            break;
        }

    return point;
}

StEquationOperator DrawEquation::GetEquationOperator() const
{
    StEquationOperator equationOperator = m_equationOperator;
    equationOperator.width = m_width;
    equationOperator.height = m_height;
    equationOperator.coord = pos().toPoint();

    return equationOperator;
}

DrawEquation* DrawEquation::CreateDrawEquation(const StEquationOperator &equationOperator_, StDrawItemGroupData* drawItemGroupData_)
{
    auto& basicsItems = m_editControl->GetBasicsItems();
    if(basicsItems.contains(equationOperator_.id))
        return nullptr;//已经存在不用添加

    if(drawItemGroupData_)
        drawItemGroupData_->equations << equationOperator_;//记录插入成功数据

    DrawEquation* drawEquation = new DrawEquation(equationOperator_);
    m_editControl->m_pSceneModule->addItem(drawEquation);
    basicsItems.insert(equationOperator_.id, drawEquation);

    return drawEquation;
}

void DrawEquation::RemoveEquation(const int id_, StDrawItemGroupData *drawItemGroupData_)
{
    auto& basicsItems = m_editControl->GetBasicsItems();
    if(!basicsItems.contains(id_))
        return;//移除图元不存在

    auto item = basicsItems[id_];
    QSet<int> removeLineIDs;
    for(int i = 0; i < 2; ++i){
        auto& links = item->GetDrawItemLink(i ? LPP_In : LPP_Out);
        for(auto& link : links)
            removeLineIDs << static_cast<DrawLine*>(link.item)->GetLineID();
    }

    for(int lineID : removeLineIDs)
        DrawLine::RemoveLine(lineID, drawItemGroupData_);

    if(drawItemGroupData_)
        drawItemGroupData_->equations << static_cast<DrawEquation*>(item)->GetEquationOperator();

    m_editControl->m_pSceneModule->removeItem(item);//场景移除
    m_editControl->m_pItemGroup->GetSelectItems().remove(item);//选中集合移除
    basicsItems.remove(item->GetID());//基础数据中移除
    delete item;//释放内存
}

void DrawEquation::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    BasicsDrawItem::mousePressEvent(event);
    const auto editState = m_editControl->GetEditState();

    if(IsSelect()){
        //框选放大图元
        if(editState & Box_State){
            m_formerRect = QRect(x(), y(), m_width, m_height);
            m_terminusPoint = QPoint(x() + m_width, y() + m_height);
            m_originalPoint = pos().toPoint();
            isLeftEquationPressed = true;
        }
    }

    //选中时不进行链接处理
    if(IsSelect() || editState & CtrlPressed)
        return;

    //多选/拖动/滚轮下不处理
    if(editState & CtrlPressed || event->button() != Qt::LeftButton)
        return BasicsDrawItem::mousePressEvent(event);

    auto portPin = IsPortPinPressed(event->pos().toPoint());
    if(portPin){//端口引脚被按下
        StLinkLine& tempLinkLine = m_editControl->GetTempLinkLine();
        if(editState & StartLink){//开始已经被按下
            if(portPin->type & LPP_In  && !portPin->inUse){//按下了输入端口引脚 且 输入端口引脚没有被使用
                tempLinkLine.inputItemID = m_id;
                tempLinkLine.inputIndex = portPin->index;
                m_editControl->CreateTempLinkLine();
                isCreateLine = true;//标识此次鼠标按下事件创建了线
            }
        }
        else if(portPin->type & LPP_Out){//开始没有按下 且为输出端口
            QPoint pinCoord = GetPortPinCoord(portPin->type, portPin->index);
            tempLinkLine.outputItemID = m_id;
            tempLinkLine.outputIndex = portPin->index;
            tempLinkLine.line << StLineSegment("", "", pinCoord, pinCoord, portPin->type & (LPP_Left | LPP_Right) ? HLINE : ZLINE);//与之同轴方向
            tempLinkLine.line << StLineSegment("", "", pinCoord, pinCoord, portPin->type & (LPP_Left | LPP_Right) ? ZLINE : HLINE);//与之不r同轴方向
            m_editControl->UpdateEditState(StartLink);//开始链接状态
            m_editControl->m_pItemGroup->ClearItem();//清楚所有选中
        }
    }
}

void DrawEquation::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //创建了线
    if(isCreateLine)
        return;

    BasicsDrawItem::mouseMoveEvent(event);
    const auto editState = m_editControl->GetEditState();

    if(isLeftEquationPressed){
        QPoint scenePont = event->scenePos().toPoint();
        int dEM = m_editControl->Config().drawEquationMinimum;//等式图元最小大小
        int sDS = m_editControl->Config().selectDiamondSize;//选框上小方块大小

        //在调整大小的过程中防止越界
        if(m_selectBox.isOriginalEnabledX)
            m_originalPoint.setX(m_terminusPoint.x() - scenePont.x() + sDS < dEM ? m_terminusPoint.x() - dEM : scenePont.x() - sDS);

        if(m_selectBox.isOriginalEnabledY)
            m_originalPoint.setY(m_terminusPoint.y() - scenePont.y() + sDS < dEM ? m_terminusPoint.y() - dEM : scenePont.y() - sDS);


        if(m_selectBox.isTerminusEnabledX)
            m_terminusPoint.setX(scenePont.x() - m_originalPoint.x() + sDS < dEM ? m_originalPoint.x() + dEM : scenePont.x() + sDS);

        if(m_selectBox.isTerminusEnabledY)
            m_terminusPoint.setY(scenePont.y() - m_originalPoint.y() + sDS < dEM ? m_originalPoint.y() + dEM : scenePont.y() + sDS);

        SetEquationRect(QRect(m_originalPoint, m_terminusPoint));
    }

    QPoint point = property("point").toPoint();
    qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>";
    for (auto& pin : m_equationOperator.portPins)
        qDebug() << point + pin.property("point").toPoint();
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<";

    //选中时不进行链接处理
    if(IsSelect() || editState & CtrlPressed)
        return;
}

void DrawEquation::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //创建了线
    if(isCreateLine){
        isCreateLine = false;
        return;
    }

    if(isLeftEquationPressed && event->button() == Qt::LeftButton){
        m_editControl->UpdateEditState(Box_State, true);//清除框选放大状态
        isLeftEquationPressed = false;

        //和其它非线和注释图元重叠
       if(IsAllowOverlap()){
           StEquationZoom* equationZoom = new StEquationZoom;
           equationZoom->id = m_id;
           equationZoom->frontRect = m_formerRect;
           equationZoom->nextRect = QRect(x(), y(), m_width, m_height);
           m_editControl->PushOperation(equationZoom, m_editControl->m_throwOperationFunction);
       }
       else//不能重叠
           SetEquationRect(m_formerRect);

       return;
    }

    BasicsDrawItem::mouseReleaseEvent(event);
}


void DrawEquation::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    BasicsDrawItem::hoverMoveEvent(event);
    const auto editState = m_editControl->GetEditState();
    //选中时不进行链接处理

    if(IsSelect()){//框选放大图元
        bool isEnter = false;
            for(int i = 0; i < 8; ++i)
                if(m_selectBoxs[i].rect.contains(event->pos().toPoint())){
                    m_selectBox = m_selectBoxs[i];
                    m_editControl->UpdateEditState(m_selectBox.state);//更新框选放大状态
                    isEnter = true;
                    break;
                }

        if(!isEnter)
            m_editControl->UpdateEditState(Box_State, true);//清除框选放大状态
    }

    if(IsSelect() || editState & CtrlPressed)
        return;


    const StPortPin* portPin = IsPortPinPressed(event->pos().toPoint());
    if(portPin){//在端口引脚中
        //之前没有进入端口引脚中，此次为初次进入。且已经开始链接状态了 且进入了输入端口引脚
        if(!(editState & EnterPortPin) && portPin->type & LPP_In && editState & StartLink){
            StLinkLine& tempLinkLine = m_editControl->GetTempLinkLine();
            auto mouseCurrentLine = tempLinkLine.line.last();//鼠标当前线
            QPoint portPinCoord = GetPortPinCoord(portPin->type, portPin->index);//得到端口引脚坐标

            //鼠标当前线和端口引脚在同轴方向
            if((mouseCurrentLine.type & HLINE && (portPin->type & LPP_Left|| portPin->type & LPP_Right)) ||
               (mouseCurrentLine.type & ZLINE && (portPin->type & LPP_Top|| portPin->type & LPP_Buttom))){
                auto rFirst = tempLinkLine.line.rbegin();
                auto rNext = rFirst + 1;
                rFirst->isFullLine = true;//变实线
                rNext->isFullLine = true;//变实线

                if(rFirst->type == HLINE){
                    rFirst->ePoint = portPinCoord;
                    rFirst->sPoint.setY(rFirst->ePoint.y());
                    rNext->ePoint.setY(rFirst->sPoint.y());
                }
                else if(rFirst->type == ZLINE){
                    rFirst->ePoint = portPinCoord;
                    rFirst->sPoint.setX(rFirst->ePoint.x());
                    rNext->ePoint.setX(rFirst->sPoint.x());
                }

            }
            else{//鼠标当前线和端口引脚在不同轴方向
                auto& sPoint = tempLinkLine.line[tempLinkLine.line.size() - 2];
                QPoint midPoint1, midPoint2;
                if(sPoint.type == HLINE){
                    midPoint1.setX((sPoint.sPoint.x() + portPinCoord.x()) / 2);
                    midPoint1.setY(sPoint.sPoint.y());
                    midPoint2.setX(midPoint1.x());
                    midPoint2.setY(portPinCoord.y());
                }
                else{
                    midPoint1.setX(sPoint.sPoint.x());
                    midPoint1.setY((sPoint.sPoint.y() + portPinCoord.y()) / 2);
                    midPoint2.setX(portPinCoord.x());
                    midPoint2.setY(midPoint1.y());
                }

                sPoint.ePoint = midPoint1;
                tempLinkLine.line.last() = StLineSegment("", "", midPoint1, midPoint2, sPoint.type == HLINE ? ZLINE : HLINE, 0, 0, true);
                tempLinkLine.line << StLineSegment("", "", midPoint2, portPinCoord, sPoint.type, 0, 0, true);
                sPoint.isFullLine = true;//变实线
                m_editControl->UpdateEditState(AddFillLine);//添加了填充线状态,在连线时鼠标当前指引线和引脚不是同轴方向，需要增加一条线来填充
            }

            m_editControl->m_pSceneModule->update();
        }

        m_editControl->UpdateEditState(PortPinOutNotEdit, !(portPin->type & LPP_Out && editState & StartLink));//更新输出引脚不能编辑状态

        if((portPin->type & LPP_Out && !(editState & StartLink)) || //进入输出引脚且没有开始链接
           (portPin->type & LPP_In && editState & StartLink && !portPin->inUse)){//进入输出引脚且已经开始链接 且输入端口引脚没有被使用
            m_editControl->UpdateEditState(EnterPortPin);//进入端口引脚状态
            return;
            }
    }

    m_editControl->UpdateEditState(PortPinOutNotEdit, !portPin);//退出输出端口引脚状态
    m_editControl->UpdateEditState(EnterPortPin, true);//进入端口引脚状态清空
}

void DrawEquation::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    //不是放大编辑状态
    if(!isLeftEquationPressed)
        m_editControl->UpdateEditState(Box_State, true);//清除框选放大状态

    m_editControl->UpdateEditState(PortPinOutNotEdit, true);//退出输出端口引脚状态
    m_editControl->UpdateEditState(EnterPortPin, true);//进入端口引脚状态清空
}


//初始化
void DrawEquation::InitDrawEquation()
{
    SetEquationRect(QRect(m_equationOperator.coord.x(), m_equationOperator.coord.y(),
                          m_equationOperator.width, m_equationOperator.height));
}

StPortPin *DrawEquation::IsPortPinPressed(const QPoint coord_)
{
    for(auto& portPin : m_equationOperator.portPins)
        if(portPin.rect.contains(coord_))
            return &portPin;

    return nullptr;
}

QString DrawEquation::GetName() const
{
    return m_equationOperator.name;
}

void DrawEquation::SetEquationRect(const QRect &rect_)
{
    //图元大小
    setPos(rect_.topLeft());
    m_width = rect_.width();
    m_height = rect_.height();

    //绘图大小
    const int pinSize = m_editControl->Config().portPinSize;
    m_drawRect = QRect(pinSize, pinSize, m_width - pinSize * 2, m_height - pinSize * 2);

    ResetPortPinRect();//重置端口引脚矩形

    ResetSelectBOXRect();//重置选择框点矩形

    ResetLinkLineCoord();//重置连线位置

    update();
    m_editControl->m_pSceneModule->update();
}

void DrawEquation::ResetPortPinRect()
{
    //记录同方向引脚
     const int pinSize = m_editControl->Config().portPinSize, offset = pinSize / 2;
     QMultiMap<int, LinkPortPinType> directionPins[4];
     for(auto& pin : m_equationOperator.portPins){
         int vecIndex = 0;
         if(pin.type & LPP_Left)
             vecIndex = 0;
         else if(pin.type & LPP_Top)
             vecIndex = 1;
         else if(pin.type & LPP_Right)
             vecIndex = 2;
         else if(pin.type & LPP_Buttom)
             vecIndex = 3;

         directionPins[vecIndex].insert(pin.index, pin.type);
     }

     //四个方向分别计算
     for(int i = 0; i < 4; ++i){
         if(directionPins[i].isEmpty())
             continue;

         double intervalRatio = 1.0 / (directionPins[i].size() + 1);
         int j = 1;
         QRect rect;

         for(auto first = directionPins[i].begin(); first != directionPins[i].end(); ++first, ++j){
             if(i == 0)//左
                 rect = QRect(0, m_height * j * intervalRatio - offset, pinSize, pinSize);
             else if(i == 1)//上
                 rect = QRect(m_width * j * intervalRatio - offset, 0, pinSize, pinSize);
             else if(i == 2)//右
                 rect = QRect(m_width - pinSize, m_height * j * intervalRatio - offset, pinSize, pinSize);
             else if(i == 3)//下
                 rect = QRect(m_width * j * intervalRatio - offset, m_height - pinSize, pinSize, pinSize);

             for(auto& pin : m_equationOperator.portPins)
                 if(pin.index == first.key() & pin.type == *first)
                     pin.rect = rect;
         }
     }
}

void DrawEquation::ResetSelectBOXRect()
{
    const int sDS = m_editControl->Config().selectDiamondSize;

    //上左
     m_selectBoxs[0].rect = QRect(m_drawRect.x() - sDS, m_drawRect.y() - sDS, sDS, sDS);
     m_selectBoxs[0].state = BOX_Corner24;
     m_selectBoxs[0].isOriginalEnabledX = true;
     m_selectBoxs[0].isOriginalEnabledY = true;

    //上中
     m_selectBoxs[1].rect = QRect(m_width / 2 - sDS / 2, m_drawRect.y() - sDS, sDS, sDS);
     m_selectBoxs[1].state = Box_Lengthways;
     m_selectBoxs[1].isOriginalEnabledY = true;

    //上右
     m_selectBoxs[2].rect = QRect(m_drawRect.right() + 1, m_drawRect.y() - sDS, sDS, sDS);
     m_selectBoxs[2].state = BOX_Corner13;
     m_selectBoxs[2].isOriginalEnabledY = true;
     m_selectBoxs[2].isTerminusEnabledX = true;

    //中左
     m_selectBoxs[3].rect = QRect(m_drawRect.x() - sDS, m_height / 2 - sDS / 2, sDS, sDS);
     m_selectBoxs[3].state = Box_Crosswise;
     m_selectBoxs[3].isOriginalEnabledX = true;

    //中右
     m_selectBoxs[4].rect = QRect(m_drawRect.right() + 1, m_height / 2 - sDS / 2, sDS, sDS);
     m_selectBoxs[4].state = Box_Crosswise;
     m_selectBoxs[4].isTerminusEnabledX = true;

    //下左
     m_selectBoxs[5].rect = QRect(m_drawRect.x() - sDS, m_drawRect.bottom() + 1, sDS, sDS);
     m_selectBoxs[5].state = BOX_Corner13;
     m_selectBoxs[5].isOriginalEnabledX = true;
     m_selectBoxs[5].isTerminusEnabledY = true;

    //下中
     m_selectBoxs[6].rect = QRect(m_width / 2 - sDS / 2, m_drawRect.bottom() + 1, sDS, sDS);
     m_selectBoxs[6].state = Box_Lengthways;
     m_selectBoxs[6].isTerminusEnabledY = true;

    //下右
     m_selectBoxs[7].rect = QRect(m_drawRect.right() + 1, m_drawRect.bottom() + 1, sDS, sDS);
     m_selectBoxs[7].state = BOX_Corner24;
     m_selectBoxs[7].isTerminusEnabledX = true;
     m_selectBoxs[7].isTerminusEnabledY = true;
}

void DrawEquation::ResetLinkLineCoord()
{
    ++ItemGroup::m_moveNumber;//没有移动都有一个唯一移动标识
    for(int i = 0; i < 2; ++i)
        for(auto link : i ? m_inputLinks : m_outputLinks){
            QPoint portPinCoord = GetPortPinCoord(i ? LPP_In : LPP_Out, link.pinIndex);
            QPoint targetPos = link.item->pos().toPoint();

            if(i == 1)//输入端口
                targetPos = static_cast<DrawLine*>(link.item)->GetLineSegment().ePoint;

            link.item->PassiveMovementAdjustment(portPinCoord - targetPos, i ? LPP_Out : LPP_In);
        }
}





