#pragma once

#include <string>
#include <stdint.h>

#include "applog.h"
#include "util.h"

#pragma pack(1)
struct rtp_hdr
{
	uint16_t cc : 4;			/**< CSRC count                     */
	uint16_t x : 1;				/**< header extension flag          */
	uint16_t p : 1;				/**< padding flag                   */
	uint16_t v : 2;				/**< packet type/version            */
	uint16_t pt : 7;			/**< payload type                   */
	uint16_t m : 1;				/**< marker bit                     */
	uint16_t seq;				/**< sequence number                */
	uint32_t ts;				/**< timestamp                      */
	uint32_t ssrc;				/**< synchronization source         */

	uint32_t timeStamp() const { return ntohl(ts); }
	void setTimeStamp(uint32_t t) { ts = htonl(t); }
	uint16_t sequence() const { return ntohs(seq); }
	//uint32_t ssrc() const { return ntohl(ssrc); }
};
#pragma pack()

/// ������ϵͳ���͵���Ƶʱ��
class AudioRtpFilter
{
public:
	AudioRtpFilter() : hop_(0), startTime_(0), needModTs_(true)
	{
		memset(&lastRtpHeader_, 0, sizeof(rtp_hdr));
	}

	bool filter(char* data, int dataLen, int sample_ratio)
	{
		assert(data != NULL);
		if (dataLen < sizeof(rtp_hdr)) {
			LOG_WARN("error rtp package.");
			return true;
		}

		if (sample_ratio == 0) {
			LOG_DEBUG("Need not filter.");
			return true;
		}

		std::string oldTs = toHexString(&data[4], 4);

		//rtp_hdr* curRtpHeader = (rtp_hdr*)&data;
		uint32_t* ps = (uint32_t*)&data[4];
		if (sample_ratio > 0) 
			*ps = htonl(ntohl(*ps)*sample_ratio);
			//curRtpHeader->setTimeStamp(curRtpHeader->timeStamp() * sample_ratio);
		else
			*ps = htonl(ntohl(*ps)/(-sample_ratio));
			//curRtpHeader->setTimeStamp(curRtpHeader->timeStamp() / (-sample_ratio));

		LOG_DEBUG("ratio " << sample_ratio << "; timestamp change from " << oldTs << "; to (" << ntohl(*ps) << ")" << toHexString(&data[4], 4));

		return true;
	}


private:
	rtp_hdr			lastRtpHeader_;
	unsigned int	hop_;			/**ÿ�������������*/
	unsigned int	startTime_;		/**rtpʱ����ʼʱ��*/

	bool			needModTs_;
};


/// ����webrtc����������Ƶ��
class VideoRtpFilter
{
public:
	/**
	*@ desc 
	*@ return true if this package is valid
	*/
	bool filter(char* data, int dataLen) {
		assert(data != NULL);
		if (dataLen < sizeof(rtp_hdr) + 2)
			return true;

		rtp_hdr* curRtpHeader = (rtp_hdr*)&data;

		int nal_type = data[sizeof(rtp_hdr)] & 0x1F;
		if (nal_type > 0 && nal_type < 24) {
			//if (nal_type == 5 || nal_type == 1 || nal_type == 7 || nal_type == 9)
			//curRtpHeader->m = 1;			
			data[1] |= 0x80;
			assert((data[1] & 0x80) == 0x80);
		}
		
		return true;
	}
};