* Monogram AAC Encoder

1��ԭ���޷���VS2008�б���Release�汾��ԭ�򼰽���������£�

We have a DLL which contains a number of DirectShow filters which inherit from the DirectShow base classes.  The DLL also uses classes from MFC.  So the DLL links against both MFC and the DirectShow base classes (strmbase.lib).  Everything compiles and works fine in VS 2005.
 
After upgrading to VS 2008, I get the following linker errors in a Release build, but not in a Debug build:
mfcs90u.lib(oleexp.obj) : error LNK2005: _DllGetClassObject@12 already defined in strmbase.lib(dllentry.obj)
mfcs90u.lib(oleexp.obj) : error LNK2005: _DllCanUnloadNow@0 already defined in strmbase.lib(dllentry.obj)
 
After some searching, I found this related thread, suggesting a conflict between MFC and COM exports from a DLL.  I tried the suggested workaround, and got past the linker error with...
1. Change DllGetClassObject to MyDllGetClassObject in dllentry.cpp from the DS base classes source.
2. Change DllCanUnloadNow to MyDllCanUnloadNow in dllentry.cpp from the DS base classes source.
3. Rebuild the DirectShow base class library as strmbase.lib.
4. Modify the .def file in our DLL project by changing it as follows:
	; Filters.def : Declares the module parameters for the DLL.
	EXPORTS
	  DllMain                                    PRIVATE
	  DllGetClassObject   = MyDllGetClassObject  PRIVATE
	  DllCanUnloadNow     = MyDllCanUnloadNow    PRIVATE
	  DllRegisterServer                          PRIVATE
	  DllUnregisterServer                        PRIVATE

(����������strmbase��̬���Ѿ�������mmaac��Ŀ��3rd\lib�У�����ʱ����ʹ����Щ�⣬����DShow·���µĿ��ļ�)
��ע��Ǩ�Ƶ�VS2012���޴����⣬�뱣��������Ŀ���ã�ʹ��DirectShow��ԭʼ��̬�⡣��Liaokz, 2013.9.9��

2��ԭ��Debug���޷�ע�ᣬ��Ϊfaac��ı���������⡣��ʹ��VC2012���±���lib�Ⲣ����mmaac\3rd�С�һ���ֱ�ӱ���mmaac��������������������Ҫ�ر���faacʱ����ʹ��faac-1.28\libfaac�е�VC2012�������±���lib�⡣

3��mmaac\binaries���ǹٷ�������ļ����ƺ���bug�������

