#define ZL_EXP_OS_IN_LINUX //在加载下面的zengl头文件之前，windows系统请定义ZL_EXP_OS_IN_WINDOWS，linux , mac系统请定义ZL_EXP_OS_IN_LINUX
#include "zengl_exportfuns.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "mySDL_Lib.h"

ZL_EXP_BOOL gl_sdlLib_initFlags[SDL_LIB_INIT_NUM] = {0}; //用于判断字体库，SDL库是否初始化的标记
SDL_Surface * gl_sdlLib_pScreen = ZL_EXP_NULL; //全局的SDL窗口表面
SDL_LIB_POINT_LIST gl_sdlLib_PointsList = {0}; //SDL存放表面和字体等指针的动态数组

/*
	用于释放SDL分配的资源
*/
ZL_EXP_VOID fun_sdlLib_destroyAll(ZL_EXP_VOID * VM_ARG)
{
	ZL_EXP_INT i;
	for(i=0;i<gl_sdlLib_PointsList.count;i++) //循环释放掉所有SDL指针
	{
		if(gl_sdlLib_PointsList.points[i].isvalid == ZL_EXP_TRUE && gl_sdlLib_PointsList.points[i].point != ZL_EXP_NULL)
		{
			switch(gl_sdlLib_PointsList.points[i].type)
			{
			case FONT_TYPE_POINT:
				TTF_CloseFont((TTF_Font *)gl_sdlLib_PointsList.points[i].point);
				break;
			case SDL_TYPE_POINT:
				if((SDL_Surface *)gl_sdlLib_PointsList.points[i].point != gl_sdlLib_pScreen) //主屏幕指针应由SDL_Quit来释放
					SDL_FreeSurface((SDL_Surface *)gl_sdlLib_PointsList.points[i].point);
				break;
			}
		}
	}
	if(gl_sdlLib_PointsList.isInit && gl_sdlLib_PointsList.points != NULL)
		zenglApi_FreeMem(VM_ARG,(ZL_EXP_VOID *)gl_sdlLib_PointsList.points);
	if(gl_sdlLib_PointsList.freelist.isInit && gl_sdlLib_PointsList.freelist.frees != NULL)
		zenglApi_FreeMem(VM_ARG,(ZL_EXP_VOID *)gl_sdlLib_PointsList.freelist.frees);
	memset((void *)&gl_sdlLib_PointsList,0,sizeof(gl_sdlLib_PointsList));
	for(i=0;i<SDL_LIB_INIT_NUM;i++)
	{
		if(gl_sdlLib_initFlags[i])
		{
			switch(i)
			{
			case INIT_FONT:
				mySDL_FontQuit(VM_ARG,0);
				break;
			case INIT_SDL:
				mySDL_Quit(VM_ARG,0);
				break;
			}
		}
	}
	return;
}

