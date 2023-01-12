#include "revocation.h"
#include "editcontrol.h"
#include "basicsdrawitem.h"
#include "itemgroup.h"
#include "drawequation.h"

#include <QDebug>

Revocation::Revocation(EditControl* editControl_):
    m_editControl(editControl_)
{
    m_count = m_editControl->Config().revocationStepCount;//撤销步骤数量
    m_index = -1;
    m_datas = new StBasicsOperateInstruct*[m_count];
    memset(m_datas, 0, m_count * sizeof(StBasicsOperateInstruct*));
}

Revocation::~Revocation()
{
    if(m_datas){
        for(int i = 0; i < m_count; ++i)
            if(m_datas[i])
                delete m_datas[i];

        delete [] m_datas;
    }
}

void Revocation::PushOperation(StBasicsOperateInstruct* revocation_)
{
    if(m_index == m_count - 1){
        delete m_datas[0];
        --m_index;

        for(int i = 1; i < m_count; ++i)
            m_datas[i - 1] = m_datas[i];
    }
    else if(m_datas[m_index + 1]){
        for(int i = m_index + 1; i < m_count && m_datas[i]; ++i){
                delete m_datas[i];
                m_datas[i] = nullptr;
            }
    }

    m_datas[++m_index] = revocation_;
}

void Revocation::FrontStep()
{
    if(m_index > -1)
        RevocationConduct(m_datas[m_index--], OI_Front);
}

void Revocation::NextStep()
{
    if(m_index < m_count - 1 && m_datas[m_index + 1])
        RevocationConduct(m_datas[++m_index], OI_Next);
}

void Revocation::RevocationConduct(StBasicsOperateInstruct *revocation_, const OperateInstructType instructType_)
{
    m_editControl->m_pItemGroup->ClearItem();//情况当前选择的
    QList<BasicsDrawItem*> selectBasicsDrawItems;//获得所有基础图元
    auto& basicsItems = m_editControl->GetBasicsItems();//基础图元数据集合

    if(revocation_->type == OI_Move){//移动指令
        auto itemGroupMoveData = static_cast<StItemGroupMoveData*>(revocation_);//图元集合移动数据信息

        for(int id : m_editControl->GetBasicsItemIDs(revocation_))
            if(basicsItems.contains(id))//存在
                selectBasicsDrawItems << basicsItems[id];
            else
                qDebug() << "Error";

        m_editControl->m_pItemGroup->AddItem(selectBasicsDrawItems);//全选
        m_editControl->m_pItemGroup->RevocationMove(instructType_ == OI_Front ? itemGroupMoveData->frontOffset : itemGroupMoveData->nextOffset);

        //向外抛出
        if (m_editControl->m_throwOperationFunction) {
            auto throwItemGroupMoveData = *itemGroupMoveData;
            if (instructType_ == OI_Front) {
                auto tempOffset = throwItemGroupMoveData.frontOffset;
                throwItemGroupMoveData.frontOffset = throwItemGroupMoveData.nextOffset;
                throwItemGroupMoveData.nextOffset = tempOffset;
            }

            m_editControl->m_throwOperationFunction(throwItemGroupMoveData);
        }
    }
    else if (revocation_->type == OI_LayoutMove) {//布局移动
        auto itemGroupLayoutMoveData = static_cast<StItemGroupLayoutMove*>(revocation_);//图元集合布局移动数据信息

        for (auto& item : itemGroupLayoutMoveData->itemGroupLayoutMoveDatas) {
            auto equation = static_cast<DrawEquation*>(basicsItems[item.id]);
            auto rect = equation->boundingRect().toRect();
            rect.moveTo(instructType_ == OI_Front ? item.frontCoord : item.nextCoord);
            equation->SetEquationRect(rect);
            selectBasicsDrawItems << equation;
        }

        m_editControl->m_pItemGroup->ClearItem();
        m_editControl->m_pItemGroup->AddItem(selectBasicsDrawItems);//全选

         //向外抛出
        if (m_editControl->m_throwOperationFunction) {
            auto throwItemGroupLayoutMoveData = *itemGroupLayoutMoveData;
            if (instructType_ == OI_Front) {
                for (auto& layoutMove : throwItemGroupLayoutMoveData.itemGroupLayoutMoveDatas) {
                    auto tempCoord = layoutMove.frontCoord;
                    layoutMove.frontCoord = layoutMove.nextCoord;
                    layoutMove.nextCoord = tempCoord;
              }
            }

            m_editControl->m_throwOperationFunction(throwItemGroupLayoutMoveData);
        }
    }
    else if(revocation_->type == OI_EquationZoom){//缩放指令
        auto equationZoom = static_cast<StEquationZoom*>(revocation_);//等式图元放大数据信息

        if(basicsItems.contains(equationZoom->id)){//存在
            auto equation = static_cast<DrawEquation*>(basicsItems[equationZoom->id]);
            equation->SetEquationRect(instructType_ == OI_Front ? equationZoom->frontRect : equationZoom->nextRect);
            m_editControl->m_pItemGroup->AddItem(equation);//全选
        }
        else
            qDebug() << "Error";

        //向外抛出
        if (m_editControl->m_throwOperationFunction) {
            auto throwEquationZoom = *equationZoom;
            if (instructType_ == OI_Front) {
                auto tempRect = throwEquationZoom.frontRect;
                throwEquationZoom.frontRect = throwEquationZoom.nextRect;
                throwEquationZoom.nextRect = tempRect;
            }

            m_editControl->m_throwOperationFunction(throwEquationZoom);
        }
    }
    else if(revocation_->type == OI_AddItemGroup || revocation_->type == OI_RemoveItemGroup){//添加和移除指令
        OperateInstructType operateInstructType = OI_AddItemGroup;//这里只有添加和删除， 默认添加
        if((instructType_ == OI_Front && revocation_->type == OI_AddItemGroup) ||
           (instructType_ == OI_Next && revocation_->type == OI_RemoveItemGroup))
            operateInstructType = OI_RemoveItemGroup;//列出所有删除情况

        auto itemGroupData = static_cast<StDrawItemGroupData*>(revocation_);//对象集合绘制数据信息

        if(operateInstructType == OI_AddItemGroup){//添加当前数据
            m_editControl->AddGroup(itemGroupData);//添加数据

            for(int id : m_editControl->GetBasicsItemIDs(revocation_))
                if(basicsItems.contains(id))//存在
                    selectBasicsDrawItems << basicsItems[id];
                else
                    qDebug() << "Error";

            m_editControl->m_pItemGroup->AddItem(selectBasicsDrawItems);//全选
        }
        else{//删除当前数据
            for(int id : m_editControl->GetBasicsItemIDs(revocation_))
                if(basicsItems.contains(id))//存在
                    selectBasicsDrawItems << basicsItems[id];
                else
                    qDebug() << "Error";

            m_editControl->m_pItemGroup->AddItem(selectBasicsDrawItems);//全选
            m_editControl->RemoveGroup();//删除集合但是不如站
        }

        //向外抛出
        if (m_editControl->m_throwOperationFunction) {
            auto throwItemGroupData = *itemGroupData;
            throwItemGroupData.type = operateInstructType;
            m_editControl->m_throwOperationFunction(throwItemGroupData);
        }
    }
}
