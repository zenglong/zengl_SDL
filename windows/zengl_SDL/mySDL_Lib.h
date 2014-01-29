#ifndef MY_SDL_LIB_H_
#define MY_SDL_LIB_H_

#define SDL_LIB_INIT_NUM 2 //SDL��ʼ��������SDL_Init,TTF_Init��
#define SDL_LIB_POINT_SIZE 20 //SDLָ�붯̬�����ʼ���Ͷ�̬���ݵĴ�С
#define SDL_LIB_HASHSIZE 211 //hash�������С
#define SDL_LIB_MAX_PATH_BUFFER_SIZE 350 //�ļ�·����������ĳߴ�

typedef enum _SDL_LIB_INIT_FLAGS{
	INIT_FONT,INIT_SDL
}SDL_LIB_INIT_FLAGS;

typedef enum _SDL_LIB_POINT_TYPE{
	FONT_TYPE_POINT,SDL_TYPE_POINT
}SDL_LIB_POINT_TYPE; //ָ�����ͣ�ĿǰΪFONT����ָ�룬SDL����ָ��

typedef struct _SDL_LIB_FREE_POINT_LIST{
	ZL_EXP_BOOL isInit; //�ж��Ƿ��ʼ��
	int size;
	int count;
	int * frees;
}SDL_LIB_FREE_POINT_LIST; //���ڴ��SDL�ͷ��˵�ָ������

typedef struct _SDL_LIB_POINT_MEMBER{
	ZL_EXP_BOOL isvalid;
	void * point;
	SDL_LIB_POINT_TYPE type;
	int next; //ָ��HASH���е���һ��Ԫ�ء�
}SDL_LIB_POINT_MEMBER; //SDL_LIB_POINT_LIST SDLָ�붯̬�����еĳ�Ա����

typedef struct _SDL_LIB_POINT_LIST{
	ZL_EXP_BOOL isInit; //�ж��Ƿ��ʼ��
	int size;
	int count;
	SDL_LIB_POINT_MEMBER * points;
	int hash[SDL_LIB_HASHSIZE]; //����points��Ӧ��hash���ұ�
	SDL_LIB_FREE_POINT_LIST freelist; //��points���ָ�뱻SDL API�ͷŵ�ʱ������������ŵ�freelist���´η���SDLָ��ʱ�Ϳ���ֱ��ʹ�ô�����
}SDL_LIB_POINT_LIST; //SDL��ű���������ָ��Ķ�̬���顣

ZL_EXP_VOID fun_sdlLib_destroyAll(ZL_EXP_VOID * VM_ARG); //�����ͷ�SDL�������Դ

ZL_EXP_VOID mySDL_Init(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDLȫ�ֳ�ʼ����������Ӧ�ű�ģ�麯����sdlInit()
ZL_EXP_VOID mySDL_Quit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL�˳�ʱ����SDL_Quit�ͷ���Դ����Ӧ�ű�ģ�麯����sdlQuit()
ZL_EXP_VOID mySDL_FontQuit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //ͨ��TTF_Quit����������ռ�õ�ϵͳ��Դ ��Ӧ�Ľű������ǣ�sdlFontQuit()
ZL_EXP_VOID mySDL_CreateWin(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL���洴����Ϸ���ڣ���Ӧ�ű�ģ�麯����sdlCreateWin(width,height,bpp,flags,repeat[option])
ZL_EXP_VOID mySDL_PollEvent(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL���¼��в����û�����İ�������������¼��� ��Ӧ�ű�������sdlPollEvent(event) 
ZL_EXP_VOID mySDL_SetBkImg(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //������Ϸ�ı���ͼƬ����Ӧ�Ľű�������sdlSetBkImg(picname | surface point)
ZL_EXP_VOID mySDL_ShowScreen(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //����Ϸ���ƺõ�֡��ʾ���� ��Ӧ�Ľű�������sdlShowScreen()
ZL_EXP_VOID mySDL_LoadBmp(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //����λͼ����Ӧ�Ľű�������sdlLoadBmp(picname,red[option],green[option],blue[option]) (option��ʾ��ѡ����)
ZL_EXP_VOID mySDL_BlitImg(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //��һ��λͼ���Ƶ���һ��λͼ���棬��Ӧ�Ľű�������sdlBlitImg(src surface,src rect,dest surface,dest rect)
ZL_EXP_VOID mySDL_GetKeyState(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //��ȡ����״̬ ��Ӧ�ű�������sdlGetKeyState(array keystate)
ZL_EXP_VOID mySDL_SetCaption(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //������Ϸ���ڵı��⣬��Ӧ�Ľű������ǣ�sdlSetCaption(caption)
ZL_EXP_VOID mySDL_SetColorKey(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //����ĳλͼ�����colorkey ��Ӧ�Ľű�����Ϊ��sdlSetColorKey(image surface , red , green , blue)
ZL_EXP_VOID mySDL_GetTicks(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //��ȡ��Ϸ���еĵδ��� ��Ӧ�Ľű�����Ϊ��sdlGetTicks()
ZL_EXP_VOID mySDL_OpenFont(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //��ĳ�����ļ� ��Ӧ�Ľű�����Ϊ��sdlOpenFont(font_filename , font_size)
ZL_EXP_VOID mySDL_DrawText(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //�����ı���Ϣ ��Ӧ�Ľű�����Ϊ��sdlDrawText(font obj , string , array fontcolor , array bgcolor[option]); (option��ʾ�ǿ�ѡ����)
ZL_EXP_VOID mySDL_CloseFont(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //�ر����� ��Ӧ�Ľű�����Ϊ��sdlCloseFont(font obj)
ZL_EXP_VOID mySDL_FreeImage(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //�ͷ�ĳ��λͼ������Դ ��Ӧ�Ľű�����Ϊ��sdlFreeImage(image surface)
ZL_EXP_VOID mySDL_FontInit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //������Դ��ʼ�� ��Ӧ�Ľű�����Ϊ��sdlFontInit()
ZL_EXP_VOID mySDL_FillRect(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //��Ŀ������������Ϊָ������ɫ ��Ӧ�Ľű�����Ϊ��sdlFillRect(dest surface,dest rect , array color_alpha)
ZL_EXP_VOID mySDL_DrawTextEx(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //�����ı���Ϣ����չ���� ��Ӧ�Ľű�����Ϊ��sdlDrawTextEx(ref surface,font obj , string , array fontcolor , array bgcolor[option]); (option��ʾ�ǿ�ѡ����)
ZL_EXP_VOID mySDL_Delay(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount); //SDL��ʱ����Ӧ�ű�ģ�麯����sdlDelay(millisecond)

/*charset.c��ĺ���*/
ZL_EXP_CHAR * GbkToUtf8ForWindows(ZL_EXP_VOID * VM_ARG,ZL_EXP_CHAR * gbkstr);

#endif /* MY_SDL_LIB_H_ */
