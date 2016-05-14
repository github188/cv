#pragma once
#include <QtGui/QtGui>

class CShape
{
public:
	CShape(int shapetype,int startx,int starty,int endx,int endy);
	CShape(CShape& sourceShape);
	void Draw(QPainter *pDC);
	void UnDraw(QPainter *pDC);
	int GetType();

	//���ص�
	QPoint GetStartPoint();
	QPoint GetEndPoint();
protected:
	int m_type;
	//���
	QPoint m_start;
	//�յ�
	QPoint m_end;

};