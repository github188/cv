1����Ŀ¼�µ�mpeg4.zip�ǹٷ�ԭ�����ѹ������


* GDCL Mpeg4 Mux �޸�

��һ���֣���дΪһ���������� File Writer �� Filter��
1����Ҫ����MuxOutput::Replace��MuxOutput::FillSpace��������CreateFile��SetFilePointerEx��WriteFile����ԭд����䡣
2��ȥMuxOutput:CBaseOutputPin�̳У��Ĺ��캯������ʵ�޹ؽ�Ҫ����
3��ȥMuxOutput�е�IStream����س�Ա��ȥMpeg4Mux�����MuxOutput:CBaseOutputPin�ĵ��á�
4��Mpeg4Mux��GetPin��GetPinCount���ٿ������Pin

�ڶ����֣����������ͱ����
1�������ָĳ���SCUT Mpeg-4 Multiplexor Writer~
2����Ҫ���޸�SyncIndex::Write��ֻ�д��ڹؼ�֡��������Ӹ�Box
3��MovieWriter::CheckQueues()��Ҫע�͵�һЩ�ж���䣬ʹ�䲻��ѭ�ϸ��Interleaving����֧������Ƶ��ͣ����ʱ�Ĵ���

�������֣������ļ��ָ�ܣ�
ԭ��
a.OnReceiveSample()�з���ֶΣ��ֶ�ʱ����Ҫ�����Ƶ�ؼ�֡���ӹؼ�֡ʱ��㿪ʼ�ָ�
b.�����յ���ĳһTrack��Sampleʱ������ڷָ��ʱ���Ը�Track��EOS��End of stream��������������Track֮���Sample������Movie��Ӧ��Track
c.������Track����EOS����ر�Movie
ʵ�֣�
1��MuxInput���ж�m_pTrack�ĵ��ø�Ϊͨ����Mpeg4Mux::GetTrack��ȡTrackWriterָ��
2���޸�Mpeg4Mux::MakeTrack����Ϊ�������TrackWriter��Mux�ڲ�������Map��¼���Ӧ��MediaType���Ա��´δ���Track
3���޸�Mpeg4Mux::Pause��Mpeg4Mux::Stop����MuxInput::Receive���Ե����µķ���
4������ʵ����ISCUTMpeg4Mux�ӿ�
5��MovieWriter::Close��*pDuration = tDur;��Ϊ*pDuration += tDur;�������ֶε�ʱ���ۻ�

���Ĳ��֣�ʵʱ������Ƶͬ��
1����OnReceiveSample�У�¼���׸��ֶ�ʱ������һ����Ƶ�ؼ�֡�����ʱ�䣬���ڴ�ʱ��֮�����Ƶ֡�ű�¼��

���岿�֣���Է��Ͷ���ͣ����Ƶ���͵Ĵ���
ISO����������ISO-IEC-14496-12��ISO_IEC_14496-14��׼������ELST BOX�ж�������Ϣ��
1���޸�Duration��Add��OffsetTime��WriteEdts����������������룬���ο�Edit list box������ϡ�
2��˵���������ԣ�����������������ELST��Ϣ����Osmo4������֧����Ƶ��ELST��VLC���������ܽϺõĴ���ͷ��empty edit��������������ȫ���ԡ���ͨ��m_bUseElst�������ر������������2013-5

�������֣���Ҫд���Track
�յ�Track����ɲ��ֲ������޷������ļ���
1����TrackWriter������m_lLength��¼д���ֽ�����
2��MovieWriter::Close�����У����ӿչ�����ƣ�������д��չ졣ע�⣺��һ����λд������Ŀ��Ҫ��ȥ�չ���Ŀ��


�ڰ˲��֣������ļ�д�뻺����
���FileWriter

�����޸ģ�
1�������ļ�����ʽ�����������ϲ���ϣ������MuxFilter.cpp��PrintFileName������
2������¼��״̬���档��RecordStat�ṹ�������ݡ�
3��H264ByteStreamHandler::WriteDescriptor�����Ӷ�H.264�����������ʵ��ͼ���С��



