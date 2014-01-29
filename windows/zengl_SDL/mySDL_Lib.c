#define ZL_EXP_OS_IN_WINDOWS //�ڼ��������zenglͷ�ļ�֮ǰ��windowsϵͳ�붨��ZL_EXP_OS_IN_WINDOWS��linux , macϵͳ�붨��ZL_EXP_OS_IN_LINUX
#include "zengl_exportfuns.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "mySDL_Lib.h"

ZL_EXP_BOOL gl_sdlLib_initFlags[SDL_LIB_INIT_NUM] = {0}; //�����ж�����⣬SDL���Ƿ��ʼ���ı��
SDL_Surface * gl_sdlLib_pScreen = ZL_EXP_NULL; //ȫ�ֵ�SDL���ڱ���
SDL_LIB_POINT_LIST gl_sdlLib_PointsList = {0}; //SDL��ű���������ָ��Ķ�̬����

/*
	�����ͷ�SDL�������Դ
*/
ZL_EXP_VOID fun_sdlLib_destroyAll(ZL_EXP_VOID * VM_ARG)
{
	ZL_EXP_INT i;
	for(i=0;i<gl_sdlLib_PointsList.count;i++) //ѭ���ͷŵ�����SDLָ��
	{
		if(gl_sdlLib_PointsList.points[i].isvalid == ZL_EXP_TRUE && gl_sdlLib_PointsList.points[i].point != ZL_EXP_NULL)
		{
			switch(gl_sdlLib_PointsList.points[i].type)
			{
			case FONT_TYPE_POINT:
				TTF_CloseFont((TTF_Font *)gl_sdlLib_PointsList.points[i].point);
				break;
			case SDL_TYPE_POINT:
				if((SDL_Surface *)gl_sdlLib_PointsList.points[i].point != gl_sdlLib_pScreen) //����Ļָ��Ӧ��SDL_Quit���ͷ�
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
	��ʼ����̬���飬�ö�̬����������SDL�ͷŵ�ָ�������
*/
ZL_EXP_VOID fun_sdlLib_InitFreePointList(ZL_EXP_VOID * VM_ARG)
{
	if(gl_sdlLib_PointsList.freelist.isInit) //����Ѿ���ʼ�������ͷ���
		return;
	gl_sdlLib_PointsList.freelist.size = SDL_LIB_POINT_SIZE; //ʹ�ú�gl_sdlLib_PointsList.pointsһ���ĳ�ʼ�������ݴ�С
	gl_sdlLib_PointsList.freelist.count = 0;
	gl_sdlLib_PointsList.freelist.frees = zenglApi_AllocMem(VM_ARG,gl_sdlLib_PointsList.freelist.size * sizeof(int));
	if(gl_sdlLib_PointsList.freelist.frees == ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_InitFreePointList���������ڴ��ʼ��ʧ��:%s\n",zenglApi_GetErrorString(VM_ARG));
	memset(gl_sdlLib_PointsList.freelist.frees , 0 , gl_sdlLib_PointsList.freelist.size * sizeof(int));
	gl_sdlLib_PointsList.freelist.isInit = ZL_EXP_TRUE;
}

/*
	���ͷŵ�ָ��������ŵ�freelist��̬�����У��´ξͿ���ֱ�������ø�����
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
			zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_FreePoint���������ڴ��Сʧ��:%s\n",zenglApi_GetErrorString(VM_ARG));
		memset(gl_sdlLib_PointsList.freelist.frees + (gl_sdlLib_PointsList.freelist.size - SDL_LIB_POINT_SIZE),0,
			SDL_LIB_POINT_SIZE * sizeof(int));
	}
	gl_sdlLib_PointsList.freelist.frees[gl_sdlLib_PointsList.freelist.count] = index;
	gl_sdlLib_PointsList.freelist.count++;
}

/*
	��ʼ��SDLָ�붯̬����
*/
ZL_EXP_VOID fun_sdlLib_InitPointsList(ZL_EXP_VOID * VM_ARG)
{
	if(gl_sdlLib_PointsList.isInit)
		return;
	gl_sdlLib_PointsList.size = SDL_LIB_POINT_SIZE;
	gl_sdlLib_PointsList.count = 1; //��1��ʼ������0������Ч������
	gl_sdlLib_PointsList.points = zenglApi_AllocMem(VM_ARG,gl_sdlLib_PointsList.size * sizeof(SDL_LIB_POINT_MEMBER));
	if(gl_sdlLib_PointsList.points == ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_InitPointsList���������ڴ��ʼ��ʧ��:%s\n",zenglApi_GetErrorString(VM_ARG));
	memset(gl_sdlLib_PointsList.points , 0 , gl_sdlLib_PointsList.size * sizeof(SDL_LIB_POINT_MEMBER));
	gl_sdlLib_PointsList.isInit = ZL_EXP_TRUE;
}

/*
	��SDLָ����ӵ�gl_sdlLib_PointsList.points��̬�����У��Ժ���ԴӴ˶�̬�����в���ָ�룬�Ӷ��ж�ĳSDLָ���Ƿ�����Ч��ָ��
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
			zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_Point���������ڴ��Сʧ��:%s\n",zenglApi_GetErrorString(VM_ARG));
		memset(gl_sdlLib_PointsList.points + (gl_sdlLib_PointsList.size - SDL_LIB_POINT_SIZE),0,
			SDL_LIB_POINT_SIZE * sizeof(SDL_LIB_POINT_MEMBER));
	}
	if(gl_sdlLib_PointsList.freelist.count > 0) //������ͷŶ�̬��������Ԫ�أ���ֱ��ʹ����ǰ�ͷŵ�����
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
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_Point�����޷���gl_sdlLib_PointsList�Ķ�̬�������ҵ�û��ռ�õ�����\n");
	return -1;
}

/*
	�ͷ�SDL���ָ�룬һ�㷢�����ֶ�����SDL API���ͷ�SDLָ����Դʱ
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
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Free_Point������gl_sdlLib_PointsList�Ķ�̬�������ҵ���Ч������[%d]\n",tmpindex);
	else if(gl_sdlLib_PointsList.points[tmpindex].type != type)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Free_Point������gl_sdlLib_PointsList�Ķ�̬�������ҵ���ָ�������뺯���ṩ�����Ͳ���\n");
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
	���ĳSDLָ���Ƿ�����Ч��ָ�롣
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
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Check_Point������gl_sdlLib_PointsList�Ķ�̬�������ҵ���Ч������[%d]\n",tmpindex);
	else if(gl_sdlLib_PointsList.points[tmpindex].type != type)
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Check_Point������gl_sdlLib_PointsList�Ķ�̬�������ҵ���ָ�������뺯���ṩ�����Ͳ���\n");
	else
	{
		return ZL_EXP_TRUE;
	}
	return ZL_EXP_FALSE;
}

/*
	��SDLָ����뵽��̬�����У����������е�������ӵ���ϣ���У������Ժ�Ĳ��Ҳ�����
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
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_HashTableForPoint������gl_sdlLib_PointsList�Ķ�̬�������ҵ���Ч������[%d]\n",tmpindex);
	else
		zenglApi_Exit(VM_ARG,"fun_sdlLib_Insert_HashTableForPoint�����ṩ��ָ��[%d]��֮ǰ�Ѿ��������",point);
}

/*
  SDLȫ�ֳ�ʼ����������Ӧ�ű�ģ�麯����sdlInit()
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
  SDL�˳�ʱ����SDL_Quit�ͷ���Դ����Ӧ�ű�ģ�麯����sdlQuit()
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
   ͨ��TTF_Quit����������ռ�õ�ϵͳ��Դ
   ��Ӧ�Ľű������ǣ�sdlFontQuit();
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
   SDL���洴����Ϸ���ڣ���Ӧ�ű�������sdlCreateWin(width,height,bpp,flags,repeat[option]); (option��ʾ��ѡ����)
*/
ZL_EXP_VOID mySDL_CreateWin(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT width,height,bpp,flags;
	if(!gl_sdlLib_initFlags[INIT_SDL])
		zenglApi_Exit(VM_ARG,"sdlCreateWin error: have not initial! (SDL��û�г�ʼ������ʹ��sdlInit�������г�ʼ��)\n");
	if(argcount < 4)
		zenglApi_Exit(VM_ARG,"sdlCreateWin���������������ĸ�����������ԭ�ͣ�sdlCreateWin(width,height,bpp,flags,repeat[option]);\n");

	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ��������ֵ����һ�������Ǵ��ڿ�
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCreateWin��һ��������������������ʾ���ڿ�");
	width = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,2,&arg); //��ȡ�ڶ���������ֵ���ڶ��������Ǵ��ڸ�
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCreateWin�ڶ���������������������ʾ���ڸ�");
	height = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,3,&arg); //��ȡ������������ֵ��������������λ��(���ڵķֱ���)
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCreateWin������������������������ʾλ��(���ڵķֱ���)");
	bpp = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,4,&arg); //��ȡ���ĸ�������ֵ�����ĸ�������SDL��Ƶģʽ��һЩ��ѡ��־
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCreateWin���ĸ�������������������ʾSDL��Ƶģʽ��һЩ��ѡ��־");
	flags = arg.val.integer;

	if(gl_sdlLib_pScreen != ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"sdlCreateWin�����Ѿ���������Ϸ�����ڱ�����!");

	gl_sdlLib_pScreen = SDL_SetVideoMode(width, height, bpp, flags);
	if(gl_sdlLib_pScreen == ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"sdlCreateWin�����ڲ�����SDL_SetVideoMode����������Ϸ�����ڱ���ʧ�ܣ�%s!",SDL_GetError());

	if(argcount == 5) //����ṩ��5����������ʹ�õ�����������ð��������ظ������SDLĬ��ֻ���һ�ΰ�����������������Ϳ��Կ����ظ���ⰴ��
	{
		zenglApi_GetFunArg(VM_ARG,5,&arg);
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlCreateWin����������������������������ð��������ظ����");
		if(arg.val.integer > 0)
		{
			SDL_EnableKeyRepeat(arg.val.integer,SDL_DEFAULT_REPEAT_INTERVAL);
		}
	}
	fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)gl_sdlLib_pScreen,SDL_TYPE_POINT); 
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,(ZL_EXP_LONG)gl_sdlLib_pScreen,0);
}

