// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
//��ı�׼�������� DLL �е������ļ��������������϶���� MSDX_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
//�κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ 
// MSDX_API ������Ϊ�ǴӴ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef MSDX_EXPORTS
#define MSDX_API __declspec(dllexport)
#else
#define MSDX_API __declspec(dllimport)
#endif

// �����Ǵ� msdx.dll ������
class MSDX_API Cmsdx {
public:
	Cmsdx(void);
	// TODO: �ڴ�������ķ�����
};

extern MSDX_API int nmsdx;

MSDX_API int fnmsdx(void);
