

#include "CSnapScreen.h"
#include "jpeglib.h"
#include <io.h>
#include <stdio.h>

ScreenSnapping::ScreenSnapping()
{
	_imageBuffer = NULL;
	_imageBufferLength = 0;

	isLoop=true;
	qualityLevel=50;
	setPosition(0,0,0,0);
	setFrameRate(1); //ÿ���֡Ƶ

	_jpegFilePath = "";

}


ScreenSnapping::~ScreenSnapping()
{
	if( _imageBuffer )
	{
		delete _imageBuffer;
		_imageBuffer = NULL;
	}
	DeleteFile( _jpegFilePath.c_str() );
}

void ScreenSnapping::deleverImage(BYTE * image,long length)
{

	//if(pFilter->m_pOutputPin->IsConnected()==false) return ;
	//BYTE * pBuffer;
	//BOOL hr=pFilter->GetSampleBuffer(&pBuffer);
	//if(!hr) return ;
	//memcpy(pBuffer,image,length);
	//pFilter->DeliverHoldingSample(length);
}
void ScreenSnapping::setPosition(int inLeft,int inUp,int inRight,int inDown)
{
	bool captureWindowChange = false;
	if( inLeft!=left||inUp!=top||inRight!=right||inDown!=bottom )
		captureWindowChange = true;

	if( captureWindowChange )
	{
		
		left=inLeft;
		top=inUp;
		right=inRight;
		bottom=inDown;

		printf("Reset Capture Window Area[%d,%d,%d,%d]\n" , left , top , right , bottom );
		if( _imageBuffer )
		{
			//�ͷ�ԭ�еĻ���
			delete _imageBuffer;
			_imageBuffer = NULL;
			_imageBufferLength = 0;
		}

		if( left>=0 && top>=0 && right>=0 &&bottom>=0 &&getWidth()>0 && getHeight()>0 )
		{
			printf("Window width:%d , hight:%d\n" , this->getWidth() , this->getHeight() );
			_imageBufferLength = this->getWidth()*this->getHeight()*3;
			_imageBuffer = new BYTE[ _imageBufferLength ];
			ZeroMemory( _imageBuffer , _imageBufferLength );
		}
	}

}


void ScreenSnapping::setFrameRate(int framerate)
{
	frameRate=framerate;
}



long ScreenSnapping::snap(BYTE * image)
{
	
	long length=0;
	

	if( _imageBuffer )
	{
		HBITMAP hBitmap=copyScreen2Bitmap();
		length=getJPEGfromBitmap(hBitmap,_imageBuffer);
		DeleteObject(hBitmap);
	}

	return length;
}


long ScreenSnapping::snapScreen()
{
	long length=0;
	
	if( _imageBuffer )
	{
		ZeroMemory( _imageBuffer , _imageBufferLength );
		HBITMAP hBitmap=copyScreen2Bitmap();
		if( hBitmap==NULL )
		{
			printf("BMP����ʧ�ܣ�\n");
		}

		length=getJPEGfromBitmap(hBitmap,_imageBuffer);
		//this->SaveBitmapToFile( hBitmap , "D:\\bitmap.bmp" );
		_imageLength = length;
		DeleteObject(hBitmap);
		return length;
	}else
	{
		printf("ͼƬ����Ϊ�գ�\n");
	}

	return -1;
}

void ScreenSnapping::RGB32toRGB24(BYTE * image,int width,int height)
{
	BYTE * temp=new BYTE[width*height*3];

	for(int i=0,j=0;i<width*height*4;i+=4,j+=3)
	{
		temp[j]=image[i];
		temp[j+1]=image[i+1];
		temp[j+2]=image[i+2];
	}
	memcpy(image,temp,width*height*3);

	delete []temp;
//	for(long i=0,j=0;i<width*height*4;i+=4,j+=3)
//	{
//		temp[j]=image[i+2];
//		temp[j+1]=image[i+1];
//		temp[j+2]=image[i];
//	}
//	    
//   //���з�ת����
//	for(int i=0;i<height;i++)
//		memcpy(image+i*width*3,temp+(height-1-i)*width*3,width*3);
//	delete []temp;
}