/*
   SDL���¼��в����û�����İ�������������¼��ȡ���Ӧ�ű�������sdlPollEvent(event) 
   event������һ��������࣬���ڴ��SDL�¼������͵���Ϣ��event����ԭ�Ϳ��Բο�������ඨ�壺
	class clsMouseMotionEvt  //SDL���������ƶ��¼�C�ṹ���Ӧ��zengl����
		type;	//SDL_MOUSEMOTION
		which;	//The mouse device index 
		state;	//The current button state 
		x; y;	//The X/Y coordinates of the mouse
		xrel;   //The relative motion in the X direction 
		yrel;	//The relative motion in the Y direction
	endcls

	class clsMouseButtonEvt  //SDL�����������¼�C�ṹ���Ӧ��zengl����
		type;	//SDL_MOUSEBUTTONDOWN or SDL_MOUSEBUTTONUP
		which;	//The mouse device index
		state;	//SDL_PRESSED or SDL_RELEASED
		x; y;	//The X/Y coordinates of the mouse at press time
		button;	//The mouse button index
	endcls

	class clsEvent  //event��Ӧ��zengl����
		type; //�¼����ͣ��ж��ǰ����¼�����������¼��ȡ�
		keytype;  //����ǰ�������keytype�д���˰���������
		clsMouseMotionEvt motion; //���������ƶ��¼������ƶ����������Ϣ��ŵ�motion�С�  
		clsMouseButtonEvt button; //�����������¼����򽫵����ص���Ϣ��ŵ�button�С�
	endcls
*/
ZL_EXP_VOID mySDL_PollEvent(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	SDL_Event gameEvent;
	ZL_EXP_INT retnum;

	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlPollEvent������������ֻ��һ������������ԭ�ͣ�sdlPollEvent(event);\n");
	retnum = SDL_PollEvent(&gameEvent);
	if(retnum != SDL_NOEVENT)
	{
		zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ��������ֵ����һ��������һ�����������������ڴ��SDL�¼������͵���Ϣ
		if(arg.type != ZL_EXP_FAT_MEMBLOCK)
			zenglApi_Exit(VM_ARG,"sdlPollEvent�������������ǳ�ʼ������������ʼ�����������\n");
		memblock = arg.val.memblock;
		arg.type = ZL_EXP_FAT_INT;
		arg.val.integer = gameEvent.type;
		zenglApi_SetMemBlock(VM_ARG,&memblock,1,&arg); //������1��Ӧ�������ڴ��ĵ�һ������Ϊ��Ϸ���¼����ͣ����Բο�ע�����clsEvent���type��Ա
		if(gameEvent.type == SDL_KEYUP || gameEvent.type == SDL_KEYDOWN)
		{
			arg.type = ZL_EXP_FAT_INT;
			arg.val.integer = gameEvent.key.keysym.sym;
			zenglApi_SetMemBlock(VM_ARG,&memblock,2,&arg); //����ǰ����¼����򽫲�������ĵڶ�������Ϊ�������ͣ����Բο�ע�����clsEvent���keytype��Ա
		}
		else if(gameEvent.type == SDL_MOUSEMOTION || gameEvent.type == SDL_MOUSEBUTTONDOWN ||
				 gameEvent.type == SDL_MOUSEBUTTONUP)
		{
			ZL_EXP_BOOL flag=ZL_EXP_FALSE;
			ZENGL_EXPORT_MEMBLOCK motionblock = {0};
			ZENGL_EXPORT_MOD_FUN_ARG mblk_val = {ZL_EXP_FAT_NONE,{0}};
			if(gameEvent.type == SDL_MOUSEMOTION)
				mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3); //���������ƶ��¼�����ȡ��������ĵ�����
			else
				mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4); //�����������¼�����ȡ��������ĵ�����
			if(mblk_val.type == ZL_EXP_FAT_MEMBLOCK) //�жϵ������������Ƿ��������ڴ�飬������Ǿ���zenglApi_CreateMemBlock����һ���ڴ�鲢����flag��ΪZL_EXP_TRUE,��ʾ�ڴ���ǽű������ڲ��·����
				motionblock = mblk_val.val.memblock;
			else
			{
				zenglApi_CreateMemBlock(VM_ARG,&motionblock,0);
				flag = ZL_EXP_TRUE;
			}
			arg.type = ZL_EXP_FAT_INT;
			arg.val.integer = gameEvent.motion.x;
			zenglApi_SetMemBlock(VM_ARG,&motionblock,4,&arg); //������Ϸ���ĵ�ǰ�����xֵ���������������ڴ��ĵ�4��Ԫ�أ����Բο�clsMouseMotionEvt��clsMouseButtonEvt���x��Ա
			arg.val.integer = gameEvent.motion.y;
			zenglApi_SetMemBlock(VM_ARG,&motionblock,5,&arg); //���õ�ǰ�����yֵ���ڴ���5��Ԫ�أ����Բο�clsMouseMotionEvt��clsMouseButtonEvt���y��Ա
			if(flag) //����ǽű������ڲ��·�����ڴ�飬���·�����ڴ�����õ������ĵ�����������
			{
				arg.type = ZL_EXP_FAT_MEMBLOCK;
				arg.val.memblock = motionblock;
				if(gameEvent.type == SDL_MOUSEMOTION)  //����ƶ��¼��������ò����ĵ�����
					zenglApi_SetMemBlock(VM_ARG,&memblock,3,&arg);
				else 
					zenglApi_SetMemBlock(VM_ARG,&memblock,4,&arg); //������¼��������ò����ڴ��ĵ�����
			}
		}//if(gameEvent.type == SDL_KEYUP || gameEvent.type == SDL_KEYDOWN)...else if(gameEvent.type == SDL_MOUSEMOTION || ....
	}//if(retnum != SDL_NOEVENT)
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
}

/*
   ������Ϸ�ı���ͼƬ����Ӧ�Ľű�������sdlSetBkImg(picname | surface point) ��������������Դ��
*/
ZL_EXP_VOID mySDL_SetBkImg(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface* pShownBMP = ZL_EXP_NULL,* tmpBmp = ZL_EXP_NULL;
	ZL_EXP_CHAR PathBuffer[SDL_LIB_MAX_PATH_BUFFER_SIZE] = {0};
	ZL_EXP_CHAR * utf8_path = ZL_EXP_NULL;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlSetBkImg������������ֻ��һ������������ԭ�ͣ�sdlSetBkImg(picname | surface point);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ��������ֵ����һ�������Ǳ���ͼƬ���ļ���
	if(arg.type == ZL_EXP_FAT_STR)
	{
		if(zenglApi_makePathFileName(VM_ARG,arg.val.str,PathBuffer,SDL_LIB_MAX_PATH_BUFFER_SIZE) == -1)
			zenglApi_Exit(VM_ARG,"sdlSetBkImg�����������ļ�������Ч·����Ϣʱ��������:%s",zenglApi_GetErrorString(VM_ARG));

#ifdef ZL_EXP_OS_IN_WINDOWS
		utf8_path = GbkToUtf8ForWindows(VM_ARG,PathBuffer);
#else
		utf8_path = PathBuffer;
#endif

		if((tmpBmp = IMG_Load(utf8_path)) == ZL_EXP_NULL) //ʹ��IMG_load�����Ϳ��Լ���.jpg , .png֮����ļ�
			zenglApi_Exit(VM_ARG,"sdlSetBkImg�ڵ���SDL��API��IMG_Loadʱ��������%s",SDL_GetError());

#ifdef ZL_EXP_OS_IN_WINDOWS
		zenglApi_FreeMem(VM_ARG,utf8_path);
#endif

		pShownBMP = SDL_DisplayFormat(tmpBmp); //��λͼתΪ��Ϸ�������ʾ��ʽ
		SDL_FreeSurface(tmpBmp);
	}
	else if(arg.type == ZL_EXP_FAT_INT)
	{
		if(fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)arg.val.integer,SDL_TYPE_POINT)) //���SDLָ���Ƿ���Ч
			pShownBMP = (SDL_Surface*)arg.val.integer;
		else
			zenglApi_Exit(VM_ARG,"sdlSetBkImg�����Ĳ�������һ����Ч��SDLָ��\n");
	}
	else
		zenglApi_Exit(VM_ARG,"sdlSetBkImg�����Ĳ���������Ч���ú�������Ҫô���ַ������ͱ�ʾҪ���ص�λͼ�ļ�����Ҫô���������ͱ�ʾһ����Ч��SDL����ָ��\n");
	
	if(pShownBMP == ZL_EXP_NULL)
		zenglApi_Exit(VM_ARG,"sdlSetBkImg����λͼָ����Ϣʧ�ܣ�%s",SDL_GetError());

	if(SDL_BlitSurface(pShownBMP, 0, gl_sdlLib_pScreen , 0)) //��λͼ������Ƶ�����Ļ����
		zenglApi_Exit(VM_ARG,"sdlSetBkImg��λͼ���Ƶ�����Ļ����ʧ�ܣ�%s",SDL_GetError());

	if(arg.type == ZL_EXP_FAT_STR) //����������ַ�����˵���������ɵ�SDL����ָ�룬������뵽ָ�붯̬�����У�����ͳһ����
		fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)pShownBMP,SDL_TYPE_POINT);

	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,(ZL_EXP_LONG)pShownBMP,0);
}

