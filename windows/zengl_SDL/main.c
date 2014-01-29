#define ZL_EXP_OS_IN_WINDOWS //�ڼ��������zenglͷ�ļ�֮ǰ��windowsϵͳ�붨��ZL_EXP_OS_IN_WINDOWS��linux , macϵͳ�붨��ZL_EXP_OS_IN_LINUX
#include "zengl_exportfuns.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include "mySDL_Lib.h"

#define STRNULL '\0'
#define DEBUG_INPUT_MAX 50

typedef struct{
	char str[50];
	int ptrIndex;	//str���ڴ���з����ָ�������
	int size;
	int count;
	int cur;
}ReadStr_Type; //�ַ�����̬���飬���ڴ���û����ն��������Ϣ

FILE * debuglog;
ReadStr_Type ReadStr;
int random_seed=0;

ZL_EXP_CHAR * getDebugArg(ZL_EXP_CHAR * str,ZL_EXP_INT * start,ZL_EXP_BOOL needNull)
{
	int i;
	char * ret;
	if((*start) < 0)
	{
		(*start) = -1;
		return ZL_EXP_NULL;
	}
	for(i=(*start);;i++)
	{
		if(str[i] == ' ' || str[i] == '\t')
			continue;
		else if(str[i] == STRNULL)
		{
			(*start) = -1;
			return ZL_EXP_NULL;
		}
		else
		{
			ret = str + i;
			while(++i)
			{
				if(str[i] == ' ' || str[i] == '\t')
				{
					if(needNull != ZL_EXP_FALSE)
						str[i] = STRNULL;
					(*start) = i+1;
					break;
				}
				else if(str[i] == STRNULL)
				{
					(*start) = -1;
					break;
				}
			}
			return ret;
		} //else
	}//for(i=(*start);;i++)
	(*start) = -1;
	return ZL_EXP_NULL;
}

ZL_EXP_VOID main_print_array(ZL_EXP_VOID * VM_ARG,ZENGL_EXPORT_MEMBLOCK memblock,ZL_EXP_INT recur_count);

ZL_EXP_VOID main_print_debug(ZL_EXP_VOID * VM_ARG,ZL_EXP_CHAR * debug_str)
{
	ZENGL_EXPORT_MOD_FUN_ARG reg_debug;
	ZL_EXP_INT debug_str_len = strlen(debug_str);
	ZL_EXP_BOOL hasSemi = ZL_EXP_FALSE;
	zenglApi_GetDebug(VM_ARG,&reg_debug);
	if(debug_str_len > 0 && debug_str[debug_str_len - 1] == ';')
	{
		hasSemi = ZL_EXP_TRUE;
		debug_str[debug_str_len - 1] = ' ';
	}
	printf("%s:",debug_str);
	switch(reg_debug.type)
	{
	case ZL_EXP_FAT_NONE:
		printf("none type , number equal %ld",reg_debug.val.integer);
		break;
	case ZL_EXP_FAT_INT:
		printf("integer:%ld",reg_debug.val.integer);
		break;
	case ZL_EXP_FAT_FLOAT:
		printf("float:%.16g",reg_debug.val.floatnum);
		break;
	case ZL_EXP_FAT_STR:
		printf("string:%s",reg_debug.val.str);
		break;
	case ZL_EXP_FAT_MEMBLOCK:
		printf("array or class obj:\n");
		main_print_array(VM_ARG,reg_debug.val.memblock,0);
		break;
	case ZL_EXP_FAT_ADDR:
	case ZL_EXP_FAT_ADDR_LOC:
	case ZL_EXP_FAT_ADDR_MEMBLK:
		printf("addr type");
		break;
	case ZL_EXP_FAT_INVALID:
		printf("invalid type");
		break;
	}
	if(hasSemi == ZL_EXP_TRUE)
		debug_str[debug_str_len - 1] = ';';
	printf("\n");
}

ZL_EXP_BOOL main_isNumber(ZL_EXP_CHAR * str)
{
	int len = strlen(str);
	int i;
	for(i=0;i<len;i++)
	{
		if(!isdigit(str[i]))
			return ZL_EXP_FALSE;
	}
	return ZL_EXP_TRUE;
}

