#ifndef CV_STREAMED_MEDIA_SERVICE_H
#define CV_STREAMED_MEDIA_SERVICE_H

#include <QtCore/QtCore>
#include <QSharedMemory>

#include <dbus/channel/type/streamedMedia/common/StreamedMediaServiceCommon.h>
#include <dbus/msdx/common/MsdxServiceCommon.h>
//#include <dbus/msdxclient/common/MsdxClientServiceCommon.h>
#include <dbus/channelDispatcher/common/ChannelDispatcherServiceCommon.h>
#include "util/report/RecordStat.h"
#include "util/report/FilePlayStat.h"
#include "util/BackupManager/BackupManager.h"

#include "msdx/IGraphController.h"

typedef void*  HANDLE;

class CvCoreIf;
class ConferenceRoomIf;
class ChannelDispatcherIf;
class QosReportSender;
class FilePlayCtrlInfo;


class StreamMediaStatus
{
public:
    bool valid;
    //for send and small
    typedef QPair<QString, unsigned short> SendDest;
    QVector<SendDest> video_send_dest;
    QVector<SendDest> audio_send_dest;
    //for recv
    int window_handle;
    int video_width;
    int video_height;
    //for send and recv
    bool enable_video;
    bool enable_audio;

public:
    //friend functions
    friend QDataStream& operator>>(QDataStream& in, StreamMediaStatus& data);
    friend QDataStream& operator<<(QDataStream& out, StreamMediaStatus& data);
};


class CvStreamedMedia:public QObject
{
	Q_OBJECT
public:
	CvStreamedMedia();
	~CvStreamedMedia();

	bool Init( const QString mediaID );

	static void RecordCallback(void * obj, RecStatItem &stat);
  static void FilePlayCallback(void * obj, FilePlayStatItem &stat);


public: // PROPERTIES
public Q_SLOTS: // METHODS
    void Destroy();
    QByteArray GetMediaInfo(int info_type);
    void Pause();
    void Run();
    void SetMediaInfo(int action_id, const QByteArray &input_garray);
    void Stop();
Q_SIGNALS: // SIGNALS
    void QosNofity(const QByteArray &output_garray, int notify_index);
    void RunningStateChanged(const QString &media_id, int current_state);

protected Q_SLOTS:
    void HandleVideoWindowCheckTimerTimeoutSlot();

private:

	//����RTPͳ����Ϣ��Coolview���ƽ���
	static int sendRtpStatInfoToClient(const RtpStatItem& rtpstat);

	/**
	 * @brief	����RTPͳ����Ϣ��Coolview���ƽ���
	 * @param pclass
	 * @param &rtpstat 
	 *@return 0��ʾ���ͳɹ�����������ʧ��
	 */
	static int deliverRtpStatInfo(void * pclass, const RtpStatItem * rtpstat);

  static void initQosReportSender();

	int CreateRecvGraph( const RecvGraphInfo& info );
	int CreateSendGraph( const SendGraphInfo& info );
  int CreateSmallVideoGraph( const RecvGraphInfo& info );
  int CreateFilePlayGraph( const FilePlayInfo& info );

	int AddSendDest(const NetInfo &info);
    int RemoveSendDest(const NetInfo &info);
	void ResetVideoWindow( const int wnd );
    void EnableVideo(bool b);
    void EnableAudio(bool b);
    void SetCurrentState(StreamedMediaState state);
    void ControlPlay(const FilePlayCtrlInfo &info);

    void BackupStatus();
    int RestoreStatus();
    void ResetBackupStatus();

private:

  static QosReportSender *qosReportSenderToCoolview;
  static QosReportSender *qosReportSenderToConfRoom;

	StreamedMediaState		_currentState;
	MediaDirection			_mediaDirection;
	int						_graphID;
	static QString			_mediaID;
	static QString			_userID; //��¼userID����Qos Signal��Ҫ�õ�

	///��¼��Ƶ��ߣ����ڵ�����Ƶ����
	int						_videoWidth;
	int						_videoHeight;

	///������ʾ����ָ��
	static CvStreamedMedia*	_instance;

	ChannelDispatcherIf*	_channelDispatcher;

    //���ز���graph�������°�msdx���ӵģ���˴���ʽ������graph��ͬ ����Liaokz��2015-1
    msdx::IFilePlayGraphController *_file_play_controller;
    //end ���ز���graph

    //����ChannelDispatcher���ݻָ������ڴ�,������ChannelDispatcher����ʱ
    //ֻҪ����һ��StreamMedia���̴���,�ù����ڴ�Ͳ��ᱻ�ر�
    //���ң�CvStreamMediaҲҪ������̵�״̬��Ϣ
    BackupManagerPtr backup_manager_;
    StreamMediaStatus stream_media_status_;

    QTimer video_window_check_timer_; //���ڼ��recv����Ƶ���ڵĴ�С���
    int video_window_handle_;
    
};

#endif