/*
   ����Ϸ���ƺõ�֡��ʾ��������ΪSDL�Ļ��Ʋ������ǻ����ڻ����еģ��������SDL_Flip���з�ת�����ܽ����������������Կ���ʾ����
   ��Ӧ�Ľű�������sdlShowScreen();
*/
ZL_EXP_VOID mySDL_ShowScreen(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZL_EXP_INT retnum;
	if((retnum = SDL_Flip(gl_sdlLib_pScreen)) != 0 ) //��SDL����Ļ���淭ת��ǰ����ʾ
		zenglApi_Exit(VM_ARG,"sdlShowScreen������������Ļʧ��:%s \n",SDL_GetError());
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
}

/*
    ����λͼ����Ӧ�Ľű�������sdlLoadBmp(picname,red[option],green[option],blue[option]) (option��ʾ��ѡ����)
*/
ZL_EXP_VOID mySDL_LoadBmp(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface* pShownBMP = ZL_EXP_NULL,* tmpBmp = ZL_EXP_NULL;
	ZL_EXP_CHAR PathBuffer[SDL_LIB_MAX_PATH_BUFFER_SIZE] = {0};
	ZL_EXP_CHAR * utf8_path = ZL_EXP_NULL;
	if(argcount != 1 && argcount !=4)
		zenglApi_Exit(VM_ARG,"sdlLoadBmp����������1����4������������ԭ�ͣ�����ԭ���ǣ�����ԭ�ͣ�sdlLoadBmp(picname,red[option],green[option],blue[option]);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ��������ֵ����һ��������λͼ���ļ���
	if(arg.type == ZL_EXP_FAT_STR)
	{
		if(zenglApi_makePathFileName(VM_ARG,arg.val.str,PathBuffer,SDL_LIB_MAX_PATH_BUFFER_SIZE) == -1)
			zenglApi_Exit(VM_ARG,"sdlLoadBmp�����������ļ�������Ч·����Ϣʱ��������:%s",zenglApi_GetErrorString(VM_ARG));

#ifdef ZL_EXP_OS_IN_WINDOWS
		utf8_path = GbkToUtf8ForWindows(VM_ARG,PathBuffer);
#else
		utf8_path = PathBuffer;
#endif

		if((tmpBmp = IMG_Load(utf8_path)) == ZL_EXP_NULL) //ʹ��IMG_load�����Ϳ��Լ���.jpg , .png֮����ļ�
			zenglApi_Exit(VM_ARG,"sdlLoadBmp�ڵ���SDL��API��IMG_Loadʱ��������%s",SDL_GetError());

#ifdef ZL_EXP_OS_IN_WINDOWS
		zenglApi_FreeMem(VM_ARG,utf8_path);
#endif

		pShownBMP = SDL_DisplayFormat(tmpBmp); //��λͼתΪ��Ϸ�������ʾ��ʽ
		SDL_FreeSurface(tmpBmp);
	}
	else
		zenglApi_Exit(VM_ARG,"sdlLoadBmp������һ�������������ַ������ͣ���ʾҪ����λͼ���ļ���");

	if(argcount == 4) //������ĸ�������������λͼ��colorkey��colorkey���Թ��˵���Ҫ����ɫ������͸������Ч��
	{
		ZL_EXP_INT r,g,b;
		Uint32 colorkey;
		zenglApi_GetFunArg(VM_ARG,2,&arg);
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlLoadBmp������2��,��3���͵�4��������������������");
		r = arg.val.integer; //colorkey��redֵ����ɫ��ֵ

		zenglApi_GetFunArg(VM_ARG,3,&arg);
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlLoadBmp������2��,��3���͵�4��������������������");
		g = arg.val.integer; //colorkey��greenֵ����ɫ���ֵ�ֵ

		zenglApi_GetFunArg(VM_ARG,4,&arg);
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlLoadBmp������2��,��3���͵�4��������������������");
		b = arg.val.integer; //colorkey��blueֵ����ɫ���ֵ�ֵ
		colorkey = SDL_MapRGB( pShownBMP->format, r, g, b );
		SDL_SetColorKey(pShownBMP, SDL_SRCCOLORKEY, colorkey); //����colorkey
	}
	fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)pShownBMP,SDL_TYPE_POINT);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,(ZL_EXP_LONG)pShownBMP,0);
}