ZL_EXP_INT main_debug_break(ZL_EXP_VOID * VM_ARG,ZL_EXP_CHAR * cur_filename,ZL_EXP_INT cur_line,ZL_EXP_INT breakIndex,ZL_EXP_CHAR * log)
{
	char * str = ZL_EXP_NULL;
	char * tmpstr = ZL_EXP_NULL;
	char ch;
	char * command,* arg;
	int i,start;
	int exit = 0;
	int str_size = 0;
	int str_count = 0;
	int tmplen;
	if(log != ZL_EXP_NULL)
	{
		if(zenglApi_Debug(VM_ARG,log) == -1)
		{
			printf("log��־�ϵ����%s",zenglApi_GetErrorString(VM_ARG));
		}
		else
		{
			main_print_debug(VM_ARG,log);
			return 0;
		}
	}
	printf("* %s:%d ",cur_filename,cur_line);
	if(breakIndex == -1)
		printf("Single Break [current]\n");
	else
		printf("Break index:%d [current]\n",breakIndex);
	if(str == ZL_EXP_NULL)
	{
		str_size = DEBUG_INPUT_MAX;
		str = zenglApi_AllocMem(VM_ARG,str_size);
	}
	while(!exit)
	{
		printf(">>> debug input:");
		ch = getchar();
		for(i=0;ch!='\n';i++)
		{
			if(i >= str_size - 10) //i�������һ��Ԫ��ʱ����str�������ݣ�str_size - 10��str������β��Ԥ��һ�οռ�
			{
				str_size += DEBUG_INPUT_MAX;
				tmpstr = zenglApi_AllocMem(VM_ARG,str_size);
				strcpy(tmpstr,str);
				zenglApi_FreeMem(VM_ARG,str);
				str = tmpstr;
			}
			str[i] = ch;
			ch = getchar();
		}
		str[i] = STRNULL;
		str_count = i;
		start = 0;
		command = getDebugArg(str,&start,ZL_EXP_TRUE);
		if(command == ZL_EXP_NULL || strlen(command) != 1)
		{
			printf("���������һ���ַ�\n");
			continue;
		}
		switch(command[0])
		{
		case 'p':
			{
				arg = getDebugArg(str,&start,ZL_EXP_FALSE);
				tmplen = arg != ZL_EXP_NULL ? strlen(arg) : 0;
				if(arg != ZL_EXP_NULL && tmplen > 0)
				{
					if(arg[tmplen - 1] != ';' && str_count < str_size - 1)
					{
						arg[tmplen] = ';';
						arg[tmplen+1] = STRNULL;
					}
					if(zenglApi_Debug(VM_ARG,arg) == -1)
					{
						printf("p���Դ���%s\n",zenglApi_GetErrorString(VM_ARG));
						continue;
					}
					main_print_debug(VM_ARG,arg);
				}
				else
					printf("p����ȱ�ٲ���\n");
			}
			break;
		case 'b':
			{
				char * filename = ZL_EXP_NULL;
				int line = 0;
				int count = 0;
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0)
					filename = arg;
				else
				{
					printf("b����ȱ���ļ�������\n");
					continue;
				}
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0)
					line = atoi(arg);
				else
				{
					printf("b����ȱ���кŲ���\n");
					continue;
				}
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0)
					count = atoi(arg);
				if(zenglApi_DebugSetBreak(VM_ARG,filename,line,ZL_EXP_NULL,ZL_EXP_NULL,count,ZL_EXP_FALSE) == -1)
					printf("b����error:%s\n",zenglApi_GetErrorString(VM_ARG));
				else
					printf("���öϵ�ɹ�\n");
			}
			break;
		case 'B':
			{
				int size;
				int totalcount;
				int i;
				char * filename = ZL_EXP_NULL;
				char * condition = ZL_EXP_NULL;
				char * log = ZL_EXP_NULL;
				int count;
				int line;
				ZL_EXP_BOOL disabled;
				if(breakIndex == -1)
					printf("* %s:%d Single Break [current]\n",cur_filename,cur_line);
				size = zenglApi_DebugGetBreak(VM_ARG,-1,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,&totalcount,ZL_EXP_NULL,ZL_EXP_NULL);
				for(i=0;i<size;i++)
				{
					if(zenglApi_DebugGetBreak(VM_ARG,i,&filename,&line,&condition,&log,&count,&disabled,ZL_EXP_NULL) == -1)
						continue;
					else
					{
						printf("[%d] %s:%d",i,filename,line);
						if(condition != ZL_EXP_NULL)
							printf(" C:%s",condition);
						if(log != ZL_EXP_NULL)
							printf(" L:%s",log);
						printf(" N:%d",count);
						if(disabled == ZL_EXP_FALSE)
							printf(" D:enable");
						else
							printf(" D:disable");
						if(i == breakIndex)
							printf(" [current]");
						printf("\n");
					}
				}
				printf("total:%d\n",totalcount);
			}
			break;
		case 'T':
			{	
				int arg = -1;
				int loc = -1;
				int pc = -1;
				int ret;
				int line = 0;
				char * fileName = ZL_EXP_NULL;
				char * className = ZL_EXP_NULL;
				char * funcName = ZL_EXP_NULL;
				while(ZL_EXP_TRUE)
				{
					ret = zenglApi_DebugGetTrace(VM_ARG,&arg,&loc,&pc,&fileName,&line,&className,&funcName);
					if(ret == 1)
					{
						printf(" %s:%d ",fileName,line);
						if(className != ZL_EXP_NULL)
							printf("%s:",className);
						if(funcName != ZL_EXP_NULL)
							printf("%s",funcName);
						printf("\n");
						continue;
					}
					else if(ret == 0)
					{
						printf(" %s:%d ",fileName,line);
						if(className != ZL_EXP_NULL)
							printf("%s:",className);
						if(funcName != ZL_EXP_NULL)
							printf("%s",funcName);
						printf("\n");
						break;
					}
					else if(ret == -1)
					{
						printf("%s",zenglApi_GetErrorString(VM_ARG));
						break;
					}
				}
			}
			break;
		case 'r':
			{
				int arg = -1;
				int loc = -1;
				int pc = -1;
				int tmpPC;
				int ret;
				int size,i;
				int line = 0;
				char * fileName = ZL_EXP_NULL;
				ZL_EXP_BOOL hasBreaked = ZL_EXP_FALSE;
				ret = zenglApi_DebugGetTrace(VM_ARG,&arg,&loc,&pc,&fileName,&line,ZL_EXP_NULL,ZL_EXP_NULL);
				if(ret == 1)
				{
					zenglApi_DebugGetTrace(VM_ARG,&arg,&loc,&pc,&fileName,&line,ZL_EXP_NULL,ZL_EXP_NULL);
					pc++;
					size = zenglApi_DebugGetBreak(VM_ARG,-1,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL);
					for(i=0;i<size;i++)
					{
						if(zenglApi_DebugGetBreak(VM_ARG,i,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,&tmpPC) == -1)
							continue;
						else if(pc == tmpPC)
						{
							hasBreaked = ZL_EXP_TRUE;
							break;
						}
					}
					if(!hasBreaked)
					{
						if(zenglApi_DebugSetBreakEx(VM_ARG,pc,ZL_EXP_NULL,ZL_EXP_NULL,1,ZL_EXP_FALSE) == -1)
							printf("%s",zenglApi_GetErrorString(VM_ARG));
						else
							exit = 1;
					}
					else
						exit = 1;
				}
				else if(ret == 0)
					exit = 1;
				else if(ret == -1)
					printf("%s",zenglApi_GetErrorString(VM_ARG));
			}
			break;
		case 'd':
			{
				int index;
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0 && main_isNumber(arg))
					index = atoi(arg);
				else
				{
					printf("d����ȱ�ٶϵ���������\n");
					continue;
				}
				if(zenglApi_DebugDelBreak(VM_ARG,index) == -1)
					printf("d����error:��Ч�Ķϵ�����");
				else
					printf("ɾ���ϵ�ɹ�");
				printf("\n");
			}
			break;
		case 'D':
			{
				int index;
				char * filename = ZL_EXP_NULL;
				char * condition = ZL_EXP_NULL;
				char * log = ZL_EXP_NULL;
				int count;
				int line;
				ZL_EXP_BOOL disabled;
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0 && main_isNumber(arg))
					index = atoi(arg);
				else
				{
					printf("D����ȱ�ٶϵ���������\n");
					continue;
				}
				if(zenglApi_DebugGetBreak(VM_ARG,index,&filename,&line,&condition,&log,&count,&disabled,ZL_EXP_NULL) == -1)
				{
					printf("D����error:��Ч�Ķϵ�����\n");
					continue;
				}
				else
				{
					if(zenglApi_DebugSetBreak(VM_ARG,filename,line,condition,log,count,ZL_EXP_TRUE) == -1)
						printf("D������öϵ�error:%s",zenglApi_GetErrorString(VM_ARG));
					else
						printf("D������öϵ�ɹ�");
					printf("\n");
				}
			}
			break;
		case 'C':
			{
				int index;
				char * newCondition;
				char * filename = ZL_EXP_NULL;
				char * condition = ZL_EXP_NULL;
				char * log = ZL_EXP_NULL;
				int count;
				int line;
				ZL_EXP_BOOL disabled;
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0 && main_isNumber(arg))
					index = atoi(arg);
				else
				{
					printf("C����ȱ�ٶϵ���������\n");
					continue;
				}
				arg = getDebugArg(str,&start,ZL_EXP_FALSE);
				tmplen = arg != ZL_EXP_NULL ? strlen(arg) : 0;
				if(arg != ZL_EXP_NULL && tmplen > 0)
				{
					if(arg[tmplen - 1] != ';' && str_count < str_size - 1)
					{
						arg[tmplen] = ';';
						arg[tmplen+1] = STRNULL;
					}
					newCondition = arg;
				}
				else
					newCondition = ZL_EXP_NULL;
				if(zenglApi_DebugGetBreak(VM_ARG,index,&filename,&line,&condition,&log,&count,&disabled,ZL_EXP_NULL) == -1)
				{
					printf("C����error:��Ч�Ķϵ�����\n");
					continue;
				}
				else
				{
					if(zenglApi_DebugSetBreak(VM_ARG,filename,line,newCondition,log,count,disabled) == -1)
						printf("C�������������ϵ�error:%s",zenglApi_GetErrorString(VM_ARG));
					else
						printf("C�������������ϵ�ɹ�");
					printf("\n");
				}
			}
			break;
		case 'L':
			{
				int index;
				char * newLog;
				char * filename = ZL_EXP_NULL;
				char * condition = ZL_EXP_NULL;
				char * log = ZL_EXP_NULL;
				int count;
				int line;
				ZL_EXP_BOOL disabled;
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0 && main_isNumber(arg))
					index = atoi(arg);
				else
				{
					printf("L����ȱ�ٶϵ���������\n");
					continue;
				}
				arg = getDebugArg(str,&start,ZL_EXP_FALSE);
				tmplen = arg != ZL_EXP_NULL ? strlen(arg) : 0;
				if(arg != ZL_EXP_NULL && tmplen > 0)
				{
					if(arg[tmplen - 1] != ';' && str_count < str_size - 1)
					{
						arg[tmplen] = ';';
						arg[tmplen+1] = STRNULL;
					}
					newLog = arg;
				}
				else
					newLog = ZL_EXP_NULL;
				if(zenglApi_DebugGetBreak(VM_ARG,index,&filename,&line,&condition,&log,&count,&disabled,ZL_EXP_NULL) == -1)
				{
					printf("L����error:��Ч�Ķϵ�����\n");
					continue;
				}
				else
				{
					if(zenglApi_DebugSetBreak(VM_ARG,filename,line,condition,newLog,count,disabled) == -1)
						printf("L����������־�ϵ�error:%s",zenglApi_GetErrorString(VM_ARG));
					else
						printf("L����������־�ϵ�ɹ�");
					printf("\n");
				}
			}
			break;
		case 'N':
			{
				int index;
				int newCount;
				char * filename = ZL_EXP_NULL;
				char * condition = ZL_EXP_NULL;
				char * log = ZL_EXP_NULL;
				int count;
				int line;
				ZL_EXP_BOOL disabled;
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0 && main_isNumber(arg))
					index = atoi(arg);
				else
				{
					printf("N����ȱ�ٶϵ���������\n");
					continue;
				}
				arg = getDebugArg(str,&start,ZL_EXP_TRUE);
				if(arg != ZL_EXP_NULL && strlen(arg) > 0 && main_isNumber(arg))
					newCount = atoi(arg);
				else
				{
					printf("N����ȱ�ٶϵ��������\n");
					continue;
				}
				if(zenglApi_DebugGetBreak(VM_ARG,index,&filename,&line,&condition,&log,&count,&disabled,ZL_EXP_NULL) == -1)
				{
					printf("N����error:��Ч�Ķϵ�����\n");
					continue;
				}
				else
				{
					if(zenglApi_DebugSetBreak(VM_ARG,filename,line,condition,log,newCount,disabled) == -1)
						printf("N�������öϵ����error:%s",zenglApi_GetErrorString(VM_ARG));
					else
						printf("N�������öϵ�����ɹ�");
					printf("\n");
				}
			}
			break;
		case 's':
			zenglApi_DebugSetSingleBreak(VM_ARG,ZL_EXP_TRUE);
			exit = 1;
			break;
		case 'S':
			zenglApi_DebugSetSingleBreak(VM_ARG,ZL_EXP_FALSE);
			exit = 1;
			break;
		case 'c':
			exit = 1;
			break;
		case 'h':
			printf(" p ���Ա�����Ϣ usage:p express\n"
				" b ���öϵ� usage:b filename lineNumber\n"
				" B �鿴�ϵ��б� usage:B\n"
				" T �鿴�ű������Ķ�ջ������Ϣ usage:T\n"
				" d ɾ��ĳ�ϵ� usage:d breakIndex\n"
				" D ����ĳ�ϵ� usage:D breakIndex\n"
				" C ���������ϵ� usage:C breakIndex condition-express\n"
				" L ������־�ϵ� usage:L breakIndex log-express\n"
				" N ���öϵ���� usage:N breakIndex count\n"
				" s �������� usage:s\n"
				" S �������� usage:S\n"
				" r ִ�е����� usage:r\n"
				" c ����ִ�� usage:c\n");
			break;
		default:
			printf("��Ч������\n");
			break;
		}
	} //while(!exit)
	if(str != ZL_EXP_NULL)
		zenglApi_FreeMem(VM_ARG,str);
	return 0;
}

