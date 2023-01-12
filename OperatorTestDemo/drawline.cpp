#include "drawline.h"

#include "editcontrol.h"
#include "scenemodule.h"
#include "itemgroup.h"
#include "drawequation.h"

#include <QPainter>
#include <QDebug>

DrawLine::DrawLine(const StLineSegment& lineSegment_):
    BasicsDrawItem(lineSegment_.type, lineSegment_.id),
    m_lineSegment(lineSegment_)
{
    //初始化
    InitDrawLine();
    m_isLineSelect = false;
}

int DrawLine::GetLineID() const
{
    return m_lineSegment.lineID;
}


QRectF DrawLine::boundingRect() const
{
    //不再边框范围类，不会及时刷新，而拐点已经超出边框范围，所有对应扩大边框
    int inFlexionRadius = m_editControl->Config().inFlexionRadius;          //拐点圆半径大小
    auto drawItemType = type();

    if(drawItemType == HLINE)
        return QRectF(-inFlexionRadius, -m_height / 2, m_width + inFlexionRadius, m_height);
    else if(drawItemType == HLINE_REV)
        return QRectF(-m_width - inFlexionRadius, -m_height / 2, m_width + inFlexionRadius, m_height);
    else if(drawItemType == ZLINE)
        return QRectF(-m_width / 2, -inFlexionRadius, m_width, m_height + inFlexionRadius);
    else if(drawItemType == ZLINE_REV)
        return QRectF(-m_width / 2, -m_height - inFlexionRadius, m_width, m_height + inFlexionRadius);

    return QRectF();
}

QRectF DrawLine::DrawLineRectF()
{
    auto drawItemType = type();
    if(drawItemType == HLINE)
        return QRectF(0, -m_height / 2, m_width, m_height);
    else if(drawItemType == HLINE_REV)
        return QRectF(-m_width, -m_height / 2, m_width, m_height);
    else if(drawItemType == ZLINE)
        return QRectF(-m_width / 2, 0, m_width, m_height);
    else if(drawItemType == ZLINE_REV)
        return QRectF(-m_width / 2, -m_height, m_width, m_height);

    return QRectF();
}

void DrawLine::UpdateLineSelect(const int lineID_, bool isSelect_)
{
    auto lines = m_editControl->GetLines();
    if(!lines.contains(lineID_))
        return;

    for(auto item = lines[lineID_]; item->type() > DrawItemAnnotation; item = item->GetDrawItemLinkFirst(LPP_Out)){
        static_cast<DrawLine*>(item)->m_isLineSelect = isSelect_;
        item->update();
    }
}

void DrawLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();

    int inFlexionRadius = m_editControl->Config().inFlexionRadius;          //拐点圆半径大小
    int selectDiamondSize = m_editControl->Config().selectDiamondSize;                  //选中边框上小方块大小
    auto drawItemType = type();
    painter->setRenderHints(QPainter::Antialiasing);
    QPen pen(m_isLineSelect ? Qt::darkGreen : Qt::black);
    pen.setWidth(1);
    painter->setPen(pen);

    //获取画线矩形范围
    QRectF rectF = DrawLineRectF();
    if(drawItemType == HLINE || drawItemType == HLINE_REV){
        painter->drawLine(rectF.x(), 0, rectF.right(), 0);
    }
    else if(drawItemType == ZLINE || drawItemType == ZLINE_REV){
        painter->drawLine(0, rectF.y(), 0, rectF.bottom());
    }

    //画拐点
    painter->setPen(Qt::NoPen);
    if(IsFlectionPoint()){
        painter->setBrush(Qt::black);
        if(drawItemType == HLINE)
            painter->drawEllipse(rectF.x() - inFlexionRadius, -inFlexionRadius, inFlexionRadius * 2, inFlexionRadius * 2);
        else if(drawItemType == HLINE_REV)
            painter->drawEllipse(rectF.right() - inFlexionRadius, -inFlexionRadius, inFlexionRadius * 2, inFlexionRadius * 2);
        else if(drawItemType == ZLINE)
            painter->drawEllipse(-inFlexionRadius, rectF.y() - inFlexionRadius, inFlexionRadius * 2, inFlexionRadius * 2);
        else if(drawItemType == ZLINE_REV)
            painter->drawEllipse(-inFlexionRadius, rectF.bottom() - inFlexionRadius, inFlexionRadius * 2, inFlexionRadius * 2);
    }

    //选中状态
    if(IsSelect()){
        if(drawItemType == HLINE || drawItemType == HLINE_REV){
            painter->fillRect(rectF.x() - 1, -selectDiamondSize / 2, selectDiamondSize, selectDiamondSize, Qt::black);
            painter->fillRect(rectF.right() - selectDiamondSize + 1, -selectDiamondSize / 2, selectDiamondSize, selectDiamondSize, Qt::black);
        }
        else if(drawItemType == ZLINE || drawItemType == ZLINE_REV){
            painter->fillRect(-selectDiamondSize / 2, rectF.y() - 1, selectDiamondSize, selectDiamondSize, Qt::black);
            painter->fillRect(-selectDiamondSize / 2, rectF.bottom() - selectDiamondSize + 1, selectDiamondSize, selectDiamondSize, Qt::black);
        }
    }

    painter->restore();
}

