/*
   Copyright 2012 zenglong (made in china)

   For more information, please see www.zengl.com

   This file is part of zengl language.

   zengl language is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   zengl language is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with zengl language,the copy is in the file licence.txt.  If not,
   see <http://www.gnu.org/licenses/>.
*/

/**
	���ļ������������Ϊ�û��ṩ���ڽ�ģ�麯��(ֻ��һЩ�ܻ����Ľű�ģ�麯��)���û����Ը�����Ҫѡ���Ƿ�ʹ����Щ�ڽ�ģ�麯��
	�û������Ը��ݴ��ļ����ģ�麯��Ϊģ�壬���Ƶ��Լ��ĳ����н����޸��滻��
*/

#include "zengl_global.h"

/*array�ڽ�ģ�麯�������ڴ���zengl�ű��Ķ�̬����*/
ZL_EXP_VOID zenglApiBMF_array(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MEMBLOCK memblock = {0};
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT i;
	ZL_EXP_CHAR * modfun_name;
	if(argcount == 0) //���array����û���������򴴽�һ��Ĭ�ϴ�С��δ��ʼ��������
	{
		if(zenglApi_CreateMemBlock(VM_ARG,&memblock,0) == -1)
			zenglApi_Exit(VM_ARG,zenglApi_GetErrorString(VM_ARG));
		zenglApi_SetRetValAsMemBlock(VM_ARG,&memblock);
	}
	else if(argcount >= 1) //������˲�����ʹ�ò�����Ϊ�����ĳ�ʼֵ
	{
		if(zenglApi_CreateMemBlock(VM_ARG,&memblock,0) == -1)
			zenglApi_Exit(VM_ARG,zenglApi_GetErrorString(VM_ARG));
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
				zenglApi_GetModFunName(VM_ARG,&modfun_name); //��ȡ�û��Զ����ģ�麯����
				zenglApi_Exit(VM_ARG,"%s������%d������������Ч",modfun_name,i);
				break;
			}
		}
		zenglApi_SetRetValAsMemBlock(VM_ARG,&memblock);
	}
	else
	{
		zenglApi_GetModFunName(VM_ARG,&modfun_name); //��ȡ�û��Զ����ģ�麯����
		zenglApi_Exit(VM_ARG,"%s�����쳣����������С��0",modfun_name);
	}
}

/*bltExitģ�麯����ֱ���˳�zengl�ű�*/
ZL_EXP_VOID zenglApiBMF_bltExit(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_CHAR * modfun_name;
	if(argcount > 0)
	{
		zenglApi_GetFunArg(VM_ARG,1,&arg); //�õ���һ������
		if(arg.type != ZL_EXP_FAT_STR)
		{
			zenglApi_GetModFunName(VM_ARG,&modfun_name); //��ȡ�û��Զ����ģ�麯����
			zenglApi_Exit(VM_ARG,"%s�����ĵ�һ�������������ַ�������ʾ�˳��ű�ʱ��Ҫ��ʾ����Ϣ",modfun_name);
		}
		zenglApi_Exit(VM_ARG,arg.val.str);
	}
	else
	{
		zenglApi_Stop(VM_ARG); //���û�в�����ֱ��ֹͣ�ű���ִ�У��������������Ϣ
		return;
	}
}

/*bltConvToIntģ�麯����������תΪ������ʽ*/
ZL_EXP_VOID zenglApiBMF_bltConvToInt(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT ret;
	ZL_EXP_CHAR * modfun_name;
	if(argcount != 1)
	{
		zenglApi_GetModFunName(VM_ARG,&modfun_name);
		zenglApi_Exit(VM_ARG,"%s�����������1������",modfun_name);
	}
	zenglApi_GetFunArg(VM_ARG,1,&arg); //�õ���һ������
	switch(arg.type)
	{
	case ZL_EXP_FAT_FLOAT:
		ret = (ZL_EXP_INT)arg.val.floatnum;
		break;
	case ZL_EXP_FAT_STR:
		ret = atoi(arg.val.str);
		break;
	case ZL_EXP_FAT_INT:
		ret = arg.val.integer;
		break;
	default:
		zenglApi_GetModFunName(VM_ARG,&modfun_name);
		zenglApi_Exit(VM_ARG,"%s��������ֻ�������������������ַ�������",modfun_name);
		break;
	}
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,ret,0);
}

