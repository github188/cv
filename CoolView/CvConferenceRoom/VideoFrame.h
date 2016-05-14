#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H

#include <QtGui/QtGui>
#include "QtConfrenceVideoWindow.h"//Add By LZY 2010-10-09


class VideoFrame : public QFrame
{
	Q_OBJECT
public:
    VideoFrame(QWidget *parent );
	~VideoFrame();

	/**
	 * @brief �ͷŵ�ǰ��������
	 */
	void Release();

protected:
	void resizeEvent( QResizeEvent *event );

public:
  QPixmap *pixmap;
	bool isUsed;
	QGridLayout layout;
	QtConfrenceVideoWindow *LinkWin;//Add By LZY 2010-10-09 ��ӹ����Ĵ���ָ��

	int		_screenIndex;		//������Ļ
	int		_seet;				//����λ��

	int		_videoWidth;		//��Ƶ����ʵ�ʸ߶�
	int		_videoHeight;		//��Ƶ����ʵ�ʸ߶�
	bool	_showRealResolution;	//��ʾ��ʵ����Ƶ����С

  QFrame* _streamIdleFrame; //û�н��ܶ���ʱ��ʾ��ͼƬ
};

#endif // VIDEOFRAME_H
