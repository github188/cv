��Ŀ¼�������ڴ�����װ����


Ŀ¼��
Debug��ReleaseĿ¼�зֱ��������汾�ĳ����ļ���
CommonĿ¼����Debug��Release��Ĺ����ļ���
FileList�ж����˸�����Ҫ������ļ��б������ļ��Զ����¡�


�ű���
UpdateFiles.bat�ű����ڽ���Ŀ�ļ����е������ļ����Ƶ���Ŀ¼��Ӧλ�á�ִ��UpdateFiles.bat�ű�ʱ���ȡFileList�е��ļ�����ע�����¼��㣺
1��RTPWare�е�NetworkMeasurer�����κ�һ���б��У�������UpdateFiles.batֱ�Ӹ��Ƶ�CoolViewĿ¼�еģ�����ԭ������Ϊ��ô���Ϸ��㡣
2��QT��VC����ʱ�ļ������ɽű��Զ����£���Ҫ�ֶ����¡������ɾ������Ҫ�ֶ��ָ�����ͨ��SVN��Revert������ɣ���
   VC����ʱ�ļ���ͨ��
       C:\Program Files\Microsoft Visual Studio 11.0\VC\redist\x86  ��Release�� ��
       C:\Program Files\Microsoft Visual Studio 11.0\VC\redist\Debug_NonRedist\x86  ��Debug����á�
3��CvSipWrapper�е�Boost�ļ������Զ����£���Ҫ�ֶ����¡������ɾ������Ҫ�ֶ��ָ�����ͨ��SVN��Revert������ɣ���
4������ļ������ڣ���û��Checkout��Ӧ��Ŀ��û�б���Debug�汾��������Ը�����Ӧ�ļ�������Ӱ��ű�ִ�С�
5����Ҫʱ���ǵü���Ƿ���³ɹ���


���������
Ϊ�˱�֤�����ļ������µģ�������ѭ���²��裺
1����CvSetupִ��Update��
2��ִ��UpdateFiles.bat�ű�������������������ļ����Ƶ��˴��������¾��ļ��滻��ʾ����ʱ�ܿ���CvSetup�е��ļ����Լ����ļ�Ҫ�£��ж��Ƿ�Ҫ�滻��
3�������ͻ����Commit��
4��ʹ��Inno Setup Compiler��CoolViewSetup.iss�ű����Release�汾�İ�װ������CoolViewSetupDebug.iss���Debug��װ��������pdb�ļ���������Զ�̵��ԣ���
5��ʹ��BuildCoolViewDevDir.bat�ű����������ļ�ͬ������������Ŀ¼��
