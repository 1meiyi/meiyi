#include "editcontrol.h"
#include "viewmodule.h"
#include "scenemodule.h"
#include "itemgroup.h"
#include "basicsdrawitem.h"
#include "drawequation.h"
#include "drawline.h"
#include "drawannotation.h"
#include "revocation.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QMenu>
#include <QSvgRenderer>
#include <QCursor>
#include <QDebug>
#include <QTimer>

EditControl::EditControl():
    m_copyItemGroupData(OI_AddItemGroup)
{
    //加载配置文件
    LoadConfig();

    BasicsDrawItem::SetEditControl(this);//绘图基类获得控制指针
    m_pViewModule = new ViewModule(this);//视图模块
    m_pSceneModule = new SceneModule(this);//场景模块
    m_pItemGroup = new ItemGroup(this);//图元选择集合模块
    m_pRevocation = new Revocation(this);//操作撤销
    m_contextMenu = new QMenu; //右键上下文菜单

    m_pViewModule->setScene(m_pSceneModule);
    m_pSceneModule->setSceneRect(0, 0, 4000, 4000);

    m_editState = EditState_Nullify;//无状态
    m_throwOperationFunction = nullptr;//无回调函数
    m_basicsItemID = 0;
    m_lineID = 0;

    
    connect(m_contextMenu, &QMenu::triggered, this, &EditControl::OnMenuTriggered);

    m_timer = new QTimer;
    connect(m_timer, &QTimer::timeout, this, &EditControl::OnDynamicTesting);
}

EditControl::~EditControl()
{
    for (auto svg : m_svgRenderers)
        delete svg;

    delete m_contextMenu;

}


//动态测试
void EditControl::DynamicTesting()
{
    if (m_timer->isActive())
        m_timer->stop();
    else
        m_timer->start(1);   
}

