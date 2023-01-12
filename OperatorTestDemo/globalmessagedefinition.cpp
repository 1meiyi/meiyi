#include "globalmessagedefinition.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QVariant>

#include <QMessageBox>


bool GlobalMessageDefinition::StructToJson(QByteArray &josnMessage_, const GlobalMessageDefinition::GMDBaseClass &stMessage_)
{
    if(stMessage_.type == GMD_WindowsEmbedded)//窗口嵌入
        return  WindowsEmbeddedToJson(josnMessage_, static_cast<const StWindowsEmbedded&>(stMessage_));
    else if(stMessage_.type == GMD_AddEquation)//添加等式
        return  AddEquationToJson(josnMessage_, static_cast<const StAddEquation&>(stMessage_));

    return false;
}

bool GlobalMessageDefinition::JsonToStruct(GlobalMessageDefinition::GMDBaseClass& stMessage_, const QByteArray &josnMessage_)
{
    if(stMessage_.type == GMD_WindowsEmbedded)//窗口嵌入
        return  JsonToWindowsEmbedded(static_cast<StWindowsEmbedded&>(stMessage_), josnMessage_);
    else if(stMessage_.type == GMD_AddEquation)//添加等式
        return  JsonToAddEquation(static_cast<StAddEquation&>(stMessage_), josnMessage_);

    return false;
}









bool GlobalMessageDefinition::WindowsEmbeddedToJson(QByteArray &josnMessage_, const GlobalMessageDefinition::StWindowsEmbedded &stMessage_)
{
    if(stMessage_.windowGroup.isEmpty())
        return false;

    QJsonObject rootObject;
    rootObject.insert("progressType", (int)stMessage_.progressType);

    QJsonArray array;
    QJsonObject object;
    for(auto first = stMessage_.windowGroup.begin(), end = stMessage_.windowGroup.end(); first != end; ++first){
        object.insert("handle", QJsonValue::fromVariant(first.key()));
        object.insert("name", *first);
        array << object;
    }

    rootObject.insert("windowGroup", array);
    QJsonDocument document(rootObject);
    josnMessage_ = document.toJson();

    return true;
}

bool GlobalMessageDefinition::JsonToWindowsEmbedded(GlobalMessageDefinition::StWindowsEmbedded& stMessage_, const QByteArray &josnMessage_)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(josnMessage_, &error);
    if(error.error != QJsonParseError::NoError)
        return false;//解析出错

    auto rootObject = document.object();
    if(rootObject.isEmpty())
        return false;//存储结构不明

    if(rootObject.value("progressType").isNull())
        return false;//存储结构不明

    stMessage_.progressType = (StWindowsEmbedded::ProgressType)(rootObject.value("progressType").toInt());

    auto array = rootObject.value("windowGroup").toArray();
    if(array.isEmpty())
        return false;//存储结构不明

    for(auto value : array){
        auto object = value.toObject();
        if(object.isEmpty())
            return false;//存储结构不明

        if(object.value("handle").isNull())
            return false;//存储结构不明

        stMessage_.windowGroup.insert(object.value("handle").toVariant().value<qint64>(), object.value("name").toString());
    }

    return true;
}

bool GlobalMessageDefinition::AddEquationToJson(QByteArray &josnMessage_, const GlobalMessageDefinition::StAddEquation &stMessage_)
{
    if(stMessage_.globalMousePos.isNull())
        return false;

    QJsonObject rootObject;
    rootObject.insert("progressID", QJsonValue::fromVariant(stMessage_.progressID));
    rootObject.insert("equationID", QJsonValue::fromVariant(stMessage_.equationID));
    rootObject.insert("globalMousePos", QJsonValue::fromVariant(stMessage_.globalMousePos));
    QJsonDocument document(rootObject);
    josnMessage_ = document.toJson();

    return true;
}

bool GlobalMessageDefinition::JsonToAddEquation(GlobalMessageDefinition::StAddEquation& stMessage_, const QByteArray &josnMessage_)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(josnMessage_, &error);
    if(error.error != QJsonParseError::NoError)
        return false;//解析出错

    auto rootObject = document.object();
    if(rootObject.isEmpty())
        return false;//存储结构不明

    if(rootObject.value("progressID").isNull() ||
       rootObject.value("equationID").isNull() ||
       rootObject.value("globalMousePos").isNull())
        return false;

    stMessage_.progressID = rootObject.value("progressID").toVariant().value<quint64>();
    stMessage_.equationID = rootObject.value("equationID").toVariant().value<quint64>();
    stMessage_.globalMousePos = rootObject.value("equationID").toVariant().toPoint();

    return true;
}