/*
   ��һ��λͼ���Ƶ���һ��λͼ���棬��Ӧ�Ľű�������sdlBlitImg(src surface,src rect,dest surface,dest rect);
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
		zenglApi_Exit(VM_ARG,"sdlBlitImg�����������ĸ�����������ԭ���ǣ�sdlBlitImg(src surface,src rect,dest surface,dest rect);\n");

	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ��������ֵ����һ��������Ҫ���Ƶ�ԭλͼ
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlBlitImg������һ�������������������ͣ���ʾҪ���Ƶ�ԭλͼ����ָ��\n");
	pSrc = (SDL_Surface*)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)pSrc,SDL_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlBlitImg������һ������������Ч��ԭλͼ����ָ��\n");

	zenglApi_GetFunArg(VM_ARG,2,&arg); //��ȡ�ڶ����������ò�����ԭλͼҪ���Ƶľ�������
	if(arg.type == ZL_EXP_FAT_NONE || arg.type == ZL_EXP_FAT_INT) //�����������˵����SDL_Rect��ָ��
	{
		srcflag = ZL_EXP_TRUE;
		srcnum = arg.val.integer;
		if(srcnum != 0) //Ŀǰֻ֧��0����ֵ����ʾԭλͼȫ���ľ�������
			zenglApi_Exit(VM_ARG,"sdlBlitImg�����ڶ�������������Ч��ԭλͼ��������\n");
	}
	else if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlBlitImg�����ڶ������������ǳ�ʼ������������ʼ�����������������0\n");
	else
	{
		memblock = arg.val.memblock;
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg�����ڶ���������ĳ�Ա����������������\n");
		else
			srcRect.x = (Sint16)mblk_val.val.integer;
		
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg�����ڶ���������ĳ�Ա����������������\n");
		else
			srcRect.y = (Sint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg�����ڶ���������ĳ�Ա����������������\n");
		else
			srcRect.w = (Uint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg�����ڶ���������ĳ�Ա����������������\n");
		else
			srcRect.h = (Uint16)mblk_val.val.integer;
	}
	
	zenglApi_GetFunArg(VM_ARG,3,&arg); //��ȡ�������������ò�����Ҫ���Ƶ�Ŀ��λͼ
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlBlitImg���������������������������ͣ���ʾҪ���Ƶ�Ŀ��λͼ����ָ��\n");
	pDest = (SDL_Surface*)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)pDest,SDL_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlBlitImg��������������������Ч��Ŀ��λͼ����ָ��\n");
	
	zenglApi_GetFunArg(VM_ARG,4,&arg); //���ĸ�������Ҫ���Ƶ�Ŀ���������
	if(arg.type == ZL_EXP_FAT_NONE || arg.type == ZL_EXP_FAT_INT) //�����������˵����SDL_Rect��ָ��
	{
		destflag = ZL_EXP_TRUE;
		destnum = arg.val.integer;
		if(destnum != 0) //Ŀǰֻ֧��0����ֵ����ʾĿ��λͼȫ���ľ�������
			zenglApi_Exit(VM_ARG,"sdlBlitImg�������ĸ�����������Ч��Ŀ��λͼ��������\n");
	}
	else if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlBlitImg�������ĸ����������ǳ�ʼ������������ʼ�����������������0\n");
	else
	{
		memblock = arg.val.memblock;
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg�������ĸ�������ĳ�Ա����������������\n");
		else
			destRect.x = (Sint16)mblk_val.val.integer;
		
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg�������ĸ�������ĳ�Ա����������������\n");
		else
			destRect.y = (Sint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg�������ĸ�������ĳ�Ա����������������\n");
		else
			destRect.w = (Uint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlBlitImg�������ĸ�������ĳ�Ա����������������\n");
		else
			destRect.h = (Uint16)mblk_val.val.integer;
	}
	
	if(srcflag == ZL_EXP_TRUE) //���ڶ��������������SDL_Rectָ��ʱ
	{
		if(destflag) //�����ĸ������������SDL_Rectָ��ʱ
			retnum = SDL_BlitSurface(pSrc,(SDL_Rect *)srcnum , pDest , (SDL_Rect *)destnum);
		else
			retnum = SDL_BlitSurface(pSrc,(SDL_Rect *)srcnum , pDest , &destRect);
		if(retnum) 
			zenglApi_Exit(VM_ARG,"sdlBlitImg��������SDL API��SDL_BlitSurfaceʧ�ܣ�%s\n",SDL_GetError());
	}
	else
	{
		if(destflag) //�����ĸ������������SDL_Rectָ��ʱ
			retnum = SDL_BlitSurface(pSrc, &srcRect, pDest , (SDL_Rect *)destnum);
		else
			retnum = SDL_BlitSurface(pSrc, &srcRect, pDest , &destRect);
		if(retnum)
			zenglApi_Exit(VM_ARG,"sdlBlitImg��������SDL API��SDL_BlitSurfaceʧ�ܣ�%s\n",SDL_GetError());
	}
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
}

/*
   ��ȡ����״̬���жϵ�ǰ�û����µ�����Щ�������������Ҽ���
   ���صĽ����һ�����飬�������¼�����м��İ���״̬�����Ե��û�ͬʱ������������������ʱ���Ϳ��������ַ��������������жϳ���
   ���û�ͬʱ�������ϼ����Ҽ�����ô������273��275��ͻ���ZL_EXP_TRUE��1ʱ��ʾ����״̬
   ��Ӧ�Ľű�������sdlGetKeyState(array keystate);
*/
ZL_EXP_VOID mySDL_GetKeyState(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT numkey,i;
	Uint8 *keystate = ZL_EXP_NULL;
	ZENGL_EXPORT_MEMBLOCK memblock = {0};

	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlGetKeyState������������ֻ��һ������������ԭ���ǣ�sdlGetKeyState(array keystate);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò����Ǵ�Ž��������
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlGetKeyState�����Ĳ��������ǳ�ʼ������������ʼ�����������\n");
	
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
   ������Ϸ���ڵı��⣬��Ӧ�Ľű������ǣ�sdlSetCaption(caption);
*/
ZL_EXP_VOID mySDL_SetCaption(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_CHAR * caption = ZL_EXP_NULL;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlSetCaption������������ֻ��һ������������ԭ���ǣ�sdlSetCaption(caption);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò�������Ҫ���õĴ��ڱ���
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
		zenglApi_Exit(VM_ARG,"sdlSetCaption�����ĵ�һ�������������ַ�������ʾ��Ҫ���õĴ��ڱ���\n");
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}

/*
   ����ĳλͼ�����colorkey
   ��Ӧ�Ľű�����Ϊ��sdlSetColorKey(image surface , red , green , blue);
*/
ZL_EXP_VOID mySDL_SetColorKey(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface * image;
	ZL_EXP_INT r,g,b;
	Uint32 colorkey;

	if(argcount != 4)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey�����������ĸ�����������ԭ���ǣ�sdlSetColorKey(image surface , red , green , blue);\n");

	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò�����λͼ�ı���ָ��
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey������һ�������������������ͣ���ʾ��Ҫ������λͼ�ı���ָ��\n");
	image = (SDL_Surface*)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)image,SDL_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlSetColorKey������һ������������Ч��λͼ����ָ��\n");
	zenglApi_GetFunArg(VM_ARG,2,&arg); 
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey�����ڶ��������������������ͣ���ʾcolorkey��redֵ\n");
	r = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,3,&arg); 
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey���������������������������ͣ���ʾcolorkey��greenֵ\n");
	g = arg.val.integer;

	zenglApi_GetFunArg(VM_ARG,4,&arg); 
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey�������ĸ������������������ͣ���ʾcolorkey��blueֵ\n");
	b = arg.val.integer;

	colorkey = SDL_MapRGB( image->format, r, g, b );
	r = SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey);
	if(r == -1)
		zenglApi_Exit(VM_ARG,"sdlSetColorKey��������SDL API��SDL_SetColorKeyʧ�ܣ�%s\n",SDL_GetError());
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,r,0);
}

