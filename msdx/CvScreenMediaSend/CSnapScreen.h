#ifndef _H_CSNAPSCREEN_
#define _H_CSNAPSCREEN_
#include <Windows.h>
#include <string>
class ScreenSnapping
{
private:
	
	int left;
	int top;
	int right;
	int bottom;
	int iBitCount;
	int qualityLevel;

   
    HBITMAP copyScreen2Bitmap();
	long getJPEGfromBitmap(HBITMAP hBitmap,BYTE * outImage);
	void jpegCodec(int image_width,int image_height,BYTE * inRGBBuffer,BYTE * outImage,long & length);
	void decodeJpegFile(BYTE * inImage,long inLength, BYTE * outImage,int & width, int & height);

	void RGB32toRGB24(BYTE * image,int width,int height);

	//����λͼ
	int SaveBitmapToFile(HBITMAP hBitmap , LPSTR lpFileName);
    

public:
	int frameRate; //ÿ���֡��
	bool isLoop;

	ScreenSnapping();
	~ScreenSnapping();

	void setPosition(int left,int up,int right,int down);
	void setFrameRate(int FramesPerSecond );
    void setLoop(bool b)
	{
		isLoop=b;
	}
	int getWidth()
	{
		return right-left;
	}
	int getHeight()
	{
		return bottom-top;
	}
	int getBitCount()
	{
		return iBitCount;
	}
	void setQualityLevel(int level)
	{
		qualityLevel=level;
	}
	//get a image in JPEG format 
	long snap(/*in/out*/BYTE * image);

	long snapScreen( );
    
	void deleverImage(BYTE * image,long length);
	
	void setStorageFilePath( const std::string& filePath );

	BYTE*	_imageBuffer;		//ͼƬ����
	int		_imageBufferLength;	//ͼƬ����صĴ�С
	int		_imageLength;		//ͼƬ��ʵ�Ĵ�С
	std::string	_jpegFilePath;	//jpegͼƬ�Ĵ洢·��
};


#endif