ZL_EXP_INT main_debug_conditionError(ZL_EXP_VOID * VM_ARG,ZL_EXP_CHAR * filename,ZL_EXP_INT line,ZL_EXP_INT breakIndex,ZL_EXP_CHAR * error)
{
	char * condition;
	zenglApi_DebugGetBreak(VM_ARG,breakIndex,ZL_EXP_NULL,ZL_EXP_NULL,&condition,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL,ZL_EXP_NULL);
	printf("\n%s [%d] <%d %s> error:%s\n",filename,line,breakIndex,condition,error);
	return 0;
}

/*
	���û����������д�뵽ReadStrȫ���ַ�����̬�����С�
*/
void makeReadStr(char ch)
{
	if(ReadStr.cur < 45)
	{
		ReadStr.str[ReadStr.cur++] = ch;
		ReadStr.str[ReadStr.cur] = STRNULL;
		ReadStr.count++;
	}
}

/*
	��ȡ��һ���û����������ݺ���Ҫ��ReadStr�ַ�����̬������α�ͼ���������Ϊ0�������´λ�ȡ��������ʱ���ܵõ���ȷ�����ݣ��������ܵ���һ���������ݵ�Ӱ�졣 
*/
void freeReadStr()
{
	ReadStr.cur = 0;
	ReadStr.count = 0;
}