/*
	初始化动态数组，该动态数组里存放了SDL释放的指针的索引
*/
ZL_EXP_VOID fun_sdlLib_InitFreePointList(ZL_EXP_VOID * VM_ARG)
{
	if(gl_sdlLib_PointsList.freelist.isInit) //如果已经初始化过，就返回
		return;
	gl_sdlLib_PointsList.freelist.size = SDL_LIB_POINT_SIZE; //使用和gl_sdlLib_PointsList.points一样的初始化和扩容大小
	gl_sdlLib_PointsList.freelist.count = 0;
	gl_sdlLib_PointsList.freelist.frees = zenglApi_AllocMem(VM_ARG,gl_sdlLib_PointsList.freelist.size * sizeof(int));
	if(gl_sdlLib_PointsList.freelist.frees == ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_InitFreePointList函数分配内存初始化失败:%s\n",zenglApi_GetErrorString(VM_ARG));
	memset(gl_sdlLib_PointsList.freelist.frees , 0 , gl_sdlLib_PointsList.freelist.size * sizeof(int));
	gl_sdlLib_PointsList.freelist.isInit = ZL_EXP_TRUE;
}

/*
	将释放的指针索引存放到freelist动态数组中，下次就可以直接重利用该索引
*/
ZL_EXP_VOID fun_sdlLib_Insert_FreePoint(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT index)
{
	if(!gl_sdlLib_PointsList.freelist.isInit)
		fun_sdlLib_InitFreePointList(VM_ARG);
	if(gl_sdlLib_PointsList.freelist.count == gl_sdlLib_PointsList.freelist.size)
	{
		gl_sdlLib_PointsList.freelist.size += SDL_LIB_POINT_SIZE;
		gl_sdlLib_PointsList.freelist.frees = zenglApi_ReAllocMem(VM_ARG,gl_sdlLib_PointsList.freelist.frees,gl_sdlLib_PointsList.freelist.size * sizeof(int));
		if(gl_sdlLib_PointsList.freelist.frees == ZL_EXP_NULL)
			zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_FreePoint函数调整内存大小失败:%s\n",zenglApi_GetErrorString(VM_ARG));
		memset(gl_sdlLib_PointsList.freelist.frees + (gl_sdlLib_PointsList.freelist.size - SDL_LIB_POINT_SIZE),0,
			SDL_LIB_POINT_SIZE * sizeof(int));
	}
	gl_sdlLib_PointsList.freelist.frees[gl_sdlLib_PointsList.freelist.count] = index;
	gl_sdlLib_PointsList.freelist.count++;
}

/*
	初始化SDL指针动态数组
*/
ZL_EXP_VOID fun_sdlLib_InitPointsList(ZL_EXP_VOID * VM_ARG)
{
	if(gl_sdlLib_PointsList.isInit)
		return;
	gl_sdlLib_PointsList.size = SDL_LIB_POINT_SIZE;
	gl_sdlLib_PointsList.count = 1; //从1开始，所以0就是无效的索引
	gl_sdlLib_PointsList.points = zenglApi_AllocMem(VM_ARG,gl_sdlLib_PointsList.size * sizeof(SDL_LIB_POINT_MEMBER));
	if(gl_sdlLib_PointsList.points == ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_InitPointsList函数分配内存初始化失败:%s\n",zenglApi_GetErrorString(VM_ARG));
	memset(gl_sdlLib_PointsList.points , 0 , gl_sdlLib_PointsList.size * sizeof(SDL_LIB_POINT_MEMBER));
	gl_sdlLib_PointsList.isInit = ZL_EXP_TRUE;
}

/*
	将SDL指针添加到gl_sdlLib_PointsList.points动态数组中，以后可以从此动态数组中查找指针，从而判断某SDL指针是否是有效的指针
*/
ZL_EXP_INT fun_sdlLib_Insert_Point(ZL_EXP_VOID * VM_ARG,ZL_EXP_VOID * point,SDL_LIB_POINT_TYPE type)
{
	int tmpindex;
	if(!gl_sdlLib_PointsList.isInit)
		fun_sdlLib_InitPointsList(VM_ARG);
	if(gl_sdlLib_PointsList.count == gl_sdlLib_PointsList.size)
	{
		gl_sdlLib_PointsList.size += SDL_LIB_POINT_SIZE;
		gl_sdlLib_PointsList.points = zenglApi_ReAllocMem(VM_ARG,gl_sdlLib_PointsList.points,gl_sdlLib_PointsList.size * sizeof(SDL_LIB_POINT_MEMBER));
		if(gl_sdlLib_PointsList.points == ZL_EXP_NULL)
			zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_Point函数调整内存大小失败:%s\n",zenglApi_GetErrorString(VM_ARG));
		memset(gl_sdlLib_PointsList.points + (gl_sdlLib_PointsList.size - SDL_LIB_POINT_SIZE),0,
			SDL_LIB_POINT_SIZE * sizeof(SDL_LIB_POINT_MEMBER));
	}
	if(gl_sdlLib_PointsList.freelist.count > 0) //如果在释放动态数组中有元素，则直接使用以前释放的索引
	{
		tmpindex = gl_sdlLib_PointsList.freelist.frees[gl_sdlLib_PointsList.freelist.count-1];
		gl_sdlLib_PointsList.freelist.count--;
	}
	else
		tmpindex = gl_sdlLib_PointsList.count;
	if(gl_sdlLib_PointsList.points[tmpindex].isvalid == ZL_EXP_FALSE)
	{
		gl_sdlLib_PointsList.points[tmpindex].point = point;
		gl_sdlLib_PointsList.points[tmpindex].type = type;
		gl_sdlLib_PointsList.points[tmpindex].isvalid = ZL_EXP_TRUE;
		if(tmpindex == gl_sdlLib_PointsList.count)
			gl_sdlLib_PointsList.count++;
		return tmpindex;
	}
	else
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_Point函数无法在gl_sdlLib_PointsList的动态数组中找到没被占用的索引\n");
	return -1;
}

/*
	释放SDL相关指针，一般发生在手动调用SDL API来释放SDL指针资源时
*/
ZL_EXP_INT fun_sdlLib_Free_Point(ZL_EXP_VOID * VM_ARG,ZL_EXP_VOID * point,SDL_LIB_POINT_TYPE type)
{
	int tmpindex,preindex;
	int h = (long)point % SDL_LIB_HASHSIZE;
	tmpindex = gl_sdlLib_PointsList.hash[h];
	while(tmpindex != 0 && gl_sdlLib_PointsList.points[tmpindex].isvalid == ZL_EXP_TRUE &&
		  point != gl_sdlLib_PointsList.points[tmpindex].point)
	{
		  preindex = tmpindex;
		  tmpindex = gl_sdlLib_PointsList.points[tmpindex].next;
	}
	if(tmpindex == 0)
		return -1;
	else if(gl_sdlLib_PointsList.points[tmpindex].isvalid == ZL_EXP_FALSE)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Free_Point函数在gl_sdlLib_PointsList的动态数组中找到无效的索引[%d]\n",tmpindex);
	else if(gl_sdlLib_PointsList.points[tmpindex].type != type)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Free_Point函数在gl_sdlLib_PointsList的动态数组中找到的指针类型与函数提供的类型不符\n");
	else
	{
		if(tmpindex != gl_sdlLib_PointsList.hash[h])
			gl_sdlLib_PointsList.points[preindex].next = gl_sdlLib_PointsList.points[tmpindex].next;
		else
			gl_sdlLib_PointsList.hash[h] = gl_sdlLib_PointsList.points[tmpindex].next;
		gl_sdlLib_PointsList.points[tmpindex].isvalid = ZL_EXP_FALSE;
		gl_sdlLib_PointsList.points[tmpindex].next = 0;
		gl_sdlLib_PointsList.points[tmpindex].point = ZL_EXP_NULL;
		gl_sdlLib_PointsList.points[tmpindex].type = 0;
		fun_sdlLib_Insert_FreePoint(VM_ARG,tmpindex);
	}
	return 0;
}

/*
	检测某SDL指针是否是有效的指针。
*/
ZL_EXP_BOOL fun_sdlLib_Check_Point(ZL_EXP_VOID * VM_ARG,ZL_EXP_VOID * point,SDL_LIB_POINT_TYPE type)
{
	int tmpindex;
	int h = (long)point % SDL_LIB_HASHSIZE;
	tmpindex = gl_sdlLib_PointsList.hash[h];
	while(tmpindex != 0 && gl_sdlLib_PointsList.points[tmpindex].isvalid == ZL_EXP_TRUE &&
		  point != gl_sdlLib_PointsList.points[tmpindex].point)
	{
		  tmpindex = gl_sdlLib_PointsList.points[tmpindex].next;
	}
	if(tmpindex == 0)
		return ZL_EXP_FALSE;
	else if(gl_sdlLib_PointsList.points[tmpindex].isvalid == ZL_EXP_FALSE)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Check_Point函数在gl_sdlLib_PointsList的动态数组中找到无效的索引[%d]\n",tmpindex);
	else if(gl_sdlLib_PointsList.points[tmpindex].type != type)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Check_Point函数在gl_sdlLib_PointsList的动态数组中找到的指针类型与函数提供的类型不符\n");
	else
	{
		return ZL_EXP_TRUE;
	}
	return ZL_EXP_FALSE;
}

/*
	将SDL指针插入到动态数组中，并将数组中的索引添加到哈希表中，方便以后的查找操作。
*/
ZL_EXP_VOID fun_sdlLib_Insert_HashTableForPoint(ZL_EXP_VOID * VM_ARG,ZL_EXP_VOID * point,SDL_LIB_POINT_TYPE type)
{
	int tmpindex;
	int h = (long)point % SDL_LIB_HASHSIZE;
	tmpindex = gl_sdlLib_PointsList.hash[h];
	while(tmpindex != 0 && gl_sdlLib_PointsList.points[tmpindex].isvalid == ZL_EXP_TRUE &&
		  point != gl_sdlLib_PointsList.points[tmpindex].point)
		  tmpindex = gl_sdlLib_PointsList.points[tmpindex].next;
	if(tmpindex == 0)
	{
		tmpindex = gl_sdlLib_PointsList.hash[h];
		gl_sdlLib_PointsList.hash[h] = fun_sdlLib_Insert_Point(VM_ARG,point,type);
		gl_sdlLib_PointsList.points[gl_sdlLib_PointsList.hash[h]].next = tmpindex;
	}
	else if(gl_sdlLib_PointsList.points[tmpindex].isvalid == ZL_EXP_FALSE)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_HashTableForPoint函数在gl_sdlLib_PointsList的动态数组中找到无效的索引[%d]\n",tmpindex);
	else
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_HashTableForPoint函数提供的指针[%d]在之前已经插入过了",point);
}

/*
  SDL全局初始化函数，对应脚本模块函数：sdlInit()
*/
ZL_EXP_VOID mySDL_Init(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	printf("Initializing SDL.\n");
	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) {
		zenglApi_Exit(VM_ARG,"Could not initialize SDL: %s.\n", SDL_GetError());
	}
	printf("SDL initialized.\n");
	gl_sdlLib_initFlags[INIT_SDL] = ZL_EXP_TRUE;
}

/*
  SDL退出时调用SDL_Quit释放资源，对应脚本模块函数：sdlQuit()
*/
ZL_EXP_VOID mySDL_Quit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	if(!gl_sdlLib_initFlags[INIT_SDL])
		return;
	SDL_Quit();
	gl_sdlLib_initFlags[INIT_SDL] = ZL_EXP_FALSE;
	printf("SDL Quit\n");
}