//动态测试
void EditControl::OnDynamicTesting()
{
//    static int name;
//    int num = qrand() % 100;
//    auto sceneRect = m_pSceneModule->sceneRect();

//    if (num < 30 && m_basicsItems.size() < 200) {//添加

//        if (num % 3 != 0) {//添加图元
//            qDebug() << "add equation start";
//            auto rect = m_pSceneModule->sceneRect().toRect();
//            qDebug() << "add equation end";
//        }
//        else {//添加连线
//            QList< BasicsDrawItem*> equations;
//            for (auto item : m_basicsItems)
//                if (item->type() < DrawItemAnnotation)
//                    equations << item;

//            StLinkLine linkLine;
//            StPortPin outPin, inPin;

//            bool isOut = false, isIn = false;

//            while (equations.size()) {
//                int index = qrand() % equations.size();
//                auto item = equations[index];
//                equations.removeAt(index);

//                if (!isOut) {
//                    linkLine.outputItemID = item->GetID();
//                    int outCount = 0;
//                    for (auto& pin : static_cast<DrawEquation*>(item)->GetEquationOperator().portPins)
//                        if (pin.type & LPP_Out)
//                            ++outCount;

//                    linkLine.outputIndex = qrand() % outCount;
//                    for (auto& pin : static_cast<DrawEquation*>(item)->GetEquationOperator().portPins)
//                        if (pin.type & LPP_Out && linkLine.outputIndex == pin.index) {
//                            DoutPin = pin;
//                            break;
//                        }

//                    isOut = true;
//                }

//                if (!isIn) {
//                    for (auto& pin : static_cast<DrawEquation*>(item)->GetEquationOperator().portPins)
//                        if (pin.type & LPP_In && !pin.inUse) {
//                            linkLine.inputItemID = item->GetID();
//                            linkLine.inputIndex = pin.index;
//                            inPin = pin;
//                            isIn = true;
//                            break;
//                        }

//                }

//                if (isOut && isIn)
//                    break;
//            }

//            if (isOut && isIn) {
//                qDebug() << "add line start";
//                StDrawItemGroupData drawItemGroupData(OI_AddItemGroup);
//                StDrawItemGroupData* revocationnewData = new StDrawItemGroupData(OI_AddItemGroup);

//                auto outItem = static_cast<DrawEquation*>(m_basicsItems[linkLine.outputItemID]);
//                auto inItem = static_cast<DrawEquation*>(m_basicsItems[linkLine.inputItemID]);
//                linkLine.lineID = GetNewID(false);
//                QRect rect = QRect(outItem->pos().toPoint(), inItem->boundingRect().bottomRight().toPoint()).normalized();

//                QPoint startCoord = outItem->GetPortPinCoord(outPin.type, outPin.index);
//                bool isHLINE = outPin.type & (LPP_Left | LPP_Right);
//                const bool tempisHLINE = isHLINE;
//                int iEnd = iEnd = (1 + qrand() % 5) * 2 + 1;
//                if (isHLINE == (bool)(inPin.type & (LPP_Left | LPP_Right)))
//                    ++iEnd;

//                for (int i = 0; i < iEnd; ++i) {
//                    auto tempCoord = startCoord;
//                    if (isHLINE)
//                        tempCoord.setX(rect.x() + qrand() % (rect.width() + 1));
//                    else
//                        tempCoord.setY(rect.y() + qrand() % (rect.height() + 1));

//                    linkLine.line << StLineSegment("", "", startCoord, tempCoord, isHLINE ? HLINE : ZLINE);
//                    startCoord = tempCoord;
//                    isHLINE = !isHLINE;
//                }

//                const QPoint endCoord = inItem->GetPortPinCoord(inPin.type, inPin.index);
//                auto& frontCoord = linkLine.line.last().ePoint;
//                if (isHLINE)
//                    frontCoord.setY(endCoord.y());
//                else
//                    frontCoord.setX(endCoord.x());

//                linkLine.line << StLineSegment("", "", frontCoord, endCoord, isHLINE ? HLINE : ZLINE);

//                //整理线段，调整方向
//                for (auto& lineSegment : linkLine.line) {
//                    if (lineSegment.type & HLINE && lineSegment.sPoint.x() > lineSegment.ePoint.x())
//                        lineSegment.type = HLINE_REV;
//                    else if (lineSegment.type & ZLINE && lineSegment.sPoint.y() > lineSegment.ePoint.y())
//                        lineSegment.type = ZLINE_REV;

//                    lineSegment.lineID = linkLine.lineID;
//                    lineSegment.id = GetNewID(true);
//                }


//                drawItemGroupData.lines << linkLine;
//                AddGroup(&drawItemGroupData, revocationnewData);
//                PushOperation(revocationnewData, m_throwOperationFunction);
//                qDebug() << "add line end";
//            }
//        }

//    }
//    else if (num < 40) {//删除
//        QList<BasicsDrawItem*> lines;
//        QList< BasicsDrawItem*> equations;
//        for (auto item : m_basicsItems)
//            if (item->type() > DrawItemAnnotation)
//                lines << item;
//            else
//                equations << item;

//        if (qrand() % 2) {//删除等式
//            if (equations.size() && false) {
//                qDebug() << "remvoe equation start";
//                m_pItemGroup->ClearItem();
//                m_pItemGroup->AddItem(equations[qrand() % equations.size()]);
//                RemoveCurrentSelectGroup();
//                qDebug() << "remvoe equation end";
//            }
//        }
//        else {//删除线
//            if (lines.size()) {
//                qDebug() << "remvoe line start";
//                m_pItemGroup->ClearItem();
//                m_pItemGroup->AddItem(lines[qrand() % lines.size()]);
//                RemoveCurrentSelectGroup();
//                qDebug() << "remvoe line end";
//            }
//        }
//    }
//    else if (num < 60) {//移动
//        QList<BasicsDrawItem*> lines;
//        QList< BasicsDrawItem*> equations;
//        for (auto item : m_basicsItems)
//            if (item->type() > DrawItemAnnotation)
//                lines << item;
//            else
//                equations << item;

//        if (qrand() % 2) {//移动等式
//            if (equations.size() && false) {
//                qDebug() << "move equation start";
//                m_pItemGroup->ClearItem();
//                m_pItemGroup->AddItem(equations[qrand() % equations.size()]);
//                m_pItemGroup->StartMove();
//                m_pItemGroup->MoveOffsetDispose(QPoint(qrand() % 10 * (qrand() % 2 ? 1 : -1), qrand() % 10 * (qrand() % 2 ? 1 : -1)));
//                m_pItemGroup->EndMove();
//                qDebug() << "move equation end";
//            }
//        }
//        else {//移动线
//            if (lines.size()) {
//                qDebug() << "move line start";
//                m_pItemGroup->ClearItem();
//                m_pItemGroup->AddItem(lines[qrand() % lines.size()]);
//                m_pItemGroup->StartMove();
//                m_pItemGroup->MoveOffsetDispose(QPoint(qrand() % 10 * (qrand() % 2 ? 1 : -1), qrand() % 10 * (qrand() % 2 ? 1 : -1)));
//                m_pItemGroup->EndMove();
//                qDebug() << "move line end";
//            }
//        }
//    }
//    else if (num < 80) {//缩放
//        QList< BasicsDrawItem*> equations;
//        for (auto item : m_basicsItems)
//            if (item->type() <= DrawItemAnnotation)
//                equations << item;

//        if (equations.size()) {

//            auto item = static_cast<DrawEquation*>(equations[qrand() % equations.size()]);
//            QRect frontRect = item->boundingRect().toRect();
//            frontRect.moveTo(item->pos().toPoint());
//            QRect nextRect;
//            nextRect.moveTo(item->pos().toPoint() + QPoint(qrand() % 10 * (qrand() % 2 ? 1 : -1), qrand() % 10 * (qrand() % 2 ? 1 : -1)));
//            nextRect.setWidth(frontRect.width() * (qrand() % 2 ? 2 : 0.5) + 1);
//            nextRect.setHeight(frontRect.height() * (qrand() % 2 ? 2 : 0.5) + 1);
//            item->SetEquationRect(nextRect);

//            if (item->IsAllowOverlap()) {
//                qDebug() << "zoom start";
//                StEquationZoom* equationZoom = new StEquationZoom;
//                equationZoom->id = item->GetID();
//                equationZoom->frontRect = frontRect;
//                equationZoom->nextRect = nextRect;
//                PushOperation(equationZoom, m_throwOperationFunction);
//                qDebug() << "zoom end";
//            }
//            else
//                item->SetEquationRect(frontRect);
//        }
//    }
//    else {//撤销
//        qDebug() << "revocation start";
//        if (qrand() % 2)
//            NextStep();
//        else
//            FrontStep();
//        qDebug() << "revocation end";
//    }

//    qDebug() << "ID: " << m_basicsItemID;


}