/*	bltIntToStrģ�麯����
	������תΪ�ַ�������ʽ������bltIntToStr(25,5,'0')��ô�õ��Ľ�������ַ���'00025'
	��Ϊ�ڶ�������5��ָ�ܿ�ȣ���һ������25�Ŀ��ֻ��2��С��5��
	������߻��Ե���������'0'���롣
	�������ʹ�ò��룬������bltIntToStr(25,0,'0')�����ĸ�ʽ�����ڶ����ܿ�Ȳ�����Ϊ0���Ͳ���Ӧ�ò��빦��
*/
ZL_EXP_VOID zenglApiBMF_bltIntToStr(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_CHAR buf[100],dest[100];
	ZL_EXP_INT num,len,i;
	ZL_EXP_CHAR * modfun_name;
	if(argcount != 3)
	{
		zenglApi_GetModFunName(VM_ARG,&modfun_name);
		zenglApi_Exit(VM_ARG,"%s�����������3������",modfun_name);
	}
	zenglApi_GetFunArg(VM_ARG,1,&arg); //�õ���һ������
	if(arg.type != ZL_EXP_FAT_INT)
	{
		zenglApi_GetModFunName(VM_ARG,&modfun_name);
		zenglApi_Exit(VM_ARG,"%s�����ĵ�һ��������������������ʾҪ����ת��������ֵ",modfun_name);
	}
	num = arg.val.integer;
	sprintf(buf,"%d",num);
	len = strlen(buf);
	zenglApi_GetFunArg(VM_ARG,2,&arg); //�õ��ڶ�������
	if(arg.type != ZL_EXP_FAT_INT)
	{
		zenglApi_GetModFunName(VM_ARG,&modfun_name);
		zenglApi_Exit(VM_ARG,"%s�����ĵڶ���������������������ʾ�ܿ�ȣ��������Ŀ�Ȳ���ʱ������ఴ�������������в���",modfun_name);
	}
	num = arg.val.integer;
	zenglApi_GetFunArg(VM_ARG,3,&arg); //�õ�����������
	if(arg.type != ZL_EXP_FAT_STR)
	{
		zenglApi_GetModFunName(VM_ARG,&modfun_name);
		zenglApi_Exit(VM_ARG,"%s�����ĵ����������������ַ������ͣ���ʾҪ���в����Ԫ��",modfun_name);
	}
	if(len < num)  //����һ���������ַ������Ȳ���ʱ�����õ��������������롣
	{
		for(i=0;i<num-len;i++)
			dest[i] = arg.val.str[0];
		strncpy(dest+num-len,buf,len);
		dest[num] = '\0';
	}
	if(len < num)
		zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_STR,(char *)dest,0,0);
	else 
		zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_STR,(char *)buf,0,0);
}

/*bltRandomģ�麯�������������*/
ZL_EXP_VOID zenglApiBMF_bltRandom(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	time_t t;
	ZL_EXP_INT * random_seed;
	if((random_seed = zenglApi_GetExtraDataEx(VM_ARG,"ZL_SYS_ExtraData_RandomSeed")) == ZL_EXP_NULL)
	{
		random_seed = (ZL_EXP_INT *)zenglApi_AllocMem(VM_ARG,sizeof(ZL_EXP_INT));
		(*random_seed) = 0;
		zenglApi_SetExtraData(VM_ARG,"ZL_SYS_ExtraData_RandomSeed",random_seed); //ʹ�ö������������ģ�麯����ľ�̬������ȫ�ֱ������Խ�����߳�ͬʱд�����⡣
	}
	if((*random_seed) == 0) //��һ��ʹ��ʱ����Ϊ������ӡ�
	{
		srand((unsigned) time(&t));
		(*random_seed) = rand();
	}
	else //����ʱ��ʹ����һ�����ɵ��������Ϊ�������
	{
		srand((unsigned) (*random_seed));
		(*random_seed) = rand();
	}
	zenglApi_SetRetVal(VM_ARG,ZL_EXP_FAT_INT,ZL_EXP_NULL,(*random_seed),0);
}

/*unsetģ�麯���������в��������õı���������Ԫ�ػ����Ա������Ϊδ��ʼ��״̬
  δ��ʼ��״̬�ںܶೡ�Ͽ��Բ���������0һ����Ч������ģ�麯������Ҫ���ǿ������������������͵ı���*/
ZL_EXP_VOID zenglApiBMF_unset(ZL_EXP_VOID * VM_ARG,ZL_EXP_INT argcount)
{
	ZENGL_EXPORT_MOD_FUN_ARG arg = {ZL_EXP_FAT_NONE,{0}};
	ZENGL_EXPORT_MOD_FUN_ARG null_arg = {ZL_EXP_FAT_NONE,{0}};
	ZL_EXP_INT i;
	ZL_EXP_CHAR * modfun_name;
	if(argcount <= 0)
	{
		zenglApi_GetModFunName(VM_ARG,&modfun_name);
		zenglApi_Exit(VM_ARG,"%s�����Ĳ��������������0",modfun_name);
	}
	for(i=1;i <= argcount;i++)
	{
		zenglApi_GetFunArgInfo(VM_ARG,i,&arg);
		if(arg.type != ZL_EXP_FAT_ADDR && 
			arg.type != ZL_EXP_FAT_ADDR_LOC &&
			arg.type != ZL_EXP_FAT_ADDR_MEMBLK
			)
		{
			zenglApi_GetModFunName(VM_ARG,&modfun_name);
			zenglApi_Exit(VM_ARG,"%s������%d������������Ч��������������������",modfun_name,i);
		}
		zenglApi_SetFunArgEx(VM_ARG,i,&null_arg,1); //ѭ�������������õı�������Ϊδ��ʼ��״̬��������������õı�����Ȼ�Ǹ����õĻ����Ϳ��Խ����������͵ı�������Ϊ��ͨ����
	}
}
