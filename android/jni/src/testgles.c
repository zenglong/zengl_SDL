/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/

//Include SDL functions and datatypes
#define ZL_EXP_OS_IN_ARM_GCC
#include "zengl_exportfuns.h"
#include <jni.h>
#include "SDL.h"
#include "SDL_main.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "mySDL_Lib.h"

typedef struct _strPrintEnv{
	JNIEnv* env;
	jobject obj;
	//jmethodID functionID;
	FILE * printlog;
	FILE * debuglog;
}strPrintEnv;

void java_printcall(strPrintEnv * myenv ,ZL_EXP_CHAR * arg)
{
	fprintf(myenv->printlog,"%s",arg);
}

ZL_EXP_INT run_print(ZL_EXP_CHAR * infoStrPtr, ZL_EXP_INT infoStrCount, ZL_EXP_VOID * VM_ARG)
{
	strPrintEnv * myenv = (strPrintEnv *)zenglApi_GetExtraData(VM_ARG,"extra");
	java_printcall(myenv,infoStrPtr);
	java_printcall(myenv,"\n");
	return 0;
}

ZL_EXP_INT debug_compile_info(ZL_EXP_CHAR * infoStrPtr, ZL_EXP_INT infoStrCount,ZL_EXP_VOID * VM_ARG)
{
	strPrintEnv * myenv = (strPrintEnv *)zenglApi_GetExtraData(VM_ARG,"extra");
	fprintf(myenv->debuglog,"%s",infoStrPtr);
	return 0;
}

ZL_EXP_INT debug_run_info(ZL_EXP_CHAR * infoStrPtr, ZL_EXP_INT infoStrCount,ZL_EXP_VOID * VM_ARG)
{
	strPrintEnv * myenv = (strPrintEnv *)zenglApi_GetExtraData(VM_ARG,"extra");
	fprintf(myenv->debuglog,"%s",infoStrPtr);
	return 0;
}

/*builtin模块函数*/
ZL_EXP_VOID builtin_printf(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT i;
	strPrintEnv * myenv = (strPrintEnv *)zenglApi_GetExtraData(VM_ARG,"extra");
	ZL_EXP_CHAR tmp[50];
	if(argcount < 1)
		zenglApi_Exit(VM_ARG,"printf函数的参数不可以为空");
	for(i=1;i<=argcount;i++)
	{
		zenglApi_GetFunArg(VM_ARG,i,&arg);
		switch(arg.type)
		{
		case ZL_EXP_FAT_INT:
			sprintf(tmp,"%ld",arg.val.integer);
			java_printcall(myenv,tmp);
			break;
		case ZL_EXP_FAT_FLOAT:
			sprintf(tmp,"%.16g",arg.val.floatnum);
			java_printcall(myenv,tmp);
			break;
		case ZL_EXP_FAT_STR:
			java_printcall(myenv,arg.val.str);
			break;
		default:
			zenglApi_Exit(VM_ARG,"printf函数第%d个参数类型无效，目前只支持字符串，整数，浮点数类型的参数",i);
			break;
		}
	}
	return ;
}

/*递归打印出数组信息*/
ZL_EXP_VOID print_array(ZL_EXP_VOID * VM_ARG,ZENGL_EXPORT_MEMBLOCK memblock,ZL_EXP_INT recur_count,strPrintEnv * myenv)
{
	ZL_EXP_INT size,i,j;
	ZL_EXP_CHAR tmp[60];
	ZENGL_EXPORT_MOD_FUN_ARG mblk_val = {ZL_EXP_FAT_NONE,{0}};
	zenglApi_GetMemBlockInfo(VM_ARG,&memblock,&size,ZL_EXP_NULL);
	for(i=1;i<=size;i++)
	{
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,i);
		switch(mblk_val.type)
		{
		case ZL_EXP_FAT_INT:
		case ZL_EXP_FAT_FLOAT:
		case ZL_EXP_FAT_STR:
		case ZL_EXP_FAT_MEMBLOCK:
			for(j=0;j<recur_count;j++)
				java_printcall(myenv,"  ");
			break;
		}
		switch(mblk_val.type)
		{
		case ZL_EXP_FAT_INT:
			sprintf(tmp,"[%d] %ld\n",i-1,mblk_val.val.integer);
			java_printcall(myenv,tmp);
			break;
		case ZL_EXP_FAT_FLOAT:
			sprintf(tmp,"[%d] %.16g\n",i-1,mblk_val.val.floatnum);
			java_printcall(myenv,tmp);
			break;
		case ZL_EXP_FAT_STR:
			sprintf(tmp,"[%d] ",i-1);
			java_printcall(myenv,tmp);
			java_printcall(myenv,mblk_val.val.str);
			break;
		case ZL_EXP_FAT_MEMBLOCK:
			sprintf(tmp,"[%d] <array or class obj type> begin:\n",i-1);
			java_printcall(myenv,tmp);
			print_array(VM_ARG,mblk_val.val.memblock,recur_count+1,myenv);
			sprintf(tmp,"[%d] <array or class obj type> end\n",i-1);
			java_printcall(myenv,tmp);
			break;
		}
	}
}