void DrawLine::CreateLine(const StLinkLine &linkLine_, StDrawItemGroupData *drawItemGroupData_)
{
    auto& basicsItems = m_editControl->GetBasicsItems();
    auto& lines = m_editControl->GetLines();

    if(lines.contains(linkLine_.lineID) || linkLine_.line.size() < 2 ||
        !basicsItems.contains(linkLine_.outputItemID) || !basicsItems.contains(linkLine_.inputItemID))
        return;//线ID已经存在, 线数量不够， 需要链接的等式不存在

    //记录插入成功的线数据
    if(drawItemGroupData_)
        drawItemGroupData_->lines << linkLine_;

    //获得输入输出等式
    auto outputItem = static_cast<DrawEquation*>(basicsItems[linkLine_.outputItemID]);
    auto inputItem = static_cast<DrawEquation*>(basicsItems[linkLine_.inputItemID]);

    //链接输出等式
    auto first = linkLine_.line.begin();
    BasicsDrawItem* startLine = new DrawLine(*first);
    BasicsDrawItem* endLine;

    lines.insert(linkLine_.lineID, startLine);
    basicsItems.insert(startLine->GetID(), startLine);
    m_editControl->m_pSceneModule->addItem(startLine);
    outputItem->InsertLinkItem(startLine, LPP_Out, linkLine_.outputIndex);
    startLine->InsertLinkItem(outputItem, LPP_In, 0);

    //逐个链接线
    ++first;
    for(auto last = linkLine_.line.end(); first != last; ++first){
        endLine = new DrawLine(*first);
        startLine->InsertLinkItem(endLine, LPP_Out, 0);
        endLine->InsertLinkItem(startLine, LPP_In, 0);
        startLine = endLine;
        basicsItems.insert(startLine->GetID(), startLine);
        m_editControl->m_pSceneModule->addItem(startLine);
    }

    //链接输入等式
    inputItem->InsertLinkItem(startLine, LPP_In, linkLine_.inputIndex);
    startLine->InsertLinkItem(inputItem, LPP_Out, 0);
    inputItem->SetPortPinInUse(linkLine_.inputIndex, true, LPP_In);//设置输入端口引脚被使用
    outputItem->OutInflectionPointAdjustment();
}