/*
   ��ȡ��Ϸ���еĵδ��������������ж���Ϸ���е�ʱ�䣬��������������Ϸ��֡���ȡ�
   ��Ӧ�Ľű�����Ϊ��sdlGetTicks();
*/
ZL_EXP_VOID mySDL_GetTicks(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,SDL_GetTicks(),0);
}

/*
   ��ĳ�����ļ�
   ��Ӧ�Ľű�����Ϊ��sdlOpenFont(font_filename , font_size);
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
		zenglApi_Exit(VM_ARG,"sdlOpenFont������������������������ԭ���ǣ�sdlOpenFont(font_filename , font_size);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò����������ļ����ļ�����һ����.ttfΪ��׺���ļ�
	if(arg.type == ZL_EXP_FAT_STR)
	{
		filename = arg.val.str;
		zenglApi_GetFunArg(VM_ARG,2,&arg); //�ڶ�������Ϊ�����С
		if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlOpenFont�����ڶ��������������������ͣ���ʾ�����С\n");
		ptsize = arg.val.integer;
		if(zenglApi_makePathFileName(VM_ARG,filename,PathBuffer,SDL_LIB_MAX_PATH_BUFFER_SIZE) == -1)
			zenglApi_Exit(VM_ARG,"sdlOpenFont�����������ļ�������Ч·����Ϣʱ��������:%s",zenglApi_GetErrorString(VM_ARG)); 

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
			zenglApi_Exit(VM_ARG,"sdlOpenFont��������SDL API��TTF_OpenFontʧ�ܣ�%s",SDL_GetError());
		else
		{
			fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)retnum,FONT_TYPE_POINT);
			zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
		}
	}
	else
		zenglApi_Exit(VM_ARG,"sdlOpenFont������һ�������������ַ������ͣ���ʾ�����ļ����ļ���\n");
}

/*
   �ڻ�������ʱ��ͨ���ṩҪ���Ƶ��ַ����������������ɫ�Ȳ�����Ȼ��TTF��API�ͻ������Щ��������һ���ɹ����Ƶ�λͼ���淵�ء�
   ���ص�λͼ������žͿ����������ᵽ���ķ����������ַ���λͼ���Ƶ���һ�������ϡ�
   ��Ӧ�Ľű�����Ϊ��sdlDrawText(font obj , string , array fontcolor , array bgcolor[option]); (option��ʾ�ǿ�ѡ����)
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
		zenglApi_Exit(VM_ARG,"sdlDrawText�����������������ĸ�����������ԭ���ǣ�sdlDrawText(font obj , string , array fontcolor , array bgcolor[option]);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò�����֮ǰ�򿪵�TTF_Font����ָ��
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawText������һ�������������������ͣ���ʾ֮ǰ�򿪵�TTF_Font����ָ��\n");
	font = (TTF_Font *)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)font,FONT_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlDrawText������һ����������һ����Ч���������ָ��\n");

	zenglApi_GetFunArg(VM_ARG,2,&arg); //�ڶ���������Ҫ���Ƶ��ַ�����Ϣ
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"sdlDrawText�����ڶ��������������ַ�������ʾҪ���Ƶ��ַ�����Ϣ\n");
	text = arg.val.str;
	
	zenglApi_GetFunArg(VM_ARG,3,&arg); //������������һ��������������ȡǰ����������Ϊ����ǰ��ɫ��r,g,bֵ
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlDrawText�������������������ǳ�ʼ������������ʼ���������������ʾǰ��ɫ��r,g,bֵ\n");
	memblock = arg.val.memblock;
	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawText����������������ĳ�Ա���붼����������\n");
	frontcolor.r =  (Uint8)mblk_val.val.integer;
	
	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawText����������������ĳ�Ա���붼����������\n");
	frontcolor.g = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawText����������������ĳ�Ա���붼����������\n");
	frontcolor.b = (Uint8)mblk_val.val.integer;

	if(argcount == 4)
	{
		zenglApi_GetFunArg(VM_ARG,4,&arg); //���ĸ�������һ��������������ȡǰ����������Ϊ���屳��ɫ��r,g,bֵ
		if(arg.type != ZL_EXP_FAT_MEMBLOCK)
			zenglApi_Exit(VM_ARG,"sdlDrawText�������ĸ����������ǳ�ʼ������������ʼ���������������ʾǰ��ɫ��r,g,bֵ\n");
		memblock = arg.val.memblock;
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawText�������ĸ�������ĳ�Ա���붼����������\n");
		bgcolor.r =  (Uint8)mblk_val.val.integer;
		
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawText�������ĸ�������ĳ�Ա���붼����������\n");
		bgcolor.g = (Uint8)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawText�������ĸ�������ĳ�Ա���붼����������\n");
		bgcolor.b = (Uint8)mblk_val.val.integer;
	}

#ifdef ZL_EXP_OS_IN_WINDOWS
	tmptext = GbkToUtf8ForWindows(VM_ARG,text);
#else
	tmptext = text;
#endif

	if(argcount == 3)	//���ֻ��������������ֻ����ǰ��ɫ
		retnum = (ZL_EXP_LONG)TTF_RenderUTF8_Solid(font,tmptext,frontcolor); //ע��ֻ����ȾUTF8�ĺ��֣���
	else	 //������ĸ�������������ǰ��ɫ�ͱ���ɫ
		retnum = (ZL_EXP_LONG)TTF_RenderUTF8_Shaded(font,tmptext,frontcolor,bgcolor);

#ifdef ZL_EXP_OS_IN_WINDOWS
	zenglApi_FreeMem(VM_ARG,tmptext);
#endif

	if(retnum == 0)
		zenglApi_Exit(VM_ARG,"sdlDrawText��������SDL API��TTF_RenderUTF8_Solid��TTF_RenderUTF8_Shadedʱʧ�ܣ�%s\n",SDL_GetError());
	fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)retnum,SDL_TYPE_POINT); //�����ص�λͼ����ָ����뵽��̬�����У�����ͳһ����
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0); //���ذ����ַ�����Ϣ�Ŀɹ����Ƶ�λͼ����ָ��
}

/*
   �ر�����
   ��Ӧ�Ľű�����Ϊ��sdlCloseFont(font obj);
*/
ZL_EXP_VOID mySDL_CloseFont(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	TTF_Font * font_point;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlCloseFont������������ֻ��һ������������ԭ���ǣ�sdlCloseFont(font obj);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò������������ָ��
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlCloseFont��һ��������������������ʾ�������ָ��\n");
	font_point = (TTF_Font *)arg.val.integer;
	if(fun_sdlLib_Free_Point(VM_ARG,(ZL_EXP_VOID *)font_point,FONT_TYPE_POINT) == -1)
		zenglApi_Exit(VM_ARG,"sdlCloseFont�Ĳ���������Ч������ָ��\n");
	TTF_CloseFont(font_point);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}

