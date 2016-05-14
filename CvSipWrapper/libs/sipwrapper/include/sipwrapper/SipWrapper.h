/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef OWSIPWRAPPER_H
#define OWSIPWRAPPER_H

#include <sipwrapper/EnumPhoneCallState.h>
#include <sipwrapper/EnumPhoneLineState.h>
#include <sipwrapper/EnumTone.h>
#include <sipwrapper/EnumNatType.h>
#include <sipwrapper/EnumVideoQuality.h>
#include <sipwrapper/CodecList.h>

#include <util/Interface.h>
#include <util/Event.h>

#include <string>

#include <pixertool/pixertool.h>
#include <Windows.h>
#include "RtpStat.h"		  //RTPͳ��ͷ�ļ�

using namespace std;

class AudioDevice;
class WebcamVideoFrame;
struct MediaInfo;

/**
 * Wrapper for SIP stacks.
 *
 * A SIP stack is a piece of code that implements the SIP protocol
 * http://www.cs.columbia.edu/sip/
 *
 * Using this interface, different SIP stacks can be implemented
 * like phApi or sipX.
 * Even other voip protocols can be implemented like IAX http://www.asterisk.org/
 *
 * SipWrapper works like a plugin interface. One can imagine to convert SipWrapper implementations
 * (SIP stacks) into .dll (or .so) in order to load them dynamically at runtime.
 * SIP stacks can also be all integrated inside the final application binary.
 * This first solution is ideally the prefered one specially if each SIP stack
 * has a lot of dependencies.
 * The last solution helps when it comes to distribute easily the final application.
 *
 * In order to integrate a new SIP implementation for WengoPhone,
 * create a subdirectory by the name of the SIP implementation.
 * Subclass the SipWrapper interface. SIP events/callbacks are handled via the Event class.
 *
 * Create a factory implementing the interface SipWrapperFactory.
 * Inside main.cpp just instanciates the right factory for your plugin.
 * Check the current SIP wrapper implementations: phApi and sipX.
 *
 * SipWrapper handles telephony and video only.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 * @author Julien Bossart
 */
class SipWrapper : Interface {
public:

	/**
	 * Phone call state changed.
	 *
	 * @param sender this class
	 * @param callId phone call identifier that changed its state
	 * @param state new call state
	 * @param from usually a SIP address (when state = PhoneCallStateIncoming) or empty string
	 */
	Event<void (SipWrapper & sender, int callId,
		EnumPhoneCallState::PhoneCallState state, const std::string & from)> phoneCallStateChangedEvent;

	/**
	 * Phone line state changed.
	 *
	 * @param sender this class
	 * @param lineId phone line identifier that changed its state
	 * @param state new line state
	 */
	Event<void (SipWrapper & sender, int lineId,
		EnumPhoneLineState::PhoneLineState state)> phoneLineStateChangedEvent;

	/**
	 * A video frame has been received from the network.
	 *
	 * @param sender this class
	 * @param callId phone call that received the video frame
	 * @param remoteVideoFrame remote (network) webcam video frame
	 * @param localVideoFrame local webcam video frame
	 */


	/************************************************************************///Add by zhuang 08-12-26
	/* �����¼�����                                                                     */
	//Event<void (SipWrapper & sender, int callId,
	//	piximage* remoteVideoFrame, piximage* localVideoFrame)> videoFrameReceivedEvent;
	Event<void (SipWrapper & sender, int callId,
		void* video_hwnd, piximage* localVideoFrame)> videoFrameReceivedEvent;
	/************************************************************************/



	/**
	 * Incoming SUBSCRIBE request
	 *
	 * @param sender this class
	 * @param sid    Subsciption ID
	 * @param from   subscription originator
	 */
	Event<void (SipWrapper & sender, int sid, const std::string& from, const std::string& evtType)> incomingSubscribeEvent;


	virtual ~SipWrapper() {
	}

