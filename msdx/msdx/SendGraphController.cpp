#include "SendGraphController.h"
#include "LogUtil.h"
#include "GraphUtil.h"
#include "Util.h"
#include "msdx\config.h"

using namespace msdx;

SendGraphController::SendGraphController(void)
	: graph_(nullptr)
	, video_source_(nullptr)
	, video_crossbar_(nullptr)
	, video_inf_pin_tee_(nullptr)
	, video_encoder_(nullptr)
	, video_rtp_sender_(nullptr)
	, video_renderer_(nullptr)
	, audio_source_(nullptr)
	, audio_encoder_(nullptr)
	, audio_rtp_sender_(nullptr)
{
	Reset();
}


SendGraphController::~SendGraphController(void)
{
	Reset();
}

void SendGraphController::Reset()
{
	if (graph_) Stop();
	graph_status_ = DX_GRAPH_STATUS_UNKNOWN;

	SAFE_DELETE_PTR(video_source_);
	SAFE_DELETE_PTR(video_crossbar_);
	SAFE_DELETE_PTR(video_encoder_);
	SAFE_DELETE_PTR(video_rtp_sender_);
	SAFE_DELETE_PTR(video_renderer_);
	SAFE_DELETE_PTR(video_inf_pin_tee_);

	SAFE_DELETE_PTR(audio_source_);
	SAFE_DELETE_PTR(audio_encoder_);
	SAFE_DELETE_PTR(audio_rtp_sender_);

	SAFE_DELETE_PTR(graph_);

	video_rtp_callback_.obj = nullptr;
	video_rtp_callback_.func = nullptr;
	audio_rtp_callback_.obj = nullptr;
	audio_rtp_callback_.func = nullptr;
}

msdx::dxGraphStatus SendGraphController::GetStatus()
{
	return graph_status_;
}

