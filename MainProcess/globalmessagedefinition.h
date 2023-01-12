#ifndef GLOBALMESSAGEDEFINITION_H
#define GLOBALMESSAGEDEFINITION_H

#include <QObject>
#include <QMap>
#include <QPoint>



//全局消息定义类
class GlobalMessageDefinition{
public:

    //全局消息类型
    enum GMDType{
        GMD_WindowsEmbedded,    //窗口嵌入
        GMD_AddEquation         //添加等式
    };


    //全局消息基础类
    struct GMDBaseClass{
        const GMDType type;

        GMDBaseClass(const GMDType type_): type(type_){}
    };



    //窗口嵌入消息结构
    struct StWindowsEmbedded : public GMDBaseClass{
        enum ProgressType{
            EditDiagram //图形编辑器
                        //仿真在线监视
                        //打印
        };

        ProgressType progressType;    //进程类型
        QMap<quint64, QString> windowGroup; //窗口集合， key是窗口句柄， value窗口名，窗口名可以为空
        //Jons 对应 handle name

        StWindowsEmbedded(): GMDBaseClass(GMD_WindowsEmbedded){}
    };

    //添加等式
    struct StAddEquation : public GMDBaseClass{
        qint64 progressID;          //进程唯一标识
        quint64 equationID;         //等式唯一标识
        QPoint globalMousePos;      //全局鼠标坐标

        StAddEquation():GMDBaseClass(GMD_AddEquation){}
    };


/*
*   注:  GMDTypeme枚举名和主题明相同
*       json Key名和结构体成员名相同
*/


    //结构体转Json
    //josnMessage_目标Json数据， stMessage_原结构体 失败返回false
    static bool StructToJson(QByteArray& josnMessage_, const GMDBaseClass& stMessage_);

    //Json转结构体
    // stMessage_目标结构体, josnMessage_原Json数据，失败返回false
    static bool JsonToStruct(GMDBaseClass& stMessage_, const QByteArray& josnMessage_);

private:
    GlobalMessageDefinition(){}

    //窗口嵌入消息结构转Json
    //josnMessage_目标Json数据， stMessage_原窗口嵌入消息结构 失败返回false
    static bool WindowsEmbeddedToJson(QByteArray& josnMessage_, const StWindowsEmbedded& stMessage_);

    //Json转窗口嵌入消息结构
    // stMessage_目标窗口嵌入消息结构, josnMessage_原Json数据， 失败返回false
    static bool JsonToWindowsEmbedded(StWindowsEmbedded& stMessage_, const QByteArray& josnMessage_);

    //添加等式转Json
    //josnMessage_目标Json数据， stMessage_原添加等式 失败返回false
    static bool AddEquationToJson(QByteArray& josnMessage_, const StAddEquation& stMessage_);

    //Json转添加等式
    // stMessage_目标添加等式, josnMessage_原Json数据， 失败返回false
    static bool JsonToAddEquation(StAddEquation& stMessage_, const QByteArray& josnMessage_);


};




#endif // GLOBALMESSAGEDEFINITION_H
