#pragma once
#include "Shape.h"
#include <string>
using namespace std;
class QtWhiteBoardWidget;

class CDrawPanel:public QWidget
{
public:
	CDrawPanel(QtWhiteBoardWidget *_pWhiteBoard);
	~CDrawPanel(void);
public:
	QVector <CShape*>m_Shape;//������¼����ͼ���
	QVector <CShape*>m_UndoShape;//���ڼ�¼������ͼ��
	// Operations
public:
	//���һ��shape
	void AddShape(int type,int x1,int y1,int x2,int y2);
	void AddShape(CShape* doShape);
	CShape* GetShape(int index);
	void DeleteShape();
	//���һ��������shape
	void AddUndoShape(CShape* undoShape);
	CShape* GetUndoShape(int index);
	void DeleteUndoShape();

	//�����ֻ�ͼ��
	void SendHandShape();
	//���»���
	void OnNetDrawMsg(const char *message);
	void OnDrawPanelBarMSG(int type);

	void Save(QString fileName);
	void Open(QString fileName);

protected:	
	bool m_bLine;//�Ƿ���ֱ��
	bool m_bRect;//�Ƿ��ڻ�����
	bool m_bEill;//�Ƿ��ڻ�Ȧ
	bool m_bHand;//�Ƿ��ֶ�
	bool m_bDrawing;
	QPoint m_Start,m_End;//��¼��ʼ�ͽ����ĵ�


	//����ı�������ͬ���ֻ�
	int m_nHandPoints; //���ٸ���ͬ������һ��
	int m_nCount; //��¼���˶��ٸ���




public:
	void ClearPanel();
	
	// Generated message map functions

	void SetDrawLine();
	void SetDrawRect();
	void SetDrawEill();
	void SetDrawHand();
	void SetDrawPict();
	void RedoDraw();
	void UndoDraw();
	void Draw(QPainter *paint);


protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent*e );

private:
	QtWhiteBoardWidget *pWhiteBoard;
};