/*
   �ͷ�ĳ��λͼ������Դ
   ��Ӧ�Ľű�����Ϊ��sdlFreeImage(image surface);
*/
ZL_EXP_VOID mySDL_FreeImage(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	SDL_Surface * surface;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlFreeImage������������ֻ��һ������������ԭ���ǣ�sdlFreeImage(image surface);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò�����Ҫ�ͷŵ�SDL����ָ��
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlFreeImage�����ĵ�һ�������������������ͣ���ʾҪ�ͷŵ�SDL����ָ��\n");
	surface = (SDL_Surface*)arg.val.integer;
	if(fun_sdlLib_Free_Point(VM_ARG,(ZL_EXP_VOID *)surface,SDL_TYPE_POINT) == -1)
		zenglApi_Exit(VM_ARG,"sdlFreeImage�Ĳ���������Ч��SDL����ָ��\n");
	SDL_FreeSurface(surface);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}

/*
   ������Դ��ʼ��
   ��Ӧ�Ľű�����Ϊ��sdlFontInit();
*/
ZL_EXP_VOID mySDL_FontInit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZL_EXP_INT retnum;
	if((retnum = TTF_Init())==-1)
		zenglApi_Exit(VM_ARG,"sdlFontInit��������SDL API��TTF_Initʧ�ܣ�%s\n",SDL_GetError());
	gl_sdlLib_initFlags[INIT_FONT] = ZL_EXP_TRUE;
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0);
}