dxStatus SendGraphController::Build(SendGraphParam &param)
{
	graph_param_ = param;
	dxStatus status = DX_ERR_NONE;

	LOG_INFO("Building graph");

	//����graph builder
	status = CreateGraph();
	CHK_FAIL_RETURN(STATUS_ERR(status), status);
	IGraphBuilder *graph_builder = graph_->GetGraphBuilder();

	//Video:
	//  crossbar(if present)-->camera-->RGB24toYUY2(RGB24 only)-->encoder-->video_rtp_sender
	if (!graph_param_.video.device.empty())
	{
		//add camera first
		video_source_ = new VideoSourceContainer;
		if (!video_source_) return DX_ERR_MEMORY_ALLOC;

		status = video_source_->AddToGraph(graph_builder, graph_param_.video.device);
		CHK_FAIL_RETURN(STATUS_ERR(status), status);
		status = video_source_->SetFormat(graph_param_.video.frame_width, 
			graph_param_.video.frame_height, graph_param_.video.fps);
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//crossbar
		if (!graph_param_.video.crossbar.empty())
		{
			video_crossbar_ = new CrossbarContainer;
			if (!video_crossbar_) return DX_ERR_MEMORY_ALLOC;

			if (graph_param_.video.crossbar == MSDX_CONF_CROSSBAR_AUTO)
			{
				status = video_crossbar_->AddFilterAuto(graph_->GetCaptureGraphBuilder2(), 
					video_source_->GetFilter());
				CHK_FAIL_RETURN(STATUS_ERR(status), status);
			} 
			else
			{
				status = video_crossbar_->AddFilter(graph_builder, graph_param_.video.crossbar);
				CHK_FAIL_RETURN(STATUS_ERR(status), status);

				status = video_crossbar_->SetPhysicalType(graph_param_.video.crossbar_physical_type);
				CHK_FAIL_RETURN(STATUS_ERR(status), status);

				//�ɼ����ж�����pin��������ȷ���ĸ�������ͷ���ʲ�Ҫ�ֶ����������������Զ�ѡ��˿�
				status = video_crossbar_->ConnectToVideoSource(video_source_->GetFilter());
				CHK_FAIL_RETURN(STATUS_ERR(status), status);
			}
		}

		CComPtr<IPin> video_out_pin = video_source_->GetOutputPin();

		//�����preview filter�������һ���ַ�filter������Ϊ�˿ɿ�preview filterҪ������
		//����û����capture graph builder2��һ���Ǹ�����̫���ܣ����ÿ��ƣ��ڶ�����˵������
		//��smart tee�ᵼ�»���ߵ�
		if (graph_param_.video.preview.window_handle != 0)
		{
			video_inf_pin_tee_ = new InfPinTeeContainer;
			if (!video_inf_pin_tee_) return DX_ERR_MEMORY_ALLOC;

			video_inf_pin_tee_->AddToGraph(graph_builder, video_out_pin, L"Video Inf");

			video_out_pin = video_inf_pin_tee_->GetUnconnectedPin();
			CHK_FAIL_MSG_RETURN(!video_out_pin, DX_ERR_ABORT, 
				"Get unconnected pin of video inf tee failed.");
		}

		//video encoder
		if (graph_param_.video.enable_hwcodec && Util::CheckIntelHwCodecSupport())
		{
		    video_encoder_ = new LeadH264EncoderContainer;
		}
		else
		{
		video_encoder_ = new ScutH264EncoderContainer;
		}
		//video_encoder_ = new ScutH264EncoderContainer;
		if (!video_encoder_) return DX_ERR_MEMORY_ALLOC;

		//status = video_encoder_->AddToGraph(graph_builder, 
		//    video_out_pin, video_source_->GetType());
		//CHK_FAIL_RETURN(STATUS_ERR(status), status);

		status = video_encoder_->AddToGraph(graph_builder, 
			video_out_pin, video_source_->GetType());
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		VideoEncoderContainer::EncoderParam encoder_param = {0}; //����Ĭ������
		status = video_encoder_->Config(encoder_param);
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//video rtp
		video_rtp_sender_ = new RTPSender2Container;
		if (!video_rtp_sender_) return DX_ERR_MEMORY_ALLOC;

		{  //������������پֲ�������¶
			CComPtr<IPin> encoder_output_pin;
			GraphUtil::GetUnconnectedPin(video_encoder_->GetFilter(), 
				PINDIR_OUTPUT, &encoder_output_pin);
			CHK_FAIL_MSG_RETURN(!encoder_output_pin, DX_ERR_ABORT, "Cannot get video encoder output pin.");

			status = video_rtp_sender_->AddToGraph(graph_builder, 
				RTPSenderContainer::RTP_TYPE_VIDEO, encoder_output_pin);
			CHK_FAIL_RETURN(STATUS_ERR(status), status);
		}

		//video preview
		if (graph_param_.video.preview.window_handle != 0)
		{
			video_renderer_ = new AutoVideoRendererContainer;//ע��˵������������render��ʽ�����kinect�����ʽ���argb32.������run�ķ�ʽȥ���С�hjf 2015.12.14
			if (!video_renderer_) return DX_ERR_MEMORY_ALLOC;

			video_out_pin = video_inf_pin_tee_->GetUnconnectedPin();
			CHK_FAIL_MSG_RETURN(!video_out_pin, DX_ERR_ABORT, "Get unconnected pin of video inf tee failed.");

			status = video_renderer_->AddToGraph(graph_builder, video_out_pin, true);
			CHK_FAIL_RETURN(STATUS_ERR(status), status);

			status = video_renderer_->SetPreviewWindow(graph_param_.video.preview);
			CHK_FAIL_RETURN(STATUS_ERR(status), status);
			//CComPtr<IBaseFilter> pRender;
			//HRESULT hr = GraphUtil::AddFilterByCLSID(graph_builder,L"Render",GraphUtil::clsid_render,&pRender);
			//GraphUtil::ConnectFilters(graph_builder,video_inf_pin_tee_->GetUnconnectedPin(),pRender);
		}
	}

	//Audio:
	//  mic(or AEC)-->encoder-->audio_rtp_sender
	if (!graph_param_.audio.input_device.empty()) 
	{
		//audio source
		if (graph_param_.audio.enable_aec) 
		{
			audio_source_ = new AECSourceContainer;
		} 
		else 
		{
			audio_source_ = new MicrophoneSourceContainer;
		}
		if (!audio_source_) return DX_ERR_MEMORY_ALLOC;

		status = audio_source_->AddFilter(graph_builder, 
			graph_param_.audio.input_device.c_str(), 
			graph_param_.audio.output_device.c_str());
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//����AAC����ʹ�ø��͵��ӳ٣�������40ms�������ԡ���Speex��AEC�ڲ�д����Ҫ80ms
		unsigned int latency = 80;
		if (graph_param_.audio.codec == DX_AUDIO_CODEC_AAC  && !graph_param_.audio.enable_aec) {
			latency = 40;
		}
		status = audio_source_->SetFormat(graph_param_.audio.sample_rate,
			graph_param_.audio.channels, graph_param_.audio.bitcount, latency);
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//audio encoder
		if (graph_param_.audio.codec == DX_AUDIO_CODEC_AAC)
		{
			audio_encoder_ = new AACEncoderContainer;
		} 
		else
		{ //Speex
			audio_encoder_ = new SpeexEncoderContainer;
		}
		if (!audio_encoder_) return DX_ERR_MEMORY_ALLOC;

		status = audio_encoder_->AddFilter(graph_builder,
			audio_source_->GetOutputPin(), audio_source_->GetType());
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//audio rtp
		audio_rtp_sender_ = new RTPSender2Container;
		if (!audio_rtp_sender_) return DX_ERR_MEMORY_ALLOC;

		{  //���������ڼ��پֲ�������©
			CComPtr<IPin> encoder_output_pin;
			GraphUtil::GetUnconnectedPin(audio_encoder_->GetFilter(), 
				PINDIR_OUTPUT, &encoder_output_pin);
			CHK_FAIL_MSG_RETURN(!encoder_output_pin, DX_ERR_ABORT, "Cannot get audio encoder output pin.");

			status = audio_rtp_sender_->AddToGraph(graph_builder, 
				RTPSenderContainer::RTP_TYPE_AUDIO, encoder_output_pin);
			CHK_FAIL_RETURN(STATUS_ERR(status), status);
		}
	}

	graph_status_ = DX_GRAPH_STATUS_INITIALIZED;
	return DX_ERR_NONE;
}