	/**
	 * @name Initialization Methods
	 * @{
	 */

	/**
	 * Initializes the SIPWrapper.
	 */
	virtual void init(int ipv6_enable) = 0;

	/**
	 * Terminates the SIP connection.
	 */
	virtual void terminate() = 0;

	/** @} */

	/**
	 * @name Virtual Line Methods
	 * @{
	 */

	/** An error occured while creating a virtual line. */
	static const int VirtualLineIdError = -1;

	/**
	 * Creates and adds a virtual phone line.
	 *
	 * TODO phAddAuthInfo() should be separated from phAddVline()
	 * This method has to be reworked
	 *
	 * @param displayName display name inside the SIP URI (e.g tanguy inside "tanguy <sip:tanguy_k@wengo.fr>")
	 * @param username most of the time same as identity
	 * @param identity first part of the SIP URI (e.g tanguy_k inside "sip:tanguy_k@wengo.fr")
	 * @param password password corresponding to the SIP URI
	 * @param realm realm/domain
	 * @param proxyServer SIP proxy server
	 * @param registerServer SIP register server
	 * @return the corresponding id of the virtual line just created or VirtualLineIdError if failed to create
	 *         the virtual line
	 */
	virtual int addVirtualLine(const std::string & displayName,
		const std::string & username,
		const std::string & identity,
		const std::string & password,
		const std::string & realm,
		const std::string & proxyServer,
		const std::string & registerServer,
		const int address_family) = 0;

	/**
	 * Registers a given virtual phone line.
	 * @brief 2011-06-15����������µĲ������ֱ���content��mime
	 * @param content ע����Ϣ����������
	 * @param mime ���ݶ�Ӧ������
	 * @param	lineId id of the virtual phone line to register
	 * @return	0 if making a register succeeds, otherwise 1
	 */
	virtual int registerVirtualLine(int lineId, const char*content , const char*mime) = 0;

	/**
	 * Removes a given virtual phone line.
	 *
	 * @param	lineId id of the virtual phone line to remove
	 * @param	force if true, forces the removal without waiting for the unregister
				if false, removal is not forced
	 */
	virtual void removeVirtualLine(int lineId, bool force = false) = 0;

	/** @} */

	/**
	 * @name Call Methods
	 * @{
	 */

	/** An error occured while creating a phone call. */
	static const int CallIdError = -1;

	/**
	 * Dials a phone number.
	 *
	 * @param lineId line to use to dial the phone number
	 * @param sipAddress SIP address to call (e.g phone number to dial)
	 * @param enableVideo enable/disable video usage
	 * @return the phone call id (callId)
	 */
	virtual int makeCall(int lineId, const std::string & sipAddress, bool enableVideo) = 0;

	/************************************************************************///Add by zhuang 08-12-23
	/* ���ش˺���                                                           */
	virtual int makeCall2(int lineId, const std::string & sipAddress, bool enableVideo,int width,int height,int bitrate,const char *codecSerial) = 0;
	/************************************************************************/

	/************************************************************************/
	/*Start Conference*/
	virtual void startConference(int lineId,const std::string & confURI,const std::string & szContent)=0;
	virtual int makeJoinConfInvite(int lineId,const std::string & focusURI,const std::string & szContent, const MediaInfo &mediaInfo)=0;
	virtual int media_send_start(char * AudioIP,int AudioPort,char * VideoIP,int VideoPort,HWND previewHwnd,int videoWidth,int videoHeight)=0;
	virtual void media_send_stop(char * AudioIP,int AudioPort,char * VideoIP,int VideoPort)=0;
	virtual int media_recv_start(std::string userId,char * AudioIP,int AudioPort,const char* audioCodecMIME , const int audioSampleRate , const int audioBps , const int audioChannels,char * VideoIP,int VideoPort,char *codecMIME,HWND remoteHwnd,int videoWidth,int videoHeight)=0;
	virtual void media_recv_stop(std::string userId)=0;
	//��Ļ����
	virtual int screen_share_send_start(char * ScreenIP,int ScreenPort,int x,int y,int width,int height)=0;
	//ȡ�����湲��
	virtual void screen_share_send_stop(char * ScreenIP,int ScreenPort)=0;
	//�������湲��
	virtual int screen_share_recv_start(char * ScreenIP,int ScreenPort,HWND remoteHwnd,int videoWidth,int videoHeight)=0;
	//ȡ���������湲��
	virtual void screen_share_recv_stop()=0;
	virtual void exit_conference(const std::string & contactId) = 0;