ZL_EXP_INT main_userdef_info(ZL_EXP_CHAR * infoStrPtr, ZL_EXP_INT infoStrCount,ZL_EXP_VOID * VM_ARG)
{
	fprintf(debuglog,"%s",infoStrPtr);
	return 0;
}

ZL_EXP_INT main_userdef_compile_error(ZL_EXP_CHAR * infoStrPtr, ZL_EXP_INT infoStrCount,ZL_EXP_VOID * VM_ARG)
{
	printf("%s",infoStrPtr);
	return 0;
}

ZL_EXP_INT main_userdef_run_info(ZL_EXP_CHAR * infoStrPtr, ZL_EXP_INT infoStrCount,ZL_EXP_VOID * VM_ARG)
{
	fprintf(debuglog,"%s",infoStrPtr);
	return 0;
}

ZL_EXP_INT main_userdef_run_print(ZL_EXP_CHAR * infoStrPtr, ZL_EXP_INT infoStrCount,ZL_EXP_VOID * VM_ARG)
{
	printf("%s\n",infoStrPtr);
	return 0;
}

ZL_EXP_INT main_userdef_run_error(ZL_EXP_CHAR * infoStrPtr, ZL_EXP_INT infoStrCount,ZL_EXP_VOID * VM_ARG)
{
	printf("%s",infoStrPtr);
	return 0;
}

/*builtinģ�麯��*/
ZL_EXP_VOID main_builtin_printf(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT i;
	if(argcount < 1)
		zenglApi_Exit(VM_ARG,"printf�����Ĳ���������Ϊ��");
	for(i=1;i<=argcount;i++)
	{
		zenglApi_GetFunArg(VM_ARG,i,&arg);
		switch(arg.type)
		{
		case ZL_EXP_FAT_INT:
			printf("%ld",arg.val.integer);
			break;
		case ZL_EXP_FAT_FLOAT:
			printf("%.16g",arg.val.floatnum);
			break;
		case ZL_EXP_FAT_STR:
			printf("%s",arg.val.str);
			break;
		default:
			zenglApi_Exit(VM_ARG,"printf������%d������������Ч��Ŀǰֻ֧���ַ��������������������͵Ĳ���",i);
			break;
		}
	}
	return;
}