/*bltPrintArray模块函数，打印数组中的元素*/
ZL_EXP_VOID builtin_print_array(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	strPrintEnv * myenv = (strPrintEnv *)zenglApi_GetExtraData(VM_ARG,"extra");
	if(argcount < 1)
		zenglApi_Exit(VM_ARG,"bltPrintArray函数参数不可为空，必须指定一个数组或类对象为参数");
	zenglApi_GetFunArg(VM_ARG,1,&arg);
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"bltPrintArray函数的参数不是数组或类对象");
	print_array(VM_ARG,arg.val.memblock,0,myenv);
}

/*bltGetZLVersion模块函数，获取当前zengl版本号信息的字符串形式*/
ZL_EXP_VOID builtin_get_zl_version(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZL_EXP_CHAR version[20] = {0};
	sprintf(version,"%d.%d.%d",ZL_EXP_MAJOR_VERSION,ZL_EXP_MINOR_VERSION,ZL_EXP_REVISION);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_STR,version,0,0);
}

ZL_EXP_VOID builtin_module_init(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT moduleID)
{
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"printf",builtin_printf);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltRandom",zenglApiBMF_bltRandom); //使用虚拟机zenglApi_BltModFuns.c中定义的bltRandom
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"array",zenglApiBMF_array);  //使用虚拟机zenglApi_BltModFuns.c中定义的array
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"unset",zenglApiBMF_unset);  //使用虚拟机zenglApi_BltModFuns.c中定义的unset
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltExit",zenglApiBMF_bltExit);  //使用虚拟机zenglApi_BltModFuns.c中定义的bltExit
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltConvToInt",zenglApiBMF_bltConvToInt);  //使用虚拟机zenglApi_BltModFuns.c中定义的bltConvToInt
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltIntToStr",zenglApiBMF_bltIntToStr);  //使用虚拟机zenglApi_BltModFuns.c中定义的bltIntToStr
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltPrintArray",builtin_print_array);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltGetZLVersion",builtin_get_zl_version);
}