HBITMAP ScreenSnapping::copyScreen2Bitmap()
{
	
	// ��Ļ���ڴ��豸������
	HDC       hScrDC = NULL, hMemDC = NULL;      
	// λͼ���
	HBITMAP    hBitmap, hOldBitmap;   
	
	// ѡ����������
	int       nX, nY, nX2, nY2;      

	// λͼ��Ⱥ͸߶�
	int       nWidth, nHeight;      
	
    // ��Ļ�ֱ���
	int       xScrn, yScrn; 	

	DWORD deviceNum = 0;
	DISPLAY_DEVICE device;
	device.cb = sizeof(DISPLAY_DEVICE );
	// ���ѡ����������
	nX = left;
	nY = top;
	nX2 = right;
	nY2 = bottom;

	//ѡȡ��ʾ��
	//while( EnumDisplayDevices(NULL , deviceNum++ , &device,NULL ) )
	//{
	//	hScrDC = CreateDC( device.DeviceName , NULL , NULL , NULL );
	//	// �����Ļ�ֱ���
	//	xScrn = GetDeviceCaps(hScrDC, HORZRES);
	//	yScrn = GetDeviceCaps(hScrDC, VERTRES);
	//	
	//	//�����߽��x������ڻ���ڵ�ǰ��Ļ�ֱ��ʣ���ѡ����һ��device(���ڶ����Ļ)
	//	//ͬʱ����x���궼��ȥ��ǰ����Ļˮƽ�ֱ���
	//	if( nX>=xScrn )
	//	{
	//		printf("��%d����ʾ���ķֱ���%d,%d", deviceNum,  xScrn, yScrn );
	//		nX -= xScrn;
	//		nX2 -= xScrn;
	//		printf("��ǰ�Ĵ�������%d,%d,%d,%d\n" , nX , nY , nX2, nY2 );
 //           DeleteDC(hScrDC);
	//	}else
	//	{
	//		//���õ�ǰ���豸
	//		break;
	//	}
	//}
    hScrDC = GetDC(NULL);
	if( hScrDC==NULL )
	{
		printf("��ȡ��Ļ�豸������ʱ��������!\n");
		return NULL;
	}

	//printf("�Ե�%d����ʾ�����в���������Ϊ%s\n", deviceNum , device.DeviceName );
	printf("��������Ϊ%d,%d,%d,%d\n" , nX , nY , nX2, nY2 );


	////Ϊ��Ļ�����豸������
	////hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	////int iBits = GetDeviceCaps(hScrDC, BITSPIXEL) * GetDeviceCaps(hScrDC, PLANES);

	//Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
	hMemDC = CreateCompatibleDC(hScrDC);


	// �����Ļ�ֱ���
	//xScrn = GetDeviceCaps(hScrDC, HORZRES);
	//yScrn = GetDeviceCaps(hScrDC, VERTRES);

	//printf("��Ļ�ֱ���:%d x %d\n" , xScrn , yScrn );
	////ȷ��ѡ�������ǿɼ���
	//if (nX<0)	nX = 0;
	//if (nY<0)	nY = 0;
	//if (nX2>xScrn)
	//{
	//	nX2 = xScrn;
	//	right=xScrn;
	//}
	//if (nY2>yScrn)
	//{
	//	nY2 = yScrn;
	//	bottom=yScrn;
	//}

	nWidth = nX2 - nX;
	nHeight = nY2 - nY;

	// ����һ������Ļ�豸��������ݵ�λͼ
	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
	// ����λͼѡ���ڴ��豸��������
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// ����Ļ�豸�����������ڴ��豸��������
	//BitBlt(hMemDC, 0, 0, 320, 240,hScrDC, nX, nY, SRCCOPY);
	BitBlt(hMemDC, 0, 0, nWidth, nHeight,hScrDC, nX, nY, SRCCOPY);
	//�õ���Ļλͼ�ľ��
	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);	
	//��� 
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	DeleteObject(hOldBitmap);
	// ����λͼ���
	return hBitmap;
}

