#ifndef DRAWANNOTATION_H
#define DRAWANNOTATION_H
#include <QGraphicsTextItem>


#include "editcontroldefine.h"
#include "basicsdrawitem.h"



class DrawTextItem : public QGraphicsTextItem
{
	Q_OBJECT
public:
	DrawTextItem(QGraphicsItem* parent_ = nullptr);

	virtual void focusOutEvent(QFocusEvent* event) override;

	virtual int type() const override;

signals:
	void SignalSizeChange(const QSize& size_);

};


class DrawAnnotation : public BasicsDrawItem
{
public:

	DrawAnnotation(const StAnnotation& annotation_);
	~DrawAnnotation();

	virtual QRectF boundingRect() const override;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

	StAnnotation GetAnnotation();

	//创建注释
	//annotation_注释项结构, drawItemGroupData_记录已经删除的数据
	static void CreateAnnotation(const StAnnotation& annotation_, StDrawItemGroupData* drawItemGroupData_ = nullptr);

	//移除注释
	//id_图元ID，不存在不处理。drawItemGroupData_记录已经删除的数据
	static void RemoveAnnotation(const int id_, StDrawItemGroupData* drawItemGroupData_ = nullptr);

protected:
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
	//初始化注释图元
	void InitDrawAnnotation();

	//重置绘图区大小
	void ResetRect(const QSize& size_);

	//重置选框大小
	void ResetSelectBOXRect();

private:
	StAnnotation m_annotation;						  //注释图元数据结构
	StEquationSelectBOX m_selectBoxs[4];              //框点矩形集合, 这里为8个
	DrawTextItem* m_drawTextItem;					  //文本绘制
	QRect m_drawRect;								  //绘图区大小
	QString m_defaultText;							  //默认文本
};







#endif // DRAWANNOTATION_H