ZL_EXP_VOID sdl_module_init(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT moduleID)
{
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlInit",mySDL_Init); //SDL全局初始化函数，对应脚本模块函数：sdlInit()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlQuit",mySDL_Quit); //SDL退出时调用SDL_Quit释放资源，对应脚本模块函数：sdlQuit()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlFontQuit",mySDL_FontQuit); //通过TTF_Quit清理字体所占用的系统资源 对应的脚本函数是：sdlFontQuit()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlCreateWin",mySDL_CreateWin); //SDL引擎创建游戏窗口，对应脚本模块函数：sdlCreateWin(width,height,bpp,flags,repeat[option])
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlPollEvent",mySDL_PollEvent); //SDL从事件中捕获用户输入的按键或鼠标点击的事件等 对应脚本函数：sdlPollEvent(event)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlAndroidLog",mySDL_AndroidLog); //sdlAndroidLog
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlSetBkImg",mySDL_SetBkImg); //设置游戏的背景图片，对应的脚本函数：sdlSetBkImg(picname | surface point)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlShowScreen",mySDL_ShowScreen); //将游戏绘制好的帧显示出来 对应的脚本函数：sdlShowScreen()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlGetWindowSurface",mySDL_GetWindowSurface); //sdlGetWindowSurface获取主窗口的表面指针
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlLoadBmp",mySDL_LoadBmp); //加载位图，对应的脚本函数：sdlLoadBmp(picname,red[option],green[option],blue[option]) (option表示可选参数)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlBlitImg",mySDL_BlitImg); //将一个位图绘制到另一个位图上面，对应的脚本函数：sdlBlitImg(src surface,src rect,dest surface,dest rect)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlGetKeyState",mySDL_GetKeyState); //获取按键状态 对应脚本函数：sdlGetKeyState(array keystate)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlSetCaption",mySDL_SetCaption); //设置游戏窗口的标题，对应的脚本函数是：sdlSetCaption(caption)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlSetColorKey",mySDL_SetColorKey); //设置某位图表面的colorkey 对应的脚本函数为：sdlSetColorKey(image surface , red , green , blue)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlGetTicks",mySDL_GetTicks); //获取游戏运行的滴答数 对应的脚本函数为：sdlGetTicks()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlOpenFont",mySDL_OpenFont); //打开某字体文件 对应的脚本函数为：sdlOpenFont(font_filename , font_size)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlDrawText",mySDL_DrawText); //绘制文本信息 对应的脚本函数为：sdlDrawText(font obj , string , array fontcolor , array bgcolor[option]); (option表示是可选参数)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlCloseFont",mySDL_CloseFont);//关闭字体 对应的脚本函数为：sdlCloseFont(font obj)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlFreeImage",mySDL_FreeImage);//释放某个位图表面资源 对应的脚本函数为：sdlFreeImage(image surface)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlFontInit",mySDL_FontInit); //字体资源初始化 对应的脚本函数为：sdlFontInit()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlFillRect",mySDL_FillRect); //将目标矩形区域填充为指定的颜色 对应的脚本函数为：sdlFillRect(dest surface,dest rect , array color_alpha)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlDrawTextEx",mySDL_DrawTextEx);//绘制文本信息的扩展函数 对应的脚本函数为：sdlDrawTextEx(ref surface,font obj , string , array fontcolor , array bgcolor[option]); (option表示是可选参数)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlDelay",mySDL_Delay); //SDL延时，对应脚本模块函数：sdlDelay(millisecond)
}

int main( int argc, char* args[] )
{
	SDL_Delay( 17000 );
    //The images
    SDL_Surface* hello = NULL;
    SDL_Surface* screen = NULL;
    SDL_Surface* loadedSurface;
    SDL_Surface* optimizedSurface = NULL;

    //Start SDL
    SDL_Init( SDL_INIT_EVERYTHING );

    //Set up screen
    screen = SDL_SetVideoMode( 320, 480, 32, SDL_SWSURFACE );

    //Load image
    //hello = SDL_LoadBMP( "hello.bmp" );
    IMG_Init(IMG_INIT_JPG);
    loadedSurface = IMG_Load("hello.jpg");
    optimizedSurface = SDL_ConvertSurface( loadedSurface,
    									screen->format, 0 );
    SDL_FreeSurface( loadedSurface );


    //Apply image to screen
    SDL_BlitSurface( optimizedSurface, NULL, screen, NULL );

    //Update Screen
    SDL_Flip( screen );

    //Pause
    SDL_Delay( 4000 );

    //Free the loaded image
    SDL_FreeSurface( optimizedSurface );

    IMG_Quit();
    //Quit SDL
    SDL_Quit();

    return 0;
}