void ScreenSnapping::jpegCodec(int image_width,int image_height, BYTE * inRGBBuffer,BYTE * outImage,long & length)
{
	struct jpeg_compress_struct cinfo;
	
	struct jpeg_error_mgr jerr;
	/* More stuff */

	//FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);
	
	/*if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}*/
	
	FILE * outfile = NULL;
	if( _jpegFilePath!="" )
	{	
		//����jpeg�ļ�
		outfile=fopen(_jpegFilePath.c_str(),"wb");
		if (outfile==NULL) 
		{
			printf("���ļ�[%s]ʧ��\n" ,_jpegFilePath.c_str());
			return;
		}

	}

	jpeg_stdio_dest(&cinfo, outfile);
	//jpeg_stdio_dest(&cinfo, outImage);
	
	cinfo.image_width = image_width; 	/* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	
	jpeg_set_defaults(&cinfo);
	
	jpeg_set_quality(&cinfo, qualityLevel, TRUE /* limit to baseline-JPEG values */);

	
	jpeg_start_compress(&cinfo, TRUE);


	//����BMPͼ���Ƿ�ת�ģ�ÿ�����صı�ʾ�����BGR�������Ҫ���������û�
	for( int i=0 ; i<image_width*image_height*3; i+=3 )
	{
		BYTE temp = *(inRGBBuffer+i);
		*(inRGBBuffer+i)=*(inRGBBuffer+i+2);
		*(inRGBBuffer+i+2)=temp;
	}



	row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */
	while (cinfo.next_scanline < cinfo.image_height) {
		//����jpg�ļ���ͼ���ǵ��ģ����Ը���һ�¶���˳��
		row_pointer[0] = &inRGBBuffer[(cinfo.image_height - cinfo.next_scanline - 1) * row_stride];
		//row_pointer[0] = & inRGBBuffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	
	jpeg_finish_compress(&cinfo);
	

	length = _filelength(fileno(outfile));

	fclose(outfile );

	outfile = fopen( _jpegFilePath.c_str() , "rb" );
	if( outfile!=NULL )
	{
		int size = fread( outImage , length , 1 , outfile );
	}

	jpeg_destroy_compress(&cinfo);
	
	fclose(outfile);
	
}


