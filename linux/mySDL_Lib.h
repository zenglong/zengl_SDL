#ifndef MY_SDL_LIB_H_
#define MY_SDL_LIB_H_

#define SDL_LIB_INIT_NUM 2 //SDL初始化数，如SDL_Init,TTF_Init等
#define SDL_LIB_POINT_SIZE 20 //SDL指针动态数组初始化和动态扩容的大小
#define SDL_LIB_HASHSIZE 211 //hash表数组大小
#define SDL_LIB_MAX_PATH_BUFFER_SIZE 350 //文件路径缓冲区域的尺寸

typedef enum _SDL_LIB_INIT_FLAGS{
	INIT_FONT,INIT_SDL
}SDL_LIB_INIT_FLAGS;

typedef enum _SDL_LIB_POINT_TYPE{
	FONT_TYPE_POINT,SDL_TYPE_POINT
}SDL_LIB_POINT_TYPE; //指针类型，目前为FONT字体指针，SDL表面指针

typedef struct _SDL_LIB_FREE_POINT_LIST{
	ZL_EXP_BOOL isInit; //判断是否初始化
	int size;
	int count;
	int * frees;
}SDL_LIB_FREE_POINT_LIST; //用于存放SDL释放了的指针索引

typedef struct _SDL_LIB_POINT_MEMBER{
	ZL_EXP_BOOL isvalid;
	void * point;
	SDL_LIB_POINT_TYPE type;
	int next; //指向HASH链中的下一个元素。
}SDL_LIB_POINT_MEMBER; //SDL_LIB_POINT_LIST SDL指针动态数组中的成员类型

typedef struct _SDL_LIB_POINT_LIST{
	ZL_EXP_BOOL isInit; //判断是否初始化
	int size;
	int count;
	SDL_LIB_POINT_MEMBER * points;
	int hash[SDL_LIB_HASHSIZE]; //上面points对应的hash查找表
	SDL_LIB_FREE_POINT_LIST freelist; //当points里的指针被SDL API释放掉时，将其索引存放到freelist，下次分配SDL指针时就可以直接使用此索引
}SDL_LIB_POINT_LIST; //SDL存放表面和字体等指针的动态数组。

ZL_EXP_VOID fun_sdlLib_destroyAll(ZL_EXP_VOID * VM_ARG); //用于释放SDL分配的资源

ZL_EXP_VOID mySDL_Init(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL全局初始化函数，对应脚本模块函数：sdlInit()
ZL_EXP_VOID mySDL_Quit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL退出时调用SDL_Quit释放资源，对应脚本模块函数：sdlQuit()
ZL_EXP_VOID mySDL_FontQuit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //通过TTF_Quit清理字体所占用的系统资源 对应的脚本函数是：sdlFontQuit()
ZL_EXP_VOID mySDL_CreateWin(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL引擎创建游戏窗口，对应脚本模块函数：sdlCreateWin(width,height,bpp,flags,repeat[option])
ZL_EXP_VOID mySDL_PollEvent(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL从事件中捕获用户输入的按键或鼠标点击的事件等 对应脚本函数：sdlPollEvent(event) 
ZL_EXP_VOID mySDL_SetBkImg(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //设置游戏的背景图片，对应的脚本函数：sdlSetBkImg(picname | surface point)
ZL_EXP_VOID mySDL_ShowScreen(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //将游戏绘制好的帧显示出来 对应的脚本函数：sdlShowScreen()
ZL_EXP_VOID mySDL_LoadBmp(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //加载位图，对应的脚本函数：sdlLoadBmp(picname,red[option],green[option],blue[option]) (option表示可选参数)
ZL_EXP_VOID mySDL_BlitImg(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //将一个位图绘制到另一个位图上面，对应的脚本函数：sdlBlitImg(src surface,src rect,dest surface,dest rect)
ZL_EXP_VOID mySDL_GetKeyState(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //获取按键状态 对应脚本函数：sdlGetKeyState(array keystate)
ZL_EXP_VOID mySDL_SetCaption(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //设置游戏窗口的标题，对应的脚本函数是：sdlSetCaption(caption)
ZL_EXP_VOID mySDL_SetColorKey(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //设置某位图表面的colorkey 对应的脚本函数为：sdlSetColorKey(image surface , red , green , blue)
ZL_EXP_VOID mySDL_GetTicks(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //获取游戏运行的滴答数 对应的脚本函数为：sdlGetTicks()
ZL_EXP_VOID mySDL_OpenFont(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //打开某字体文件 对应的脚本函数为：sdlOpenFont(font_filename , font_size)
ZL_EXP_VOID mySDL_DrawText(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //绘制文本信息 对应的脚本函数为：sdlDrawText(font obj , string , array fontcolor , array bgcolor[option]); (option表示是可选参数)
ZL_EXP_VOID mySDL_CloseFont(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //关闭字体 对应的脚本函数为：sdlCloseFont(font obj)
ZL_EXP_VOID mySDL_FreeImage(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //释放某个位图表面资源 对应的脚本函数为：sdlFreeImage(image surface)
ZL_EXP_VOID mySDL_FontInit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //字体资源初始化 对应的脚本函数为：sdlFontInit()
ZL_EXP_VOID mySDL_FillRect(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //将目标矩形区域填充为指定的颜色 对应的脚本函数为：sdlFillRect(dest surface,dest rect , array color_alpha)
ZL_EXP_VOID mySDL_DrawTextEx(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //绘制文本信息的扩展函数 对应的脚本函数为：sdlDrawTextEx(ref surface,font obj , string , array fontcolor , array bgcolor[option]); (option表示是可选参数)
ZL_EXP_VOID mySDL_Delay(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL延时，对应脚本模块函数：sdlDelay(millisecond)

/*charset.c里的函数*/
ZL_EXP_CHAR * GbkToUtf8ForWindows(ZL_EXP_VOID * VM_ARG,ZL_EXP_CHAR * gbkstr);

#endif /* MY_SDL_LIB_H_ */
