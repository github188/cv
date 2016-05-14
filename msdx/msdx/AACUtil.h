/************************************************************************/
/* ����AAC�������صĹ��߷���                                          */
/************************************************************************/

#ifndef AAC_UTIL_H
#define AAC_UTIL_H

struct AACConfig;

class AACUtil
{
public:
	static AACConfig GetAACEncConfig();
	static void GetDecoderSpecificInfo(unsigned char **ppBuffer, unsigned long &nSize, int nSampleRate, int nChannels, int nBitsPerSample);
    static void FreeDecoderSpecificBuffer(unsigned char *);

private:
	AACUtil(void);
	~AACUtil(void);
};


#endif