void EditControl::UpdateEditState(const EditState editState_, const bool clear_)
{
    auto tempState = (EditState)(m_editState | editState_);//更新状态;
    if(clear_)
        tempState = (EditState)(tempState ^ editState_);//需要清空

    if(tempState == m_editState)
        return;

    m_editState = tempState;

    if(m_editState & SpacePressed)//空格键按下, 拖动场景
        m_pViewModule->setCursor(Qt::OpenHandCursor);
    else if(m_editState & DragAddEquation)//拖拽添加等式状态
        m_pViewModule->setCursor(Qt::OpenHandCursor);
    else if(m_editState & EnterPortPin)//进入端口引脚状态
        m_pViewModule->setCursor(Qt::PointingHandCursor);
    else if(m_editState & PortPinOutNotEdit)//进入输出引脚不能编辑状态
        m_pViewModule->setCursor(Qt::ForbiddenCursor);
    else if(m_editState & SamePointDrawLine)//在同一点画线,左键点击新拐点是很上次在同一点
        m_pViewModule->setCursor(Qt::ForbiddenCursor);
    else if(m_editState & StartLink)//处于开始连接状态
        m_pViewModule->setCursor(Qt::CrossCursor);
    //框选状态
    else if(m_editState & BOX_Corner24)//框选状态，2和4象方向
        m_pViewModule->setCursor(Qt::SizeFDiagCursor);
    else if(m_editState & BOX_Corner13)//框选状态，1和3象方向
        m_pViewModule->setCursor(Qt::SizeBDiagCursor);
    else if(m_editState & Box_Crosswise)//框选状态，横向
        m_pViewModule->setCursor(Qt::SizeHorCursor);
    else if(m_editState & Box_Lengthways)//框选状态，纵向
        m_pViewModule->setCursor(Qt::SizeVerCursor);
    else //默认光标样式
        m_pViewModule->setCursor(Qt::ArrowCursor);
}

void EditControl::UpdateSvgRenderer(const QString& svgName_)
{
    if (!m_svgRenderers.contains(svgName_))
        m_svgRenderers[svgName_] = new QSvgRenderer;


    m_svgRenderers[svgName_]->load(m_config.svgDirPath + svgName_ + ".svg");
}

QSvgRenderer* EditControl::GetSvgRenderer(const QString& svgName_)
{
    if (!m_svgRenderers.contains(svgName_)) {
        m_svgRenderers[svgName_] = new QSvgRenderer;
        if (!m_svgRenderers[svgName_]->load(m_config.svgDirPath + svgName_ + ".svg"))
            m_svgRenderers[svgName_]->load(m_config.svgDirPath + "default.svg");
    }

    return  m_svgRenderers[svgName_];
}

void EditControl::AddSvg()
{
    QString fileNamePath = QFileDialog::getOpenFileName(nullptr, tr("add svg"), "", "SVG(*.svg)");
    if (!QSvgRenderer().load(fileNamePath)) {
        emit SignalErrorThrow(tr("error: add svg load"));
        return;
    }

    for(auto item : m_pItemGroup->GetSelectItems())
        if (item->type() < DrawItemAnnotation) {
            QString newSvgFileName = m_config.svgDirPath + static_cast<DrawEquation*>(item)->GetName() + ".svg";
            QFile file(fileNamePath), newFile(newSvgFileName);
            file.open(QIODevice::ReadOnly);
            newFile.open(QIODevice::WriteOnly);
            newFile.write(file.readAll());
            file.close();
            newFile.close();

            UpdateSvgRenderer(static_cast<DrawEquation*>(item)->GetName());
            m_pSceneModule->update();
            break;
        }
}