/*readģ�麯��*/
ZL_EXP_VOID main_builtin_read(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	char ch;
	ch = getchar();
	while(ch!='\n')
	{
		makeReadStr(ch);
		ch = getchar();
	}
	if(ReadStr.count == 0)
		zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_STR,"",0,0);
	else
		zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_STR,ReadStr.str,0,0);
	freeReadStr();
}

/*bltRandomģ�麯��*/
ZL_EXP_VOID main_builtin_random(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	time_t t;
	if(random_seed == 0) //��һ��ʹ��ʱ����Ϊ������ӡ�
	{
		srand((unsigned) time(&t));
		random_seed = rand();
	}
	else //����ʱ��ʹ����һ�����ɵ��������Ϊ�������
	{
		srand(random_seed);
		random_seed = rand();
	}
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,(long)random_seed,0);
}

/*arrayģ�麯�������ڴ���zengl�ű��Ķ�̬����*/
ZL_EXP_VOID main_builtin_array(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT i;
	if(argcount == 0) //���array����û���������򴴽�һ��Ĭ�ϴ�С��δ��ʼ��������
	{
		zenglApi_CreateMemBlock(VM_ARG,&memblock,0);
		zenglApi_SetRetValAsMemBlock(VM_ARG,&memblock);
	}
	else if(argcount >= 1) //������˲�����ʹ�ò�����Ϊ�����ĳ�ʼֵ
	{
		zenglApi_CreateMemBlock(VM_ARG,&memblock,0);
		for(i=1;i<=argcount;i++)
		{
			zenglApi_GetFunArg(VM_ARG,i,&arg);
			switch(arg.type)
			{
			case ZL_EXP_FAT_INT:
			case ZL_EXP_FAT_FLOAT:
			case ZL_EXP_FAT_STR:
			case ZL_EXP_FAT_MEMBLOCK:
			case ZL_EXP_FAT_ADDR:
			case ZL_EXP_FAT_ADDR_LOC:
			case ZL_EXP_FAT_ADDR_MEMBLK:
				zenglApi_SetMemBlock(VM_ARG,&memblock,i,&arg);
				break;
			default:
				zenglApi_Exit(VM_ARG,"array������%d������������Ч",i);
				break;
			}
		}
		zenglApi_SetRetValAsMemBlock(VM_ARG,&memblock);
	}
	else
		zenglApi_Exit(VM_ARG,"array�����쳣����������С��0");
}

/*�ݹ��ӡ��������Ϣ*/
ZL_EXP_VOID main_print_array(ZL_EXP_VOID * VM_ARG,ZENGL_EXPORT_MEMBLOCK memblock,ZL_EXP_INT recur_count)
{
	ZL_EXP_INT size,i,j;
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
				printf("  ");
			break;
		}
		switch(mblk_val.type)
		{
		case ZL_EXP_FAT_INT:
			printf("[%d] %ld\n",i-1,mblk_val.val.integer);
			break;
		case ZL_EXP_FAT_FLOAT:
			printf("[%d] %.16g\n",i-1,mblk_val.val.floatnum);
			break;
		case ZL_EXP_FAT_STR:
			printf("[%d] %s\n",i-1,mblk_val.val.str);
			break;
		case ZL_EXP_FAT_MEMBLOCK:
			printf("[%d] <array or class obj type> begin:\n",i-1);
			main_print_array(VM_ARG,mblk_val.val.memblock,recur_count+1);
			printf("[%d] <array or class obj type> end\n",i-1);
			break;
		}
	}
}

/*bltPrintArrayģ�麯������ӡ�����е�Ԫ��*/
ZL_EXP_VOID main_builtin_print_array(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	if(argcount < 1)
		zenglApi_Exit(VM_ARG,"bltPrintArray������������Ϊ�գ�����ָ��һ������������Ϊ����");
	zenglApi_GetFunArg(VM_ARG,1,&arg);
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"bltPrintArray�����Ĳ�����������������");
	main_print_array(VM_ARG,arg.val.memblock,0);
}

/*bltTestAddrģ�麯��(��������)�����Բ�������*/
ZL_EXP_VOID main_builtin_test_addr(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	if(argcount < 2)
		zenglApi_Exit(VM_ARG,"bltTestAddr��������������������");
	zenglApi_GetFunArgInfo(VM_ARG,1,&arg);
	if(arg.type != ZL_EXP_FAT_ADDR && 
		arg.type != ZL_EXP_FAT_ADDR_LOC &&
		arg.type != ZL_EXP_FAT_ADDR_MEMBLK
		)
		zenglApi_Exit(VM_ARG,"��һ�����������Ǳ��������ã��������Ա�����ã�����������Ա������");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ��������ֵ��zenglApi_GetFunArg���Եݹ����ã��ҵ����õı�����ֵ
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"bltTestAddr����Ŀǰֻ�ܽ����ַ�����Ϊ����");
	printf("the value of first arg is %s\n",arg.val.str);
	zenglApi_GetFunArg(VM_ARG,2,&arg); //�õ��ڶ������������øò������ַ���ֵ���õ�һ���������õı���
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"bltTestAddr����Ŀǰֻ�ܽ����ַ�����Ϊ����");
	zenglApi_SetFunArg(VM_ARG,1,&arg);
}

/*bltSetArrayģ�麯����ʹ�õ�2������3���Ȳ��������õ�һ��������Ӧ�������е�Ԫ��*/
ZL_EXP_VOID main_builtin_set_array(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	ZL_EXP_INT i;
	if(argcount < 2)
		zenglApi_Exit(VM_ARG,"bltSetArray��������������������");
	zenglApi_GetFunArg(VM_ARG,1,&arg);
	if(arg.type != ZL_EXP_FAT_MEMBLOCK)
		zenglApi_Exit(VM_ARG,"bltSetArray������һ����������������������������ߵ���������");
	memblock = arg.val.memblock;
	for(i=2;i<=argcount;i++)
	{
		zenglApi_GetFunArg(VM_ARG,i,&arg);
		zenglApi_SetMemBlock(VM_ARG,&memblock,i-1,&arg);
	}
}