long ScreenSnapping::getJPEGfromBitmap(HBITMAP hBitmap,BYTE * outImage)
{
	 if(hBitmap==NULL ||outImage==NULL) return 0;

 /*
	  //lpFileName Ϊλͼ�ļ���
	HDC            hDC;         
	//�豸������
	int            iBits;      
	//��ǰ��ʾ�ֱ�����ÿ��������ռ�ֽ���
	WORD            wBitCount;   
	//λͼ��ÿ��������ռ�ֽ���
	//�����ɫ���С�� λͼ�������ֽڴ�С ��
	//λͼ�ļ���С �� д���ļ��ֽ���
	DWORD           dwPaletteSize=0,dwBmBitsSize;
	BITMAP          Bitmap;        
    
	//λͼ�ļ�ͷ�ṹ
	BITMAPINFOHEADER   bi;            
	//λͼ��Ϣͷ�ṹ 
	LPBITMAPINFOHEADER lpbi;          
	//ָ��λͼ��Ϣͷ�ṹ
	HANDLE           hPal,hOldPal=NULL;
	//�����ļ��������ڴ�������ɫ����

	//����λͼ�ļ�ÿ��������ռ�ֽ���
	hDC = CreateDC("DISPLAY",NULL,NULL,NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);

	if(iBits<16) return 0;
	wBitCount=iBits;

	//�����ɫ���С
	if (wBitCount<= 8)
		dwPaletteSize = (1<<wBitCount) *
		sizeof(RGBQUAD);

	//����λͼ��Ϣͷ�ṹ
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	bi.biSize            = sizeof(BITMAPINFOHEADER);
	bi.biWidth           = Bitmap.bmWidth;
	bi.biHeight          = Bitmap.bmHeight;
	bi.biPlanes          = 1;
	bi.biBitCount         = wBitCount;
	bi.biCompression      = BI_RGB;
	bi.biSizeImage        = 0;
	bi.biXPelsPerMeter     = 0;
	bi.biYPelsPerMeter     = 0;
	bi.biClrUsed         = 0;
	bi.biClrImportant      = 0;

	dwBmBitsSize = ((Bitmap.bmWidth *wBitCount+31)/32)* 4*Bitmap.bmHeight ;
	//Ϊλͼ���ݷ����ڴ�
	//hDib  = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize);

	//lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	lpbi=(LPBITMAPINFOHEADER) new BYTE[dwBmBitsSize+dwPaletteSize];

	*lpbi = bi;

	//// �����ɫ��   
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC  = GetDC(NULL);
		hOldPal = SelectPalette(hDC,(HPALETTE) hPal, FALSE);
		RealizePalette(hDC);
	}
	// ��ȡ�õ�ɫ�����µ�����ֵ
	int temp=GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight,	(LPSTR)lpbi,(BITMAPINFO *)	&bi, DIB_RGB_COLORS);		

	 if (hOldPal)
	{
      SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
    }

	long length=0;   	
    if(wBitCount==32)	RGB32toRGB24((BYTE *)lpbi,Bitmap.bmWidth,Bitmap.bmHeight);
    if(wBitCount==16)  ;
*/
	 HDC hDC;
	 //��ǰ�ֱ�����ÿ������ռ�ֽ���
	 int iBits;
	 //λͼ��ÿ������ռ�ֽ���
	 WORD wBitCount;
	 //�����ɫ���С�� λͼ�������ֽڴ�С ��λͼ�ļ���С �� д���ļ��ֽ��� 
	 DWORD dwPaletteSize=0, dwBmBitsSize=0, dwDIBSize=0, dwWritten=0; 
	 //λͼ���Խṹ 
	 BITMAP Bitmap;  
	 //λͼ�ļ�ͷ�ṹ
	 BITMAPFILEHEADER bmfHdr;  
	 //λͼ��Ϣͷ�ṹ 
	 BITMAPINFOHEADER bi;  
	 //ָ��λͼ��Ϣͷ�ṹ  
	 LPBITMAPINFOHEADER lpbi;  
	 //�����ļ��������ڴ�������ɫ���� 
	 HANDLE fh, hDib, hPal,hOldPal=NULL; 
	 //ѹ����jpeg���ͼƬ���ݴ�С
	 long length = 0;

	 //����λͼ�ļ�ÿ��������ռ�ֽ��� 
	 hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	 iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES); 
	 DeleteDC(hDC); 
	 if (iBits <= 1)  wBitCount = 1; 
	 else if (iBits <= 4)  wBitCount = 4; 
	 else if (iBits <= 8)  wBitCount = 8; 
	 else      wBitCount = 24; 

	 GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	 bi.biSize   = sizeof(BITMAPINFOHEADER);
	 bi.biWidth   = Bitmap.bmWidth;
	 bi.biHeight   = Bitmap.bmHeight;
	 bi.biPlanes   = 1;
	 bi.biBitCount  = wBitCount;
	 bi.biCompression = BI_RGB;
	 bi.biSizeImage  = 0;
	 bi.biXPelsPerMeter = 0;
	 bi.biYPelsPerMeter = 0;
	 bi.biClrImportant = 0;
	 bi.biClrUsed  = 0;

	 dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	 //Ϊλͼ���ݷ����ڴ� 
	 hDib = GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER)); 
	 lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib); 
	 *lpbi = bi;

	 // �����ɫ��  
	 hPal = GetStockObject(DEFAULT_PALETTE); 
	 if (hPal) 
	 { 
		 hDC = ::GetDC(NULL); 
		 hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE); 
		 RealizePalette(hDC); 
	 }

	 // ��ȡ�õ�ɫ�����µ�����ֵ 
	 GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) 
		 +dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS); 

	 //�ָ���ɫ��  
	 if (hOldPal) 
	 { 
		 ::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE); 
		 RealizePalette(hDC); 
		 ::ReleaseDC(NULL, hDC); 
	 }


	 // ����λͼ�ļ�ͷ 
	 bmfHdr.bfType = 0x4D42; // "BM" 
	 dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;  
	 bmfHdr.bfSize = dwDIBSize; 
	 bmfHdr.bfReserved1 = 0; 
	 bmfHdr.bfReserved2 = 0; 
	 bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize; 

     //jpegѹ��
	jpegCodec(Bitmap.bmWidth,Bitmap.bmHeight,(BYTE *)lpbi+ sizeof(BITMAPINFOHEADER)+dwPaletteSize,outImage,length);
	
	//���  
	GlobalUnlock(hDib); 
	GlobalFree(hDib); 

	return length;
}