/*
   通过TTF_Quit清理字体所占用的系统资源
   对应的脚本函数是：sdlFontQuit();
*/
ZL_EXP_VOID mySDL_FontQuit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	if(!gl_sdlLib_initFlags[INIT_FONT])
		return;
	TTF_Quit();
	gl_sdlLib_initFlags[INIT_FONT] = ZL_EXP_FALSE;
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}

/*
   SDL引擎创建游戏窗口，对应脚本函数：sdlCreateWin(width,height,bpp,flags,repeat[option]); (option表示可选参数)
*/
ZL_EXP_VOID mySDL_CreateWin(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT width,height,bpp,flags;
	if(!gl_sdlLib_initFlags[INIT_SDL])
		zenglApi_Exit(VM_ARG,"sdlCreateWin error: have not initial! (SDL库没有初始化，请使用sdlInit函数进行初始化)\n");
	if(argcount < 4)
		zenglApi_Exit(VM_ARG,"sdlCreateWin函数必须至少有四个参数，函数原型：sdlCreateWin(width,height,bpp,flags,repeat[option]);\n");

	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数的值，第一个参数是窗口宽
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCreateWin第一个参数必须是整数，表示窗口宽");
	width = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,2,&arg); //获取第二个参数的值，第二个参数是窗口高
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCreateWin第二个参数必须是整数，表示窗口高");
	height = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,3,&arg); //获取第三个参数的值，第三个参数是位深(窗口的分辨率)
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCreateWin第三个参数必须是整数，表示位深(窗口的分辨率)");
	bpp = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,4,&arg); //获取第四个参数的值，第四个参数是SDL视频模式的一些可选标志
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCreateWin第四个参数必须是整数，表示SDL视频模式的一些可选标志");
	flags = arg.val.integer;

	if(gl_sdlLib_pScreen != ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"sdlCreateWin错误：已经创建过游戏主窗口表面了!");

	gl_sdlLib_pScreen = SDL_SetVideoMode(width, height, bpp, flags);
	if(gl_sdlLib_pScreen == ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"sdlCreateWin错误：内部调用SDL_SetVideoMode函数创建游戏主窗口表面失败：%s!",SDL_GetError());

	if(argcount == 5) //如果提供了5个参数，则使用第五个参数设置按键检测的重复间隔，SDL默认只检测一次按键，有了这个参数就可以开启重复检测按键
	{
		zenglApi_GetFunArg(VM_ARG,5,&arg);
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlCreateWin第五个参数必须是整数，用于设置按键检测的重复间隔");
		if(arg.val.integer > 0)
		{
			SDL_EnableKeyRepeat(arg.val.integer,SDL_DEFAULT_REPEAT_INTERVAL);
		}
	}
	fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)gl_sdlLib_pScreen,SDL_TYPE_POINT); 
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,(ZL_EXP_LONG)gl_sdlLib_pScreen,0);
}

/*
   SDL从事件中捕获用户输入的按键或鼠标点击的事件等。对应脚本函数：sdlPollEvent(event) 
   event参数是一个数组或类，用于存放SDL事件的类型等信息，event类型原型可以参考下面的类定义：
	class clsMouseMotionEvt  //SDL定义的鼠标移动事件C结构体对应的zengl类型
		type;	//SDL_MOUSEMOTION
		which;	//The mouse device index 
		state;	//The current button state 
		x; y;	//The X/Y coordinates of the mouse
		xrel;   //The relative motion in the X direction 
		yrel;	//The relative motion in the Y direction
	endcls

	class clsMouseButtonEvt  //SDL定义的鼠标点击事件C结构体对应的zengl类型
		type;	//SDL_MOUSEBUTTONDOWN or SDL_MOUSEBUTTONUP
		which;	//The mouse device index
		state;	//SDL_PRESSED or SDL_RELEASED
		x; y;	//The X/Y coordinates of the mouse at press time
		button;	//The mouse button index
	endcls

	class clsEvent  //event对应的zengl类型
		type; //事件类型，判断是按键事件，还是鼠标事件等。
		keytype;  //如果是按键，则keytype中存放了按键的类型
		clsMouseMotionEvt motion; //如果是鼠标移动事件，则将移动的坐标等信息存放到motion中。  
		clsMouseButtonEvt button; //如果是鼠标点击事件，则将点击相关的信息存放到button中。
	endcls
*/
ZL_EXP_VOID mySDL_PollEvent(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	SDL_Event gameEvent;
	ZL_EXP_INT retnum;

	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlPollEvent函数必须有且只有一个参数，函数原型：sdlPollEvent(event);\n");
	retnum = SDL_PollEvent(&gameEvent);
	if(retnum != SDL_NOEVENT)
	{
		zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数的值，第一个参数是一个数组或类变量，用于存放SDL事件的类型等信息
		if(arg.type != ZL_EXP_FAT_MEMBLOCK)
			zenglApi_Exit(VM_ARG,"sdlPollEvent函数参数必须是初始化过的数组或初始化过的类变量\n");
		memblock = arg.val.memblock;
		arg.type = ZL_EXP_FAT_INT;
		arg.val.integer = gameEvent.type;
		zenglApi_SetMemBlock(VM_ARG,&memblock,1,&arg); //将参数1对应的数组内存块的第一项设置为游戏的事件类型，可以参考注释里的clsEvent类的type成员
		if(gameEvent.type == SDL_KEYUP || gameEvent.type == SDL_KEYDOWN)
		{
			arg.type = ZL_EXP_FAT_INT;
			arg.val.integer = gameEvent.key.keysym.sym;
			zenglApi_SetMemBlock(VM_ARG,&memblock,2,&arg); //如果是按键事件，则将参数数组的第二项设置为按键类型，可以参考注释里的clsEvent类的keytype成员
		}
		else if(gameEvent.type == SDL_MOUSEMOTION || gameEvent.type == SDL_MOUSEBUTTONDOWN ||
				 gameEvent.type == SDL_MOUSEBUTTONUP)
		{
			ZL_EXP_BOOL flag=ZL_EXP_FALSE;
			ZENGL_EXPORT_MEMBLOCK motionblock = {0};
			ZENGL_EXPORT_MOD_FUN_ARG mblk_val = {ZL_EXP_FAT_NONE,{0}};
			if(gameEvent.type == SDL_MOUSEMOTION)
				mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3); //如果是鼠标移动事件，获取参数数组的第三项
			else
				mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4); //如果是鼠标点击事件，获取参数数组的第四项
			if(mblk_val.type == ZL_EXP_FAT_MEMBLOCK) //判断第三项或第四项是否是数组内存块，如果不是就用zenglApi_CreateMemBlock分配一个内存块并设置flag设为ZL_EXP_TRUE,表示内存块是脚本函数内部新分配的
				motionblock = mblk_val.val.memblock;
			else
			{
				zenglApi_CreateMemBlock(VM_ARG,&motionblock,0);
				flag = ZL_EXP_TRUE;
			}
			arg.type = ZL_EXP_FAT_INT;
			arg.val.integer = gameEvent.motion.x;
			zenglApi_SetMemBlock(VM_ARG,&motionblock,4,&arg); //设置游戏鼠标的当前坐标的x值到第三项或第四项内存块的第4个元素，可以参考clsMouseMotionEvt或clsMouseButtonEvt类的x成员
			arg.val.integer = gameEvent.motion.y;
			zenglApi_SetMemBlock(VM_ARG,&motionblock,5,&arg); //设置当前坐标的y值到内存块第5个元素，可以参考clsMouseMotionEvt或clsMouseButtonEvt类的y成员
			if(flag) //如果是脚本函数内部新分配的内存块，则将新分配的内存块设置到参数的第三项或第四项
			{
				arg.type = ZL_EXP_FAT_MEMBLOCK;
				arg.val.memblock = motionblock;
				if(gameEvent.type == SDL_MOUSEMOTION)  //鼠标移动事件，则设置参数的第三项
					zenglApi_SetMemBlock(VM_ARG,&memblock,3,&arg);
				else 
					zenglApi_SetMemBlock(VM_ARG,&memblock,4,&arg); //鼠标点击事件，则设置参数内存块的第四项
			}
		}//if(gameEvent.type == SDL_KEYUP || gameEvent.type == SDL_KEYDOWN)...else if(gameEvent.type == SDL_MOUSEMOTION || ....
	}//if(retnum != SDL_NOEVENT)
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
}