msdx::dxStatus SendGraphController::CreateGraph()
{
	graph_ = new GraphContainer;
	CHK_FAIL_MSG_RETURN(!graph_, DX_ERR_MEMORY_ALLOC, "Create graph container failed");

	dxStatus status = graph_->InitializeGraph();
	CHK_FAIL_RETURN(!graph_, DX_ERR_MEMORY_ALLOC);

	graph_->AddToRot(L"Send");

	return DX_ERR_NONE;
}

msdx::dxStatus SendGraphController::Run()
{
	CHK_FAIL_MSG_RETURN((!graph_ || !graph_->GetMediaControl()), 
		DX_ERR_ABORT, "Run graph failed: graph not initialized.");
	CHK_FAIL_MSG_RETURN(DX_GRAPH_STATUS_RUNNING == graph_status_, DX_ERR_NONE, "Graph already running.");

	IMediaControl *media_control = graph_->GetMediaControl();
	HRESULT hr = media_control->Run();
	CHK_FAIL_MSG_RETURN(FAILED(hr), DX_ERR_SYS, "Run graph failed: 0x%08x", hr);

	LOG_INFO("Graph started");
	graph_status_ = DX_GRAPH_STATUS_RUNNING;
	return DX_ERR_NONE;
}

msdx::dxStatus SendGraphController::Stop()
{
	if (!graph_ || !graph_->GetMediaControl()) return DX_ERR_NONE;
	IMediaControl *media_control = graph_->GetMediaControl();

	HRESULT hr = media_control->Stop();
	CHK_FAIL_MSG_RETURN(FAILED(hr), DX_ERR_SYS, "Stop graph failed: 0x%08x", hr);

	LOG_INFO("Graph stopped");
	graph_status_ = DX_GRAPH_STATUS_STOPPED;
	return DX_ERR_NONE;
}