jstring
Java_com_zengl_SDL_SDLActivity_RunZenglFromJNI( JNIEnv* env,
												  jobject obj,
                                                  jstring Path,
                                                  jstring s,
                                                  jint debuginfo)
{
	jstring ret;
	ZL_EXP_CHAR * err = ZL_EXP_NULL;
	FILE * debuglog = ZL_EXP_NULL;
	FILE * printlog = ZL_EXP_NULL;
	strPrintEnv myenv = {env,obj,printlog,debuglog};
	ZL_EXP_VOID * VM = ZL_EXP_NULL;
	ZL_EXP_CHAR scriptPath[100] = {0};
	ZL_EXP_CHAR * tmp;
	ZL_EXP_CHAR * tmpPath;
	ZL_EXP_CHAR * GameStartScript;
	SDL_Android_Init(env, obj);
	SDL_Log("hello first");
	//SDL_Delay( 5000 );
	ret = (*env)->NewStringUTF(env,""); //需要初始化为空字符串，否则返回时就容易出现 libc SIGABRT 错误
	tmp = (ZL_EXP_CHAR *)((*env)->GetStringUTFChars(env, s, NULL));
	tmpPath = (ZL_EXP_CHAR *)((*env)->GetStringUTFChars(env, Path, NULL));
	sprintf(scriptPath,"%s/zengl_debuglogs.txt",tmpPath);
	tmpPath = (ZL_EXP_CHAR *)((*env)->GetStringUTFChars(env, Path, NULL));
	debuglog = fopen(scriptPath,"w+");
	myenv.debuglog = debuglog;
	sprintf(scriptPath,"%s/zengl_printlogs.txt",tmpPath);
	printlog = fopen(scriptPath,"w+");
	myenv.printlog = printlog;
	sprintf(scriptPath,"%s/%s",tmpPath,tmp);
	VM = zenglApi_Open();
	zenglApi_SetFlags(VM,(ZENGL_EXPORT_VM_MAIN_ARG_FLAGS)(ZL_EXP_CP_AF_IN_DEBUG_MODE));
	SDL_Log("hello 2");
	if(zenglApi_Run(VM,scriptPath) == -1) //编译执行zengl脚本
		err = zenglApi_GetErrorString(VM);
	if(err != ZL_EXP_NULL)
	{
		java_printcall(&myenv,err);
		goto end;
	}
	if((GameStartScript = zenglApi_GetValueAsString(VM,"GameStartScript")) == ZL_EXP_NULL)
	{
		err = zenglApi_GetErrorString(VM);
		java_printcall(&myenv,err);
		goto end;
	}
	SDL_Log("hello 3");
	sprintf(scriptPath,"%s/%s",tmpPath,GameStartScript);
	zenglApi_Reset(VM);
	SDL_Log("hello 4");
	if(debuginfo == 1)
		zenglApi_SetFlags(VM,(ZENGL_EXPORT_VM_MAIN_ARG_FLAGS)(ZL_EXP_CP_AF_IN_DEBUG_MODE | ZL_EXP_CP_AF_OUTPUT_DEBUG_INFO));
	else
		zenglApi_SetFlags(VM,(ZENGL_EXPORT_VM_MAIN_ARG_FLAGS)(ZL_EXP_CP_AF_IN_DEBUG_MODE));
	zenglApi_SetHandle(VM,ZL_EXP_VFLAG_HANDLE_COMPILE_INFO,debug_compile_info);
	zenglApi_SetHandle(VM,ZL_EXP_VFLAG_HANDLE_RUN_INFO,debug_run_info);
	zenglApi_SetHandle(VM,ZL_EXP_VFLAG_HANDLE_RUN_PRINT,run_print);
	zenglApi_SetModInitHandle(VM,"builtin",builtin_module_init);
	zenglApi_SetModInitHandle(VM,"sdl",sdl_module_init);
	zenglApi_SetExtraData(VM,"extra",&myenv);
	SDL_Log("hello 5");
	if(zenglApi_Run(VM,scriptPath) == -1) //编译执行zengl脚本
		err = zenglApi_GetErrorString(VM);
	if(err != ZL_EXP_NULL)
		java_printcall(&myenv,err);
end:
	SDL_Log("hello 6");
	fun_sdlLib_destroyAll(VM); //用于释放SDL分配的资源
	zenglApi_Close(VM);
	fclose(debuglog);
	fclose(printlog);
	if(err != ZL_EXP_NULL)
	{
		SDL_Log("error occured,see zengl_printlogs.txt! wait 4s to Exit!");
		SDL_Delay(4000);
	}
	exit(0);
	return 0;
}