/*
   设置游戏的背景图片，对应的脚本函数：sdlSetBkImg(picname | surface point) 参数含义见下面的源码
*/
ZL_EXP_VOID mySDL_SetBkImg(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface* pShownBMP = ZL_EXP_NULL,* tmpBmp = ZL_EXP_NULL;
	ZL_EXP_CHAR PathBuffer[SDL_LIB_MAX_PATH_BUFFER_SIZE] = {0};
	ZL_EXP_CHAR * utf8_path = ZL_EXP_NULL;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlSetBkImg函数必须有且只有一个参数，函数原型：sdlSetBkImg(picname | surface point);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数的值，第一个参数是背景图片的文件名
	if(arg.type == ZL_EXP_FAT_STR)
	{
		if(zenglApi_makePathFileName(VM_ARG,arg.val.str,PathBuffer,SDL_LIB_MAX_PATH_BUFFER_SIZE) == -1)
			zenglApi_Exit(VM_ARG,"sdlSetBkImg函数在生成文件名的有效路径信息时发生错误:%s",zenglApi_GetErrorString(VM_ARG));

#ifdef ZL_EXP_OS_IN_WINDOWS
		utf8_path = GbkToUtf8ForWindows(VM_ARG,PathBuffer);
#else
		utf8_path = PathBuffer;
#endif

		if((tmpBmp = IMG_Load(utf8_path)) == ZL_EXP_NULL) //使用IMG_load函数就可以加载.jpg , .png之类的文件
			zenglApi_Exit(VM_ARG,"sdlSetBkImg在调用SDL的API：IMG_Load时发生错误：%s",SDL_GetError());

#ifdef ZL_EXP_OS_IN_WINDOWS
		zenglApi_FreeMem(VM_ARG,utf8_path);
#endif

		pShownBMP = SDL_DisplayFormat(tmpBmp); //将位图转为游戏画面的显示格式
		SDL_FreeSurface(tmpBmp);
	}
	else if(arg.type == ZL_EXP_FAT_INT)
	{
		if(fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)arg.val.integer,SDL_TYPE_POINT)) //检测SDL指针是否有效
			pShownBMP = (SDL_Surface*)arg.val.integer;
		else
			zenglApi_Exit(VM_ARG,"sdlSetBkImg函数的参数不是一个有效的SDL指针\n");
	}
	else
		zenglApi_Exit(VM_ARG,"sdlSetBkImg函数的参数类型无效，该函数参数要么是字符串类型表示要加载的位图文件名，要么是整数类型表示一个有效的SDL表面指针\n");
	
	if(pShownBMP == ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"sdlSetBkImg加载位图指针信息失败：%s",SDL_GetError());

	if(SDL_BlitSurface(pShownBMP, 0, gl_sdlLib_pScreen , 0)) //将位图表面绘制到主屏幕表面
		zenglApi_Exit(VM_ARG,"sdlSetBkImg将位图绘制到主屏幕表面失败：%s",SDL_GetError());

	if(arg.type == ZL_EXP_FAT_STR) //如果参数是字符串，说明是新生成的SDL表面指针，则将其加入到指针动态数组中，进行统一管理
		fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)pShownBMP,SDL_TYPE_POINT);

	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,(ZL_EXP_LONG)pShownBMP,0);
}

/*
   将游戏绘制好的帧显示出来，因为SDL的绘制操作都是绘制在缓存中的，必须调用SDL_Flip进行翻转，才能将缓存的数据输出到显卡显示出来
   对应的脚本函数：sdlShowScreen();
*/
ZL_EXP_VOID mySDL_ShowScreen(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZL_EXP_INT retnum;
	if((retnum = SDL_Flip(gl_sdlLib_pScreen)) != 0 ) //将SDL主屏幕表面翻转到前端显示
		zenglApi_Exit(VM_ARG,"sdlShowScreen函数更新主屏幕失败:%s \n",SDL_GetError());
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
}

/*
    加载位图，对应的脚本函数：sdlLoadBmp(picname,red[option],green[option],blue[option]) (option表示可选参数)
*/
ZL_EXP_VOID mySDL_LoadBmp(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface* pShownBMP = ZL_EXP_NULL,* tmpBmp = ZL_EXP_NULL;
	ZL_EXP_CHAR PathBuffer[SDL_LIB_MAX_PATH_BUFFER_SIZE] = {0};
	ZL_EXP_CHAR * utf8_path = ZL_EXP_NULL;
	if(argcount != 1 && argcount !=4)
		zenglApi_Exit(VM_ARG,"sdlLoadBmp函数必须有1个或4个参数，函数原型：函数原型是：函数原型：sdlLoadBmp(picname,red[option],green[option],blue[option]);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数的值，第一个参数是位图的文件名
	if(arg.type == ZL_EXP_FAT_STR)
	{
		if(zenglApi_makePathFileName(VM_ARG,arg.val.str,PathBuffer,SDL_LIB_MAX_PATH_BUFFER_SIZE) == -1)
			zenglApi_Exit(VM_ARG,"sdlLoadBmp函数在生成文件名的有效路径信息时发生错误:%s",zenglApi_GetErrorString(VM_ARG));

#ifdef ZL_EXP_OS_IN_WINDOWS
		utf8_path = GbkToUtf8ForWindows(VM_ARG,PathBuffer);
#else
		utf8_path = PathBuffer;
#endif

		if((tmpBmp = IMG_Load(utf8_path)) == ZL_EXP_NULL) //使用IMG_load函数就可以加载.jpg , .png之类的文件
			zenglApi_Exit(VM_ARG,"sdlLoadBmp在调用SDL的API：IMG_Load时发生错误：%s",SDL_GetError());

#ifdef ZL_EXP_OS_IN_WINDOWS
		zenglApi_FreeMem(VM_ARG,utf8_path);
#endif

		pShownBMP = SDL_DisplayFormat(tmpBmp); //将位图转为游戏画面的显示格式
		SDL_FreeSurface(tmpBmp);
	}
	else
		zenglApi_Exit(VM_ARG,"sdlLoadBmp函数第一个参数必须是字符串类型，表示要加载位图的文件名");

	if(argcount == 4) //如果有四个参数，则设置位图的colorkey，colorkey可以过滤掉不要的颜色，产生透明背景效果
	{
		ZL_EXP_INT r,g,b;
		Uint32 colorkey;
		zenglApi_GetFunArg(VM_ARG,2,&arg);
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlLoadBmp函数第2个,第3个和第4个参数必须是整数类型");
		r = arg.val.integer; //colorkey的red值即红色的值

		zenglApi_GetFunArg(VM_ARG,3,&arg);
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlLoadBmp函数第2个,第3个和第4个参数必须是整数类型");
		g = arg.val.integer; //colorkey的green值即绿色部分的值

		zenglApi_GetFunArg(VM_ARG,4,&arg);
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlLoadBmp函数第2个,第3个和第4个参数必须是整数类型");
		b = arg.val.integer; //colorkey的blue值即蓝色部分的值
		colorkey = SDL_MapRGB( pShownBMP->format, r, g, b );
		SDL_SetColorKey(pShownBMP, SDL_SRCCOLORKEY, colorkey); //设置colorkey
	}
	fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)pShownBMP,SDL_TYPE_POINT);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,(ZL_EXP_LONG)pShownBMP,0);
}