msdx::dxStatus SendGraphController::AddVideoDestination( const NetAddr &addr )
{
	if (!video_rtp_sender_)
	{
		LOG_WARN("Add video destination failed: null video rtp sender.");
		return DX_WARN_OP_IGNORED;
	}
	return video_rtp_sender_->AddDest(addr, &video_rtp_callback_);
}

msdx::dxStatus SendGraphController::DeleteVideoDestination( const NetAddr &addr )
{
	if (!video_rtp_sender_)
	{
		LOG_WARN("Delete video destination failed: null video rtp sender.");
		return DX_WARN_OP_IGNORED;
	}
	return video_rtp_sender_->RemoveDest(addr);
}

msdx::dxStatus SendGraphController::AddAudioDestination( const NetAddr &addr )
{
	if (!audio_rtp_sender_)
	{
		LOG_WARN("Add audio destination failed: null audio rtp sender.");
		return DX_WARN_OP_IGNORED;
	}
	return audio_rtp_sender_->AddDest(addr, &video_rtp_callback_);
}

msdx::dxStatus SendGraphController::DeleteAudioDestination( const NetAddr &addr )
{
	if (!audio_rtp_sender_)
	{
		LOG_WARN("Delete audio destination failed: null audio rtp sender.");
		return DX_WARN_OP_IGNORED;
	}
	return audio_rtp_sender_->RemoveDest(addr);
}

msdx::dxStatus SendGraphController::StopSendingVideo( bool stop )
{
	//��Ƶ�����֡������ǰһ֡���������ضϲ��ָ�������ɽ��ն˿���������
	//����ֹͣ��Ƶ����Ҫ��Ա���������
	CHK_FAIL_MSG_RETURN(!video_encoder_, DX_ERR_ABORT, "Cannot stop null video encoder.");

	return video_encoder_->PauseEncoding(stop);
}

msdx::dxStatus SendGraphController::StopSendingAudio( bool stop )
{
	//������Ƶ����Ϊ�޹ؼ�֡�����Կ�����ʱ�ָ����ͣ�
	//ͬʱΪ�˼򻯹�����������ÿ�ֱ��������һ�����ƽӿڣ���ֹͣ������Ƶ���ý����rtpģ��
	CHK_FAIL_MSG_RETURN(!audio_rtp_sender_, DX_ERR_ABORT, "Cannot stop null audio rtp sender.");

	return audio_rtp_sender_->PauseOverallSending(stop);
}

msdx::dxStatus SendGraphController::SetVideoRtpStatCallBack( void *obj, RtpStatCallback func )
{
	video_rtp_callback_.obj = obj;
	video_rtp_callback_.func = func;
	return DX_ERR_NONE;
}

msdx::dxStatus SendGraphController::SetAudioRtpStatCallBack( void *obj, RtpStatCallback func )
{
	audio_rtp_callback_.obj = obj;
	audio_rtp_callback_.func = func;
	return DX_ERR_NONE;
}