	Event <void (SipWrapper & sender, const std::string & message,const std::string & from)> cvMessageReceivedEvent;
	Event <void (SipWrapper & sender, const RtpStatItem &rtpstat)> rtpStatRecvEvent;
	Event <void (const TranStat &transtat)> tranStatRecvEvent;

	//video����resize
	virtual void resizeVideoWindow(const std::string &userId,const HWND &remoteHwnd,int fullscreenflag) = 0;

	//��ֹ����
	virtual void forbidSpeak(char * AudioIP,int AudioPort) = 0;

	virtual void forbidSpeak(char * userId) = 0;
	//������
	virtual void permitSpeak(char * AudioIP,int AudioPort) = 0;

	virtual int makeInfoCall(int callId,char * szContent) = 0;

	//ָ�������õĿ�͸�
	virtual void setVideoWidthAndHeight(const int&width, const int &height) = 0;

	
	//��������Ƶ����ͣ...zhenHua.sun 2010-08-06
	/**
	 * @brief ������Ƶ�ķ���
	 * @param state �����true����������Ƶ�ķ��ͣ�����ֹͣ��Ƶ�ķ���
	 */
	virtual void controlAudioSend( bool state ) = 0;
	/**
	 * @brief ������Ƶ�ķ���
	 * @param state �����true����������Ƶ�ķ��ͣ�����ֹͣ��Ƶ�ķ���
	 */
	virtual void controlVideoSend( bool state ) = 0;
	//end

	/************************************************************************/
	
	//......................................................................add by zhenHua.sun 2010-07-17
	/**
	 * @brief ������crossbar filter�йص����ݡ�Crossbarָ�������Ƶ�ɼ�����
	 * @param crossbarName �豸����
	 * @param crossbarInputType �豸����ý������
	 */
	virtual void setCrossBar( const std::string& crossbarName , const std::string& crossbarInputType ) = 0;
	
	//.......................................................................end



	/**
	 * Notifies the remote side (the caller) that this phone is ringing.
	 *
	 * This corresponds to the SIP code "180 Ringing".
	 *
	 * @param callId id of the phone call to make ringing
	 */
	virtual void sendRingingNotification(int callId, bool enableVideo) = 0;

	/**
	 * Accepts a given phone call.
	 *
	 * @param callId id of the phone call to accept
	 * @param enableVideo enable/disable video usage
	 */
	virtual void acceptCall(int callId, bool enableVideo) = 0;

	/************************************************************************///Add by zhuang 08-12-23
	/* ���ش˺���                                                           */
	virtual void acceptCall2(int callId, bool enableVideo,int width,int height,int bitrate,const char *codecSerial) = 0;
	/************************************************************************/
	/**
	 * Rejects a given phone call.
	 *
	 * @param callId id of the phone call to reject
	 */
	virtual void rejectCall(int callId) = 0;

	/**
	 * Closes a given phone call.
	 *
	 * @param callId id of the phone call to close
	 */
	virtual void closeCall(int callId) = 0;

	/**
	 * Holds a given phone call.
	 *
	 * @param callId id of the phone call to hold
	 */
	virtual void holdCall(int callId) = 0;

	/**
	 * Resumes a given phone call.
	 *
	 * @param callId id of the phone call to resume
	 */
	virtual void resumeCall(int callId) = 0;