ZL_EXP_VOID main_userdef_module_init(ZL_EXP_VOID * VM_ARG);

/*bltLoadScriptģ�麯�����½�һ������������ز�ִ��ĳ���ű�*/
ZL_EXP_VOID main_builtin_load_script(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_CHAR * scriptName = ZL_EXP_NULL;
	ZENGL_EXPORT_VM_MAIN_ARGS vm_main_args = {main_userdef_info , 
											  main_userdef_compile_error,
											  main_userdef_run_info,
											  main_userdef_run_print,
											  main_userdef_run_error,
											  main_userdef_module_init,
											  ZL_EXP_CP_AF_IN_DEBUG_MODE | 
											  ZL_EXP_CP_AF_OUTPUT_DEBUG_INFO};
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ������Ϊ�ű���
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"bltLoadScript������һ�����������ַ���������Ҫ���صĽű��ļ���");
	scriptName = arg.val.str;
	zenglApi_Load(scriptName,&vm_main_args); 
}

/*bltGetZLVersionģ�麯������ȡ��ǰzengl�汾����Ϣ���ַ�����ʽ*/
ZL_EXP_VOID main_builtin_get_zl_version(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZL_EXP_CHAR version[20] = {0};
	sprintf(version,"%d.%d.%d",ZL_EXP_MAJOR_VERSION,ZL_EXP_MINOR_VERSION,ZL_EXP_REVISION);
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_STR,version,0,0);
}

/*bltGetExtraDataģ�麯������ȡ�û���������*/
ZL_EXP_VOID main_builtin_get_extraData(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_CHAR * extraName;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"bltGetExtraData����������һ������");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ������Ϊ�ű���
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"bltGetExtraData������һ�������������ַ�������ʾ������������");
	extraName = arg.val.str;
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_STR,(ZL_EXP_CHAR *)zenglApi_GetExtraData(VM_ARG,extraName),0,0);
}

/*debug����ģ�麯��*/
ZL_EXP_VOID main_builtin_debug(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_CHAR * debug_str;
	if(argcount != 1)
		zenglApi_Exit(VM_ARG,"debug����������һ������");
	zenglApi_GetFunArg(VM_ARG,1,&arg); //��ȡ��һ������Ϊ�ű���
	if(arg.type != ZL_EXP_FAT_STR)
		zenglApi_Exit(VM_ARG,"debug������һ�������������ַ�������ʾ�����ַ���");
	debug_str = arg.val.str;
	if(zenglApi_Debug(VM_ARG,debug_str) == -1)
	{
		zenglApi_Exit(VM_ARG,"%s",zenglApi_GetErrorString(VM_ARG));
	}
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,0,0);
}

ZL_EXP_VOID main_builtin_module_init(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT moduleID)
{
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"printf",main_builtin_printf);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"read",main_builtin_read);
	//zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltRandom",main_builtin_random); //ʹ��main.c�ж����bltRandom
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltRandom",zenglApiBMF_bltRandom); //ʹ�������zenglApi_BltModFuns.c�ж����bltRandom
	//zenglApi_SetModFunHandle(VM_ARG,moduleID,"array",main_builtin_array); //ʹ��main.c�ж����array
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"array",zenglApiBMF_array);  //ʹ�������zenglApi_BltModFuns.c�ж����array
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"unset",zenglApiBMF_unset);  //ʹ�������zenglApi_BltModFuns.c�ж����unset
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltExit",zenglApiBMF_bltExit);  //ʹ�������zenglApi_BltModFuns.c�ж����bltExit
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltConvToInt",zenglApiBMF_bltConvToInt);  //ʹ�������zenglApi_BltModFuns.c�ж����bltConvToInt
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltIntToStr",zenglApiBMF_bltIntToStr);  //ʹ�������zenglApi_BltModFuns.c�ж����bltIntToStr
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltPrintArray",main_builtin_print_array);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltTestAddr",main_builtin_test_addr);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltSetArray",main_builtin_set_array);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltLoadScript",main_builtin_load_script);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltGetZLVersion",main_builtin_get_zl_version);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"bltGetExtraData",main_builtin_get_extraData);
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"debug",main_builtin_debug);
}

