#include "drawannotation.h"
#include "editcontrol.h"
#include "scenemodule.h"

#include <QGraphicsSceneMouseEvent>
#include <QFocusEvent>
#include <QPainter>
#include <QObject>




DrawTextItem::DrawTextItem(QGraphicsItem* parent_):
    QGraphicsTextItem(parent_)
{
    setTextInteractionFlags(Qt::NoTextInteraction);
}

void DrawTextItem::focusOutEvent(QFocusEvent* event)
{
    QGraphicsTextItem::focusOutEvent(event);
    setTextInteractionFlags(Qt::NoTextInteraction);
    emit SignalSizeChange(boundingRect().size().toSize());
}

int DrawTextItem::type() const
{
    return DrawItemText;//注释文本类型
}


DrawAnnotation::DrawAnnotation(const StAnnotation& annotation_):
	BasicsDrawItem(annotation_.type, annotation_.id),
	m_annotation(annotation_)
{
    m_drawTextItem = new DrawTextItem(this);
    m_defaultText = QObject::tr("new annotation");
	InitDrawAnnotation();

    QObject::connect(m_drawTextItem, &DrawTextItem::SignalSizeChange, [=](const QSize& size_) {
        ResetRect(size_);
        });
}

DrawAnnotation::~DrawAnnotation()
{
    delete m_drawTextItem;
}


QRectF DrawAnnotation::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

void DrawAnnotation::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    for (int i = 0; IsSelect() && i < 4; ++i)
        painter->fillRect(m_selectBoxs[i].rect, Qt::black);
}

StAnnotation DrawAnnotation::GetAnnotation()
{
    m_annotation.coord = pos().toPoint();
    return m_annotation;
}

void DrawAnnotation::CreateAnnotation(const StAnnotation& annotation_, StDrawItemGroupData* drawItemGroupData_)
{
    auto& basicsItems = m_editControl->GetBasicsItems();
    if (basicsItems.contains(annotation_.id))
        return;//已经存在不用添加

    if (drawItemGroupData_)
        drawItemGroupData_->annotations << annotation_;//记录插入成功数据

    auto drawAnnotation = new DrawAnnotation(annotation_);
    basicsItems.insert(drawAnnotation->GetID(), drawAnnotation);
    m_editControl->m_pSceneModule->addItem(drawAnnotation);
}


void DrawAnnotation::RemoveAnnotation(const int id_, StDrawItemGroupData* drawItemGroupData_)
{
    auto& basicsItems = m_editControl->GetBasicsItems();
    if (!basicsItems.contains(id_))
        return;//不存在不用移除

    auto drawAnnotation = static_cast<DrawAnnotation*>(basicsItems[id_]);
    if (drawItemGroupData_)
        drawItemGroupData_->annotations << drawAnnotation->GetAnnotation();

    basicsItems.remove(id_);
    m_editControl->m_pSceneModule->removeItem(drawAnnotation);
    delete drawAnnotation;
}

void DrawAnnotation::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    BasicsDrawItem::mouseDoubleClickEvent(event);
    m_drawTextItem->setTextInteractionFlags(Qt::TextEditable);
}

void DrawAnnotation::InitDrawAnnotation()
{
    const int sDS = m_editControl->Config().selectDiamondSize;
    if (m_annotation.text.isEmpty())
        m_annotation.text = m_defaultText;

    m_drawTextItem->setPlainText(m_annotation.text);
    ResetRect(m_drawTextItem->boundingRect().size().toSize());
    setPos(m_annotation.coord);
}

void DrawAnnotation::ResetRect(const QSize& size_)
{
    m_drawRect = QRect(0, 0, size_.width(), size_.height());
    const int sDS = m_editControl->Config().selectDiamondSize;

    m_width = size_.width() + sDS * 2;
    m_height = size_.height() + sDS * 2;
    m_annotation.text = m_drawTextItem->toPlainText();
    if (m_annotation.text.isEmpty()) {
        m_annotation.text = m_defaultText;
        m_drawTextItem->setPlainText(m_annotation.text);
        ResetRect(m_drawTextItem->boundingRect().size().toSize());
    }

    ResetSelectBOXRect();
}


void DrawAnnotation::ResetSelectBOXRect()
{
    const int sDS = m_editControl->Config().selectDiamondSize;

    //上左
    m_selectBoxs[0].rect = QRect(m_drawRect.x() - sDS, m_drawRect.y() - sDS, sDS, sDS);

    //上右
    m_selectBoxs[1].rect = QRect(m_drawRect.right() + 1, m_drawRect.y() - sDS, sDS, sDS);

    //下左
    m_selectBoxs[2].rect = QRect(m_drawRect.x() - sDS, m_drawRect.bottom() + 1, sDS, sDS);

    //下右
    m_selectBoxs[3].rect = QRect(m_drawRect.right() + 1, m_drawRect.bottom() + 1, sDS, sDS);
}