msdx::dxStatus TestSendGraphController::Build(SendGraphParam& param)
{
	graph_param_ = param;
	dxStatus status = DX_ERR_NONE;

	LOG_INFO("Building graph");

	//����graph builder
	status = CreateGraph();
	CHK_FAIL_RETURN(STATUS_ERR(status), status);
	IGraphBuilder *graph_builder = graph_->GetGraphBuilder();

	//Video:
	//  crossbar(if present)-->camera-->ffdshow-->encoder-->video_rtp_sender
	if (!graph_param_.video.device.empty())
	{
		//add camera first
		video_source_ = new VideoSourceContainer;
		if (!video_source_) return DX_ERR_MEMORY_ALLOC;

		status = video_source_->AddToGraph(graph_builder, graph_param_.video.device);
		CHK_FAIL_RETURN(STATUS_ERR(status), status);
		status = video_source_->SetFormat(graph_param_.video.frame_width, 
			graph_param_.video.frame_height, graph_param_.video.fps);
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//crossbar
		if (!graph_param_.video.crossbar.empty())
		{
			video_crossbar_ = new CrossbarContainer;
			if (!video_crossbar_) return DX_ERR_MEMORY_ALLOC;

			if (graph_param_.video.crossbar == MSDX_CONF_CROSSBAR_AUTO)
			{
				status = video_crossbar_->AddFilterAuto(graph_->GetCaptureGraphBuilder2(), 
					video_source_->GetFilter());
				CHK_FAIL_RETURN(STATUS_ERR(status), status);
			} 
			else
			{
				status = video_crossbar_->AddFilter(graph_builder, graph_param_.video.crossbar);
				CHK_FAIL_RETURN(STATUS_ERR(status), status);

				status = video_crossbar_->SetPhysicalType(graph_param_.video.crossbar_physical_type);
				CHK_FAIL_RETURN(STATUS_ERR(status), status);

				//�ɼ����ж�����pin��������ȷ���ĸ�������ͷ���ʲ�Ҫ�ֶ����������������Զ�ѡ��˿�
				status = video_crossbar_->ConnectToVideoSource(video_source_->GetFilter());
				CHK_FAIL_RETURN(STATUS_ERR(status), status);
			}
		}

		CComPtr<IPin> video_out_pin = video_source_->GetOutputPin();

		//�����preview filter�������һ���ַ�filter������Ϊ�˿ɿ�preview filterҪ������
		//����û����capture graph builder2��һ���Ǹ�����̫���ܣ����ÿ��ƣ��ڶ�����˵������
		//��smart tee�ᵼ�»���ߵ�
		//if (graph_param_.video.preview.window_handle != 0)
		//{
		//	video_inf_pin_tee_ = new InfPinTeeContainer;
		//	if (!video_inf_pin_tee_) return DX_ERR_MEMORY_ALLOC;

		//	video_inf_pin_tee_->AddToGraph(graph_builder, video_out_pin, L"Video Inf");

		//	video_out_pin = video_inf_pin_tee_->GetUnconnectedPin();
		//	CHK_FAIL_MSG_RETURN(!video_out_pin, DX_ERR_ABORT, 
		//		"Get unconnected pin of video inf tee failed.");
		//}


		video_encoder_ = new LeadH264EncoderContainer;
		if (!video_encoder_) return DX_ERR_MEMORY_ALLOC;

		//status = video_encoder_->AddToGraph(graph_builder, 
		//    video_out_pin, video_source_->GetType());
		//CHK_FAIL_RETURN(STATUS_ERR(status), status);

		status = video_encoder_->AddToGraph(graph_builder, 
			video_out_pin, video_source_->GetType());
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		VideoEncoderContainer::EncoderParam encoder_param = {0}; //����Ĭ������
		status = video_encoder_->Config(encoder_param);                             //leadò�Ʋ��ܻ�ȡ�ӿڣ���ע��
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//video rtp
		video_rtp_sender_ = new RTPSender2Container;
		if (!video_rtp_sender_) return DX_ERR_MEMORY_ALLOC;

		{  //������������پֲ�������¶
			CComPtr<IPin> encoder_output_pin;
			GraphUtil::GetUnconnectedPin(video_encoder_->GetFilter(), 
				PINDIR_OUTPUT, &encoder_output_pin);
			CHK_FAIL_MSG_RETURN(!encoder_output_pin, DX_ERR_ABORT, "Cannot get video encoder output pin.");

			status = video_rtp_sender_->AddToGraph(graph_builder, 
				RTPSenderContainer::RTP_TYPE_VIDEO, encoder_output_pin);
			CHK_FAIL_RETURN(STATUS_ERR(status), status);
		}

		//video preview
		if (graph_param_.video.preview.window_handle != 0)
		{
			//video_renderer_ = new AutoVideoRendererContainer;//ע��˵������������render��ʽ�����kinect�����ʽ���argb32.������run�ķ�ʽȥ���С�hjf 2015.12.14
			//if (!video_renderer_) return DX_ERR_MEMORY_ALLOC;

			//video_out_pin = video_inf_pin_tee_->GetUnconnectedPin();
			//CHK_FAIL_MSG_RETURN(!video_out_pin, DX_ERR_ABORT, "Get unconnected pin of video inf tee failed.");

			//status = video_renderer_->AddToGraph(graph_builder, video_out_pin, true);
			//CHK_FAIL_RETURN(STATUS_ERR(status), status);

			//status = video_renderer_->SetPreviewWindow(graph_param_.video.preview);
			//CHK_FAIL_RETURN(STATUS_ERR(status), status);

			//hjf
			//CComPtr<IBaseFilter> pRender;
			//HRESULT hr = GraphUtil::AddFilterByCLSID(graph_builder,L"Render",GraphUtil::clsid_render,&pRender);
			//GraphUtil::ConnectFilters(graph_builder,video_inf_pin_tee_->GetUnconnectedPin(),pRender);
		}
	}

	//Audio:
	//  mic(or AEC)-->encoder-->audio_rtp_sender
	if (!graph_param_.audio.input_device.empty()) 
	{
		//audio source
		if (graph_param_.audio.enable_aec) 
		{
			audio_source_ = new AECSourceContainer;
		} 
		else 
		{
			audio_source_ = new MicrophoneSourceContainer;
		}
		if (!audio_source_) return DX_ERR_MEMORY_ALLOC;

		status = audio_source_->AddFilter(graph_builder, 
			graph_param_.audio.input_device.c_str(), 
			graph_param_.audio.output_device.c_str());
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//����AAC����ʹ�ø��͵��ӳ٣�������40ms�������ԡ���Speex��AEC�ڲ�д����Ҫ80ms
		unsigned int latency = 80;
		if (graph_param_.audio.codec == DX_AUDIO_CODEC_AAC  && !graph_param_.audio.enable_aec) {
			latency = 40;
		}
		status = audio_source_->SetFormat(graph_param_.audio.sample_rate,
			graph_param_.audio.channels, graph_param_.audio.bitcount, latency);
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//audio encoder
		if (graph_param_.audio.codec == DX_AUDIO_CODEC_AAC)
		{
			audio_encoder_ = new AACEncoderContainer;
		} 
		else
		{ //Speex
			audio_encoder_ = new SpeexEncoderContainer;
		}
		if (!audio_encoder_) return DX_ERR_MEMORY_ALLOC;

		status = audio_encoder_->AddFilter(graph_builder,
			audio_source_->GetOutputPin(), audio_source_->GetType());
		CHK_FAIL_RETURN(STATUS_ERR(status), status);

		//audio rtp
		audio_rtp_sender_ = new RTPSender2Container;
		if (!audio_rtp_sender_) return DX_ERR_MEMORY_ALLOC;

		{  //���������ڼ��پֲ�������©
			CComPtr<IPin> encoder_output_pin;
			GraphUtil::GetUnconnectedPin(audio_encoder_->GetFilter(), 
				PINDIR_OUTPUT, &encoder_output_pin);
			CHK_FAIL_MSG_RETURN(!encoder_output_pin, DX_ERR_ABORT, "Cannot get audio encoder output pin.");

			status = audio_rtp_sender_->AddToGraph(graph_builder, 
				RTPSenderContainer::RTP_TYPE_AUDIO, encoder_output_pin);
			CHK_FAIL_RETURN(STATUS_ERR(status), status);
		}
	}

	graph_status_ = DX_GRAPH_STATUS_INITIALIZED;
	return DX_ERR_NONE;
}