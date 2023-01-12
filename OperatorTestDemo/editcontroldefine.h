/*****************************************************************
**图形编辑定义头文件
**
**
**
**
** Author：laichao
** Date：2022-03-28
******************************************************************/
#ifndef EDITCONTROLDEFINE_H
#define EDITCONTROLDEFINE_H

#include <QObject>
#include <QRect>
#include <QMap>
#include <QList>

class BasicsDrawItem;//基础绘图项



//编辑器状态枚举
enum EditState{
    EditState_Nullify = 0,    //无状态
    CtrlPressed = 0X1,        //Ctrl键处于按下状态
    SpacePressed = 0X2,       //空格键按下, 拖动场景
    SamePointDrawLine = 0X4,  //在同一点画线,左键点击新拐点是很上次在同一点
    StartLink = 0X8,          //处于开始连接状态
    EnterPortPin = 0X10,      //进入端口引脚状态, 编辑引脚状态
    PortPinOutNotEdit = 0x20, //进入输出引脚不能编辑状态
    AddFillLine = 0X40,       //添加了填充线状态,在连线时鼠标当前指引线和引脚不是同轴方向，需要增加一条线来填充
    PressedDrewItem = 0X80,   //上一次按下了图元
    DragAddEquation = 0X100,  //拖拽添加等式状态

    BOX_Corner24 = 0X1000,    //框选状态，2和4象限方向
    BOX_Corner13 = 0X2000,    //框选状态，1和3象限方向
    Box_Crosswise = 0X4000,   //框选状态，横向
    Box_Lengthways = 0X8000,  //框选状态，纵向
    Box_State = BOX_Corner24 | BOX_Corner13 | Box_Crosswise | Box_Lengthways //框选状态集合
};

//上下文菜单枚举
enum ContextMenu {
    CM_Nature,                              //属性
    CM_Copy,                                //复制
    CM_Paste,                               //粘贴
    CM_AddSvg,                              //添加SVG矢量图
    CM_AddAnnotation,                       //添加注释
    CM_LeftAlign,                           //左对齐
    CM_TopAlign,                            //上对齐
    CM_RightAlign,                          //右对齐
    CM_ButtomAlign,                         //下对齐
    CM_VerticallyCenterAlign,               //纵向居中对齐
    CM_CrosswiseCenterAlign,                //横向居中对齐
    CM_TestOper                             //添加测试图元
};

//绘图项目类型
enum DrawItemType{
    DrawItem_Nullify = 0,
    Operator,

    DrawItemAnnotation = 0XFFFFFF,      //注释类型 大于此类型为线类型 小于此类型为等式类型

    DrawItem_REV = 0X01000000,          //逆向标识
    HLINE = 0X02000000,                 //横向线
    ZLINE = 0X04000000,                 //纵向线
    HLINE_REV = HLINE | DrawItem_REV,   //逆横向线
    ZLINE_REV = ZLINE | DrawItem_REV,   //逆纵向线

    DrawItemText = 0X08000000           //注释文本类型， 嵌入在助手图元中的,不作为基础图元
};

//链接端口引脚类型
enum LinkPortPinType{
    LPP_Nullify = 0,                            //空类型

    LPP_Out = 0X1,                              //输出类型
    LPP_In = 0X2,                               //输入类型

    LPP_Left = 0X4,                             //左边端口
    LPP_Top = 0X8,                              //上面端口
    LPP_Right = 0X10,                           //右边端口
    LPP_Buttom = 0X20,                          //下面端口

    LPP_LeftOut = LPP_Left | LPP_Out,           //左边输出端口
    LPP_TopOut = LPP_Top | LPP_Out,             //上面输出端口
    LPP_RightOut = LPP_Right | LPP_Out,         //右边输出端口
    LPP_ButtomOut = LPP_Buttom | LPP_Out,       //下面输出端口

    LPP_LeftIn = LPP_Left | LPP_In,             //左边输入端口
    LPP_TopIn = LPP_Top | LPP_In,               //上面输入端口
    LPP_RightIn = LPP_Right | LPP_In,           //右边输入端口
    LPP_ButtomIn = LPP_Buttom | LPP_In          //下面输入端口
};

//移动状态判断  用于判断是否可以移动
enum MobileJudgmentType{
    MJ_Nullify = 0X0,                           //不能移动
    MJ_XEnabled = 0X1,                          //X可以移动
    MJ_YEnabled = 0X2,                          //Y可以移动
    MJ_XYEnabled = MJ_XEnabled | MJ_YEnabled    //X/Y都可以移动
};


//操作指令类型
enum OperateInstructType{
    OI_Front,               //上一步
    OI_Next,                //下一步
    OI_Move,                //移动
    OI_LayoutMove,          //布局移动
    OI_EquationZoom,        //等式缩放
    OI_AddItemGroup,        //添加集合
    OI_RemoveItemGroup,     //移除集合
    OI_RemoveCategory       //移除同一类, 如其它进程删除变量，常量，操作符
};

//端口引脚接口体结构
struct StPortPin : public QObject{
    Q_PROPERTY(QPoint point READ GetPoint)
    Q_OBJECT

public:
    QRect rect;             //位置和大小
    LinkPortPinType type;   //端口引脚类型
    int index;              //编号， 输出和输入引脚编号可以一样
    bool inUse;             //是否在使用，输出引脚可以链接任意条线， 输入引脚最多链接一条线

    StPortPin(const StPortPin& portPin_){
        *this = portPin_;
    }

