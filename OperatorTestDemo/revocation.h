#ifndef REVOCATION_H
#define REVOCATION_H

#include "editcontroldefine.h"

class EditControl;

class Revocation
{
public:
    Revocation(EditControl* editControl_);

    ~Revocation();

    //操作数据入撤销容器
    //revocation_ 操作数据
    void PushOperation(StBasicsOperateInstruct* revocation_);

    //返回上一步
    void FrontStep();

    //返回下一步
    void NextStep();

private:
    //撤销
    //revocation_撤销数据, instructType_上一步 还是下一步
    void RevocationConduct(StBasicsOperateInstruct* revocation_, const OperateInstructType instructType_);

private:
    StBasicsOperateInstruct** m_datas;
    EditControl* m_editControl;             //编辑控制指针
    int m_count;
    int m_index;
};


#endif // REVOCATION_H