	/**
	 * Blind transfer the specified call to another party.
	 *
	 * @param callId id of the phone call to transfer
	 * @param sipAddress transfer target
	 */
	virtual void blindTransfer(int callId, const std::string & sipAddress) = 0;

	/**
	 * Sends a DTMF to a given phone call.
	 *
	 * @param callId phone call id to send a DTMF
	 * @param dtmf DTMF tone to send
	 */
	virtual void playDtmf(int callId, char dtmf) = 0;

	/**
	 * Sends and plays a sound file to a given phone call.
	 *
	 * @param callId phone call id to play the sound file
	 * @param soundFile sound file to play
	 */
	virtual void playSoundFile(int callId, const std::string & soundFile) = 0;

	/**
	 * Sets the audio codec list sorted by preference
	 *
	 * @param audioCodecList the audio codec list
	 */
	virtual void setAudioCodecList(const StringList & audioCodecList) = 0;

	/**
	 * Gets the audio codec in use by a given phone call.
	 *
	 * @param callId phone call id
	 * @return audio codec in use
	 */
	virtual CodecList::AudioCodec getAudioCodecUsed(int callId) = 0;

	/**
	 * Gets the video codec in use by a given phone call.
	 *
	 * @param callId phone call id
	 * @return video codec in use
	 */
	virtual CodecList::VideoCodec getVideoCodecUsed(int callId) = 0;

	/**
	 * Sets the video device.
	 *
	 * @param deviceName the name of the video device
	 */
	virtual void setVideoDevice(const std::string & deviceName) = 0;

	/**
	 * Sets calls encrytion.
	 *
	 * @param enable if True encryption is activated
	 */
	virtual void setCallsEncryption(bool enable) = 0;

	/**
	 * @param callId phone call id
	 * @return  True if encrytion is activated for the given call
	 */
	virtual bool isCallEncrypted(int callId) = 0;

	/**
	 * @return  True if is Initialized
	 */

	virtual bool isInitialized(void) = 0;

	/** @} */

	/**
	 * @name Configuration Methods
	 * @{
	 */

	/**
	 * Sets proxy parameters.
	 */
	virtual void setProxy(const std::string & address, unsigned port,
		const std::string & login, const std::string & password) = 0;

	/**
	 * Sets HTTP tunnel parameters.
	 */
	virtual void setTunnel(const std::string & address, unsigned port, bool ssl) = 0;

	/**
	 * Sets the NAT type.
	 *
	 * @see EnumNatType
	 */
	virtual void setNatType(EnumNatType::NatType natType) = 0;

	/**
	 * Sets the NAT type.
	 *
	 * @see EnumVideoQuality
	 */
	virtual void setVideoQuality(EnumVideoQuality::VideoQuality videoQuality) = 0;

	/**
	 * Sets the SIP parameters.
	 */
	virtual void setSIP(const std::string & server, unsigned serverPort, unsigned localPort) = 0;

	/** @} */


	/**
	 * @name Conference Methods
	 * @{
	 */

	/** An error occured while creating a phone call. */
	static const int ConfIdError = -1;

	/**
	 * Creates a new conference given a phone call.
	 *
	 * Conferences are an association of calls
	 * where the audio media is mixed.
	 *
	 * @param callid phone call that will be transformed into a conference call
	 * @return conference id or ConfIdError if an error occured
	 */
	virtual int createConference() = 0;

	/**
	 * Joins (adds) an existing call into a conference.
	 *
	 * @param confId conference id
	 * @param callId phone call id
	 */
	virtual void joinConference(int confId, int callId) = 0;

	/**
	 * Splits (removes) an existing call from a conference.
	 *
	 * @param confId conference id
	 * @param callId existing call to remove from the conference
	 */
	virtual void splitConference(int confId, int callId) = 0;

	/** @} */


