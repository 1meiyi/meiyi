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

	//����ע��
	//annotation_ע����ṹ, drawItemGroupData_��¼�Ѿ�ɾ��������
	static void CreateAnnotation(const StAnnotation& annotation_, StDrawItemGroupData* drawItemGroupData_ = nullptr);

	//�Ƴ�ע��
	//id_ͼԪID�������ڲ�����drawItemGroupData_��¼�Ѿ�ɾ��������
	static void RemoveAnnotation(const int id_, StDrawItemGroupData* drawItemGroupData_ = nullptr);

protected:
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
	//��ʼ��ע��ͼԪ
	void InitDrawAnnotation();

	//���û�ͼ����С
	void ResetRect(const QSize& size_);

	//����ѡ���С
	void ResetSelectBOXRect();

private:
	StAnnotation m_annotation;						  //ע��ͼԪ���ݽṹ
	StEquationSelectBOX m_selectBoxs[4];              //�����μ���, ����Ϊ8��
	DrawTextItem* m_drawTextItem;					  //�ı�����
	QRect m_drawRect;								  //��ͼ����С
	QString m_defaultText;							  //Ĭ���ı�
};







#endif // DRAWANNOTATION_H