void EditControl::PopContextMenu(const DrawItemType type_)
{
    m_contextMenu->clear();

    if (type_ == DrawItem_Nullify) {//场景请求弹窗
        m_contextMenu->addAction(tr("add annotation"))->setData(CM_AddAnnotation);
        m_contextMenu->addAction(tr("add testOper"))->setData(CM_TestOper);

        if (!m_copyItemGroupData.equations.isEmpty() || !m_copyItemGroupData.annotations.isEmpty())
            m_contextMenu->addAction(tr("paste"))->setData(CM_Paste);
    }
    else if (type_ == DrawItemAnnotation) {//注释请求弹窗
        m_contextMenu->addAction(tr("nature"))->setData(CM_Nature);
        m_contextMenu->addAction(tr("copy"))->setData(CM_Copy);
    }
    else if (type_ < DrawItemAnnotation) {//等式请求弹窗
        int selectEquation = 0;
        for (auto item : m_pItemGroup->GetSelectItems())
            if (item->type() < DrawItemAnnotation)
                ++selectEquation;

        m_contextMenu->addAction(tr("copy"))->setData(CM_Copy);
        if (selectEquation > 1) {
            m_contextMenu->addAction(tr("left Align"))->setData(CM_LeftAlign);
            m_contextMenu->addAction(tr("top Align"))->setData(CM_TopAlign);
            m_contextMenu->addAction(tr("right Align"))->setData(CM_RightAlign);
            m_contextMenu->addAction(tr("buttom Align"))->setData(CM_ButtomAlign);
            m_contextMenu->addAction(tr("vertically Center Align"))->setData(CM_VerticallyCenterAlign);
            m_contextMenu->addAction(tr("crosswise Center Align"))->setData(CM_CrosswiseCenterAlign);
        }
        else {
            m_contextMenu->addAction(tr("nature"))->setData(CM_Nature);
            m_contextMenu->addAction(tr("add svg"))->setData(CM_AddSvg);
        }
    }
    else if (type_ > DrawItemAnnotation) {//线段请求弹窗
        m_contextMenu->addAction(tr("nature"))->setData(CM_Nature);
    }

    if (!m_contextMenu->isEmpty())
        m_contextMenu->popup(QCursor::pos());
}

void EditControl::SetThrowOperationFunction(ThrowOperationFunction function_)
{
    m_throwOperationFunction = function_;
}

void EditControl::ItemLayout(const ContextMenu type_)
{
    QSet<BasicsDrawItem*> selectEquations;//选中等式图元集合
    StItemGroupLayoutMove* itemGroupLayoutMove = new StItemGroupLayoutMove;
    StItemGroupLayoutMove::StItemCoord itemCoord;

    for (auto item : m_pItemGroup->GetSelectItems())
        if (item->type() < DrawItemAnnotation) {
            itemCoord.frontCoord = itemCoord.nextCoord = item->pos().toPoint();
            itemCoord.id = item->GetID();
            itemGroupLayoutMove->itemGroupLayoutMoveDatas << itemCoord;
            selectEquations << item;
        }

    if (selectEquations.size() < 2)
        return;

    QRect layoutRect = m_pItemGroup->GetRectF(selectEquations).toRect();//获取整体布局大小
    for (auto& itemCoord : itemGroupLayoutMove->itemGroupLayoutMoveDatas) {
        auto item = static_cast<DrawEquation*>(m_basicsItems[itemCoord.id]);
        auto rect = item->boundingRect();
        auto pos = item->pos();
        
        if (type_ == CM_LeftAlign)//左对齐
            pos.rx() = layoutRect.x();
        else if (type_ == CM_TopAlign)//上对齐
            pos.ry() = layoutRect.y();
        else if (type_ == CM_RightAlign)//右对齐
            pos.rx() = layoutRect.right() - rect.width();
        else if (type_ == CM_ButtomAlign)//下对齐
            pos.ry() = layoutRect.bottom() - rect.height();
        else if (type_ == CM_VerticallyCenterAlign)//纵向居中对齐
            pos.rx() = layoutRect.center().x() - rect.width() / 2;
        else if (type_ == CM_CrosswiseCenterAlign)//横向居中对齐
            pos.ry() = layoutRect.center().y() - rect.height() / 2;
        
        rect.moveTo(pos);
        bool isOverlap = false;//重叠判断标志位
        if(!IsAreaAllowOverlap(rect.toRect(), item)) {
                isOverlap = true;//和其它等式重叠
                break;
            }
            
        if (!isOverlap) {//没有重叠
            itemCoord.nextCoord = pos.toPoint();
            item->SetEquationRect(rect.toRect());
        }
    }

    PushOperation(itemGroupLayoutMove, m_throwOperationFunction);
}

EditState EditControl::GetEditState() const
{
    return m_editState;
}

void EditControl::LoadConfig()
{
    m_config.svgDirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/SVG/";  //svg矢量图目录路径
    m_config.sceneBackColor = QColor(220, 220, 220);                  //场景背景颜色
    m_config.BOXSelectColor = QColor(0, 200, 100);                  //框选颜色
    m_config.outPortPinColor = QColor(0, 100, 100);                 //输出端口颜色
    m_config.inPortPinColor = QColor(0, 0, 0);                  //输出端口颜色
    m_config.selectLineColor = QColor(200, 200, 0);                 //选中线颜色
    m_config.lineTrueColor = QColor(255, 0, 0);                   //在线监视时线类型为true颜色
    m_config.lineFalseColor = QColor(0, 255, 0);                  //在线监视时线类型为false颜色
    m_config.nameColor = QColor(50, 50, 50);                       //名称颜色
    m_config.drawEquationMinimum = 50;              //等式图元最小值
    m_config.lineSelectWidth = 8;                  //线选择宽度大小, 不是线宽度
    m_config.selectDiamondSize = 6;               //选中边框上小方块边长大小
    m_config.portPinSize = 10;                     //端口引脚正方形范围边长
    m_config.inFlexionRadius = 3;                 //拐点圆半径大小
    m_config.revocationStepCount = 1000;            //撤销步骤数量

    QDir().mkdir(m_config.svgDirPath);
}