    StPortPin(QRect rect_ = QRect(), LinkPortPinType type_ = LPP_Nullify, int index_ = 0, bool inUse_ = false):
        rect(rect_), type(type_), index(index_), inUse(inUse_){}


    StPortPin& operator=(const StPortPin& portPin_){
        rect = portPin_.rect;
        type = portPin_.type;
        index = portPin_.index;
        inUse = portPin_.inUse;

        return *this;
    }

private:
    QPoint GetPoint() const{
        return rect.center();
    }
};


//绘图项链接结构
struct StDrawItemLink{
    BasicsDrawItem* item;      //链接的项指针
    int pinIndex;              //端口引脚下标

    StDrawItemLink(BasicsDrawItem* item_ = nullptr, const int pinIndex_ = 0):item(item_), pinIndex(pinIndex_){}
};

//等式矩形框选结构
struct StEquationSelectBOX{
    QRect rect;                     //边框小矩形
    EditState state;                //对应状态
    bool isOriginalEnabledX;        //原点X有效
    bool isOriginalEnabledY;        //远点Y有效
    bool isTerminusEnabledX;        //终点X有效
    bool isTerminusEnabledY;        //终点Y有效

    StEquationSelectBOX(): state(EditState_Nullify), isOriginalEnabledX(false), isOriginalEnabledY(false), isTerminusEnabledX(false), isTerminusEnabledY(false){}
};

//线段结构
struct StLineSegment {
    QString name;               //名称
    QString annotation;         //注释
    QPoint sPoint;              //开始点
    QPoint ePoint;              //结束点
    DrawItemType type;          //类型
    int id;                     //线段唯一标识，也是图元唯一编号
    int lineID;                 //线段所属线ID
    bool isFullLine ;           //是实线, 只有在鼠标点击连线是才有效

    StLineSegment(const QString& name_, const QString& annotation_, QPoint sPoint_ = QPoint(), QPoint ePoint_ = QPoint(), DrawItemType type_= DrawItem_Nullify, const int id_ = 0, const int lineID_ = 0, bool isFullLine_ = false):
    name(name_), annotation(annotation_), sPoint(sPoint_), ePoint(ePoint_), type(type_), id(id_), lineID(lineID_), isFullLine(isFullLine_){}
};


//链接线结构
struct StLinkLine{
    QList<StLineSegment> line;      //一条有线段组成的线
    int outputItemID;               //输出图元id,每个图元唯一编号
    int inputItemID;                //输入图元id,每个图元唯一编号
    int outputIndex;                //输出下标编号，为端口引脚编号
    int inputIndex;                 //输入下标编号, 为端口引脚编号
    int lineID;                     //连线唯一编号

    StLinkLine(){}
    StLinkLine(const QList<StLineSegment>& line_, const int outputItemID_, const int inputItemID_, const int outputIndex_, const int inputIndex_, const int lineID_):
        line(line_), outputItemID(outputItemID_), inputItemID(inputItemID_), outputIndex(outputIndex_), inputIndex(inputIndex_), lineID(lineID_){}
};

//等式操作符相结构
struct StEquationOperator{
    QList<StPortPin> portPins;                      //此等式操作符拥有的端口引脚集合
    QMap<int, int> tempVariates;                    //临时变量集合, 一个输出端口对应一个临时变量, int为输出端口引脚下标编号
    QString name;                                   //名称
    QString annotation;                             //注释
    QPoint coord;                                   //坐标
    DrawItemType type;                              //类型
    int id;                                         //每个图元唯一编号
    int width;                                      //宽度
    int height;                                     //高度
};

//注释图元结构
struct StAnnotation{
    QString text;                                   //注释文本
    QPoint coord;                                   //坐标
    DrawItemType type;                              //类型
    int id;                                         //每个图元唯一编号
    int width;                                      //宽度
    int height;                                     //高度

    StAnnotation() : type(DrawItemAnnotation) {}
};

//基础操作类型结构
struct StBasicsOperateInstruct{
    OperateInstructType type;       //操作指令类型
    StBasicsOperateInstruct(const OperateInstructType type_):type(type_){}
};

//图元集合移动数据信息
struct StItemGroupMoveData : public StBasicsOperateInstruct{
    QList<int> itemGroupIDs;    //移动的集合项
    QPoint frontOffset;         //上一次位置
    QPoint nextOffset;          //下一次位置

    StItemGroupMoveData(): StBasicsOperateInstruct(OI_Move){}
};

//图元集合布局移动数据信息
struct StItemGroupLayoutMove : public StBasicsOperateInstruct {
    struct StItemCoord{
        QPoint frontCoord;                         //上一次位置
        QPoint nextCoord;                          //下一次位置
        int id;
    };

    QList<StItemCoord> itemGroupLayoutMoveDatas;    //图元集合布局移动信息
    StItemGroupLayoutMove(): StBasicsOperateInstruct(OI_LayoutMove) {}
};

//等式图元放大数据信息
struct StEquationZoom : public StBasicsOperateInstruct{
    QRect frontRect;         //上一次位置大小
    QRect nextRect;          //下一次位置大小
    int id;                  //唯一编号

    StEquationZoom(): StBasicsOperateInstruct(OI_EquationZoom){}
};

//对象集合绘制数据信息
struct StDrawItemGroupData : public StBasicsOperateInstruct{
    QList<StEquationOperator> equations;        //等式操作符集合
    QList<StLinkLine> lines;                    //链接连线集合
    QList<StAnnotation> annotations;            //注释文本集合

    StDrawItemGroupData(const StBasicsOperateInstruct type_): StBasicsOperateInstruct(type_){}
};

















#endif // EDITCONTROLDEFINE_H