ZL_EXP_VOID main_sdl_module_init(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT moduleID)
{
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlInit",mySDL_Init); //SDLȫ�ֳ�ʼ����������Ӧ�ű�ģ�麯����sdlInit()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlQuit",mySDL_Quit); //SDL�˳�ʱ����SDL_Quit�ͷ���Դ����Ӧ�ű�ģ�麯����sdlQuit()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlFontQuit",mySDL_FontQuit); //ͨ��TTF_Quit����������ռ�õ�ϵͳ��Դ ��Ӧ�Ľű������ǣ�sdlFontQuit()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlCreateWin",mySDL_CreateWin); //SDL���洴����Ϸ���ڣ���Ӧ�ű�ģ�麯����sdlCreateWin(width,height,bpp,flags,repeat[option])
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlPollEvent",mySDL_PollEvent); //SDL���¼��в����û�����İ�������������¼��� ��Ӧ�ű�������sdlPollEvent(event) 
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlSetBkImg",mySDL_SetBkImg); //������Ϸ�ı���ͼƬ����Ӧ�Ľű�������sdlSetBkImg(picname | surface point)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlShowScreen",mySDL_ShowScreen); //����Ϸ���ƺõ�֡��ʾ���� ��Ӧ�Ľű�������sdlShowScreen()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlLoadBmp",mySDL_LoadBmp); //����λͼ����Ӧ�Ľű�������sdlLoadBmp(picname,red[option],green[option],blue[option]) (option��ʾ��ѡ����)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlBlitImg",mySDL_BlitImg); //��һ��λͼ���Ƶ���һ��λͼ���棬��Ӧ�Ľű�������sdlBlitImg(src surface,src rect,dest surface,dest rect)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlGetKeyState",mySDL_GetKeyState); //��ȡ����״̬ ��Ӧ�ű�������sdlGetKeyState(array keystate)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlSetCaption",mySDL_SetCaption); //������Ϸ���ڵı��⣬��Ӧ�Ľű������ǣ�sdlSetCaption(caption)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlSetColorKey",mySDL_SetColorKey); //����ĳλͼ�����colorkey ��Ӧ�Ľű�����Ϊ��sdlSetColorKey(image surface , red , green , blue)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlGetTicks",mySDL_GetTicks); //��ȡ��Ϸ���еĵδ��� ��Ӧ�Ľű�����Ϊ��sdlGetTicks()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlOpenFont",mySDL_OpenFont); //��ĳ�����ļ� ��Ӧ�Ľű�����Ϊ��sdlOpenFont(font_filename , font_size)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlDrawText",mySDL_DrawText); //�����ı���Ϣ ��Ӧ�Ľű�����Ϊ��sdlDrawText(font obj , string , array fontcolor , array bgcolor[option]); (option��ʾ�ǿ�ѡ����)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlCloseFont",mySDL_CloseFont);//�ر����� ��Ӧ�Ľű�����Ϊ��sdlCloseFont(font obj)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlFreeImage",mySDL_FreeImage);//�ͷ�ĳ��λͼ������Դ ��Ӧ�Ľű�����Ϊ��sdlFreeImage(image surface)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlFontInit",mySDL_FontInit); //������Դ��ʼ�� ��Ӧ�Ľű�����Ϊ��sdlFontInit()
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlFillRect",mySDL_FillRect); //��Ŀ������������Ϊָ������ɫ ��Ӧ�Ľű�����Ϊ��sdlFillRect(dest surface,dest rect , array color_alpha)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlDrawTextEx",mySDL_DrawTextEx);//�����ı���Ϣ����չ���� ��Ӧ�Ľű�����Ϊ��sdlDrawTextEx(ref surface,font obj , string , array fontcolor , array bgcolor[option]); (option��ʾ�ǿ�ѡ����)
	zenglApi_SetModFunHandle(VM_ARG,moduleID,"sdlDelay",mySDL_Delay); //SDL��ʱ����Ӧ�ű�ģ�麯����sdlDelay(millisecond)
}

ZL_EXP_VOID main_userdef_module_init(ZL_EXP_VOID * VM_ARG)
{
	zenglApi_SetModInitHandle(VM_ARG,"builtin",main_builtin_module_init);
	zenglApi_SetModInitHandle(VM_ARG,"sdl",main_sdl_module_init);
}

void main_exit(void * VM,char * err_format,...)
{
	va_list arg;
	va_start(arg,err_format);
	vprintf(err_format,arg);
	va_end(arg);
	fun_sdlLib_destroyAll(VM); //�����ͷ�SDL�������Դ
	zenglApi_Close(VM);
	fclose(debuglog);
	#ifdef ZL_EXP_OS_IN_WINDOWS
		system("pause");
	#endif
	exit(-1);
}

 /* 
    ��������getFileSize(char * strFileName)  
    ���ܣ���ȡָ���ļ��Ĵ�С 
    ������ 
        strFileName (char *)���ļ��� 
    ����ֵ�� 
        size (int)���ļ���С 
 */  
int getFileSize(char * strFileName)   
{  
    FILE * fp = fopen(strFileName, "r"); 
	int size;
    fseek(fp, 0L, SEEK_END);  
    size = ftell(fp);  
    fclose(fp);  
    return size;  
}

int main_output_xor_source(char * src_filename,char * dest_filename,char * xor_key_str)
{
	FILE * src_pt = ZL_EXP_NULL ,* dest_pt = ZL_EXP_NULL;
	int src_filesize = 0;
	unsigned char buf[1024],enc_buf[1024],ch,enc_ch;
	int buf_len = 0,totalwrite_len = 0;
	int prev_percent = 0,cur_percent = 0;
	int cur = 0;
	int xor_key_cur = 0;
	int xor_key_len;

	if(xor_key_str == ZL_EXP_NULL)
	{
		printf("xor_key_str��Կ����Ϊ���ַ���ָ��\n");
		return -1;
	}
	xor_key_len = strlen(xor_key_str);
	src_filesize = getFileSize(src_filename);
	src_pt = fopen(src_filename,"rb");
	if(src_pt == ZL_EXP_NULL)
	{
		printf("xor���ܵ�Դ�ű��ļ��޷���\n");
		return -1;
	}
	dest_pt = fopen(dest_filename,"wb+");
	if(dest_pt == ZL_EXP_NULL)
	{
		printf("xor���ܵ�Ŀ��ű��ļ��޷��򿪻��޷�����\n");
		return -1;
	}
	while((buf_len = fread(buf,sizeof(unsigned char),1024,src_pt)) != 0)
	{
		for(cur = 0;cur < buf_len;cur++)
		{
			ch = buf[cur];
			enc_ch = ch ^ (unsigned char)xor_key_str[xor_key_cur];
			if((++xor_key_cur) >= xor_key_len)
				xor_key_cur = 0;
			enc_buf[cur] = enc_ch;
		}
		fwrite(enc_buf,sizeof(unsigned char),buf_len,dest_pt);
		totalwrite_len += buf_len;
		cur_percent = (int)(((float)totalwrite_len / (float)src_filesize) * 100);
		if(cur_percent != prev_percent)
			printf("%d%%...",cur_percent);
	}
	printf(" xor '%s' to '%s' is ok!\n",src_filename,dest_filename);
	fclose(src_pt);
	fclose(dest_pt);
	return 0;
}