/*
   将一个位图绘制到另一个位图上面，对应的脚本函数：sdlBlitImg(src surface,src rect,dest surface,dest rect);
*/
ZL_EXP_VOID mySDL_BlitImg(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MOD_FUN_ARG mblk_val = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface* pSrc = ZL_EXP_NULL;
	SDL_Surface* pDest = ZL_EXP_NULL;
	SDL_Rect srcRect = {0};
	SDL_Rect destRect = {0};
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	ZL_EXP_INT srcflag = ZL_EXP_FALSE;
	ZL_EXP_INT destflag = ZL_EXP_FALSE;
	ZL_EXP_LONG srcnum,destnum;
	ZL_EXP_INT retnum;

	if(argcount != 4)
		zenglApi_Exit(VM_ARG,"sdlBlitImg函数必须有四个参数，函数原型是：sdlBlitImg(src surface,src rect,dest surface,dest rect);\n");

	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数的值，第一个参数是要绘制的原位图
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlBlitImg函数第一个参数必须是整数类型，表示要绘制的原位图表面指针\n");
	pSrc = (SDL_Surface*)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)pSrc,SDL_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlBlitImg函数第一个参数不是有效的原位图表面指针\n");

	zenglApi_GetFunArg(VM_ARG,2,&arg); //获取第二个参数，该参数是原位图要绘制的矩形区域
	if(arg.type == ZL_EXP_FAT_NONE || arg.type == ZL_EXP_FAT_INT) //如果是整数，说明是SDL_Rect的指针
	{
		srcflag = ZL_EXP_TRUE;
		srcnum = arg.val.integer;
		if(srcnum != 0) //目前只支持0整数值，表示原位图全部的矩形区域
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第二个参数不是有效的原位图矩形区域\n");
	}
	else if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlBlitImg函数第二个参数必须是初始化过的数组或初始化过的类变量或整数0\n");
	else
	{
		memblock = arg.val.memblock;
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第二个参数里的成员都必须是整数类型\n");
		else
			srcRect.x = (Sint16)mblk_val.val.integer;
		
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第二个参数里的成员都必须是整数类型\n");
		else
			srcRect.y = (Sint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第二个参数里的成员都必须是整数类型\n");
		else
			srcRect.w = (Uint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第二个参数里的成员都必须是整数类型\n");
		else
			srcRect.h = (Uint16)mblk_val.val.integer;
	}
	
	zenglApi_GetFunArg(VM_ARG,3,&arg); //获取第三个参数，该参数是要绘制的目标位图
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlBlitImg函数第三个参数必须是整数类型，表示要绘制的目标位图表面指针\n");
	pDest = (SDL_Surface*)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)pDest,SDL_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlBlitImg函数第三个参数不是有效的目标位图表面指针\n");
	
	zenglApi_GetFunArg(VM_ARG,4,&arg); //第四个参数是要绘制的目标矩形区域
	if(arg.type == ZL_EXP_FAT_NONE || arg.type == ZL_EXP_FAT_INT) //如果是整数，说明是SDL_Rect的指针
	{
		destflag = ZL_EXP_TRUE;
		destnum = arg.val.integer;
		if(destnum != 0) //目前只支持0整数值，表示目标位图全部的矩形区域
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第四个参数不是有效的目标位图矩形区域\n");
	}
	else if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlBlitImg函数第四个参数必须是初始化过的数组或初始化过的类变量或整数0\n");
	else
	{
		memblock = arg.val.memblock;
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第四个参数里的成员都必须是整数类型\n");
		else
			destRect.x = (Sint16)mblk_val.val.integer;
		
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第四个参数里的成员都必须是整数类型\n");
		else
			destRect.y = (Sint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第四个参数里的成员都必须是整数类型\n");
		else
			destRect.w = (Uint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数第四个参数里的成员都必须是整数类型\n");
		else
			destRect.h = (Uint16)mblk_val.val.integer;
	}
	
	if(srcflag == ZL_EXP_TRUE) //当第二个参数本身就是SDL_Rect指针时
	{
		if(destflag) //当第四个参数本身就是SDL_Rect指针时
			retnum = SDL_BlitSurface(pSrc,(SDL_Rect *)srcnum , pDest , (SDL_Rect *)destnum);
		else
			retnum = SDL_BlitSurface(pSrc,(SDL_Rect *)srcnum , pDest , &destRect);
		if(retnum) 
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数调用SDL API：SDL_BlitSurface失败：%s\n",SDL_GetError());
	}
	else
	{
		if(destflag) //当第四个参数本身就是SDL_Rect指针时
			retnum = SDL_BlitSurface(pSrc, &srcRect, pDest , (SDL_Rect *)destnum);
		else
			retnum = SDL_BlitSurface(pSrc, &srcRect, pDest , &destRect);
		if(retnum)
			zenglApi_Exit(VM_ARG,"sdlBlitImg函数调用SDL API：SDL_BlitSurface失败：%s\n",SDL_GetError());
	}
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
}

/*
   获取按键状态，判断当前用户按下的是哪些键，如上下左右键等
   返回的结果是一个数组，数组里记录了所有键的按键状态，所以当用户同时按下了两个或三个键时，就可以用这种方法，在数组中判断出来
   如用户同时按下了上键和右键，那么数组中273和275项就会是ZL_EXP_TRUE即1时表示按下状态
   对应的脚本函数：sdlGetKeyState(array keystate);
*/
ZL_EXP_VOID mySDL_GetKeyState(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT numkey,i;
	Uint8 *keystate = ZL_EXP_NULL;
	ZENGL_EXPORT_MEMBLOCK memblock = {0};

	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlGetKeyState函数必须有且只有一个参数，函数原型是：sdlGetKeyState(array keystate);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数是存放结果的数组
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlGetKeyState函数的参数必须是初始化过的数组或初始化过的类变量\n");
	
	memblock = arg.val.memblock;
	keystate = SDL_GetKeyState(&numkey);
	arg.type = ZL_EXP_FAT_INT;
	for(i=1;i <= numkey;i++)
	{
		arg.val.integer = keystate[i];
		zenglApi_SetMemBlock(VM_ARG,&memblock,i,&arg);
	}
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,numkey,0);
}

/*
   设置游戏窗口的标题，对应的脚本函数是：sdlSetCaption(caption);
*/
ZL_EXP_VOID mySDL_SetCaption(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_CHAR * caption = ZL_EXP_NULL;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlSetCaption函数必须有且只有一个参数，函数原型是：sdlSetCaption(caption);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数是需要设置的窗口标题
	if(arg.type == ZL_EXP_FAT_STR)
	{

#ifdef ZL_EXP_OS_IN_WINDOWS
		caption = GbkToUtf8ForWindows(VM_ARG,arg.val.str);
#else
		caption = arg.val.str;
#endif

		SDL_WM_SetCaption(caption,ZL_EXP_NULL);

#ifdef ZL_EXP_OS_IN_WINDOWS
		zenglApi_FreeMem(VM_ARG,caption);
#endif

	}
	else
		zenglApi_Exit(VM_ARG,"sdlSetCaption函数的第一个参数必须是字符串，表示需要设置的窗口标题\n");
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}

/*
   设置某位图表面的colorkey
   对应的脚本函数为：sdlSetColorKey(image surface , red , green , blue);
*/
ZL_EXP_VOID mySDL_SetColorKey(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface * image;
	ZL_EXP_INT r,g,b;
	Uint32 colorkey;

	if(argcount != 4)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey函数必须有四个参数，函数原型是：sdlSetColorKey(image surface , red , green , blue);\n");

	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数是位图的表面指针
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey函数第一个参数必须是整数类型，表示需要操作的位图的表面指针\n");
	image = (SDL_Surface*)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)image,SDL_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlSetColorKey函数第一个参数不是有效的位图表面指针\n");
	zenglApi_GetFunArg(VM_ARG,2,&arg); 
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey函数第二个参数必须是整数类型，表示colorkey的red值\n");
	r = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,3,&arg); 
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey函数第三个参数必须是整数类型，表示colorkey的green值\n");
	g = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,4,&arg); 
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey函数第四个参数必须是整数类型，表示colorkey的blue值\n");
	b = arg.val.integer;

	colorkey = SDL_MapRGB( image->format, r, g, b );
	r = SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey);
	if(r == -1)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey函数调用SDL API：SDL_SetColorKey失败：%s\n",SDL_GetError());
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,r,0);
}

