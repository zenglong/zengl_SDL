#define ZL_EXP_OS_IN_LINUX //在加载下面的zengl头文件之前，windows系统请定义ZL_EXP_OS_IN_WINDOWS，linux , mac系统请定义ZL_EXP_OS_IN_LINUX
#include "zengl_exportfuns.h"

#ifdef ZL_EXP_OS_IN_WINDOWS
	#include <windows.h>
#endif

ZL_EXP_CHAR * GbkToUtf8ForWindows(ZL_EXP_VOID * VM_ARG,ZL_EXP_CHAR * gbkstr)
{
	#ifdef ZL_EXP_OS_IN_WINDOWS
		int len;
		wchar_t * unicode;
		char* utf8str;
		len = MultiByteToWideChar( CP_ACP,0,gbkstr,-1,NULL,0);
		unicode = (wchar_t *)zenglApi_AllocMem(VM_ARG,(len+1)*sizeof(wchar_t));
		memset(unicode,0,(len+1)*sizeof(wchar_t)); 
		MultiByteToWideChar( CP_ACP, 0,gbkstr,-1,(LPWSTR)unicode,len); //先将gbk字符串转为unicode
		len = WideCharToMultiByte( CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL ); 
		utf8str =(char *)zenglApi_AllocMem(VM_ARG,(len+1)*sizeof(char));
		memset( utf8str, 0, sizeof(char) * ( len + 1 ) ); 
		WideCharToMultiByte( CP_UTF8, 0, unicode, -1, utf8str, len, NULL, NULL ); //再有unicode转为utf8
		zenglApi_FreeMem(VM_ARG,(void *)unicode); //释放指针
		return utf8str;
	#else
		return ZL_EXP_NULL;
	#endif
}