void EditControl::AddAnnotation()
{
    StAnnotation annotation;
    annotation.id = GetNewID(true);
    annotation.coord = m_pSceneModule->GetMusePoint();

    StDrawItemGroupData drawItemGroupData(OI_AddItemGroup);
    StDrawItemGroupData* revocationnewData = new StDrawItemGroupData(OI_AddItemGroup);

    drawItemGroupData.annotations << annotation;
    *revocationnewData = drawItemGroupData;
    ExecuteOperation(revocationnewData, m_throwOperationFunction);
}

const EditControl::StConfig &EditControl::Config() const
{
    return m_config;
}

StLinkLine &EditControl::GetTempLinkLine()
{
    return m_tempLinkLine;
}

void EditControl::ClearTempLinkLine()
{
    m_tempLinkLine.line.clear();
    //清空临时链接状态
    UpdateEditState((EditState)(SamePointDrawLine | StartLink | EnterPortPin | PortPinOutNotEdit | AddFillLine), true);
    m_pSceneModule->update();
}

void EditControl::CreateTempLinkLine()
{
    StDrawItemGroupData drawItemGroupData(OI_AddItemGroup);
    StDrawItemGroupData* revocationnewData = new StDrawItemGroupData(OI_AddItemGroup);

    m_tempLinkLine.lineID = GetNewID(false);//获得线ID
    //整理线段，调整方向
    for(auto& lineSegment : m_tempLinkLine.line){
        if(lineSegment.type & HLINE && lineSegment.sPoint.x() > lineSegment.ePoint.x())
                lineSegment.type = HLINE_REV;
        else if(lineSegment.type & ZLINE && lineSegment.sPoint.y() > lineSegment.ePoint.y())
                lineSegment.type = ZLINE_REV;

        lineSegment.lineID = m_tempLinkLine.lineID;
        lineSegment.id = GetNewID(true);
    }

    drawItemGroupData.lines << m_tempLinkLine;
    *revocationnewData = drawItemGroupData;
    ExecuteOperation(revocationnewData, m_throwOperationFunction);
    ClearTempLinkLine();
}

void EditControl::BOXSelect(const QPolygonF& polygonF_)
{
    QList<BasicsDrawItem*> DrawItems_;
    for(auto item : m_pSceneModule->items(polygonF_, Qt::IntersectsItemShape))
        if(item->type() > DrawItem_Nullify && item->type() < DrawItemText)
            DrawItems_ << static_cast<BasicsDrawItem*>(item);

    m_pItemGroup->ClearItem();
    m_pItemGroup->AddItem(DrawItems_);
}

void EditControl::PushOperation(StBasicsOperateInstruct *revocation_, ThrowOperationFunction function_)
{
    if(!revocation_)
        return;

    bool isCorrect = true;

    if(revocation_->type == OI_Move){//移动指令
        auto moveData = static_cast<StItemGroupMoveData*>(revocation_);
        if(moveData->frontOffset == moveData->nextOffset || moveData->itemGroupIDs.isEmpty())//前后移动点不能相同
            isCorrect = false;
    }
    else if (revocation_->type == OI_LayoutMove) {//布局移动
        isCorrect = false;
        for(auto& item : static_cast<StItemGroupLayoutMove*>(revocation_)->itemGroupLayoutMoveDatas)
            if (item.frontCoord != item.nextCoord) {
                isCorrect = true;
                break;
            }
    }
    else if(revocation_->type == OI_EquationZoom){//缩放指令
        auto zoomData = static_cast<StEquationZoom*>(revocation_);
       if(zoomData->frontRect == zoomData->nextRect)
           isCorrect = false;//前后矩形不能相同 且 图元id必须存在于基础数据中
    }
    else if(revocation_->type == OI_AddItemGroup || revocation_->type == OI_RemoveItemGroup){//添加或删除
        auto drawItemGroupData = static_cast<StDrawItemGroupData*>(revocation_);
        if(drawItemGroupData->equations.isEmpty() && 
            drawItemGroupData->lines.isEmpty() && 
            drawItemGroupData->annotations.isEmpty())
            isCorrect = false;
    }

    if(isCorrect){//验证成功添加
        m_pRevocation->PushOperation(revocation_);

        if (function_)
            function_(*revocation_);
    }
    else{//验证失败
        emit SignalErrorThrow("PushOperation: error");
        delete revocation_;
    }
}

void EditControl::FrontStep()
{
    m_pRevocation->FrontStep();
}

void EditControl::NextStep()
{
    m_pRevocation->NextStep();
}

void EditControl::ExecuteOperation(StBasicsOperateInstruct* revocation_, ThrowOperationFunction function_)
{
    QList<BasicsDrawItem*> selectBasicsDrawItems;
    m_pItemGroup->ClearItem();

    if (revocation_->type == OI_Move) {//移动
        auto itemGroupMoveData = static_cast<StItemGroupMoveData*>(revocation_);
        for (auto& id : itemGroupMoveData->itemGroupIDs)
            selectBasicsDrawItems << m_basicsItems[id];

        m_pItemGroup->AddItem(selectBasicsDrawItems);
        m_pItemGroup->RevocationMove(itemGroupMoveData->nextOffset);//不加入撤销， 向外抛出
        PushOperation(revocation_, function_);//加入撤销但不向外抛出
    }
    else if (revocation_->type == OI_LayoutMove) {//布局移动
        auto itemGroupLayoutMove = static_cast<StItemGroupLayoutMove*>(revocation_);

        for (auto& item : itemGroupLayoutMove->itemGroupLayoutMoveDatas) {
            auto equation = static_cast<DrawEquation*>(m_basicsItems[item.id]);
            auto rect = equation->boundingRect().toRect();
            rect.moveTo(item.nextCoord);
            equation->SetEquationRect(rect);
            selectBasicsDrawItems << equation;
        }
        
        m_pItemGroup->AddItem(selectBasicsDrawItems);//全选
        PushOperation(revocation_, function_);//加入撤销但不向外抛出
    }
    else if (revocation_->type == OI_EquationZoom) {//等式缩放
        auto equationZoom = static_cast<StEquationZoom*>(revocation_);//等式图元放大数据信息

        auto equation = static_cast<DrawEquation*>(m_basicsItems[equationZoom->id]);
        equation->SetEquationRect(equationZoom->nextRect);
        m_pItemGroup->AddItem(equation);//全选
        PushOperation(revocation_, function_);//加入撤销但不向外抛出
    }
    else if (revocation_->type == OI_AddItemGroup) {//添加集合
        auto itemGroupData = static_cast<StDrawItemGroupData*>(revocation_);//对象集合绘制数据信息
        AddGroup(itemGroupData);
        for (int id : GetBasicsItemIDs(revocation_))
                selectBasicsDrawItems << m_basicsItems[id];

        m_pItemGroup->AddItem(selectBasicsDrawItems);//全选
        PushOperation(revocation_, function_);//加入撤销但不向外抛出
    }
    else if (revocation_->type == OI_RemoveItemGroup) {//移除集合
        for (int id : GetBasicsItemIDs(revocation_))
            selectBasicsDrawItems << m_basicsItems[id];

        m_pItemGroup->AddItem(selectBasicsDrawItems);//全选
        RemoveGroup();
        PushOperation(revocation_, function_);//加入撤销但不向外抛出
    }
    else if (revocation_->type == OI_RemoveCategory) {//移除同一类

    }
    
}


void EditControl::AddTestEquation(QString text_, QPoint scenePos_)//添加测试等式
{
    QFont font("Microsoft YaHei");//微软雅黑
    font.setBold(true);
    font.setPixelSize(11);
    QFontMetrics fontMetrics(font);
    QRect nFontRect = fontMetrics.boundingRect(text_);

    int tempWidth = nFontRect.width() + m_config.portPinSize * 2 + 10;
    int tempHeight = m_config.drawEquationMinimum + qrand() % 80;
    if (tempWidth < m_config.drawEquationMinimum)
        tempWidth = m_config.drawEquationMinimum;

    if (!IsAreaAllowOverlap(QRect(scenePos_.x(), scenePos_.y(), tempWidth, tempHeight))) {//等式不能重叠, 注释，线可以重叠
        emit SignalErrorThrow(tr("Can't overlap"));
        return;
    }

    StDrawItemGroupData equationOperator(OI_AddItemGroup);
    StDrawItemGroupData* revocationnewData = new StDrawItemGroupData(OI_AddItemGroup);


    //添加测试图元
    for(int i = 0, iEnd = 1; i < iEnd; ++i){
        StEquationOperator equation;

        //添加测试引脚
        LinkPortPinType direction[] = {LPP_Left, LPP_Top, LPP_Right, LPP_Buttom};
        LinkPortPinType inAndOut[] = {LPP_In, LPP_Out};
        int index[] = {0, 0};

        for (int pinCount = 3 + qrand() % 11, i = 0; i < pinCount; ++i) {
            int num = qrand() % 100;
            bool tempValue = qrand() % 2;
            equation.portPins << StPortPin(QRect(),
                (LinkPortPinType)((tempValue ? direction[2] : direction[0]) | inAndOut[tempValue]),//随机一个引脚
                index[!i || num < 10]++);
        }

        //for(int pinCount = 3 + qrand() % 11, i = 0; i < pinCount; ++i){
        //    int num = qrand() % 100;
        //    equation.portPins << StPortPin(QRect(),
        //        (LinkPortPinType)(inAndOut[!i || num < 10] | direction[qrand() % 4]),//随机一个引脚
        //            index[!i || num < 10]++);
        //}

        equation.id = GetNewID(true);//获得一个新等式ID
        equation.name = text_;
        equation.width = tempWidth;
        equation.height = tempHeight;
        equation.coord = scenePos_;
        equation.type = Operator;

        equationOperator.equations << equation;
    }

    *revocationnewData = equationOperator;
    ExecuteOperation(revocationnewData, m_throwOperationFunction);
}

void EditControl::RemoveCurrentSelectGroup()//移除测试等式
{
    if(m_pItemGroup->GetSelectItems().isEmpty())
        return;

    StDrawItemGroupData* revocationnewData = new StDrawItemGroupData(OI_RemoveItemGroup);
    RemoveGroup(revocationnewData);
    PushOperation(revocationnewData, m_throwOperationFunction);
}

QList<int> EditControl::GetBasicsItemIDs(const StBasicsOperateInstruct *revocation_)
{
    QList<int> ids;

    if(revocation_->type == OI_Move){//移动指令
        auto moveData = static_cast<const StItemGroupMoveData*>(revocation_);
        for(int id : moveData->itemGroupIDs)
           ids << id;

    }
    else if(revocation_->type == OI_EquationZoom){//缩放指令
        auto zoomData = static_cast<const StEquationZoom*>(revocation_);
        ids << zoomData->id;
    }
    else if(revocation_->type == OI_AddItemGroup || revocation_->type == OI_RemoveItemGroup){//添加和移除指令
        auto itemGroupData = static_cast<const StDrawItemGroupData*>(revocation_);

        //等式图元
        for(auto& equation : itemGroupData->equations)
               ids << equation.id;

        //线图元
        for(const  StLinkLine& line : itemGroupData->lines)
           for(const  StLineSegment& lineSegment : line.line)
             ids << lineSegment.id;

        //注释图元
        for (auto& annotation : itemGroupData->annotations)
            ids << annotation.id;
    }

    return ids;
}

//添加对象集合
//drawItemGroupData_象集合绘制数据信息
void EditControl::AddGroup(StDrawItemGroupData* addDrawItemGroupData_, StDrawItemGroupData* drawItemGroupData_)
{
    //添加图元
    for(auto& equation : addDrawItemGroupData_->equations)
        DrawEquation::CreateDrawEquation(equation, drawItemGroupData_);

    //添加线
    for(auto& linkLine : addDrawItemGroupData_->lines)
        DrawLine::CreateLine(linkLine, drawItemGroupData_);

    //添加注释
    for (auto& annotation : addDrawItemGroupData_->annotations)
        DrawAnnotation::CreateAnnotation(annotation, drawItemGroupData_);

}

//添加对象集合
//drawItemGroupData_记录已经删除的数据
void EditControl::RemoveGroup(StDrawItemGroupData* drawItemGroupData_)
{
    auto& selectItems = m_pItemGroup->GetSelectItems();
    while(selectItems.size()){
        auto iter = selectItems.begin();
        auto item = *iter;
        selectItems.erase(iter);

        if (item->type() > DrawItemAnnotation)//线
            DrawLine::RemoveLine(static_cast<DrawLine*>(item)->GetLineID(), drawItemGroupData_);//移除线
        else if (item->type() < DrawItemAnnotation)
            DrawEquation::RemoveEquation(item->GetID(), drawItemGroupData_);//移除等式
        else 
            DrawAnnotation::RemoveAnnotation(item->GetID(), drawItemGroupData_);//移除注释
    }

    m_pItemGroup->ClearItem();
}

void  EditControl::CopySelectArea()
{
    m_copyItemGroupData.equations.clear();
    m_copyItemGroupData.lines.clear();
    m_copyItemGroupData.annotations.clear();
    QSet<BasicsDrawItem*> selectItem;//选中的等式图元/注释集合
    QSet<int> selectEquationIDs;//选中的等式图元ID集合
    QSet<int> selectLineIDs;//选中的线集合

    for (auto item : m_pItemGroup->GetSelectItems())
        if (item->type() < DrawItemAnnotation) {//等式
           //拷贝等式图元
            m_copyItemGroupData.equations << static_cast<DrawEquation*>(item)->GetEquationOperator();
            selectEquationIDs << item->GetID();
            selectItem << item;
        }
        else if (item->type() > DrawItemAnnotation) {//线
            selectLineIDs << static_cast<DrawLine*>(item)->GetLineID();
        }
        else {//注释
            //拷贝注释
            m_copyItemGroupData.annotations << static_cast<DrawAnnotation*>(item)->GetAnnotation();
            selectItem << item;
        }


    m_copyRect = m_pItemGroup->GetRectF(selectItem).toRect();//拷贝区域矩形大小

    // 拷贝线图元
    for (auto lineID : selectLineIDs) {
        auto line = DrawLine::GetLinkLine(lineID);
        if (!selectEquationIDs.contains(line.outputItemID) || !selectEquationIDs.contains(line.inputItemID))
            continue;

        m_copyItemGroupData.lines << line;
    }
}

void  EditControl::PasteAddGroup()
{
    if (m_copyItemGroupData.equations.isEmpty() && m_copyItemGroupData.annotations.isEmpty())
        return;

    QRect copyRect = m_copyRect;
    copyRect.moveTo(m_pSceneModule->GetMusePoint());

    if (!IsAreaAllowOverlap(copyRect)){//等式不能重叠, 注释，线可以重叠
            emit SignalErrorThrow(tr("Can't overlap"));
            return;
        }

    auto copyItemGroupData = CreateCopyItemGroupData(m_pSceneModule->GetMusePoint());
    StDrawItemGroupData* revocationnewData = new StDrawItemGroupData(OI_AddItemGroup);
    *revocationnewData = copyItemGroupData;
    ExecuteOperation(revocationnewData, m_throwOperationFunction);

    QList<BasicsDrawItem*> selectItems;//把新粘贴的图元选中
    for (auto& item : copyItemGroupData.equations)
        selectItems << m_basicsItems[item.id];

    for (auto& line : copyItemGroupData.lines)
        for(auto& lineSegment : line.line)
            selectItems << m_basicsItems[lineSegment.id];

    m_pItemGroup->ClearItem();
    m_pItemGroup->AddItem(selectItems);
}

StDrawItemGroupData EditControl::CreateCopyItemGroupData(const QPoint sceneCoord_)
{
    StDrawItemGroupData drawItemGroupData(OI_AddItemGroup);
    QMap<int, int> idMap;//原id和新id映射
    QPoint offsetCoord = sceneCoord_ - m_copyRect.topLeft();//偏移坐标

    //等式
    for (const auto& equation : m_copyItemGroupData.equations) {
        auto newEquation = equation;
        newEquation.id = GetNewID(true);//图元id
        newEquation.coord += offsetCoord;
        idMap.insert(equation.id, newEquation.id);
        drawItemGroupData.equations << newEquation;
    }

    //线
    for (const auto& line : m_copyItemGroupData.lines) {
        auto newLine = line;
        newLine.lineID = GetNewID(false);//整条线ID
        newLine.outputItemID = idMap[newLine.outputItemID];
        newLine.inputItemID = idMap[newLine.inputItemID];

        for (auto& lineSegment : newLine.line) {
            lineSegment.lineID = newLine.lineID;
            lineSegment.id = GetNewID(true);//图元id
            lineSegment.sPoint += offsetCoord;
            lineSegment.ePoint += offsetCoord;
        }

        drawItemGroupData.lines << newLine;
    }

    //注释
    for (const auto& annotation : m_copyItemGroupData.annotations) {
        auto newAnnotation = annotation;
        newAnnotation.id = GetNewID(true);//图元id
        newAnnotation.coord += offsetCoord;
        drawItemGroupData.annotations << newAnnotation;
    }

    return drawItemGroupData;
}




int EditControl::GetNewID(const bool isBasicsItem)
{
    if(isBasicsItem)
        return ++m_basicsItemID;

    return ++m_lineID;
}

//获取视图窗口指针
QWidget* EditControl::GetViewPWindget()
{
    return m_pViewModule;
}

QMap<int, BasicsDrawItem *> &EditControl::GetBasicsItems()
{
    return m_basicsItems;
}

QMap<int, BasicsDrawItem *> &EditControl::GetLines()
{
    return m_lines;
}

QRectF EditControl::PrintscreenRectF() const
{
    QSet<BasicsDrawItem*> items;
    for(auto item : m_basicsItems)
        items.insert(item);

    auto rectF = m_pItemGroup->GetRectF(items);
    rectF = QRectF(rectF.x() - 5, rectF.y() - 5, rectF.width() + 10, rectF.height() + 10);

    if(rectF.width() > 10000)
        rectF.setWidth(10000);

    if(rectF.height() > 7000)
        rectF.setHeight(7000);

    return  rectF;
}

QImage EditControl::Printscreen() const
{
    m_pItemGroup->ClearItem();//清空选择
    m_pSceneModule->update();//更新场景，以便截取最新
    QRectF rectF = PrintscreenRectF();//所有图元截图区域大小

    QImage image(rectF.width(), rectF.height(), QImage::Format_ARGB32);
    QPainter painter(&image);

    m_pSceneModule->render(&painter, QRect(0, 0, rectF.width(), rectF.height()), rectF);

    return image;
}

bool EditControl::IsAreaAllowOverlap(const QRect& rect_, const BasicsDrawItem* item_)
{
    if (item_ && item_->type() >= DrawItemAnnotation)
        return true;//线和注释可以重叠的

    for (auto tempItem : m_pSceneModule->items(rect_))
        if (item_ != tempItem && tempItem->type() < DrawItemAnnotation)
            return false;
  
    return true;
}

void EditControl::OnMenuTriggered(QAction* action)
{
    switch (action->data().toInt())
    {
    case CM_Nature: 

        break;//属性

    case CM_Copy: 
        CopySelectArea();
        break;//拷贝

    case CM_Paste: 
        PasteAddGroup();
        break;//粘贴

    case CM_AddSvg:
        AddSvg();
        break;//添加SVG矢量图

    case CM_AddAnnotation:
        AddAnnotation();
        break;//添加注释

    case CM_LeftAlign: //左对齐
    case CM_TopAlign: //上对齐
    case CM_RightAlign: //右对齐
    case CM_ButtomAlign://下对齐
    case CM_VerticallyCenterAlign: //纵向居中对齐
    case CM_CrosswiseCenterAlign: //横向居中对齐
        
        ItemLayout((ContextMenu)action->data().toInt());
        break;

    case CM_TestOper:
        AddTestEquation("Operator1", m_pSceneModule->GetMusePoint());
        break;

    default:
        break;
    }
}

void EditControl::Printscreen(QPainter *painter_, const QRectF &target_, const QRectF &source_)
{
    m_pItemGroup->ClearItem();
    m_pSceneModule->update();
    m_pSceneModule->render(painter_, target_, source_);
}








