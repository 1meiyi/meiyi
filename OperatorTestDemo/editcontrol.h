#ifndef EDITCONTROL_H
#define EDITCONTROL_H

#include "editcontroldefine.h"

#include <QMap>
#include <QPolygonF>
#include <QObject>
#include <QColor>

class BasicsDrawItem;
class ViewModule;
class SceneModule;
class ItemGroup;
class Revocation;
class QWidget;
class QPainter;
class QMenu;
class QAction;
class QSvgRenderer;

class QTimer;


typedef void (*ThrowOperationFunction)(const StBasicsOperateInstruct&);//抛出操作函数类型

class  EditControl : public QObject
{
    Q_OBJECT

    //配置结构体
    struct StConfig{
        QString svgDirPath;                     //svg矢量图存储路径
        QColor sceneBackColor;                  //场景背景颜色
        QColor BOXSelectColor;                  //框选颜色
        QColor outPortPinColor;                 //输出端口颜色
        QColor inPortPinColor;                  //输出端口颜色
        QColor selectLineColor;                 //选中线颜色
        QColor lineTrueColor;                   //在线监视时线类型为true颜色
        QColor lineFalseColor;                  //在线监视时线类型为false颜色
        QColor nameColor;                       //名称颜色
        int drawEquationMinimum;                //等式图元最小值
        int lineSelectWidth;                    //线选择宽度大小, 不是线宽度
        int selectDiamondSize;                  //选框上小方块大小
        int portPinSize;                        //端口引脚正方形范围边长
        int inFlexionRadius;                    //拐点圆半径大小
        int revocationStepCount;                //撤销步骤数量
    };


public:
    EditControl();
    ~EditControl();

    //动态测试
    QTimer* m_timer;
    void DynamicTesting();

    void OnDynamicTesting();


    //更新状态
    //editState_ 需要更新的状态, clear_为true清空， 为false更新
    void UpdateEditState(const EditState editState_, const bool clear_ = false);

    //更新svg
    //svgName_文件名称， 右则更新，无则创建
    void UpdateSvgRenderer(const QString& svgName_);

    //获得SVG渲染器
    //svgName_文件名
    QSvgRenderer* GetSvgRenderer(const QString& svgName_);

    //添加Svg矢量图
    void AddSvg();

    //弹出上下文菜单
    //type_要弹出上下文菜单的类型
    void PopContextMenu(const DrawItemType type_);

    //设置抛出操作回调函数
    //function_ 函数指针
    void SetThrowOperationFunction(ThrowOperationFunction function_);

    //图元布局，只布局等式图元
    void ItemLayout(const ContextMenu type_);

    //获取编辑状态
    EditState GetEditState() const;

    //查看配置文件
    const StConfig& Config() const;

    //获取临时链接线
    StLinkLine& GetTempLinkLine();

    //情况临时链接线
    void ClearTempLinkLine();

    //根据已有临时连线数据,创建连线
    void CreateTempLinkLine();

    //框选图元
    void BOXSelect(const QPolygonF& polygonF_);

    //操作数据入撤销容器
    //revocation_ 操作数据, revocation_指针被接收者得到所有权, function_回调函数 是否向外抛出
    void PushOperation(StBasicsOperateInstruct* revocation_, ThrowOperationFunction function_);

    //返回上一步
    void FrontStep();

    //返回下一步
    void NextStep();

    //执行操作
    //revocation_操作数据, function_回调函数 是否向外抛出
    void ExecuteOperation(StBasicsOperateInstruct* revocation_, ThrowOperationFunction function_);

    void AddTestEquation(QString text_, QPoint scenePos_);//添加测试等式

    //移除现在选中集合
    void RemoveCurrentSelectGroup();

    //获取指定数据中所有的基础图元ID集合
    //revocation_操作数据
    QList<int> GetBasicsItemIDs(const StBasicsOperateInstruct* revocation_);

    //添加对象集合
    //addDrawItemGroupData_需要添加象集合绘制数据信息, drawItemGroupData_记录已经删除的数据
    void AddGroup(StDrawItemGroupData* addDrawItemGroupData_, StDrawItemGroupData* drawItemGroupData_ = nullptr);

    //添加对象集合
    //drawItemGroupData_记录已经删除的数据
    void RemoveGroup(StDrawItemGroupData* drawItemGroupData_ = nullptr);

    //拷贝选中区域
    //复制选择区域到拷贝缓存
    void CopySelectArea();

    //粘贴添加拷贝缓存数据，若存在
    void PasteAddGroup();

    //生成新拷贝数据
    StDrawItemGroupData CreateCopyItemGroupData(const QPoint sceneCoord_);

    //获取新ID
    //isBasicsItem为true是基础图元ID， 为false线ID
    int GetNewID(const bool isBasicsItem);

    //获取视图窗口指针
    QWidget* GetViewPWindget();

    //获得基础图元集合数据引用
    QMap<int, BasicsDrawItem*>& GetBasicsItems();

    //获得线集合数据引用
    QMap<int, BasicsDrawItem*>& GetLines();

    //获得截取区域矩形大小
    QRectF PrintscreenRectF() const;

    //获得截图位图
    QImage Printscreen() const;

    //打印到设备
    //painter_需要打印设备的画家指针, target_打印目标矩形, source_场景中矩形
    void Printscreen(QPainter *painter_, const QRectF &target_, const QRectF &source_);

    //判断此区域是否可以重叠
    //rect_需要判断的区域, item_图元对象调用
    bool IsAreaAllowOverlap(const QRect& rect_, const BasicsDrawItem* item_ = nullptr);


public slots:
    //上下文按下活动事件
    void OnMenuTriggered(QAction* action);

signals:
    //错误抛出
    void SignalErrorThrow(const QString& error_);


protected:


private:
    //加载配置文件
    void LoadConfig();

    //添加注销
    void AddAnnotation();

    //添加图元集合

    //移除图元集合


public:
    ViewModule* m_pViewModule;                          //视图模块指针
    SceneModule* m_pSceneModule;                        //场景模块指针
    ItemGroup* m_pItemGroup;                            //选中集合指针
    ThrowOperationFunction m_throwOperationFunction;    //抛出操作函数，此为回调函数

private:
    QMap<int, BasicsDrawItem*> m_basicsItems;//所有基础图元项集合,分为三种[1]等式(操作符，变量，常量...),[2]线段，[3]注释
    QMap<int, BasicsDrawItem*> m_lines;//一个唯ID对应第一条线段，对线段组成的线集中管理
    QMap<QString, QSvgRenderer*> m_svgRenderers; //已经添加的操作符SVG矢量图渲染器集合
    StDrawItemGroupData m_copyItemGroupData;//拷贝数据， 拷贝缓存
    QRect m_copyRect;   //拷贝区域矩形大小
    StLinkLine m_tempLinkLine;//在手动连线时使用
    StConfig m_config;        //图形编辑配置
    EditState m_editState;//编辑器状态枚举
    Revocation* m_pRevocation;   //操作撤销
    QMenu* m_contextMenu;       //右键的上下文菜单
    int m_basicsItemID;     //基础图元ID
    int m_lineID;           //线ID
};

#endif // EDITCONTROL_H
