void DrawLine::RemoveLine(const int lineID_, StDrawItemGroupData *drawItemGroupData_)
{
    auto& basicsItems = m_editControl->GetBasicsItems();
    auto& lines = m_editControl->GetLines();

    if(!lines.contains(lineID_))
        return;//删除线不存在

    if(drawItemGroupData_)
        drawItemGroupData_->lines << GetLinkLine(lineID_);

    BasicsDrawItem* lineSegment = lines[lineID_];//第一条线段
    BasicsDrawItem* outputItem = lineSegment->GetDrawItemLinkFirst(LPP_In);//输入等式
    outputItem->RemoveLinkItem(lineSegment, LPP_Out);//和输入等式断开链接
    lines.remove(lineID_);//从线集合移除

    //移除线
    auto& selectItems = m_editControl->m_pItemGroup->GetSelectItems();//选中项集合
    BasicsDrawItem* tempLine;
    while(true){
        m_editControl->m_pSceneModule->removeItem(lineSegment);//从场景移除
        selectItems.remove(lineSegment);//选中集合移除
        basicsItems.remove(lineSegment->GetID());//从基础数据移除
        tempLine = lineSegment->GetDrawItemLinkFirst(LPP_Out);//临时存放下一个图元
        delete lineSegment;//野指针不可访问地址，但可以作为值比较

        if(tempLine->type() < DrawItemAnnotation){//是等式图元
            //移除输出端
            auto inputItem = static_cast<DrawEquation*>(tempLine);
            inputItem->SetPortPinInUse(inputItem->GetPinIndex(lineSegment), false);//设置输入端口已经没有使用
            inputItem->RemoveLinkItem(lineSegment, LPP_In);//移除输入链接
            break;
        }

        lineSegment = tempLine;
    }
}

 StLinkLine DrawLine::GetLinkLine(const int lineID_)
 {
     StLinkLine lnkLine;
     auto lines = m_editControl->GetLines();
     if(!lines.contains(lineID_))
         return lnkLine;

     BasicsDrawItem* outputItem = lines[lineID_]->GetDrawItemLinkFirst(LPP_In);
     BasicsDrawItem* lineSegment = lines[lineID_];
     lnkLine.outputItemID = outputItem->GetID();               //输出图元id,每个图元唯一编号
     lnkLine.outputIndex = outputItem->GetPortPinIndex(lineSegment);                //输出下标编号，为端口引脚编号

     //遍历线,逐个创建线段
     for(; lineSegment->type() > DrawItemAnnotation; lineSegment = lineSegment->GetDrawItemLinkFirst(LPP_Out))
         lnkLine.line << static_cast<DrawLine*>(lineSegment)->GetLineSegment();      //一条有线段组成的线

     BasicsDrawItem* inputItem = lineSegment;
     lineSegment = lineSegment->GetDrawItemLinkFirst(LPP_In);
     lnkLine.inputItemID = inputItem->GetID();                //输入图元id,每个图元唯一编号
     lnkLine.inputIndex = inputItem->GetPortPinIndex(lineSegment);       //输入下标编号, 为端口引脚编号
     lnkLine.lineID = lineID_;                     //连线唯一编号

     return lnkLine;
 }

 StLineSegment DrawLine::GetLineSegment() const
 {
     StLineSegment lineSegment = m_lineSegment;
     lineSegment.sPoint = pos().toPoint();
     lineSegment.type = (DrawItemType)type();

     if(lineSegment.type == HLINE)
         lineSegment.ePoint = QPoint(x() + m_width, y());
     else if(lineSegment.type == HLINE_REV)
         lineSegment.ePoint = QPoint(x() - m_width, y());
     else if(lineSegment.type == ZLINE)
         lineSegment.ePoint = QPoint(x(), y() + m_height);
     else if(lineSegment.type == ZLINE_REV)
         lineSegment.ePoint = QPoint(x(), y() - m_height);

     return lineSegment;
 }

void DrawLine::InitDrawLine()
{
    //构建大小位置
    if(m_lineSegment.type & HLINE){
        m_width = qAbs(m_lineSegment.sPoint.x() - m_lineSegment.ePoint.x());
        m_height = m_editControl->Config().lineSelectWidth;
    }
    else if(m_lineSegment.type & ZLINE){
        m_width = m_editControl->Config().lineSelectWidth;
        m_height = qAbs(m_lineSegment.sPoint.y() - m_lineSegment.ePoint.y());
    }

    setPos(m_lineSegment.sPoint);
}
