#include "help-widget.h"
#include "util/GetFileVersion.h"
#include <tchar.h>

QString getVersionByName(const TCHAR * cName)
{
	char strVersion[20];

	memset(strVersion, 0, sizeof(strVersion));
	//ʹ��CGetFileVersion���ȡrtpware.dll�İ汾��Ϣ
	CGetFileVersion verReader;
	verReader.Open(cName);	//���ļ�����ȡ�汾��Ϣ
	if (!verReader.IsValid())
	{
		//��ȡ����
		printf("error to read the version of %s \n", cName);
	}
	else
	{
		//����汾����Ϣ
		sprintf(strVersion, "%d.%d.%d.%d", 
			verReader.GetFileVersionMajor(),
			verReader.GetFileVersionMinor(), 
			verReader.GetFileVersionBuild(), 
			verReader.GetFileVersionQFE());
	}
	return QString::fromStdString(std::string(strVersion));
}


HelpWidget::HelpWidget(QWidget *parent) : QWidget(parent) {
    this->setupUi(this);

	this->CoolviewVersionStringLabel->setText("<b>" + 
		QString(QObject::tr("CoolView")) + " " +
		getVersionByName(L"coolview.exe")
		);


	this->DependenciesLabel->setText(
		"MSDX: "		+getVersionByName(TEXT("msdx.dll")) + "<BR>" +
		"Video-Codec: "	+getVersionByName(TEXT("filter\\SCUT_H264_Encoder.dll")) + "<BR>" +
		"Audio-Codec: "	+getVersionByName(TEXT("filter\\SpeexEnc.dll")) + "<BR>" +
		"RTP:"			+getVersionByName(TEXT("filter\\RtpWare3.dll")) + "<BR>" +//ԭRTP��RTPout_Video.dll��ʱ����㷨�����⣬�ṹ���ӣ����ԸĽ���������gmlan20150714
		"R-CTRL:"		+getVersionByName(TEXT("CvTelecontrollerSocket.exe")) + "<BR>"
		);

    this->show();
}

HelpWidget::~HelpWidget()
{

}