/*
   获取游戏运行的滴答数，可以用来判断游戏运行的时间，或者用来计算游戏的帧数等。
   对应的脚本函数为：sdlGetTicks();
*/
ZL_EXP_VOID mySDL_GetTicks(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,SDL_GetTicks(),0);
}

/*
   打开某字体文件
   对应的脚本函数为：sdlOpenFont(font_filename , font_size);
*/
ZL_EXP_VOID mySDL_OpenFont(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_CHAR * filename = ZL_EXP_NULL;
	ZL_EXP_CHAR * utf8_Path = ZL_EXP_NULL;
	ZL_EXP_CHAR PathBuffer[SDL_LIB_MAX_PATH_BUFFER_SIZE] = {0};
	ZL_EXP_INT ptsize;
	ZL_EXP_LONG retnum;
	if(argcount != 2)
		zenglApi_Exit(VM_ARG,"sdlOpenFont函数必须有两个参数，函数原型是：sdlOpenFont(font_filename , font_size);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数是字体文件的文件名，一般是.ttf为后缀的文件
	if(arg.type == ZL_EXP_FAT_STR)
	{
		filename = arg.val.str;
		zenglApi_GetFunArg(VM_ARG,2,&arg); //第二个参数为字体大小
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlOpenFont函数第二个参数必须是整数类型，表示字体大小\n");
		ptsize = arg.val.integer;
		if(zenglApi_makePathFileName(VM_ARG,filename,PathBuffer,SDL_LIB_MAX_PATH_BUFFER_SIZE) == -1)
			zenglApi_Exit(VM_ARG,"sdlOpenFont函数在生成文件名的有效路径信息时发生错误:%s",zenglApi_GetErrorString(VM_ARG)); 

#ifdef ZL_EXP_OS_IN_WINDOWS
		utf8_Path = GbkToUtf8ForWindows(VM_ARG,PathBuffer);
#else
		utf8_Path = PathBuffer;
#endif

		retnum = (ZL_EXP_LONG)TTF_OpenFont(utf8_Path,ptsize);

#ifdef ZL_EXP_OS_IN_WINDOWS
		zenglApi_FreeMem(VM_ARG,utf8_Path);
#endif

		if(retnum == 0)
			zenglApi_Exit(VM_ARG,"sdlOpenFont函数调用SDL API：TTF_OpenFont失败：%s",SDL_GetError());
		else
		{
			fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)retnum,FONT_TYPE_POINT);
			zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
		}
	}
	else
		zenglApi_Exit(VM_ARG,"sdlOpenFont函数第一个参数必须是字符串类型，表示字体文件的文件名\n");
}

/*
   在绘制文字时，通过提供要绘制的字符串参数和字体的颜色等参数，然后TTF的API就会根据这些参数生成一个可供绘制的位图表面返回。
   返回的位图表面接着就可以用上面提到过的方法，将该字符串位图绘制到另一个表面上。
   对应的脚本函数为：sdlDrawText(font obj , string , array fontcolor , array bgcolor[option]); (option表示是可选参数)
*/
ZL_EXP_VOID mySDL_DrawText(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	TTF_Font *font = ZL_EXP_NULL;
	ZL_EXP_CHAR * text = ZL_EXP_NULL;
	ZL_EXP_CHAR * tmptext = ZL_EXP_NULL;
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MOD_FUN_ARG mblk_val = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	SDL_Color frontcolor={0};
	SDL_Color bgcolor={0};
	ZL_EXP_LONG retnum;
	if(argcount != 4 && argcount != 3)
		zenglApi_Exit(VM_ARG,"sdlDrawText函数必须有三个或四个参数，函数原型是：sdlDrawText(font obj , string , array fontcolor , array bgcolor[option]);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数是之前打开的TTF_Font字体指针
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawText函数第一个参数必须是整数类型，表示之前打开的TTF_Font字体指针\n");
	font = (TTF_Font *)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)font,FONT_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlDrawText函数第一个参数不是一个有效的字体对象指针\n");

	zenglApi_GetFunArg(VM_ARG,2,&arg); //第二个参数是要绘制的字符串信息
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"sdlDrawText函数第二个参数必须是字符串，表示要绘制的字符串信息\n");
	text = arg.val.str;
	
	zenglApi_GetFunArg(VM_ARG,3,&arg); //第三个参数是一个数组或类变量，取前三项依次作为字体前景色的r,g,b值
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlDrawText函数第三个参数必须是初始化过的数组或初始化过的类变量，表示前景色的r,g,b值\n");
	memblock = arg.val.memblock;
	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawText函数第三个参数里的成员必须都是整数类型\n");
	frontcolor.r =  (Uint8)mblk_val.val.integer;
	
	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawText函数第三个参数里的成员必须都是整数类型\n");
	frontcolor.g = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawText函数第三个参数里的成员必须都是整数类型\n");
	frontcolor.b = (Uint8)mblk_val.val.integer;

	if(argcount == 4)
	{
		zenglApi_GetFunArg(VM_ARG,4,&arg); //第四个参数是一个数组或类变量，取前三项依次作为字体背景色的r,g,b值
		if(arg.type != ZL_EXP_FAT_MEMBLOCK)
			zenglApi_Exit(VM_ARG,"sdlDrawText函数第四个参数必须是初始化过的数组或初始化过的类变量，表示前景色的r,g,b值\n");
		memblock = arg.val.memblock;
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawText函数第四个参数里的成员必须都是整数类型\n");
		bgcolor.r =  (Uint8)mblk_val.val.integer;
		
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawText函数第四个参数里的成员必须都是整数类型\n");
		bgcolor.g = (Uint8)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawText函数第四个参数里的成员必须都是整数类型\n");
		bgcolor.b = (Uint8)mblk_val.val.integer;
	}