/*rc4��ʼ��״̬����*/
void rc4_init(unsigned char * state, unsigned char * key, int len)
{
   int i,j=0,t; 
   
   for (i=0; i < 256; ++i) //���������Ԫ����0��255��ʼ��
      state[i] = i; 
   for (i=0; i < 256; ++i) { //���������Ԫ��˳�����
      j = (j + state[i] + key[i % len]) % 256; 
      t = state[i]; 
      state[i] = state[j]; 
      state[j] = t; 
   }   
}

int main_output_rc4_source(char * src_filename,char * dest_filename,char * rc4_key_str)
{
	FILE * src_pt = ZL_EXP_NULL ,* dest_pt = ZL_EXP_NULL;
	int src_filesize = 0;
	unsigned char buf[1024],enc_buf[1024];
	int buf_len = 0,totalwrite_len = 0;
	int prev_percent = 0,cur_percent = 0;
	int cur = 0;
	unsigned char state[256]; //rc4����������Կ����״̬����
	int i,j,t;
	int rc4_key_len;

	if(rc4_key_str == ZL_EXP_NULL)
	{
		printf("rc4_key_str��Կ����Ϊ���ַ���ָ��\n");
		return -1;
	}
	rc4_key_len = strlen(rc4_key_str);
	if(rc4_key_len > 256)
	{
		printf("rc4_key_str��Կ���Ȳ�Ҫ����256���ֽ�\n");
		return -1;
	}
	src_filesize = getFileSize(src_filename);
	src_pt = fopen(src_filename,"rb");
	if(src_pt == ZL_EXP_NULL)
	{
		printf("rc4���ܵ�Դ�ű��ļ��޷���\n");
		return -1;
	}
	dest_pt = fopen(dest_filename,"wb+");
	if(dest_pt == ZL_EXP_NULL)
	{
		printf("rc4���ܵ�Ŀ��ű��ļ��޷��򿪻��޷�����\n");
		return -1;
	}
	rc4_init(state,rc4_key_str,rc4_key_len);
	i = 0;
	j = 0;
	while((buf_len = fread(buf,sizeof(unsigned char),1024,src_pt)) != 0)
	{
		for(cur = 0;cur < buf_len;cur++)
		{
			i = (i + 1) % 256;
			j = (j + state[i]) % 256;
			t = state[i];
			state[i] = state[j]; 
			state[j] = t;
			enc_buf[cur] = state[(state[i] + state[j]) % 256] ^ buf[cur];
		}
		fwrite(enc_buf,sizeof(unsigned char),buf_len,dest_pt);
		totalwrite_len += buf_len;
		cur_percent = (int)(((float)totalwrite_len / (float)src_filesize) * 100);
		if(cur_percent != prev_percent)
			printf("%d%%...",cur_percent);
	}
	printf(" rc4 '%s' to '%s' is ok!\n",src_filename,dest_filename);
	fclose(src_pt);
	fclose(dest_pt);
	return 0;
}

/**
	�û�����ִ����ڡ�
*/
int main(int argc,char * argv[])
{
	ZL_EXP_VOID * VM;

	if(argc < 2)
	{
		printf("usage: %s <filename> ... (�÷�����Ӧ���ǣ������� + zengl�ű��ļ�������ʽ���ں����-d�������Կ�������)\n",argv[0]);
		#ifdef ZL_EXP_OS_IN_WINDOWS
			system("pause");
		#endif
		exit(-1);
	}

	debuglog = fopen("main_debuglogs.txt","w+");
	VM = zenglApi_Open();
	zenglApi_SetFlags(VM,(ZENGL_EXPORT_VM_MAIN_ARG_FLAGS)(ZL_EXP_CP_AF_IN_DEBUG_MODE | ZL_EXP_CP_AF_OUTPUT_DEBUG_INFO));
	zenglApi_SetHandle(VM,ZL_EXP_VFLAG_HANDLE_COMPILE_INFO,main_userdef_info);
	zenglApi_SetHandle(VM,ZL_EXP_VFLAG_HANDLE_RUN_INFO,main_userdef_run_info);
	zenglApi_SetHandle(VM,ZL_EXP_VFLAG_HANDLE_RUN_PRINT,main_userdef_run_print);
	zenglApi_SetHandle(VM,ZL_EXP_VFLAG_HANDLE_MODULE_INIT,main_userdef_module_init); //Ҳ�����ڴ˴�����ģ���ʼ�����

	if(argc >= 3 && strcmp(argv[2],"-d") == 0)
		zenglApi_DebugSetBreakHandle(VM,main_debug_break,main_debug_conditionError,ZL_EXP_TRUE,ZL_EXP_FALSE); //���õ���API

	if(zenglApi_Run(VM,argv[1]) == -1) //����ִ��zengl�ű�
		main_exit(VM,"���󣺱���ִ��<%s>ʧ�ܣ�%s\n",argv[1],zenglApi_GetErrorString(VM));

	fun_sdlLib_destroyAll(VM); //�����ͷ�SDL�������Դ

	zenglApi_Close(VM);

	fclose(debuglog);

	#ifdef ZL_EXP_OS_IN_WINDOWS
		system("pause");
	#endif
	
	return 0;
}
