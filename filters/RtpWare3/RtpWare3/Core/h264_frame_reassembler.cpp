#include "h264_frame_reassembler.h"
#include "H264/h264_rtp_packer.h"
#include "H264/h264_nal.h"
#include "ortp_packet.h"
#include "ortp/ortp.h"
#include "Log/Log.h"


//
// Util
//

mblk_t *RtpPkt2Msgb(IRtpPacket *pkt) {
  mblk_t *m = nullptr;
  const unsigned int size = pkt->GetPlayloadSize();
  m = allocb(size+10, 0);
  if (m) {
    memcpy(m->b_wptr, pkt->GetPayload(), size);
    m->b_wptr += size;
  }
  return m;
}

//
// H264FrameReassembler
//

H264FrameReassembler::H264FrameReassembler(ILogger *logger)
  : GenericFrameReassembler(logger)
  , packer_(nullptr)
  , is_key_frame_(false)
{
  packer_ = new H264RtpPacker;
}


H264FrameReassembler::~H264FrameReassembler(void)
{
  if (packer_) {
    delete packer_;
    packer_ = nullptr;
  }
}

unsigned long H264FrameReassembler::ReassembleFrameData( 
  unsigned short begin_pos, unsigned short end_pos )
{
  if (!packer_) return 0;

  unsigned char *buf_tail = reassemble_buffer_;
  reassembled_data_size_ = 0;
  is_key_frame_ = false;
  MSQueue nals;
  ms_queue_init(&nals);

  for (unsigned short i = begin_pos; i <= end_pos; ++i) {
    IRtpPacket *pkt = rtp_packet_buffer_[i];
    if (pkt) {
      /* ����˵��һ�£������Ƿ��ͽ����õ���ortp������H264�ı�׼�����������룬����linphone
         ��Դ��Ŀ��һ���֣���API���ܵ����ݽṹ����mblt_t����������Ҫ��һ��ת������ */
      mblk_t *rtpm = nullptr;
      ORtpPacket *ortp_pkt = dynamic_cast<ORtpPacket*>(pkt);
      if (ortp_pkt) {
        rtpm = ortp_pkt->GetMsgb(); //���︴����һ�ݣ�������free����Unpack�л��ͷ�
      } else {
        //dynamic_castʧ�ܣ�����ORtpPacket���󣬲���ֱ��ȡ��mblt_t���ý�һ��mblt_t
        rtpm = RtpPkt2Msgb(pkt);
      }

      if (rtpm) {
        if (i == end_pos) {
          //UnpackҪ���mark��ǣ������ƣ���֪��Ϊʲô���Ͷ�����û��
          mblk_set_marker_info(rtpm, 1);
        }

        unsigned char *payload = nullptr;
        rtp_get_payload(rtpm, &payload); //���RTPͷ���ʵ�������غ�
        rtpm->b_rptr = payload; //�����ݶ�ָ���ƶ����������Ա���ý������

        packer_->Unpack(rtpm, nals);

        while (!ms_queue_empty(&nals)) {
          mblk_t *m = ms_queue_get(&nals);
          msgpullup(m, -1); //mblk_t�ڲ����ݿ��������������÷����������ϳ�һ���ڵ㣬���㸴������

          //����H264 nal��ʼ��
          unsigned long len = sizeof(start_code);
          memcpy_s(buf_tail, reassemble_buffer_size_ - reassembled_data_size_,
            start_code, len);
          buf_tail += len;
          reassembled_data_size_ += len;
          //��������
          len = m->b_wptr - m->b_rptr;
          const unsigned long buf_size_remain = reassemble_buffer_size_ - reassembled_data_size_;
          if (buf_size_remain < len) {
            LOG_WARN_EX(logger_, "reassemble buffer insufficient, need=%uB, remain=%uB",
              len, buf_size_remain);
            len = buf_size_remain;
          }
          memcpy_s(buf_tail, reassemble_buffer_size_ - reassembled_data_size_,
            m->b_rptr, len);
          buf_tail += len;
          reassembled_data_size_ += len;

          //����Ƿ�Ϊ�ؼ�֡����������ݿ鱾�����ǰ���nal���ֵ��ˣ�ֱ����H264Nal���й����ݽṹ
          //I֡�У������һ��I-Slice����nal���ͱ��Ϊ5���ҵ���ȷ������һ��I֡
          H264Nal::NALHeader *nal_header = reinterpret_cast<H264Nal::NALHeader*>(m->b_rptr);
          if (nal_header->nal_unit_type == H264Nal::NALType_IDR) {
            is_key_frame_ = true; 
            //LOG_PRINTF_EX(logger_, "I-Frame received"); // debug
          }

          freemsg(m);
        }
        //ms_queue_flush(&nals); //��������п϶��ǿյ��ˣ��������
        //freemsg(rtpm); //unpack()�л��ͷţ���Ҫ�ٴ��ͷ�
      }

    } else {
      //���������ﲻ����null�����������Ҫ��¼�쳣
      LOG_WARN_EX(logger_, "null pkt found when reassembling frame");
    }
  }

  return reassembled_data_size_;
}

bool H264FrameReassembler::CheckSyncPoint()
{
  return is_key_frame_;
}