#ifdef ZL_EXP_OS_IN_WINDOWS
	tmptext = GbkToUtf8ForWindows(VM_ARG,text);
#else
	tmptext = text;
#endif

	if(argcount == 3)	//如果只有三个参数，则只设置前景色
		retnum = (ZL_EXP_LONG)TTF_RenderUTF8_Solid(font,tmptext,frontcolor); //注意只能渲染UTF8的汉字！！
	else	 //如果有四个参数，则设置前景色和背景色
		retnum = (ZL_EXP_LONG)TTF_RenderUTF8_Shaded(font,tmptext,frontcolor,bgcolor);

#ifdef ZL_EXP_OS_IN_WINDOWS
	zenglApi_FreeMem(VM_ARG,tmptext);
#endif

	if(retnum == 0)
		zenglApi_Exit(VM_ARG,"sdlDrawText函数调用SDL API：TTF_RenderUTF8_Solid或TTF_RenderUTF8_Shaded时失败：%s\n",SDL_GetError());
	fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)retnum,SDL_TYPE_POINT); //将返回的位图表面指针加入到动态数组中，进行统一管理
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0); //返回包含字符串信息的可供绘制的位图表面指针
}

/*
   关闭字体
   对应的脚本函数为：sdlCloseFont(font obj);
*/
ZL_EXP_VOID mySDL_CloseFont(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	TTF_Font * font_point;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlCloseFont函数必须有且只有一个参数，函数原型是：sdlCloseFont(font obj);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数是字体对象指针
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCloseFont第一个参数必须是整数，表示字体对象指针\n");
	font_point = (TTF_Font *)arg.val.integer;
	if(fun_sdlLib_Free_Point(VM_ARG,(ZL_EXP_VOID *)font_point,FONT_TYPE_POINT) == -1)
		zenglApi_Exit(VM_ARG,"sdlCloseFont的参数不是有效的字体指针\n");
	TTF_CloseFont(font_point);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}

/*
   释放某个位图表面资源
   对应的脚本函数为：sdlFreeImage(image surface);
*/
ZL_EXP_VOID mySDL_FreeImage(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface * surface;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlFreeImage函数必须有且只有一个参数，函数原型是：sdlFreeImage(image surface);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数是要释放的SDL表面指针
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlFreeImage函数的第一个参数必须是整数类型，表示要释放的SDL表面指针\n");
	surface = (SDL_Surface*)arg.val.integer;
	if(fun_sdlLib_Free_Point(VM_ARG,(ZL_EXP_VOID *)surface,SDL_TYPE_POINT) == -1)
		zenglApi_Exit(VM_ARG,"sdlFreeImage的参数不是有效的SDL表面指针\n");
	SDL_FreeSurface(surface);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}

/*
   字体资源初始化
   对应的脚本函数为：sdlFontInit();
*/
ZL_EXP_VOID mySDL_FontInit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZL_EXP_INT retnum;
	if((retnum = TTF_Init())==-1)
		zenglApi_Exit(VM_ARG,"sdlFontInit函数调用SDL API：TTF_Init失败：%s\n",SDL_GetError());
	gl_sdlLib_initFlags[INIT_FONT] = ZL_EXP_TRUE;
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
}

/*
    将目标矩形区域填充为指定的颜色。
	对应的脚本函数为：sdlFillRect(dest surface,dest rect , array color_alpha);
*/
ZL_EXP_VOID mySDL_FillRect(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MOD_FUN_ARG mblk_val = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface* pDest = ZL_EXP_NULL;
	SDL_Rect Rect = {0};
	SDL_Rect * pDestRect = &Rect;
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	Uint32 color;
	Uint8 r,g,b,a;
	ZL_EXP_INT ret;
	ZL_EXP_INT mblk_count;

	if(argcount != 3)
		zenglApi_Exit(VM_ARG,"sdlFillRect函数必须有三个参数，函数原型是：sdlFillRect(dest surface,dest rect , array color_alpha);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数里存放有目标表面的指针值
	if(arg.type != ZL_EXP_FAT_INT) 
		zenglApi_Exit(VM_ARG,"sdlFillRect函数第一个参数必须是整数类型，表示目标表面的指针值\n");
	pDest = (SDL_Surface*)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)pDest,SDL_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlFillRect函数第一个参数不是有效的SDL表面指针\n");
	zenglApi_GetFunArg(VM_ARG,2,&arg);
	if(arg.type == ZL_EXP_FAT_NONE || (arg.type == ZL_EXP_FAT_INT && arg.val.integer == 0)) //第二个参数如果是0则说明填充目标的所有区域
		pDestRect = ZL_EXP_NULL;
	else if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlFillRect函数第二个参数必须是初始化过的数组或初始化过的类变量或0，表示要填充的矩形区域\n");
	else
	{
		memblock = arg.val.memblock; //第二个参数如果是数组内存块，则前四项分别对应目标矩形区域的x,y,w(宽),h(高)
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect函数第二个参数里的成员都必须是整数类型\n");
		pDestRect->x = (Sint16)mblk_val.val.integer;
		
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect函数第二个参数里的成员都必须是整数类型\n");
		pDestRect->y = (Sint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect函数第二个参数里的成员都必须是整数类型\n");
		pDestRect->w = (Uint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect函数第二个参数里的成员都必须是整数类型\n");
		pDestRect->h = (Uint16)mblk_val.val.integer;
	}
	
	zenglApi_GetFunArg(VM_ARG,3,&arg); //第三个参数为要设置的颜色数组
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlFillRect函数第三个参数必须是初始化过的数组或初始化过的类变量，表示要填充的颜色的r,g,b,a值\n");

	memblock = arg.val.memblock;
	zenglApi_GetMemBlockInfo(VM_ARG,&memblock,ZL_EXP_NULL,&mblk_count);
	if(mblk_count !=3 && mblk_count != 4)
		zenglApi_Exit(VM_ARG,"sdlFillRect函数第三个参数的元素个数必须有三个或四个，表示r,g,b值或r,g,b,a值\n");

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlFillRect函数第三个参数里的成员都必须是整数类型\n");
	r = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlFillRect函数第三个参数里的成员都必须是整数类型\n");
	g = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlFillRect函数第三个参数里的成员都必须是整数类型\n");
	b = (Uint8)mblk_val.val.integer;

	if(mblk_count == 4) //如果有第四项，则设置颜色的alpha透明值
	{
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect函数第三个参数里的成员都必须是整数类型\n");
		a = (Uint8)mblk_val.val.integer;
		color = SDL_MapRGBA(pDest->format,r,g,b,a);
	}
	else
		color = SDL_MapRGB(pDest->format,r,g,b);

	ret = SDL_FillRect(pDest,pDestRect,color); //调用SDL_FillRect填充目标
	if(ret == -1)
		zenglApi_Exit(VM_ARG,"sdlFillRect函数调用SDL API：SDL_FillRect失败：%s\n",SDL_GetError());
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,ret,0);
}

/*
   在绘制文字时，通过提供要绘制的字符串参数和字体的颜色等参数，然后TTF的API就会根据这些参数生成一个可供绘制的位图表面返回。
   返回的位图表面接着就可以用上面提到过的方法，将该字符串位图绘制到另一个表面上。
   对应的脚本函数为：sdlDrawTextEx(ref surface,font obj , string , array fontcolor , array bgcolor[option]); (option表示是可选参数)
   sdlDrawTextEx 和 sdlDrawText区别在于，sdlDrawTextEx可以自动释放第一个参数的原表面指针，sdlDrawText返回的表面指针需要手动释放掉。
*/
ZL_EXP_VOID mySDL_DrawTextEx(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	TTF_Font *font = ZL_EXP_NULL;
	SDL_Surface * textSurface = ZL_EXP_NULL;
	ZL_EXP_CHAR * text = ZL_EXP_NULL;
	ZL_EXP_CHAR * tmptext = ZL_EXP_NULL;
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MOD_FUN_ARG mblk_val = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	SDL_Color frontcolor={0};
	SDL_Color bgcolor={0};
	ZL_EXP_LONG retnum;

	if(argcount != 4 && argcount != 5)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数必须有四个或五个参数，"
			"函数原型是：sdlDrawTextEx(ref surface,font obj , string , array fontcolor , array bgcolor[option]); (option表示是可选参数)\n");
	zenglApi_GetFunArgInfo(VM_ARG,1,&arg);
	if(arg.type != ZL_EXP_FAT_ADDR && 
		arg.type != ZL_EXP_FAT_ADDR_LOC &&
		arg.type != ZL_EXP_FAT_ADDR_MEMBLK
		)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第一个参数必须是变量的引用\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数，该参数是字符串表面指针的变量引用，通过此引用可以直接将其之前的表面给释放掉
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第一个参数必须是整数类型的变量的引用\n");
	textSurface = (SDL_Surface *)arg.val.integer;
	if(fun_sdlLib_Free_Point(VM_ARG,(ZL_EXP_VOID *)textSurface,SDL_TYPE_POINT) != -1) //如果找到对应的SDL指针则释放，没有则跳过
		SDL_FreeSurface(textSurface);

	zenglApi_GetFunArg(VM_ARG,2,&arg); //第二个参数是之前打开的TTF_Font字体指针
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第二个参数必须是整数类型，表示TTF_Font字体指针\n");
	font = (TTF_Font *)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)font,FONT_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第二个参数不是一个有效的字体对象指针\n");
	
	zenglApi_GetFunArg(VM_ARG,3,&arg); //第三个参数是要绘制的字符串信息
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第三个参数必须是字符串，表示要绘制的字符串信息\n");
	text = arg.val.str;
	
	zenglApi_GetFunArg(VM_ARG,4,&arg); //第四个参数是一个数组或类，取前三项依次作为字体前景色的r,g,b值
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第四个参数必须是初始化过的数组或初始化过的类变量，表示前景色\n");
	memblock = arg.val.memblock;
	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1); 
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第四个参数里的成员必须都是整数类型\n");
	frontcolor.r = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2); 
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第四个参数里的成员必须都是整数类型\n");
	frontcolor.g = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3); 
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第四个参数里的成员必须都是整数类型\n");
	frontcolor.b = (Uint8)mblk_val.val.integer;

	if(argcount == 5)
	{
		zenglApi_GetFunArg(VM_ARG,5,&arg); //第五个参数是一个数组或类，取前三项依次作为字体背景色的r,g,b值
		if(arg.type != ZL_EXP_FAT_MEMBLOCK)
			zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第五个参数必须是初始化过的数组或初始化过的类变量，表示背景色\n");
		memblock = arg.val.memblock;
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1); 
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第五个参数里的成员必须都是整数类型\n");
		bgcolor.r = (Uint8)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2); 
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第五个参数里的成员必须都是整数类型\n");
		bgcolor.g = (Uint8)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3); 
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数第五个参数里的成员必须都是整数类型\n");
		bgcolor.b = (Uint8)mblk_val.val.integer;
	}

#ifdef ZL_EXP_OS_IN_WINDOWS
	tmptext = GbkToUtf8ForWindows(VM_ARG,text);
#else
	tmptext = text;
#endif

	if(argcount == 4)	//如果只有四个参数，则只设置前景色
		retnum = (ZL_EXP_LONG)TTF_RenderUTF8_Solid(font,tmptext,frontcolor); //注意只能渲染UTF8的汉字！！
	else	//如果有五个参数，则设置前景色和背景色
		retnum = (ZL_EXP_LONG)TTF_RenderUTF8_Shaded(font,tmptext,frontcolor,bgcolor);

#ifdef ZL_EXP_OS_IN_WINDOWS
	zenglApi_FreeMem(VM_ARG,tmptext);
#endif

	if(retnum == 0)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx函数调用SDL API：TTF_RenderUTF8_Solid或TTF_RenderUTF8_Shaded失败：%s\n",SDL_GetError());
	fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)retnum,SDL_TYPE_POINT); //将返回的位图表面指针加入到动态数组中，进行统一管理
	arg.type = ZL_EXP_FAT_INT;
	arg.val.integer = retnum;
	zenglApi_SetFunArg(VM_ARG,1,&arg); //将第一个参数引用的变量设置为新的表面指针
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0); //返回包含字符串信息的可供绘制的位图表面指针
}

/*
   SDL延时
   用于控制游戏帧数时会用到。
   对应的脚本函数为：sdlDelay(millisecond);
*/
ZL_EXP_VOID mySDL_Delay(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlDelay函数必须有一个参数，函数原型是：sdlDelay(millisecond);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //获取第一个参数的值，第一个参数是要延时的毫秒数
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDelay第一个参数必须是整数，表示要延时的毫秒数");
	SDL_Delay((Uint32)arg.val.integer);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}