int ScreenSnapping::SaveBitmapToFile( HBITMAP hBitmap , LPSTR lpFileName )
{     
	HDC hDC;
	//��ǰ�ֱ�����ÿ������ռ�ֽ���
	int iBits;
	//λͼ��ÿ������ռ�ֽ���
	WORD wBitCount;
	//�����ɫ���С�� λͼ�������ֽڴ�С ��λͼ�ļ���С �� д���ļ��ֽ��� 
	DWORD dwPaletteSize=0, dwBmBitsSize=0, dwDIBSize=0, dwWritten=0; 
	//λͼ���Խṹ 
	BITMAP Bitmap;  
	//λͼ�ļ�ͷ�ṹ
	BITMAPFILEHEADER bmfHdr;  
	//λͼ��Ϣͷ�ṹ 
	BITMAPINFOHEADER bi;  
	//ָ��λͼ��Ϣͷ�ṹ  
	LPBITMAPINFOHEADER lpbi;  
	//�����ļ��������ڴ�������ɫ���� 
	HANDLE fh, hDib, hPal,hOldPal=NULL; 

	//����λͼ�ļ�ÿ��������ռ�ֽ��� 
	hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES); 
	DeleteDC(hDC); 
	if (iBits <= 1)  wBitCount = 1; 
	else if (iBits <= 4)  wBitCount = 4; 
	else if (iBits <= 8)  wBitCount = 8; 
	else      wBitCount = 24; 

	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize   = sizeof(BITMAPINFOHEADER);
	bi.biWidth   = Bitmap.bmWidth;
	bi.biHeight   = Bitmap.bmHeight;
	bi.biPlanes   = 1;
	bi.biBitCount  = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage  = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed  = 0;

	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	//Ϊλͼ���ݷ����ڴ� 
	hDib = GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER)); 
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib); 
	*lpbi = bi;

	// �����ɫ��  
	hPal = GetStockObject(DEFAULT_PALETTE); 
	if (hPal) 
	{ 
		hDC = ::GetDC(NULL); 
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE); 
		RealizePalette(hDC); 
	}

	// ��ȡ�õ�ɫ�����µ�����ֵ 
	GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) 
		+dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS); 

	//�ָ���ɫ��  
	if (hOldPal) 
	{ 
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE); 
		RealizePalette(hDC); 
		::ReleaseDC(NULL, hDC); 
	}

	//����λͼ�ļ�  
	fh = CreateFile(lpFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 

	if (fh == INVALID_HANDLE_VALUE)  return FALSE; 

	// ����λͼ�ļ�ͷ 
	bmfHdr.bfType = 0x4D42; // "BM" 
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;  
	bmfHdr.bfSize = dwDIBSize; 
	bmfHdr.bfReserved1 = 0; 
	bmfHdr.bfReserved2 = 0; 
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize; 
	// д��λͼ�ļ�ͷ 
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL); 
	// д��λͼ�ļ��������� 
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL); 
	//���  
	GlobalUnlock(hDib); 
	GlobalFree(hDib); 
	CloseHandle(fh);

	return dwWritten;
} 

void ScreenSnapping::setStorageFilePath( const std::string& filePath )
{
	printf("set jpeg storage file:%s" , filePath.c_str() );
	_jpegFilePath = filePath;
}