	/**
	 * @name Audio Methods
	 * @{
	 */

	/**
	 * Sets the call input device (in-call microphone).
	 *
	 * @param device input device
	 * @return true if no error, false otherwise
	 */
	virtual bool setCallInputAudioDevice(const AudioDevice & device) = 0;

	/**
	 * Sets the call ringer/alerting device.
	 *
	 * @param device ringer device
	 * @return true if no error, false otherwise
	 */
	virtual bool setRingerOutputAudioDevice(const AudioDevice & device) = 0;

	/**
	 * Sets the call output device (in-call speaker).
	 *
	 * @param device output device
	 * @return true if no error, false otherwise
	 */
	virtual bool setCallOutputAudioDevice(const AudioDevice & device) = 0;

	/**
	 * Enables or disables Acoustic Echo Cancellation (AEC).
	 *
	 * @param enable true if AEC enable, false if AEC should be disabled
	 */
	virtual void enableAEC(bool enable) = 0;

	/**
	 * Enables or disables half duplex mode.
	 *
	 * Two modes exist:
	 * - one where microphone signal has priority over speaker signal
	 * - one where speaker signal has priority over microphone signal
	 *
	 * @param enable true if half duplex enable, false if half duplex should be disabled
	 */
	virtual void enableHalfDuplex(bool enable) = 0;

	/**
	 * Enables or disables presence and IM.
	 *
	 * @param enable true to enable presence and IM, false if they should be disabled
	 */
	virtual void enablePIM(bool enable) = 0;

	/**
	 * Plays the designed file (sipX).
	 *
	 * Mix a sound file into the outgoing network audio stream (phApi).
	 *
	 * The file may be a raw 16 bit signed PCM at 8000 samples/sec, mono, little endian (sipX).
	 * RAW audio files containing 16Bit signed PCM sampled at 16KHZ are supported (phApi).
	 *
	 * @param soundFilename sound file to play
	 */
	//virtual playSoundFile(const std::string & soundFilename) = 0;

	/**
	 * Retrieves current codec preference setting.
	 *
	 * @return current codec list supported by the SIP stack
	 */
	//virtual List<AudioCodec> getCodecList() const = 0;

	/**
	 * Sets current codec preference setting.
	 *
	 * @param codecList new codec list
	 */
	//virtual void setCodecList(const List<AudioCodec> & codecList) = 0;

	/** @} */

	/**
	 * @name Video Methods
	 * @{
	 */

	/**
	 * Set video image flip.
	 * This parameter is dynamic so it can be set during a call.
	 *
	 * @param flip if true flip the image
	 */
	virtual void flipVideoImage(bool flip) = 0;

	/** @} */

	/**
	 * Sets the plugins path.
	 *
	 * @param path the absolute path to the plugins (e.g: "/home/robert/plugins")
	 */
	virtual void setPluginPath(const std::string & path) = 0;


	/**
	   
	 */
	virtual void setUaName(const std::string& name) { }
	virtual void setSipOptions(const std::string& optname, const std::string& optval) { }


	virtual void acceptSubscription(int sid) { }
	virtual void rejectSubscription(int sid) { }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief	��QOS��������������. 
	///
	/// @author	qhf
	/// @date	2010-08-18
	///
	/// @param	lineId		Identifier for the line. 
	/// @param	focusURI	URI of the focus. 
	/// @param	szContent	The size content. 
	////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void sendQosPara(int lineId, const std::string & focusURI,const std::string & szContent)=0;
		
	/**
	 *	����dscpֵ
	 * @param dscp ��Чֵ��0-63
	 */
	virtual void setDscp(const int dscp) = 0;

	//����һ��sip��Ϣ��Ŀ��uri��һ����request��Ϣ������MessageId
	virtual int sendSipMessage(int lineId, string remoteURI, string msgContent) =0;

};

#endif	//OWSIPWRAPPER_H