/*
    ��Ŀ������������Ϊָ������ɫ��
	��Ӧ�Ľű�����Ϊ��sdlFillRect(dest surface,dest rect , array color_alpha);
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
		zenglApi_Exit(VM_ARG,"sdlFillRect������������������������ԭ���ǣ�sdlFillRect(dest surface,dest rect , array color_alpha);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò���������Ŀ������ָ��ֵ
	if(arg.type != ZL_EXP_FAT_INT) 
		zenglApi_Exit(VM_ARG,"sdlFillRect������һ�������������������ͣ���ʾĿ������ָ��ֵ\n");
	pDest = (SDL_Surface*)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)pDest,SDL_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlFillRect������һ������������Ч��SDL����ָ��\n");
	zenglApi_GetFunArg(VM_ARG,2,&arg);
	if(arg.type == ZL_EXP_FAT_NONE || (arg.type == ZL_EXP_FAT_INT && arg.val.integer == 0)) //�ڶ������������0��˵�����Ŀ�����������
		pDestRect = ZL_EXP_NULL;
	else if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlFillRect�����ڶ������������ǳ�ʼ������������ʼ�������������0����ʾҪ���ľ�������\n");
	else
	{
		memblock = arg.val.memblock; //�ڶ�����������������ڴ�飬��ǰ����ֱ��ӦĿ����������x,y,w(��),h(��)
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect�����ڶ���������ĳ�Ա����������������\n");
		pDestRect->x = (Sint16)mblk_val.val.integer;
		
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect�����ڶ���������ĳ�Ա����������������\n");
		pDestRect->y = (Sint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect�����ڶ���������ĳ�Ա����������������\n");
		pDestRect->w = (Uint16)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect�����ڶ���������ĳ�Ա����������������\n");
		pDestRect->h = (Uint16)mblk_val.val.integer;
	}
	
	zenglApi_GetFunArg(VM_ARG,3,&arg); //����������ΪҪ���õ���ɫ����
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlFillRect�������������������ǳ�ʼ������������ʼ���������������ʾҪ������ɫ��r,g,b,aֵ\n");

	memblock = arg.val.memblock;
	zenglApi_GetMemBlockInfo(VM_ARG,&memblock,ZL_EXP_NULL,&mblk_count);
	if(mblk_count !=3 && mblk_count != 4)
		zenglApi_Exit(VM_ARG,"sdlFillRect����������������Ԫ�ظ����������������ĸ�����ʾr,g,bֵ��r,g,b,aֵ\n");

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlFillRect����������������ĳ�Ա����������������\n");
	r = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlFillRect����������������ĳ�Ա����������������\n");
	g = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3);
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlFillRect����������������ĳ�Ա����������������\n");
	b = (Uint8)mblk_val.val.integer;

	if(mblk_count == 4) //����е������������ɫ��alpha͸��ֵ
	{
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,4);
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlFillRect����������������ĳ�Ա����������������\n");
		a = (Uint8)mblk_val.val.integer;
		color = SDL_MapRGBA(pDest->format,r,g,b,a);
	}
	else
		color = SDL_MapRGB(pDest->format,r,g,b);

	ret = SDL_FillRect(pDest,pDestRect,color); //����SDL_FillRect���Ŀ��
	if(ret == -1)
		zenglApi_Exit(VM_ARG,"sdlFillRect��������SDL API��SDL_FillRectʧ�ܣ�%s\n",SDL_GetError());
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,ret,0);
}

/*
   �ڻ�������ʱ��ͨ���ṩҪ���Ƶ��ַ����������������ɫ�Ȳ�����Ȼ��TTF��API�ͻ������Щ��������һ���ɹ����Ƶ�λͼ���淵�ء�
   ���ص�λͼ������žͿ����������ᵽ���ķ����������ַ���λͼ���Ƶ���һ�������ϡ�
   ��Ӧ�Ľű�����Ϊ��sdlDrawTextEx(ref surface,font obj , string , array fontcolor , array bgcolor[option]); (option��ʾ�ǿ�ѡ����)
   sdlDrawTextEx �� sdlDrawText�������ڣ�sdlDrawTextEx�����Զ��ͷŵ�һ��������ԭ����ָ�룬sdlDrawText���صı���ָ����Ҫ�ֶ��ͷŵ���
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
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx�����������ĸ������������"
			"����ԭ���ǣ�sdlDrawTextEx(ref surface,font obj , string , array fontcolor , array bgcolor[option]); (option��ʾ�ǿ�ѡ����)\n");
	zenglApi_GetFunArgInfo(VM_ARG,1,&arg);
	if(arg.type != ZL_EXP_FAT_ADDR && 
		arg.type != ZL_EXP_FAT_ADDR_LOC &&
		arg.type != ZL_EXP_FAT_ADDR_MEMBLK
		)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx������һ�����������Ǳ���������\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ���������ò������ַ�������ָ��ı������ã�ͨ�������ÿ���ֱ�ӽ���֮ǰ�ı�����ͷŵ�
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx������һ�������������������͵ı���������\n");
	textSurface = (SDL_Surface *)arg.val.integer;
	if(fun_sdlLib_Free_Point(VM_ARG,(ZL_EXP_VOID *)textSurface,SDL_TYPE_POINT) != -1) //����ҵ���Ӧ��SDLָ�����ͷţ�û��������
		SDL_FreeSurface(textSurface);

	zenglApi_GetFunArg(VM_ARG,2,&arg); //�ڶ���������֮ǰ�򿪵�TTF_Font����ָ��
	if(arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx�����ڶ��������������������ͣ���ʾTTF_Font����ָ��\n");
	font = (TTF_Font *)arg.val.integer;
	if(!fun_sdlLib_Check_Point(VM_ARG,(ZL_EXP_VOID *)font,FONT_TYPE_POINT))
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx�����ڶ�����������һ����Ч���������ָ��\n");
	
	zenglApi_GetFunArg(VM_ARG,3,&arg); //������������Ҫ���Ƶ��ַ�����Ϣ
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx���������������������ַ�������ʾҪ���Ƶ��ַ�����Ϣ\n");
	text = arg.val.str;
	
	zenglApi_GetFunArg(VM_ARG,4,&arg); //���ĸ�������һ��������࣬ȡǰ����������Ϊ����ǰ��ɫ��r,g,bֵ
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx�������ĸ����������ǳ�ʼ������������ʼ���������������ʾǰ��ɫ\n");
	memblock = arg.val.memblock;
	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1); 
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx�������ĸ�������ĳ�Ա���붼����������\n");
	frontcolor.r = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2); 
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx�������ĸ�������ĳ�Ա���붼����������\n");
	frontcolor.g = (Uint8)mblk_val.val.integer;

	mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3); 
	if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx�������ĸ�������ĳ�Ա���붼����������\n");
	frontcolor.b = (Uint8)mblk_val.val.integer;

	if(argcount == 5)
	{
		zenglApi_GetFunArg(VM_ARG,5,&arg); //�����������һ��������࣬ȡǰ����������Ϊ���屳��ɫ��r,g,bֵ
		if(arg.type != ZL_EXP_FAT_MEMBLOCK)
			zenglApi_Exit(VM_ARG,"sdlDrawTextEx������������������ǳ�ʼ������������ʼ���������������ʾ����ɫ\n");
		memblock = arg.val.memblock;
		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,1); 
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawTextEx���������������ĳ�Ա���붼����������\n");
		bgcolor.r = (Uint8)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,2); 
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawTextEx���������������ĳ�Ա���붼����������\n");
		bgcolor.g = (Uint8)mblk_val.val.integer;

		mblk_val = zenglApi_GetMemBlock(VM_ARG,&memblock,3); 
		if(mblk_val.type != ZL_EXP_FAT_NONE && mblk_val.type != ZL_EXP_FAT_INT)
			zenglApi_Exit(VM_ARG,"sdlDrawTextEx���������������ĳ�Ա���붼����������\n");
		bgcolor.b = (Uint8)mblk_val.val.integer;
	}

#ifdef ZL_EXP_OS_IN_WINDOWS
	tmptext = GbkToUtf8ForWindows(VM_ARG,text);
#else
	tmptext = text;
#endif

	if(argcount == 4)	//���ֻ���ĸ���������ֻ����ǰ��ɫ
		retnum = (ZL_EXP_LONG)TTF_RenderUTF8_Solid(font,tmptext,frontcolor); //ע��ֻ����ȾUTF8�ĺ��֣���
	else	//��������������������ǰ��ɫ�ͱ���ɫ
		retnum = (ZL_EXP_LONG)TTF_RenderUTF8_Shaded(font,tmptext,frontcolor,bgcolor);

#ifdef ZL_EXP_OS_IN_WINDOWS
	zenglApi_FreeMem(VM_ARG,tmptext);
#endif

	if(retnum == 0)
		zenglApi_Exit(VM_ARG,"sdlDrawTextEx��������SDL API��TTF_RenderUTF8_Solid��TTF_RenderUTF8_Shadedʧ�ܣ�%s\n",SDL_GetError());
	fun_sdlLib_Insert_HashTableForPoint(VM_ARG,(ZL_EXP_VOID *)retnum,SDL_TYPE_POINT); //�����ص�λͼ����ָ����뵽��̬�����У�����ͳһ����
	arg.type = ZL_EXP_FAT_INT;
	arg.val.integer = retnum;
	zenglApi_SetFunArg(VM_ARG,1,&arg); //����һ���������õı�������Ϊ�µı���ָ��
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,retnum,0); //���ذ����ַ�����Ϣ�Ŀɹ����Ƶ�λͼ����ָ��
}

/*
   SDL��ʱ
   ���ڿ�����Ϸ֡��ʱ���õ���
   ��Ӧ�Ľű�����Ϊ��sdlDelay(millisecond);
*/
ZL_EXP_VOID mySDL_Delay(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"sdlDelay����������һ������������ԭ���ǣ�sdlDelay(millisecond);\n");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ��������ֵ����һ��������Ҫ��ʱ�ĺ�����
	if(arg.type != ZL_EXP_FAT_NONE && arg.type != ZL_EXP_FAT_INT)
		zenglApi_Exit(VM_ARG,"sdlDelay��һ��������������������ʾҪ��ʱ�ĺ�����");
	SDL_Delay((Uint32)arg.val.integer);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}
