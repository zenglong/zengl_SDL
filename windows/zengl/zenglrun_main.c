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
	���ļ�Ϊ��������������������ļ���
*/

#include "zengl_global.h"

/*
	��ӻ��ָ�ʹ��ZL_DOUBLE��ֵ��doubleռ8���ֽڣ����Դ���8�ֽ����ڵ�ֵ����ǿ��ת�����ͼ���
*/
ZL_VOID zenglrun_AddInst(ZL_VOID * VM_ARG,ZL_INT pc,ZL_INT nodenum,ZENGL_RUN_INST_TYPE type, 
						 ZENGL_RUN_INST_OP_DATA_TYPE dest_type , ZL_DOUBLE dest_val ,
						 ZENGL_RUN_INST_OP_DATA_TYPE src_type , ZL_DOUBLE src_val)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT tmpint;
	if(!run->inst_list.isInit)
		run->initInstList(VM_ARG);
	if(run->inst_list.count == run->inst_list.size)
	{
		run->inst_list.size += ZL_R_INST_LIST_SIZE;
		run->inst_list.insts = run->memReAlloc(VM_ARG,run->inst_list.insts,run->inst_list.size * sizeof(ZENGL_RUN_INST_LIST_MEMBER),
												&run->inst_list.mempool_index);
		ZENGL_SYS_MEM_SET((run->inst_list.insts + (run->inst_list.size - ZL_R_INST_LIST_SIZE)),0,
					ZL_R_INST_LIST_SIZE * sizeof(ZENGL_RUN_INST_LIST_MEMBER));
	}
	if(run->inst_list.insts[run->inst_list.count].isvalid == ZL_FALSE)
	{
		run->inst_list.insts[run->inst_list.count].pc = pc;
		run->inst_list.insts[run->inst_list.count].nodenum = nodenum;
		run->inst_list.insts[run->inst_list.count].type = type;
		run->inst_list.insts[run->inst_list.count].dest.type = dest_type;
		switch(dest_type)
		{
		case ZL_R_DT_REG:
			run->inst_list.insts[run->inst_list.count].dest.val.reg = (ZENGL_RUN_REG_TYPE)dest_val;
			break;
		case ZL_R_DT_ARGMEM:
		case ZL_R_DT_LOCMEM:
		case ZL_R_DT_MEM:
			run->inst_list.insts[run->inst_list.count].dest.val.mem = (ZL_INT)dest_val;
			break;
		case ZL_R_DT_NUM:
			run->inst_list.insts[run->inst_list.count].dest.val.num = (ZL_INT)dest_val;
			break;
		case ZL_R_DT_FLOAT:
			run->inst_list.insts[run->inst_list.count].dest.val.floatnum = dest_val;
			break;
		case ZL_R_DT_STR:
			tmpint = (ZL_INT)dest_val;
			run->inst_list.insts[run->inst_list.count].dest.val.str_Index = run->InstDataStringPoolAdd(VM_ARG,(ZL_CHAR * )((ZL_LONG)tmpint)); //64λ������תΪlong����תΪָ��
			break;
		default:
			run->inst_list.insts[run->inst_list.count].dest.val.num = (ZL_INT)dest_val;
			break;
		}
		run->inst_list.insts[run->inst_list.count].src.type = src_type;
		switch(src_type)
		{
		case ZL_R_DT_REG:
			run->inst_list.insts[run->inst_list.count].src.val.reg = (ZENGL_RUN_REG_TYPE)src_val;
			break;
		case ZL_R_DT_ARGMEM:
		case ZL_R_DT_LOCMEM:
		case ZL_R_DT_MEM:
			run->inst_list.insts[run->inst_list.count].src.val.mem = (ZL_INT)src_val;
			break;
		case ZL_R_DT_NUM:
			run->inst_list.insts[run->inst_list.count].src.val.num = (ZL_INT)src_val;
			break;
		case ZL_R_DT_FLOAT:
			run->inst_list.insts[run->inst_list.count].src.val.floatnum = src_val;
			break;
		case ZL_R_DT_STR:
			tmpint = (ZL_INT)src_val;
			run->inst_list.insts[run->inst_list.count].src.val.str_Index = run->InstDataStringPoolAdd(VM_ARG,(ZL_CHAR * )((ZL_LONG)tmpint)); //64λ������תΪlong����תΪָ��
			break;
		default:
			run->inst_list.insts[run->inst_list.count].src.val.num = (ZL_INT)src_val;
			break;
		}
		run->inst_list.insts[run->inst_list.count].isvalid = ZL_TRUE;
		run->inst_list.count++;
	}
}

/*
	��ʼ���������Ļ��ָ�
*/
ZL_VOID zenglrun_initInstList(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(run->inst_list.isInit)
		return;
	run->inst_list.size = ZL_R_INST_LIST_SIZE;
	run->inst_list.count = 0;
	run->inst_list.insts = run->memAlloc(VM_ARG,run->inst_list.size * sizeof(ZENGL_RUN_INST_LIST_MEMBER),
											&run->inst_list.mempool_index);
	if(run->inst_list.insts == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_RUN_INST_LIST_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(run->inst_list.insts,0,run->inst_list.size * sizeof(ZENGL_RUN_INST_LIST_MEMBER));
	run->inst_list.isInit = ZL_TRUE;
}

/**
	������������ڴ��������
	��һ������ZENGL_RUN_VMEM_OP_TYPE opTypeΪ�ڴ��������
	�� ZL_R_VMOPT_GETMEM Ϊ��ĳ�����ڴ��ȡֵ��
*/
ZENGL_RUN_VIRTUAL_MEM_STRUCT zenglrun_VMemListOps(ZL_VOID * VM_ARG,ZENGL_RUN_VMEM_OP_TYPE opType,ZL_INT memloc,
												  ZENGL_RUN_VIRTUAL_MEM_STRUCT argval)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VIRTUAL_MEM_STRUCT retmem = argval;
	ZL_VOID ** tmptr;
	ZL_INT len;
	if(!run->vmem_list.isInit)
		run->initVMemList(VM_ARG);
	if(run->vmem_list.count == run->vmem_list.size) //�����ڴ涯̬����ռ����ˣ��������ڴ档
	{
realloc:
		run->vmem_list.size += ZL_R_VMEM_LIST_SIZE;
		run->vmem_list.mem_array = run->memReAlloc(VM_ARG,run->vmem_list.mem_array,run->vmem_list.size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT),
												&run->vmem_list.mempool_index);
		ZENGL_SYS_MEM_SET(run->vmem_list.mem_array + (run->vmem_list.size - ZL_R_VMEM_LIST_SIZE),0,
			ZL_R_VMEM_LIST_SIZE * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT));
	}
	if(memloc >= run->vmem_list.size) //����ڴ�Ѱַ�����ڴ涯̬����Ĵ�С���������ڴ档
		goto realloc;

	if(opType != ZL_R_VMOPT_ADDMEM_ADDR && opType != ZL_R_VMOPT_SETMEM_ADDR) //�����������ĳ�ڴ�Ϊ���õĻ����ͽ�������Ĵ���
	{
		switch(run->vmem_list.mem_array[memloc].runType)
		{
		case ZL_R_RDT_ADDR: //���������Ԫ����ȫ�ֱ������������VMemListOps
			return run->VMemListOps(VM_ARG,opType,run->vmem_list.mem_array[memloc].val.dword,argval);
			break;
		case ZL_R_RDT_ADDR_LOC: //�ֲ��������������VStackListOps
			return run->VStackListOps(VM_ARG,opType,run->vmem_list.mem_array[memloc].val.dword,argval,ZL_TRUE);
			break;
		case ZL_R_RDT_ADDR_MEMBLK: //�������һ������Ԫ�ص����ã������VMemBlockOps
			return run->VMemBlockOps(VM_ARG,opType,run->vmem_list.mem_array[memloc].val.memblock,run->vmem_list.mem_array[memloc].val.dword,&argval);
			break;
		}
	}

	if(opType != ZL_R_VMOPT_GETMEM)
	{
		//��������ʵ��ڴ�ԭ���Ǹ�����֮����ڴ��Ļ����ͽ����ڴ����ڴ�ռ��ͷŵ�
		if(run->vmem_list.mem_array[memloc].runType == ZL_R_RDT_MEM_BLOCK && run->vmem_list.mem_array[memloc].val.memblock != ZL_NULL)
		{
			run->memblock_free(VM_ARG,run->vmem_list.mem_array[memloc].val.memblock,
				&run->vmem_list.mem_array[memloc].memblk_Index);
			run->vmem_list.mem_array[memloc].val.memblock = ZL_NULL;
		}
		else if(run->vmem_list.mem_array[memloc].runType == ZL_R_RDT_ADDR_MEMBLK) //������ڴ�����ã���ֱ�ӽ�memblock�ڴ��ָ����ΪZL_NULL
			run->vmem_list.mem_array[memloc].val.memblock = ZL_NULL;
	}

	switch(opType) //ִ�в�ͬ���ڴ����
	{
	case ZL_R_VMOPT_SETMEM_INT: //SETMEM�Ĳ������������ã��������
	case ZL_R_VMOPT_ADDMEM_INT: //ADDMEM�Ĳ�����������ӣ�����memlocλ�õ��ڴ�ֵΪ����
		run->vmem_list.mem_array[memloc].runType = ZL_R_RDT_INT;
		run->vmem_list.mem_array[memloc].val.dword = argval.val.dword;
		if(run->vmem_list.mem_array[memloc].isvalid == ZL_FALSE) //���memloc���������ڴ�û�б�ռ�ã���vmem_list���ռ������һ
		{
			run->vmem_list.count++;
			run->vmem_list.mem_array[memloc].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_DOUBLE:
	case ZL_R_VMOPT_ADDMEM_DOUBLE: //����memlocλ�õ��ڴ�ֵΪ������
		run->vmem_list.mem_array[memloc].runType = ZL_R_RDT_FLOAT;
		run->vmem_list.mem_array[memloc].val.qword = argval.val.qword;
		if(run->vmem_list.mem_array[memloc].isvalid == ZL_FALSE)
		{
			run->vmem_list.count++;
			run->vmem_list.mem_array[memloc].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_STR:
	case ZL_R_VMOPT_ADDMEM_STR: //����memlocλ�õ��ڴ�ֵΪ�ַ���
		run->vmem_list.mem_array[memloc].runType = ZL_R_RDT_STR;
		tmptr = &run->vmem_list.mem_array[memloc].val.str;
		len = ZENGL_SYS_STRLEN(argval.val.str);
		(*tmptr) = run->memReUsePtr(VM_ARG,(*tmptr),(len+1) * sizeof(ZL_CHAR),
									&run->vmem_list.mem_array[memloc].str_Index);
		ZENGL_SYS_STRNCPY(*tmptr,argval.val.str,len);
		(*((ZL_CHAR **)tmptr))[len] = ZL_STRNULL;
		if(run->vmem_list.mem_array[memloc].isvalid == ZL_FALSE)
		{
			run->vmem_list.count++;
			run->vmem_list.mem_array[memloc].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_MEMBLOCK:
	case ZL_R_VMOPT_ADDMEM_MEMBLOCK: //����memlocλ�õ��ڴ�Ϊ�ڴ��
		run->vmem_list.mem_array[memloc].runType = ZL_R_RDT_MEM_BLOCK; //������ʱ������ΪZL_R_RDT_MEM_BLOCK������֮����ڴ��
		run->assign_memblock(VM_ARG,&(run->vmem_list.mem_array[memloc].val.memblock),argval.val.memblock); //ͨ��assign_memblockΪmemloc�ڴ���Ϊargval.val.memblock�����ڴ��
		run->vmem_list.mem_array[memloc].memblk_Index = argval.memblk_Index;
		if(run->vmem_list.mem_array[memloc].isvalid == ZL_FALSE)
		{
			run->vmem_list.count++;
			run->vmem_list.mem_array[memloc].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_ADDR:
	case ZL_R_VMOPT_ADDMEM_ADDR: //��memloc�ڴ�����Ϊ��һ���ڴ������
		run->vmem_list.mem_array[memloc].runType = argval.runType;
		run->vmem_list.mem_array[memloc].val.dword = argval.val.dword;
		if(argval.runType == ZL_R_RDT_ADDR_MEMBLK) //������ڴ������ã���memblock�ֶ�����Ϊ���ڴ���ָ��memblock
		{
			run->vmem_list.mem_array[memloc].val.memblock = argval.val.memblock;
			run->vmem_list.mem_array[memloc].memblk_Index = argval.memblk_Index;
		}
		if(run->vmem_list.mem_array[memloc].isvalid == ZL_FALSE)
		{
			run->vmem_list.count++;
			run->vmem_list.mem_array[memloc].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_NONE:
	case ZL_R_VMOPT_ADDMEM_NONE: //��memloc�ڴ�����Ϊδ��ʼ��״̬
		run->vmem_list.mem_array[memloc].runType = ZL_R_RDT_NONE;
		if(run->vmem_list.mem_array[memloc].isvalid == ZL_FALSE)
		{
			run->vmem_list.count++;
			run->vmem_list.mem_array[memloc].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_GETMEM: //��ȡmemlocλ�õ��ڴ�ֵ��
		retmem = run->vmem_list.mem_array[memloc];
		return retmem;
		break;
	}
	return retmem; //�������õ��ڴ�ֵ��
}

/**
	��ʼ��ȫ�������ڴ涯̬����
*/
ZL_VOID zenglrun_initVMemList(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(run->vmem_list.isInit)
		return;
	run->vmem_list.size = ZL_R_VMEM_LIST_SIZE;
	run->vmem_list.count = 0;
	run->vmem_list.mem_array = (ZENGL_RUN_VIRTUAL_MEM_STRUCT *)run->memAlloc(VM_ARG,run->vmem_list.size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT),
											&run->vmem_list.mempool_index); //����size��Ա�Ĵ�С��ZENGL_RUN_VIRTUAL_MEM_STRUCT(ÿ�������ڴ浥Ԫ�Ľṹ��)�Ĵ�С������õ�mem_array�����ڴ�����Ŀռ䡣
	if(run->vmem_list.mem_array == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_RUN_VMEM_LIST_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(run->vmem_list.mem_array,0,run->vmem_list.size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT)); //���ڴ涯̬�����ʼ��Ϊ0
	run->vmem_list.isInit = ZL_TRUE;
}

/**
	�����ջ�ռ�Ĳ���������������ж�ջ�ĸ������ƻ��������Ķ�ջ������ڱ���ͻָ����ݵĵط����������ڴ�žֲ������������ķ��ص�ַ���Ĵ�����ֵ�ȡ�
	����valid�����ж��Ƿ���Ҫ��ĳջ�ڴ���Ϊռ��״̬��ռ�ñ�ʾ��ջ�ڴ��Ѿ�����ʼ������Ҫ�����жϺ��������Ƿ񱻳�ʼ��
*/
ZENGL_RUN_VIRTUAL_MEM_STRUCT zenglrun_VStackListOps(ZL_VOID * VM_ARG,ZENGL_RUN_VMEM_OP_TYPE opType,ZL_INT index,
												  ZENGL_RUN_VIRTUAL_MEM_STRUCT argval,ZL_BOOL valid)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VIRTUAL_MEM_STRUCT retmem=argval;
	ZL_VOID ** tmptr;
	ZL_INT len;
	if(!run->vstack_list.isInit)
		run->initVStackList(VM_ARG);
	if(run->vstack_list.count == run->vstack_list.size) //��̬��������װ���˾����ݡ�
	{
		run->vstack_list.size += ZL_R_VSTACK_LIST_SIZE;
		run->vstack_list.mem_array = run->memReAlloc(VM_ARG,run->vstack_list.mem_array,
			run->vstack_list.size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT),
			&run->vstack_list.mempool_index);
		ZENGL_SYS_MEM_SET(run->vstack_list.mem_array + (run->vstack_list.size - ZL_R_VSTACK_LIST_SIZE),0,
			ZL_R_VSTACK_LIST_SIZE * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT));
	}
	if(index < 0) //С��0����ѹջ����
	{
		switch(opType)
		{
		case ZL_R_VMOPT_ADDMEM_NONE:
			run->vstack_list.mem_array[run->vstack_list.count].runType = ZL_R_RDT_NONE;
			run->vstack_list.mem_array[run->vstack_list.count].isvalid = valid;
			run->vstack_list.count++;
			break;
		case ZL_R_VMOPT_ADDMEM_INT:
			run->vstack_list.mem_array[run->vstack_list.count].runType = ZL_R_RDT_INT;
			run->vstack_list.mem_array[run->vstack_list.count].val.dword = argval.val.dword;
			run->vstack_list.mem_array[run->vstack_list.count].isvalid = valid;
			run->vstack_list.count++;
			break;
		case ZL_R_VMOPT_ADDMEM_DOUBLE:
			run->vstack_list.mem_array[run->vstack_list.count].runType = ZL_R_RDT_FLOAT;
			run->vstack_list.mem_array[run->vstack_list.count].val.qword = argval.val.qword;
			run->vstack_list.mem_array[run->vstack_list.count].isvalid = valid;
			run->vstack_list.count++;
			break;
		case ZL_R_VMOPT_ADDMEM_STR:
			run->vstack_list.mem_array[run->vstack_list.count].runType = ZL_R_RDT_STR;
			tmptr = &run->vstack_list.mem_array[run->vstack_list.count].val.str;
			len = ZENGL_SYS_STRLEN(argval.val.str);
			(*tmptr) = run->memReUsePtr(VM_ARG,(*tmptr),(len+1) * sizeof(ZL_CHAR),
				&run->vstack_list.mem_array[run->vstack_list.count].str_Index);
			ZENGL_SYS_STRNCPY(*tmptr,argval.val.str,len);
			(*((ZL_CHAR **)tmptr))[len] = ZL_STRNULL;
			run->vstack_list.mem_array[run->vstack_list.count].isvalid = valid;
			run->vstack_list.count++;
			break;
		case ZL_R_VMOPT_ADDMEM_ADDR: //����һ�ڴ������ѹ��ջ
			run->vstack_list.mem_array[run->vstack_list.count].runType = argval.runType;
			run->vstack_list.mem_array[run->vstack_list.count].val.dword = argval.val.dword;
			if(argval.runType == ZL_R_RDT_ADDR_MEMBLK)
			{
				run->vstack_list.mem_array[run->vstack_list.count].val.memblock = argval.val.memblock;
				run->vstack_list.mem_array[run->vstack_list.count].memblk_Index = argval.memblk_Index;
			}
			run->vstack_list.count++;
			break;
		case ZL_R_VMOPT_ADDMEM_MEMBLOCK: //ͨ��assign_memblock��ջ�ڴ���Ϊargval.val.memblockָ����ڴ��
			run->vstack_list.mem_array[run->vstack_list.count].runType = ZL_R_RDT_MEM_BLOCK;
			run->assign_memblock(VM_ARG,&(run->vstack_list.mem_array[run->vstack_list.count].val.memblock),
				argval.val.memblock);
			run->vstack_list.mem_array[run->vstack_list.count].memblk_Index = argval.memblk_Index;
			run->vstack_list.count++;
			break;
		case ZL_R_VMOPT_GETMEM: //��ȡ��ջ�����һ��ֵ����������ֵ���е������POP������
			if(run->vstack_list.count <= 0) //���ֵ������ݻ�δ֪ԭ���¶�ջ���������
				run->exit(VM_ARG,ZL_ERR_RUN_VSTACK_LIST_OVERFLOW,ZL_R_REG_PC,ZL_R_REG_PC);
			switch(run->vstack_list.mem_array[run->vstack_list.count - 1].runType)
			{
			case ZL_R_RDT_ADDR: //�����ȫ�ֱ����ڴ�����ã���ͨ��VMemListOps������
				retmem = run->VMemListOps(VM_ARG,opType,run->vstack_list.mem_array[run->vstack_list.count - 1].val.dword,argval);
				break;
			case ZL_R_RDT_ADDR_LOC: ////����Ǿֲ�����ջ�ڴ�����ã���ͨ��VStackListOps������
				retmem = run->VStackListOps(VM_ARG,opType,run->vstack_list.mem_array[run->vstack_list.count - 1].val.dword,argval,ZL_TRUE);
				break;
			default:
				retmem = run->vstack_list.mem_array[run->vstack_list.count - 1]; //����������ã��ͽ�ջ�����һ��Ԫ�ط���
				break;
			}
			run->vstack_list.count--; //ջ����������һ��
			return retmem;
			break;
		}
	}
	else //���index���ڵ���0���ͶԶ�ջ���ĳ��Ԫ�ؽ��в�������Ҫ�ǶԺ����Ĳ����;ֲ��������в���ʱ���õ���
	{
		if(run->is_inMemBlkSetVal == ZL_TRUE) //��Ϊ����������Ԫ�ص�ʱ��ֵ�����ȷ���ջ�еģ��ٴ�ջ����ȡ��������󸳸�����Ԫ�صģ���Ҫ��ֵ����һ������ʱ������test[0] = &test��test��������Ϣ���ȷ���ջ�У����û�д˴��Ĵ������жϵĻ����ͻ�����������ý��еݹ飬�����͵ò���������Ϣ�ˣ���������������ж� >>> �������������Ԫ��ʱ����ֱ�ӷ���Ԫ��
			return run->vstack_list.mem_array[index];

		if(opType != ZL_R_VMOPT_ADDMEM_ADDR && opType != ZL_R_VMOPT_SETMEM_ADDR) //������������ڴ����ã��ͽ�������Ĵ���
		{
			switch(run->vstack_list.mem_array[index].runType)
			{
			case ZL_R_RDT_ADDR: //�����ȫ�ֱ������ã���ֱ�ӽ���������VMemListOps
				return run->VMemListOps(VM_ARG,opType,run->vstack_list.mem_array[index].val.dword,argval);
				break;
			case ZL_R_RDT_ADDR_LOC: //����Ǿֲ���������������ã���ֱ�ӽ���������VStackListOps���������ֱ�ӷ���
				return run->VStackListOps(VM_ARG,opType,run->vstack_list.mem_array[index].val.dword,argval,ZL_TRUE);
				break;
			case ZL_R_RDT_ADDR_MEMBLK: //������ڴ�����ã������memop_array_addr
				return run->VMemBlockOps(VM_ARG,opType,run->vstack_list.mem_array[index].val.memblock,
					run->vstack_list.mem_array[index].val.dword,&argval);
				break;
			}
		}
		
		if(opType != ZL_R_VMOPT_GETMEM)
		{
			if(run->vstack_list.mem_array[index].runType == ZL_R_RDT_MEM_BLOCK && 
				run->vstack_list.mem_array[index].val.memblock != ZL_NULL) //���ջ�и�Ԫ��֮ǰ�Ǹ�����֮����ڴ�飬�ͽ��ڴ������ͷ�
			{
				run->memblock_free(VM_ARG,run->vstack_list.mem_array[index].val.memblock,
					&run->vstack_list.mem_array[index].memblk_Index);
				run->vstack_list.mem_array[index].val.memblock = ZL_NULL;
			}
			else if(run->vstack_list.mem_array[index].runType == ZL_R_RDT_ADDR_MEMBLK) //������ڴ�����ã���ֱ�ӽ�memblock�ڴ��ָ����ΪZL_NULL
				run->vstack_list.mem_array[index].val.memblock = ZL_NULL;
		}

		switch(opType)
		{
		case ZL_R_VMOPT_SETMEM_NONE:
			run->vstack_list.mem_array[index].runType = ZL_R_RDT_NONE;
			break;
		case ZL_R_VMOPT_SETMEM_INT:
			run->vstack_list.mem_array[index].runType = ZL_R_RDT_INT;
			run->vstack_list.mem_array[index].val.dword = argval.val.dword;
			break;
		case ZL_R_VMOPT_SETMEM_DOUBLE:
			run->vstack_list.mem_array[index].runType = ZL_R_RDT_FLOAT;
			run->vstack_list.mem_array[index].val.qword = argval.val.qword;
			break;
		case ZL_R_VMOPT_SETMEM_STR:
			run->vstack_list.mem_array[index].runType = ZL_R_RDT_STR;
			tmptr = &run->vstack_list.mem_array[index].val.str;
			len = ZENGL_SYS_STRLEN(argval.val.str);
			(*tmptr) = run->memReUsePtr(VM_ARG,(*tmptr),(len+1) * sizeof(ZL_CHAR),
				&run->vstack_list.mem_array[index].str_Index);
			ZENGL_SYS_STRNCPY(*tmptr,argval.val.str,len);
			(*((ZL_CHAR **)tmptr))[len] = ZL_STRNULL;
			break;
		case ZL_R_VMOPT_SETMEM_ADDR: //����Ϊ��һ�ڴ������
			run->vstack_list.mem_array[index].runType = argval.runType;
			run->vstack_list.mem_array[index].val.dword = argval.val.dword;
			if(argval.runType == ZL_R_RDT_ADDR_MEMBLK)
			{
				run->vstack_list.mem_array[index].val.memblock = argval.val.memblock;
				run->vstack_list.mem_array[index].memblk_Index = argval.memblk_Index;
			}
			break;
		case ZL_R_VMOPT_SETMEM_MEMBLOCK: //ͨ��assign_memblock��ջ�ڴ�����Ϊargval.val.memblockָ����ڴ��
			run->vstack_list.mem_array[index].runType = ZL_R_RDT_MEM_BLOCK;
			run->assign_memblock(VM_ARG,&(run->vstack_list.mem_array[index].val.memblock),argval.val.memblock);
			run->vstack_list.mem_array[index].memblk_Index = argval.memblk_Index;
			break;
		case ZL_R_VMOPT_GETMEM:
			retmem = run->vstack_list.mem_array[index];
			return retmem;
			break;
		}
	}
	return retmem; //�������õĶ�ջֵ��
}

/**
	��ʼ��������Ķ�ջ�ռ䡣
*/
ZL_VOID zenglrun_initVStackList(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(run->vstack_list.isInit)
		return;
	run->vstack_list.size = ZL_R_VSTACK_LIST_SIZE;
	run->vstack_list.count = 0;
	run->vstack_list.mem_array = (ZENGL_RUN_VIRTUAL_MEM_STRUCT *)run->memAlloc(VM_ARG,run->vstack_list.size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT),
		&run->vstack_list.mempool_index);
	if(run->vstack_list.mem_array == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_RUN_VSTACK_LIST_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(run->vstack_list.mem_array,0,run->vstack_list.size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT)); //�����ջ��̬�����ʼ��Ϊ0
	run->vstack_list.isInit = ZL_TRUE;
}

/*
	��ӡ���ָ�
*/
ZL_VOID zenglrun_printInstList(ZL_VOID * VM_ARG,ZL_CHAR * head_title)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_COMPILE_TYPE * compile = &VM->compile;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZENGL_AST_NODE_TYPE * nodes = compile->AST_nodes.nodes;
	ZL_INT i,j,nodenum;
	ZENGL_RUN_INST_OP_DATA op_data;
	ZL_CLOCK_T start_time = ZENGL_SYS_TIME_CLOCK();
	ZL_CLOCK_T end_time;

	if(head_title != ZL_NULL) //��ӡ���Ա���
		run->info(VM_ARG,head_title);
	for(i=0;i<run->inst_list.count;i++)
	{
		run->info(VM_ARG,"%d %s ",run->inst_list.insts[i].pc , run->instTypeString[run->inst_list.insts[i].type]);
		for(j=1;j<=2;j++)
		{
			if(j==1)
				op_data = run->inst_list.insts[i].dest;
			else
				op_data = run->inst_list.insts[i].src;
			switch(op_data.type)
			{
			case ZL_R_DT_REG:
				run->info(VM_ARG,"%s ",run->registerTypeString[op_data.val.reg]);
				break;
			case ZL_R_DT_MEM:
				run->info(VM_ARG,"(%d) ",op_data.val.mem);
				break;
			case ZL_R_DT_ARGMEM:
				run->info(VM_ARG,"arg(%d) ",op_data.val.mem);
				break;
			case ZL_R_DT_LOCMEM:
				run->info(VM_ARG,"loc(%d) ",op_data.val.mem);
				break;
			case ZL_R_DT_NUM:
				run->info(VM_ARG,"%ld ",op_data.val.num);
				break;
			case ZL_R_DT_FLOAT:
				run->info(VM_ARG,"%.16g ",op_data.val.floatnum);
				break;
			case ZL_R_DT_STR:
				run->info(VM_ARG,"\"%s\" ",run->InstData_StringPool.ptr + op_data.val.str_Index);
				break;
			case ZL_R_DT_LDADDR: //α��ַ
				run->info(VM_ARG,"adr%ld ",op_data.val.num);
				break;
			case ZL_R_DT_LDFUNID: //��������ʱ��ʹ�õĺ���ID�������滻ʱ����תΪα��ַ
				run->info(VM_ARG,"funid%ld ",op_data.val.num);
				break;
			}
		} //for(j=1;j<=2;j++)
		nodenum = run->inst_list.insts[i].nodenum;
		if(nodenum == -1)
			run->info(VM_ARG,"[%d]\n",nodenum);
		else
			run->info(VM_ARG,"[%d line:%d,col:%d,%s]\n",nodenum,nodes[nodenum].line_no,nodes[nodenum].col_no,nodes[nodenum].filename);
	} //for(i=0;i<run->inst_list.count;i++)
	end_time = ZENGL_SYS_TIME_CLOCK();
	compile->total_print_time += end_time - start_time;
}

/**
	�����������ִ�л��ָ�������ʽ��
*/
ZL_VOID zenglrun_RunInsts(ZL_VOID * VM_ARG)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZENGL_DEBUG_TYPE * debug = &VM->debug;
	ZENGL_RUN_RUNTIME_OP_DATA src; //��ʱ���������ڴ��Դ�������ȡ�
	ZENGL_RUN_VIRTUAL_MEM_STRUCT tmpmem;  //��ʱ�������ڴ������
	ZL_CHAR tmpchar[30]; //��ʱ�ַ�������
	ZENGL_API_STATES origState;

	while(ZL_R_CUR_INST.type != ZL_R_IT_END && run->isUserWantStop == ZL_FALSE) //����PC�Ĵ�����ֵ����ȡ��Ӧ�Ļ��ָ������ǰָ���ENDָ�����û�û��Ҫ��ֹͣ�ű����ͼ������С�
	{
start:
		if(ZL_R_CUR_INST.isvalid == ZL_FALSE || 
			ZL_R_CUR_INST.pc != ZL_R_REG_PC)
			run->exit(VM_ARG,ZL_ERR_RUN_INST_INVALID_PC,ZL_R_CUR_INST.pc,ZL_R_REG_PC);

		src.runType = ZL_R_RDT_INT; //�Ƚ�Դ���������ͳ�ʼ��Ϊ���Ρ�
		switch(ZL_R_CUR_INST.type) //���ݵ�ǰ��ָ������ִ�в�ͬ�Ĳ���
		{
		case ZL_R_IT_MOV: //MOVָ�����ԭ��������Ŀ��������������ݴ��ڴ�ת���Ĵ�������ӼĴ���ת���ڴ�ȡ�
			switch(ZL_R_CUR_INST.src.type)
			{
			case ZL_R_DT_REG: //����ǼĴ������ͽ��Ĵ�����������򸡵������ַ�����ֵ��src
				switch(ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType)
				{
				case ZL_R_RDT_INT:
					src.val.dword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword;
					break;
				case ZL_R_RDT_FLOAT:
					src.runType = ZL_R_RDT_FLOAT;
					src.val.qword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).qword;
					break;
				case ZL_R_RDT_STR:
					src.runType = ZL_R_RDT_STR;
					src.val.str = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).str;
					src.str_Index = ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).str_Index;
					break;
				case ZL_R_RDT_ADDR:
				case ZL_R_RDT_ADDR_LOC: //��ԭ������Ϊ����ʱ
					src.runType = ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType;
					src.val.dword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword;
					break;
				case ZL_R_RDT_MEM_BLOCK: //�ڴ��
					src.runType = ZL_R_RDT_MEM_BLOCK;
					src.val.memblock = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).memblock;
					src.memblk_Index = ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).memblk_Index;
					break;
				case ZL_R_RDT_ADDR_MEMBLK: //�ڴ������
					src.runType = ZL_R_RDT_ADDR_MEMBLK;
					src.val.dword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword;
					src.val.memblock = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).memblock;
					src.memblk_Index = ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).memblk_Index;
					break;
				} //switch(ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType)
				break; //case ZL_R_DT_REG: //����ǼĴ���......
			case ZL_R_DT_ARGMEM:
			case ZL_R_DT_LOCMEM:
			case ZL_R_DT_MEM: //�����ȫ�ֱ������ڴ�
				ZENGL_RUN_VMEM_OP_GET(ZL_R_CUR_INST.src.type,tmpmem,src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
				switch(tmpmem.runType)
				{
				case ZL_R_RDT_INT:
					src.val.dword = tmpmem.val.dword;
					break;
				case ZL_R_RDT_FLOAT:
					src.runType = ZL_R_RDT_FLOAT;
					src.val.qword = tmpmem.val.qword;
					break;
				case ZL_R_RDT_STR:
					src.runType = ZL_R_RDT_STR;
					src.val.str = tmpmem.val.str;
					src.str_Index = tmpmem.str_Index;
					break;
				case ZL_R_RDT_NONE: //���ڴ�û�б���ֵ�����
					src.val.dword = 0;
					break;
				case ZL_R_RDT_MEM_BLOCK: //�ڴ��
					src.runType = ZL_R_RDT_MEM_BLOCK;
					src.val.memblock = tmpmem.val.memblock;
					src.memblk_Index = tmpmem.memblk_Index;
					break;
				}
				break; //case ZL_R_DT_MEM: //�����ȫ�ֱ������ڴ�
			case ZL_R_DT_NUM: //MOVָ���Դ������Ϊ����
				src.val.dword = ZL_R_CUR_INST.src.val.num;
				break;
			case ZL_R_DT_FLOAT:
				src.runType = ZL_R_RDT_FLOAT;
				src.val.qword = ZL_R_CUR_INST.src.val.floatnum;
				break;
			case ZL_R_DT_STR: //MOVָ���Դ������Ϊ�ַ���
				src.runType = ZL_R_RDT_STR;
				src.val.str = run->InstData_StringPool.ptr + ZL_R_CUR_INST.src.val.str_Index;
				break;
			default:
				run->exit(VM_ARG,ZL_ERR_RUN_INVALID_INST_SRC_TYPE,ZL_R_REG_PC);
				break;
			} //switch(ZL_R_CUR_INST.src.type)
			switch(ZL_R_CUR_INST.dest.type) //��ǰMOVָ���Ŀ�������
			{
			case ZL_R_DT_REG: //��ǰMOVָ���Ŀ�������Ϊ�Ĵ�������
				switch(src.runType)
				{
				case ZL_R_RDT_INT:
					ZENGL_RUN_REG(ZL_R_CUR_INST.dest.val.reg).runType = ZL_R_RDT_INT;
					ZENGL_RUN_REGVAL(ZL_R_CUR_INST.dest.val.reg).dword = src.val.dword;
					break;
				case ZL_R_RDT_FLOAT:
					ZENGL_RUN_REG(ZL_R_CUR_INST.dest.val.reg).runType = ZL_R_RDT_FLOAT;
					ZENGL_RUN_REGVAL(ZL_R_CUR_INST.dest.val.reg).qword = src.val.qword;
					break;
				case ZL_R_RDT_STR:
					ZENGL_RUN_REG(ZL_R_CUR_INST.dest.val.reg).runType = ZL_R_RDT_STR;
					run->RegAssignStr(VM_ARG,ZL_R_CUR_INST.dest.val.reg,src.val.str);
					break;
				case ZL_R_RDT_ADDR:
				case ZL_R_RDT_ADDR_LOC:
				case ZL_R_RDT_ADDR_MEMBLK: //���Դ�����������ã��ͽ����õ��ڴ����ͺ��ڴ�������ֵ��Ŀ��Ĵ���
					ZENGL_RUN_REG(ZL_R_CUR_INST.dest.val.reg).runType = src.runType;
					ZENGL_RUN_REGVAL(ZL_R_CUR_INST.dest.val.reg).dword = src.val.dword;
					if(src.runType == ZL_R_RDT_ADDR_MEMBLK)
					{
						ZENGL_RUN_REGVAL(ZL_R_CUR_INST.dest.val.reg).memblock = src.val.memblock;
						ZENGL_RUN_REG(ZL_R_CUR_INST.dest.val.reg).memblk_Index = src.memblk_Index;
					}
					break;
				case ZL_R_RDT_MEM_BLOCK: //�ڴ��
					ZENGL_RUN_REG(ZL_R_CUR_INST.dest.val.reg).runType = ZL_R_RDT_MEM_BLOCK;
					ZENGL_RUN_REGVAL(ZL_R_CUR_INST.dest.val.reg).memblock = src.val.memblock;  //�ڴ��ָ��
					ZENGL_RUN_REG(ZL_R_CUR_INST.dest.val.reg).memblk_Index = src.memblk_Index; //�ڴ��ָ�����ڴ���е�����
					break;
				}
				break; //case ZL_R_DT_REG:
			case ZL_R_DT_ARGMEM:
			case ZL_R_DT_LOCMEM:
			case ZL_R_DT_MEM: //��ǰMOVָ���Ŀ�������Ϊ�ڴ�Ѱַ����
				switch(src.runType)
				{
				case ZL_R_RDT_INT:
					tmpmem.val.dword = src.val.dword;
					ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.dest.type,ZL_R_VMOPT_ADDMEM_INT,ZL_R_VMOPT_SETMEM_INT,tmpmem,dest,ZL_ERR_RUN_INVALID_VMEM_TYPE)
					break;
				case ZL_R_RDT_FLOAT:
					tmpmem.val.qword = src.val.qword;
					ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.dest.type,ZL_R_VMOPT_ADDMEM_DOUBLE,ZL_R_VMOPT_SETMEM_DOUBLE,tmpmem,dest,ZL_ERR_RUN_INVALID_VMEM_TYPE)
					break;
				case ZL_R_RDT_STR:
					tmpmem.val.str = src.val.str;
					ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.dest.type,ZL_R_VMOPT_ADDMEM_STR,ZL_R_VMOPT_SETMEM_STR,tmpmem,dest,ZL_ERR_RUN_INVALID_VMEM_TYPE)
					break;
				case ZL_R_RDT_ADDR:
				case ZL_R_RDT_ADDR_LOC:
				case ZL_R_RDT_ADDR_MEMBLK: //���Դ�����������ã�Ŀ����������ڴ棬��ͨ��ZENGL_RUN_VMEM_OP����в�������Ŀ���ڴ���Ϊ����
					tmpmem.runType = src.runType;
					tmpmem.val.dword = src.val.dword;
					if(src.runType == ZL_R_RDT_ADDR_MEMBLK)
					{
						tmpmem.val.memblock = src.val.memblock;
						tmpmem.memblk_Index = src.memblk_Index;
					}
					ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.dest.type,ZL_R_VMOPT_ADDMEM_ADDR,ZL_R_VMOPT_SETMEM_ADDR,tmpmem,dest,ZL_ERR_RUN_INVALID_VMEM_TYPE)
					break;
				case ZL_R_RDT_MEM_BLOCK:
					tmpmem.val.memblock = src.val.memblock;
					tmpmem.memblk_Index = src.memblk_Index;
					ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.dest.type,ZL_R_VMOPT_ADDMEM_MEMBLOCK,ZL_R_VMOPT_SETMEM_MEMBLOCK,tmpmem,dest,ZL_ERR_RUN_INVALID_VMEM_TYPE)
					break;
				} //switch(src.runType)
				break;
			default:
				run->exit(VM_ARG,ZL_ERR_RUN_INVALID_INST_DEST_TYPE,ZL_R_REG_PC);
				break;
			} //switch(ZL_R_CUR_INST.dest.type) //��ǰMOVָ���Ŀ�������
			break; //case ZL_R_IT_MOV //MOVָ��....
		case ZL_R_IT_PRINT: //PRINTָ��
			switch(ZL_R_CUR_INST.src.type) //�õ�PRINT��Դ������
			{
			case ZL_R_DT_REG: //print ָ��Դ����������Ϊ�Ĵ���
				switch(ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType)
				{
				case ZL_R_RDT_INT:
					src.val.dword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword;
					break;
				case ZL_R_RDT_FLOAT:
					src.runType = ZL_R_RDT_FLOAT;
					src.val.qword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).qword;
					break;
				case ZL_R_RDT_STR:
					src.runType = ZL_R_RDT_STR;
					src.val.str = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).str;
					break;
				case ZL_R_RDT_MEM_BLOCK:
				case ZL_R_RDT_ADDR:
				case ZL_R_RDT_ADDR_LOC:
				case ZL_R_RDT_ADDR_MEMBLK:
					src.runType = ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType;
					break;
				}
				break;
			case ZL_R_DT_ARGMEM:
			case ZL_R_DT_LOCMEM:
			case ZL_R_DT_MEM: //print ָ��Դ����������Ϊ�ڴ�Ѱַ
				ZENGL_RUN_VMEM_OP_GET(ZL_R_CUR_INST.src.type,tmpmem,src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
				switch(tmpmem.runType)
				{
				case ZL_R_RDT_INT:
					src.val.dword = tmpmem.val.dword;
					break;
				case ZL_R_RDT_FLOAT:
					src.runType = ZL_R_RDT_FLOAT;
					src.val.qword = tmpmem.val.qword;
					break;
				case ZL_R_RDT_STR:
					src.runType = ZL_R_RDT_STR;
					src.val.str = tmpmem.val.str;
					break;
				case ZL_R_RDT_NONE: //û�б���ֵ��ʼ��ʱ������0�Դ�
					src.val.dword = 0;
					break;
				case ZL_R_RDT_MEM_BLOCK:
					src.runType = ZL_R_RDT_MEM_BLOCK;
					break;
				}
				break;
			case ZL_R_DT_NUM: //print ָ��Դ����������Ϊ����
				src.val.dword = ZL_R_CUR_INST.src.val.num;
				break;
			case ZL_R_DT_FLOAT: //print ָ��Դ����������Ϊ������
				src.runType = ZL_R_RDT_FLOAT;
				src.val.qword = ZL_R_CUR_INST.src.val.floatnum;
				break;
			case ZL_R_DT_STR: //print ָ��Դ����������Ϊ�ַ���
				src.runType = ZL_R_RDT_STR;
				src.val.str = run->InstData_StringPool.ptr + ZL_R_CUR_INST.src.val.str_Index;
				break;
			default:
				run->exit(VM_ARG,ZL_ERR_RUN_INVALID_INST_SRC_TYPE,ZL_R_REG_PC);
				break;
			} //switch(ZL_R_CUR_INST.src.type) //�õ�PRINT��Դ������
			switch(src.runType)
			{
			case ZL_R_RDT_INT: //��ӡ����
				run->print(VM_ARG,"%ld",src.val.dword);
				break;
			case ZL_R_RDT_FLOAT: //��ӡ������
				run->print(VM_ARG,"%.16g",src.val.qword);
				break;
			case ZL_R_RDT_STR: //��ӡ�ַ���
				run->print(VM_ARG,"%s",(char *)src.val.str);
				break;
			case ZL_R_RDT_MEM_BLOCK:
				run->print(VM_ARG,"[array or class obj type]");
				break;
			case ZL_R_RDT_ADDR:
			case ZL_R_RDT_ADDR_LOC:
			case ZL_R_RDT_ADDR_MEMBLK:
				run->print(VM_ARG,"[addr type]");
				break;
			}
			break; //case ZL_R_IT_PRINT: //PRINTָ��
		case ZL_R_IT_PLUS: //PLUS�ӷ�ָ��
			if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT && 
			   ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_INT) //�ӷ�ָ��AX BX��������ʱ�����
				ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword += ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
			else if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_STR)
			{
				switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
				{
				case ZL_R_RDT_INT: //�ӷ�ָ�AX Ϊ�ַ�����BXΪ������������תΪ�ַ���������ӵ�AX�ַ�����ĩβ
					ZENGL_SYS_SPRINTF(tmpchar,"%ld",ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword);
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).str = (run->strcat)(VM_ARG,ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,&ZENGL_RUN_REG(ZL_R_RT_AX).str_Index,tmpchar);
					break;
				case ZL_R_RDT_FLOAT: //�ӷ�ָ�AX Ϊ�ַ�����BXΪ����������������תΪ�ַ���������ӵ�AX�ַ�����ĩβ
					ZENGL_SYS_SPRINTF(tmpchar,"%.16g",ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword);
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).str = (run->strcat)(VM_ARG,ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,&ZENGL_RUN_REG(ZL_R_RT_AX).str_Index,tmpchar);
					break;
				case ZL_R_RDT_STR: //�ӷ�ָ�AX Ϊ�ַ�����BXҲΪ�ַ�������BX�ַ�����ӵ�AX�ַ�����ĩβ
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).str = (run->strcat)(VM_ARG,ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,
																&ZENGL_RUN_REG(ZL_R_RT_AX).str_Index,ZENGL_RUN_REGVAL(ZL_R_RT_BX).str);
					break;
				}
			}
			else if(ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_STR)
			{
				switch(ZENGL_RUN_REG(ZL_R_RT_AX).runType)
				{
				case ZL_R_RDT_INT: //�ӷ�ָ�BXΪ�ַ�����AXΪ��������AXתΪ�ַ������ٺ�BX�ַ��������ӡ�
					ZENGL_SYS_SPRINTF(tmpchar,"%ld",ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword);
					run->memFreeIndex(VM_ARG,ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,&ZENGL_RUN_REG(ZL_R_RT_AX).str_Index); //�ͷ�AX��ԭ�����ַ���ָ��
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).str = run->strcat2(VM_ARG,ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,
															&ZENGL_RUN_REG(ZL_R_RT_AX).str_Index,tmpchar,ZENGL_RUN_REGVAL(ZL_R_RT_BX).str);
					break;
				case ZL_R_RDT_FLOAT: //�ӷ�ָ�BXΪ�ַ�����AXΪ����������AXתΪ�ַ������ٺ�BX�ַ��������ӡ�
					ZENGL_SYS_SPRINTF(tmpchar,"%.16g",ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword);
					run->memFreeIndex(VM_ARG,ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,&ZENGL_RUN_REG(ZL_R_RT_AX).str_Index); //�ͷ�AX��ԭ�����ַ���ָ��
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).str = run->strcat2(VM_ARG,ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,
															&ZENGL_RUN_REG(ZL_R_RT_AX).str_Index,tmpchar,ZENGL_RUN_REGVAL(ZL_R_RT_BX).str);
					break;
				}
				ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_STR;
			}
			else if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_FLOAT)
			{
				switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
				{
				case ZL_R_RDT_FLOAT: //�ӷ�ָ�AXΪ��������BXҲΪ��������AX BX��������ӣ���������AX��qword��Ա�
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword += ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
					break;
				case ZL_R_RDT_INT: //�ӷ�ָ�AXΪ��������BXΪ������AX BX��ӣ���������AX��qword��Ա�
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword += ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
					break;
				}
			}
			else if(ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_FLOAT &&
					ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT) //�ӷ�ָ�AXΪ������BXΪ��������AX BX��ӣ���������AX��qword��Ա���AX������Ϊ��������
			{
				ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword + ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
				ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
			}
			break; //case ZL_R_IT_PLUS: //PLUS�ӷ�ָ��
		case ZL_R_IT_MINIS: //MINIS����ָ��
			run->op_minis(VM_ARG); //��ΪMINIS����ָ��ʱ������op_minis����������
			break;
		case ZL_R_IT_MOD: //ȡ��ָ�
			switch(ZENGL_RUN_REG(ZL_R_RT_AX).runType)
			{
			case ZL_R_RDT_INT:
				switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
				{
				case ZL_R_RDT_INT:
					if(ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword == 0)
						run->exit(VM_ARG,ZL_ERR_RUN_MOD_BY_ZERO,ZL_R_REG_PC);
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword %= ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
					break;
				case ZL_R_RDT_FLOAT:
					if(ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword == 0)
						run->exit(VM_ARG,ZL_ERR_RUN_MOD_BY_ZERO,ZL_R_REG_PC);
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword %= (ZL_INT)ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
					break;
				}
				break;
			case ZL_R_RDT_FLOAT:
				switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
				{
				case ZL_R_RDT_FLOAT:
					if(ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword == 0)
						run->exit(VM_ARG,ZL_ERR_RUN_MOD_BY_ZERO,ZL_R_REG_PC);
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = (ZL_INT)ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword % (ZL_INT)ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
					break;
				case ZL_R_RDT_INT:
					if(ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword == 0)
						run->exit(VM_ARG,ZL_ERR_RUN_MOD_BY_ZERO,ZL_R_REG_PC);
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = (ZL_INT)ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword % ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
					break;
				}
				ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
				break;
			} //switch(ZENGL_RUN_REG(ZL_R_RT_AX).runType)
			break; //case ZL_R_IT_MOD: //ȡ��ָ�
		case ZL_R_IT_TIMES:
			if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT && 
			   ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_INT) //�˷�ָ�AX,BXΪ������������ˣ���������AX��dword��Ա�
				ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword *= ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
			else if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_FLOAT)
			{
				switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
				{
				case ZL_R_RDT_FLOAT: //�˷�ָ�AX������,BX��������������ˣ���������AX��qword��Ա�
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword *= ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
					break;
				case ZL_R_RDT_INT: //�˷�ָ�AX������,BX������������ˣ���������AX��qword��Ա�
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword *= ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
					break;
				}
			}
			else if(ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_FLOAT && 
				    ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT) //�˷�ָ�AX����,BX��������������ˣ���������AX��qword��Ա�AX��Ϊ�����͡�
			{
				ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword * ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
				ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
			}
			break;
		case ZL_R_IT_DIVIDE:
			if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT && 
			   ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_INT) //����ָ�AX��BX������ת��double�������������������Ը���������ʽ�����AX��qword�����1/2�������0.5����C������ĳ������������ǵ�ϰ�ߡ�
			{
				if(ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword == 0)
					run->exit(VM_ARG,ZL_ERR_RUN_DIVIDE_BY_ZERO,ZL_R_REG_PC);
				ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = (ZL_DOUBLE)ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword / (ZL_DOUBLE)ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword; //��dword�������תΪdouble���ͣ��Ը��㷽ʽ������������ڸ����Աqword�С�
				ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT; //��AX������Ϊ����������
			}
			else if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_FLOAT)
			{
				switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
				{
				case ZL_R_RDT_FLOAT: //����ָ�AX��������BX���������������������AX��qword�
					if(ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword == 0)
						run->exit(VM_ARG,ZL_ERR_RUN_DIVIDE_BY_ZERO,ZL_R_REG_PC);
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword /= ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
					break;
				case ZL_R_RDT_INT: //����ָ�AX��������BX�������������������AX��qword�
					if(ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword == 0)
						run->exit(VM_ARG,ZL_ERR_RUN_DIVIDE_BY_ZERO,ZL_R_REG_PC);
					ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword /= ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
					break;
				}
			}
			else if(ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_FLOAT && 
				    ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT) //����ָ�AX������BX���������������������AX��qword�AX��Ϊ�����͡�
			{
				if(ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword == 0)
					run->exit(VM_ARG,ZL_ERR_RUN_DIVIDE_BY_ZERO,ZL_R_REG_PC);
				ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword / ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
				ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
			}
			break; //case ZL_R_IT_DIVIDE:
		case ZL_R_IT_PUSH: //��ջ��ѹջ�����������ݴ��������ջ�ռ䡣
			if(ZL_R_CUR_INST.src.type == ZL_R_DT_REG) //ѹջԴ������Ϊ�Ĵ�������
			{
				switch(ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType)
				{
				case ZL_R_RDT_NONE: //�Ĵ���δ��ʼ��������0�Դ�
					tmpmem.val.dword = 0;
					run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_INT,-1,tmpmem,ZL_TRUE);
					break;
				case ZL_R_RDT_INT:
					tmpmem.val.dword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword;
					run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_INT,-1,tmpmem,ZL_TRUE); //���������ջ�������������������ջ�ռ�����ݡ�
					break;
				case ZL_R_RDT_FLOAT:
					tmpmem.val.qword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).qword;
					run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_DOUBLE,-1,tmpmem,ZL_TRUE);
					break;
				case ZL_R_RDT_STR:
					tmpmem.val.str = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).str;
					run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_STR,-1,tmpmem,ZL_TRUE);
					break;
				case ZL_R_RDT_ADDR:
				case ZL_R_RDT_ADDR_LOC:
				case ZL_R_RDT_ADDR_MEMBLK: //������ѹ��ջ
					tmpmem.runType = ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType;
					tmpmem.val.dword = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword;
					if(tmpmem.runType == ZL_R_RDT_ADDR_MEMBLK)
					{
						tmpmem.val.memblock = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).memblock;
						tmpmem.memblk_Index = ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).memblk_Index;
					}
					run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_ADDR,-1,tmpmem,ZL_TRUE);
					break;
				case ZL_R_RDT_MEM_BLOCK:
					tmpmem.val.memblock = ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).memblock;
					tmpmem.memblk_Index = ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).memblk_Index;
					run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_MEMBLOCK,-1,tmpmem,ZL_TRUE);
					break;
				} //switch(ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType)
			}
			else if(ZL_R_CUR_INST.src.type == ZL_R_DT_NUM) //���PUSH�Ĳ����������֣��ͽ�����ѹ��ջ�С�
			{
				tmpmem.val.dword = ZL_R_CUR_INST.src.val.num;
				run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_INT,-1,tmpmem,ZL_TRUE);
			}
			break; //case ZL_R_IT_PUSH: //��ջ��ѹջ�����������ݴ��������ջ�ռ䡣
		case ZL_R_IT_POP: //��ջ�ĵ���ջ�����������ݴ������ջ�е���ȥ��
			if(ZL_R_CUR_INST.src.type == ZL_R_DT_REG)
			{
				tmpmem = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,tmpmem,ZL_TRUE); //��ȡ��ջ���һ�����ݡ�
				switch(tmpmem.runType)
				{
				case ZL_R_RDT_INT: //��ջ����Ϊ����ʱ��
					ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword = tmpmem.val.dword; //����ջ���������õ�Ŀ��Ĵ�����,popֻ��һ��������������������src��ʾĿ�ꡣ
					ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType = ZL_R_RDT_INT;
					break;
				case ZL_R_RDT_FLOAT: //��ջ����Ϊ������ʱ��
					ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).qword = tmpmem.val.qword;
					ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType = ZL_R_RDT_FLOAT;
					break;
				case ZL_R_RDT_STR: //��ջ����Ϊ�ַ���ʱ��
					run->RegAssignStr(VM_ARG,ZL_R_CUR_INST.src.val.reg,tmpmem.val.str);
					ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType = ZL_R_RDT_STR;
					break;
				case ZL_R_RDT_ADDR:
				case ZL_R_RDT_ADDR_LOC:
				case ZL_R_RDT_ADDR_MEMBLK: //�����õ���ջ
					ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType = tmpmem.runType;
					ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword = tmpmem.val.dword;
					if(tmpmem.runType == ZL_R_RDT_ADDR_MEMBLK)
					{
						ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).memblock = tmpmem.val.memblock;
						ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).memblk_Index = tmpmem.memblk_Index;
					}
					break;
				case ZL_R_RDT_MEM_BLOCK: //���ڴ�鵯��ջ
					ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType = tmpmem.runType;
					ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).memblock = tmpmem.val.memblock;
					ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).memblk_Index = tmpmem.memblk_Index;
					break;
				} //switch(tmpmem.runType)
			}
			break; //case ZL_R_IT_POP: //��ջ�ĵ���ջ�����������ݴ������ջ�е���ȥ��
		case ZL_R_IT_JE:
			if(ZL_R_CUR_INST.src.type == ZL_R_DT_NUM)
			{
				src.val.dword = ZL_R_CUR_INST.src.val.num;
				run->op_je(VM_ARG,&src);
			}
			break;
		case ZL_R_IT_JNE:
			if(ZL_R_CUR_INST.src.type == ZL_R_DT_NUM)
			{
				src.val.dword = ZL_R_CUR_INST.src.val.num;
				run->op_jne(VM_ARG,&src);
			}
			break;
		case ZL_R_IT_JMP: //JMP��������תָ�ֱ���޸�PC�Ĵ���ֵ��ʹ�ű�������ת��
			if(ZL_R_CUR_INST.src.type == ZL_R_DT_NUM)
			{
				ZL_R_REG_PC = ZL_R_CUR_INST.src.val.num;
				ZL_R_REG_PC--;
			}
			break;
		case ZL_R_IT_GREAT:
		case ZL_R_IT_LESS:
		case ZL_R_IT_EQUAL:
		case ZL_R_IT_NOT_EQ:
		case ZL_R_IT_GREAT_EQ:
		case ZL_R_IT_LESS_EQ:
			run->op_relation(VM_ARG); //��Ϊ���ڵ���֮��ıȽ������ָ��ʱ������op_relation����������
			break;
		case ZL_R_IT_AND:
		case ZL_R_IT_OR:
		case ZL_R_IT_REVERSE:
			run->op_logic(VM_ARG); //��Ϊ�һ�ǵ�ָ��ʱ������op_logic����������
			break;
		case ZL_R_IT_ADDGET:
		case ZL_R_IT_MINIS_GET:
			switch(ZL_R_CUR_INST.src.type)
			{
			case ZL_R_DT_MEM:
			case ZL_R_DT_ARGMEM:
			case ZL_R_DT_LOCMEM:
				run->op_addminisget(VM_ARG,&tmpmem,ZL_R_CUR_INST.type,ZL_R_CUR_INST.src.type);
				break;
			}
			break;
		case ZL_R_IT_GETADD:
		case ZL_R_IT_GET_MINIS:
			switch(ZL_R_CUR_INST.src.type)
			{
			case ZL_R_DT_MEM:
			case ZL_R_DT_ARGMEM:
			case ZL_R_DT_LOCMEM:
				run->op_getaddminis(VM_ARG,&tmpmem,ZL_R_CUR_INST.type,ZL_R_CUR_INST.src.type);
				break;
			}
			break;
		case ZL_R_IT_ADDONE:
		case ZL_R_IT_MINIS_ONE:
			run->op_addminisone(VM_ARG,ZL_R_CUR_INST.type);
			break;
		case ZL_R_IT_FUNARG: //FUNARGָ�Ϊ�����Ĳ��������㹻��ջ�ռ䡣��ʱ����������3�����������ڵ���ʱֻ������1��������ôͨ��FUNARGָ�ϵͳ������ջ�з��������ڴ棬�Ӷ�ʹ�ú�������������������Ȼ�ڶ��͵�����������ֵ��Ϊû�ṩ������Ĭ����NONEδ��ʼ��״̬��
			if(ZL_R_CUR_INST.src.type == ZL_R_DT_NUM)
			{
				ZL_INT argnum,i,count,retadr;
				argnum = ZENGL_RUN_REGVAL(ZL_R_RT_LOC).dword - ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword - 1;
				count = ZL_R_CUR_INST.src.val.num - argnum;
				if(count > 0)
				{
					tmpmem = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,tmpmem,ZL_TRUE);
					if(tmpmem.runType == ZL_R_RDT_INT)
						retadr = tmpmem.val.dword;
					else
						run->exit(VM_ARG,ZL_ERR_RUN_MAIN_INVALID_FUN_RET_ADDRESS,ZL_R_REG_PC);
					for(i=0;i<count;i++)
					{
						run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_NONE,-1,tmpmem,ZL_TRUE);
					}
					tmpmem.val.dword = retadr;
					run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_INT,-1,tmpmem,ZL_TRUE);
					ZENGL_RUN_REGVAL(ZL_R_RT_LOC).dword = run->vstack_list.count;
				}
			}
			break; //case ZL_R_IT_FUNARG: //FUNARGָ��
		case ZL_R_IT_ARG_SET: //ARG_SETָ����ĳ����û�б���ֵ��������һ��Ĭ��ֵ������test(a=3)���aû�и�ֵ����Ĭ��ֵΪ3
			if(ZL_R_CUR_INST.dest.type == ZL_R_DT_ARGMEM)
			{
				ZL_INT tmpStackIndex;
				tmpStackIndex = ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword + ZL_R_CUR_INST.dest.val.mem;
				tmpmem = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,tmpStackIndex,tmpmem,ZL_TRUE);
				if(tmpmem.runType == ZL_R_RDT_NONE)
				{
					switch(ZL_R_CUR_INST.src.type)
					{
					case ZL_R_DT_NUM:
						tmpmem.val.dword = ZL_R_CUR_INST.src.val.num;
						run->VStackListOps(VM_ARG,ZL_R_VMOPT_SETMEM_INT,tmpStackIndex,tmpmem,ZL_TRUE);
						break;
					case ZL_R_DT_FLOAT:
						tmpmem.val.qword = ZL_R_CUR_INST.src.val.floatnum;
						run->VStackListOps(VM_ARG,ZL_R_VMOPT_SETMEM_DOUBLE,tmpStackIndex,tmpmem,ZL_TRUE);
						break;
					case ZL_R_DT_STR:
						tmpmem.val.str = (ZL_VOID *)run->InstDataStringPoolGetPtr(VM_ARG,ZL_R_CUR_INST.src.val.str_Index);
						run->VStackListOps(VM_ARG,ZL_R_VMOPT_SETMEM_STR,tmpStackIndex,tmpmem,ZL_TRUE);
						break;
					}
				}
			}
			break; //case ZL_R_IT_ARG_SET: //ARG_SETָ��
		case ZL_R_IT_PUSH_LOC: //PUSH_LOCָ�Ϊ�ű������ֲ���������ջ�ռ�
			run->VStackListOps(VM_ARG,ZL_R_VMOPT_ADDMEM_NONE,-1,tmpmem,ZL_TRUE);
			break;
		case ZL_R_IT_RET: //RETָ��ӽű���������
run_ret:
			run->memblock_freeall_local(VM_ARG);
			run->vstack_list.count = ZENGL_RUN_REGVAL(ZL_R_RT_LOC).dword; //��ջ����ΪLOC�Ĵ�����ֵ������ջ����ָ��ű������ķ��ص�ַ
			tmpmem = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,tmpmem,ZL_TRUE);
			if(tmpmem.runType == ZL_R_RDT_INT)
			{
				ZL_R_REG_PC = tmpmem.val.dword;
				ZL_R_REG_PC--;
			}
			else
				run->exit(VM_ARG,ZL_ERR_RUN_MAIN_FUN_VSTACK_STRUCT_EXCEPTION,ZL_R_REG_PC);
			run->vstack_list.count = ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword;
			tmpmem = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,tmpmem,ZL_TRUE); //�ٵ���ARGTMP
			if(tmpmem.runType == ZL_R_RDT_INT)
				ZENGL_RUN_REGVAL(ZL_R_RT_ARGTMP).dword = tmpmem.val.dword;
			else
				run->exit(VM_ARG,ZL_ERR_RUN_MAIN_FUN_VSTACK_STRUCT_EXCEPTION,ZL_R_REG_PC);
			tmpmem = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,tmpmem,ZL_TRUE); //������LOC�Ĵ���
			if(tmpmem.runType == ZL_R_RDT_INT)
				ZENGL_RUN_REGVAL(ZL_R_RT_LOC).dword = tmpmem.val.dword;
			else
				run->exit(VM_ARG,ZL_ERR_RUN_MAIN_FUN_VSTACK_STRUCT_EXCEPTION,ZL_R_REG_PC);
			tmpmem = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,tmpmem,ZL_TRUE); //�����ARG�Ĵ���������˳��պú�ѹջ˳���෴��
			if(tmpmem.runType == ZL_R_RDT_INT)
				ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword = tmpmem.val.dword;
			else
				run->exit(VM_ARG,ZL_ERR_RUN_MAIN_FUN_VSTACK_STRUCT_EXCEPTION,ZL_R_REG_PC);
			break; //case ZL_R_IT_RET: //RETָ��ӽű���������
		case ZL_R_IT_RESET: //RESETָ�� ����ǰ����ջ���ļ�����ֵ��ֵ��Ŀ������Ĵ�����
			if(ZL_R_CUR_INST.src.type == ZL_R_DT_REG)
			{
				ZENGL_RUN_REG(ZL_R_CUR_INST.src.val.reg).runType = ZL_R_RDT_INT;
				ZENGL_RUN_REGVAL(ZL_R_CUR_INST.src.val.reg).dword = run->vstack_list.count;
			}
			break;
		case ZL_R_IT_USE: //USEָ�� �����û��Զ����ģ���ʼ������
			if(ZL_R_CUR_INST.src.type == ZL_R_DT_STR)
			{
				ZL_CHAR * moduleName = run->InstDataStringPoolGetPtr(VM_ARG,ZL_R_CUR_INST.src.val.str_Index);
				ZL_INT moduleIndex = run->LookupModuleTable(VM_ARG,moduleName);
				if(run->moduleTable.modules[moduleIndex].init_func != ZL_NULL)
				{
					origState = VM->ApiState;
					VM->ApiState = ZL_API_ST_MOD_INIT_HANDLE;
					run->moduleTable.modules[moduleIndex].init_func(VM_ARG,run->moduleTable.modules[moduleIndex].ID);
					VM->ApiState = origState;
				}
			}
			break; //case ZL_R_IT_USE: //USEָ�� �����û��Զ����ģ���ʼ������
		case ZL_R_IT_CALL: //CALLָ�� �����û��Զ����ģ�麯��
			switch(ZL_R_CUR_INST.src.type)
			{
			case ZL_R_DT_STR:
				{
					ZL_CHAR * functionName = run->InstDataStringPoolGetPtr(VM_ARG,ZL_R_CUR_INST.src.val.str_Index);
					ZL_INT ModFunIndex = run->LookupModFunTable(VM_ARG,functionName);
					ZL_R_CUR_INST.src.type = ZL_R_DT_NUM;
					ZL_R_CUR_INST.src.val.num = ModFunIndex;
					if(run->ModFunTable.mod_funs[ModFunIndex].handle != ZL_NULL)
					{
						ZL_INT argcount = ZENGL_RUN_REGVAL(ZL_R_RT_LOC).dword - ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword - 1;
						run->CurRunModFunIndex = ModFunIndex; //���õ�ǰ���е�ģ�麯����ģ�麯����̬�����е�����ֵ
						origState = VM->ApiState;
						VM->ApiState = ZL_API_ST_MOD_FUN_HANDLE; //����API����״̬����ģ�麯���У������������󲿷�API�ӿڶ���ֱ�ӷ���-1����������ִ�оͻ�������
						run->ModFunTable.mod_funs[ModFunIndex].handle(VM_ARG,argcount);
						VM->ApiState = origState;
					}
					else
						run->exit(VM_ARG,ZL_ERR_RUN_FUNCTION_IS_INVALID,functionName,ZL_R_REG_PC,functionName);
				}
				goto run_ret;
				break;
			case ZL_R_DT_NUM:
				{
					ZL_INT ModFunIndex = ZL_R_CUR_INST.src.val.num;
					if(run->ModFunTable.mod_funs[ModFunIndex].handle != ZL_NULL)
					{
						ZL_INT argcount = ZENGL_RUN_REGVAL(ZL_R_RT_LOC).dword - ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword - 1;
						run->CurRunModFunIndex = ModFunIndex; //���õ�ǰ���е�ģ�麯����ģ�麯����̬�����е�����ֵ
						origState = VM->ApiState;
						VM->ApiState = ZL_API_ST_MOD_FUN_HANDLE; //����API����״̬����ģ�麯���У������������󲿷�API�ӿڶ���ֱ�ӷ���-1����������ִ�оͻ�������
						run->ModFunTable.mod_funs[ModFunIndex].handle(VM_ARG,argcount);
						VM->ApiState = origState;
					}
				}
				goto run_ret;
				break;
			}
			break; //case ZL_R_IT_CALL: //CALLָ�� �����û��Զ����ģ�麯��
		case ZL_R_IT_ADDR: //�����ADDR���û��ָ��͵���op_addr����������
			run->op_addr(VM_ARG,&tmpmem);
			break; //case ZL_R_IT_ADDR: //�����ADDR���û��ָ��͵���op_addr����������
		case ZL_R_IT_SET_ARRAY: //��������Ԫ��
			run->op_set_array(VM_ARG,&tmpmem);
			break;
		case ZL_R_IT_GET_ARRAY: //��ȡ�����е�ĳԪ��
			run->op_get_array(VM_ARG,&tmpmem);
			break;
		case ZL_R_IT_GET_ARRAY_ADDR: //��ȡ����Ԫ������
			run->op_get_array_addr(VM_ARG,&tmpmem);
			break;
		case ZL_R_IT_ADDGET_ARRAY: //����++test[0]
		case ZL_R_IT_MINISGET_ARRAY: //���� --test[0]
		case ZL_R_IT_GETADD_ARRAY: //���� test[0]++
		case ZL_R_IT_GETMINIS_ARRAY: //����test[0]--
			run->op_addminis_one_array(VM_ARG,&tmpmem,ZL_R_CUR_INST.type);
			break;
		case ZL_R_IT_SWITCH: //SWITCHָ�� �����op_switch���д���
			run->op_switch(VM_ARG);
			break;
		case ZL_R_IT_BIT_AND:
		case ZL_R_IT_BIT_OR:
		case ZL_R_IT_BIT_XOR:
		case ZL_R_IT_BIT_RIGHT:
		case ZL_R_IT_BIT_LEFT:
		case ZL_R_IT_BIT_REVERSE:
			run->op_bits(VM_ARG); //��λ�룬������λ����ָ��Ĵ����ʽ
			break;
		case ZL_R_IT_BREAK: //����ǵ��Զϵ�
			{
				ZL_INT conRet = 0;
				ZL_BOOL hasCon = ZL_FALSE;
				ZL_INT debug_break = ZL_R_CUR_INST.src.val.num;
				if(debug->BreakPoint.members[debug_break].disabled == ZL_FALSE)
				{
					if(debug->BreakPoint.members[debug_break].condition != ZL_NULL)
					{
						if((conRet = debug->CheckCondition(VM_ARG,debug->BreakPoint.members[debug_break].condition)) == -1)
							debug->userdef_debug_conditionError(VM_ARG,debug->BreakPoint.members[debug_break].filename,
										debug->BreakPoint.members[debug_break].line,debug_break,zenglApi_GetErrorString(VM_ARG));
						conRet = conRet == -1 ? 1 : conRet;
						hasCon = ZL_TRUE;
					}
					if(hasCon == ZL_FALSE || conRet == 1)
					{
						if(debug->userdef_debug_break != ZL_NULL)
						{
							origState = VM->ApiState;
							VM->ApiState = ZL_API_ST_DEBUG_HANDLE;
							debug->userdef_debug_break(VM_ARG,debug->BreakPoint.members[debug_break].filename,
											debug->BreakPoint.members[debug_break].line,debug_break,debug->BreakPoint.members[debug_break].log);
							VM->ApiState = origState;
						}
					}
				}
				if(ZL_R_CUR_INST.type == ZL_R_IT_BREAK)
				{
					ZL_R_CUR_INST.type = debug->BreakPoint.members[debug_break].orig_inst_type;
					ZL_R_CUR_INST.src.val.num = debug->BreakPoint.members[debug_break].orig_inst_src_num;
					debug->BreakPoint.members[debug_break].needRestore = ZL_TRUE;
				}
				debug->flag =  debug->flag | ZL_DBG_FLAG_RESTORE_BREAK;
			}
			goto start; //�ָ�ԭ����ָ���goto start��ִ��ԭ����ָ��
			break;
		case ZL_R_IT_SINGLE_BREAK:
			{
				ZENGL_COMPILE_TYPE * compile = &VM->compile;
				ZENGL_AST_NODE_TYPE * nodes = compile->AST_nodes.nodes;
				ZL_INT tmp;
				tmp = ZL_R_CUR_INST.nodenum;
				origState = VM->ApiState;
				VM->ApiState = ZL_API_ST_DEBUG_HANDLE;
				debug->userdef_debug_break(VM_ARG,nodes[tmp].filename,nodes[tmp].line_no,-1,ZL_NULL);
				VM->ApiState = origState;
				if(ZL_R_CUR_INST.type == ZL_R_IT_SINGLE_BREAK)
				{
					debug->RestoreSingleBreak(VM_ARG);
				}
			}
			goto start; //�ָ�ԭ����ָ���goto start��ִ��ԭ����ָ��
			break;
		case ZL_R_IT_END:
			ZL_R_REG_PC--;
			break;
		default:
			run->exit(VM_ARG,ZL_ERR_RUN_INVALID_INST_TYPE,ZL_R_REG_PC);
			break;
		} //switch(ZL_R_CUR_INST.type) //���ݵ�ǰ��ָ������ִ�в�ͬ�Ĳ���
		if(debug->flag != 0)
		{
			if((debug->flag & ZL_DBG_FLAG_RESTORE_BREAK) == ZL_DBG_FLAG_RESTORE_BREAK)
				debug->RestoreBreaks(VM_ARG); //ִ����ԭ��ָ��󣬻ָ����Զϵ�
			if(((debug->flag & ZL_DBG_FLAG_SET_SINGLE_STEP_IN) == ZL_DBG_FLAG_SET_SINGLE_STEP_IN) ||
			   ((debug->flag & ZL_DBG_FLAG_SET_SINGLE_STEP_OUT) == ZL_DBG_FLAG_SET_SINGLE_STEP_OUT))
			{
				debug->ChkAndSetSingleBreak(VM_ARG); //������Ҫ���õ����ж�
			}
		}
		ZL_R_REG_PC++; //����PC�Ĵ�����ֵ��
	} //while(ZL_R_CUR_INST.type != ZL_R_IT_END)
}

/*
	��ĳ�ַ�����ֵ��reg�Ĵ���,ͨ��memReUsePtr�ط���Ĵ������str�ַ���ָ���Ӧ���ڴ��С���Ա����㹻�ռ����ַ�������ͨ��strncpy���ַ�����Ϣ�������Ĵ�����str�ֶ�
*/
ZL_VOID zenglrun_RegAssignStr(ZL_VOID * VM_ARG,ZENGL_RUN_REG_TYPE reg,ZL_VOID * str)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_VOID ** tmptr;
	ZL_INT len;
	tmptr = &ZENGL_RUN_REGVAL(reg).str;
	if(str == ZL_NULL) //���strΪZL_NULL����Ŀ��Ĵ�������Ϊ����ַ�����2���ֽڵ��ڴ棬��һ���ֽ���ΪSTRNULL��'\0'�ַ�����β����
	{
		(*tmptr) = run->memReUsePtr(VM_ARG,(*tmptr),2 * sizeof(ZL_CHAR),&ZENGL_RUN_REG(reg).str_Index); //ʹ��memReUsePtr����ڴ���������ʡ�
		(*((ZL_CHAR **)tmptr))[0] = ZL_STRNULL;
		return;
	}
	len = ZENGL_SYS_STRLEN(str);
	(*tmptr) = run->memReUsePtr(VM_ARG,(*tmptr),(len+1) * sizeof(ZL_CHAR),&ZENGL_RUN_REG(reg).str_Index);
	ZENGL_SYS_STRNCPY(*tmptr,str,len);
	(*((ZL_CHAR **)tmptr))[len] = ZL_STRNULL;
}

/*
	MINIS����ָ�����ش����ʽ��op��operate��������д
*/
ZENGL_RUN_RUNTIME_OP_DATA_TYPE zenglrun_op_minis(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT &&
	   ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_INT) //AX,BX��������ʱ��AX��BX��dword��Ա�������������AX�С�
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword -= ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
	else if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_STR) //��AX���ַ���ʱ
	{
		switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
		{
		case ZL_R_RDT_INT: //AX�ַ�����BXΪ��������AXͨ��atoi��C�⺯��תΪ�������ٺ�BX�������������AX�У�ͬʱ��AX������ΪZL_R_RDT_INT���������͡�
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_SYS_STR_TO_LONG_NUM(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str);
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword -= ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
			ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
			break;
		case ZL_R_RDT_FLOAT:
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = ZENGL_SYS_STR_TO_FLOAT(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str); //��AXתΪ��������AX,BX��qword��Ա�����
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword -= ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
			ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
			break;
		case ZL_R_RDT_STR: //AX,BX�����ַ���ʱ��ͨ��strcmp�������ַ������бȽϣ������ȽϵĽ�������AX��dword�С�
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_SYS_STRCMP(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,ZENGL_RUN_REGVAL(ZL_R_RT_BX).str);
			ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
			break;
		}
	}
	else if(ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_STR) //BXΪ�ַ���ʱ�����
	{
		switch(ZENGL_RUN_REG(ZL_R_RT_AX).runType)
		{
		case ZL_R_RDT_INT:
			ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword = ZENGL_SYS_STR_TO_LONG_NUM(ZENGL_RUN_REGVAL(ZL_R_RT_BX).str);
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword -= ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
			ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
			break;
		case ZL_R_RDT_FLOAT:
			ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword = ZENGL_SYS_STR_TO_FLOAT(ZENGL_RUN_REGVAL(ZL_R_RT_BX).str);
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword -= ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
			ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
			break;
		}
	}
	else if(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_FLOAT)
	{
		switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
		{
		case ZL_R_RDT_FLOAT:
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword -= ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
			break;
		case ZL_R_RDT_INT:
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword -= ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
			break;
		}
	}
	else if(ZENGL_RUN_REG(ZL_R_RT_BX).runType == ZL_R_RDT_FLOAT &&
			ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT)
	{
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword - ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
	}
	return ZENGL_RUN_REG(ZL_R_RT_AX).runType; //����AX(������)������
}

/*
	JEָ��ִ�еĲ�����AX�Ĵ���������֮ǰ���ʽ�Ľ������AXΪ0���ʱ�����޸�PC�Ĵ�����ֵ��ʹ�ű�������ת
*/
ZL_VOID zenglrun_op_je(ZL_VOID * VM_ARG,ZENGL_RUN_RUNTIME_OP_DATA * src)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if((ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT && ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword == 0) || 
		(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_FLOAT && ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword == 0) || 
		(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_STR && !ZENGL_SYS_STRCMP(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,"")))
	{
		if(src->runType == ZL_R_RDT_INT)
		{
			ZL_R_REG_PC = src->val.dword;
			ZL_R_REG_PC--;
		}
	}
}

/*
	JNEָ���JEָ��պ��෴��AX�Ĵ���������֮ǰ���ʽ�Ľ������AX��Ϊ0����ΪTRUEʱ�����޸�PC�Ĵ�����ֵ��ʹ�ű�������ת
*/
ZL_VOID zenglrun_op_jne(ZL_VOID * VM_ARG,ZENGL_RUN_RUNTIME_OP_DATA * src)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if((ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_INT && ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword != 0) || 
		(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_FLOAT && ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword != 0) || 
		(ZENGL_RUN_REG(ZL_R_RT_AX).runType == ZL_R_RDT_STR && ZENGL_SYS_STRCMP(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,"")))
	{
		if(src->runType == ZL_R_RDT_INT)
		{
			ZL_R_REG_PC = src->val.dword;
			ZL_R_REG_PC--;
		}
	}
}

/*��λ�룬������λ����ָ��Ĵ����ʽ*/
ZL_VOID zenglrun_op_bits(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	switch(ZENGL_RUN_REG(ZL_R_RT_AX).runType)
	{
	case ZL_R_RDT_FLOAT:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = (ZL_LONG)ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword;
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		break;
	case ZL_R_RDT_STR:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_SYS_STR_TO_LONG_NUM(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str);
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		break;
	case ZL_R_RDT_INT:
		break;
	default: //��������ʹ��0
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = 0;
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		break;
	}
	if(ZL_R_CUR_INST.type != ZL_R_IT_BIT_REVERSE)
	{
		switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
		{
		case ZL_R_RDT_FLOAT:
			ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword = (ZL_LONG)ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword;
			ZENGL_RUN_REG(ZL_R_RT_BX).runType = ZL_R_RDT_INT;
			break;
		case ZL_R_RDT_STR:
			ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword = ZENGL_SYS_STR_TO_LONG_NUM(ZENGL_RUN_REGVAL(ZL_R_RT_BX).str);
			ZENGL_RUN_REG(ZL_R_RT_BX).runType = ZL_R_RDT_INT;
			break;
		case ZL_R_RDT_INT:
			break;
		default:
			ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword = 0;
			ZENGL_RUN_REG(ZL_R_RT_BX).runType = ZL_R_RDT_INT;
			break;
		}
	}
	switch(ZL_R_CUR_INST.type)
	{
	case ZL_R_IT_BIT_AND:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword & ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
		break;
	case ZL_R_IT_BIT_OR:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword | ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
		break;
	case ZL_R_IT_BIT_XOR:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword ^ ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
		break;
	case ZL_R_IT_BIT_RIGHT:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword >> ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
		break;
	case ZL_R_IT_BIT_LEFT:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword << ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
		break;
	case ZL_R_IT_BIT_REVERSE:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ~ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword;
		break;
	}
}

/*
	����С�ڵ���֮��ıȽ������ָ��Ĵ����ʽ��
*/
ZL_VOID zenglrun_op_relation(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_RUNTIME_OP_DATA_TYPE retval;
	ZL_DOUBLE tmpret;
	retval = run->op_minis(VM_ARG);
	if(retval == ZL_R_RDT_FLOAT) //�Ƚ�AX,BX���м��������ٸ��ݴ���Ľ���Ͷ�Ӧ�ıȽ��������������Ӧ�Ľ����
	{
		tmpret = ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword;
		if(tmpret > 0)
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = 1;
		else if(tmpret < 0)
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = -1;
		else
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = 0;
		//ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = (ZL_INT)ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword; //������ֱ�ӽ������Աqwordǿ��תΪ���Σ���Ϊ�����Ļ�0.5������С���ͻ��Ϊ0���ͻ�Ӱ������ıȽϽ����
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
	}
	switch(ZL_R_CUR_INST.type)
	{
	case ZL_R_IT_GREAT: //����Ǵ���ָ���AX,BX������������0ʱ��˵��AX����BX����1��Ϊ��������AX�С�����C���ԣ�������1����PHP���true����ֵ��
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword > 0 ? 1 : 0;
		break;
	case ZL_R_IT_LESS:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword < 0 ? 1 : 0;
		break;
	case ZL_R_IT_EQUAL:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword == 0 ? 1 : 0;
		break;
	case ZL_R_IT_NOT_EQ:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword != 0 ? 1 : 0;
		break;
	case ZL_R_IT_GREAT_EQ:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword >= 0 ? 1 : 0;
		break;
	case ZL_R_IT_LESS_EQ:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword <= 0 ? 1 : 0;
		break;
	}
}

/*
	AND(��)��OR���򣩣�REVERSE��ȡ�����߼�������Ĵ����ʽ���ȵõ�AX�Ĳ���ֵ�����Ϊ0��Ϊ���ַ�����AXΪFALSE����ֵ��������0��ʾPHP���FALSE������֮��Ϊ1 ��
	ͬ��õ�BX�Ĳ���ֵ�����AX,BX����&&��||,!�����㡣
*/
ZL_VOID zenglrun_op_logic(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	switch(ZENGL_RUN_REG(ZL_R_RT_AX).runType)
	{
	case ZL_R_RDT_FLOAT:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword == 0 ? 0 : 1;
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		break;
	case ZL_R_RDT_STR:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_SYS_STRCMP(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,"") == 0 ? 0 : 1;
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		break;
	default:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword != 0 ? 1 : 0;
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		break;
	}
	if(ZL_R_CUR_INST.type != ZL_R_IT_REVERSE)
	{
		switch(ZENGL_RUN_REG(ZL_R_RT_BX).runType)
		{
		case ZL_R_RDT_FLOAT:
			ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_BX).qword == 0 ? 0 : 1;
			ZENGL_RUN_REG(ZL_R_RT_BX).runType = ZL_R_RDT_INT;
			break;
		case ZL_R_RDT_STR:
			ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword = ZENGL_SYS_STRCMP(ZENGL_RUN_REGVAL(ZL_R_RT_BX).str,"") == 0 ? 0 : 1;
			ZENGL_RUN_REG(ZL_R_RT_BX).runType = ZL_R_RDT_INT;
			break;
		default:
			ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword != 0 ? 1 : 0;
			ZENGL_RUN_REG(ZL_R_RT_BX).runType = ZL_R_RDT_INT;
			break;
		}
	}
	switch(ZL_R_CUR_INST.type)
	{
	case ZL_R_IT_AND:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword && ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
		break;
	case ZL_R_IT_OR:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword || ZENGL_RUN_REGVAL(ZL_R_RT_BX).dword;
		break;
	case ZL_R_IT_REVERSE:
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = !ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword;
		break;
	}
}

/*���Ӽӻ����������ڱ�����ʾ����ǰ��ʱ���Ƚ�����ֵ��һ���һ���ٷ��ؽ����memtype���ڴ����ͣ���Ҫ��ȫ�ֱ����������ڴ�Ͳ������ֲ��������ڵĶ�ջ��*/
ZL_VOID zenglrun_op_addminisget(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem,ZENGL_RUN_INST_TYPE type,ZENGL_RUN_INST_OP_DATA_TYPE memtype)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VMEM_OP_GET(memtype,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	switch(tmpmem->runType)
	{
	case ZL_R_RDT_NONE: //���û����ʼ����������0�Դ�
		tmpmem->runType = ZL_R_RDT_INT;
		tmpmem->val.dword = 0;
	case ZL_R_RDT_INT:
		switch(type)
		{
		case ZL_R_IT_ADDGET:
			++tmpmem->val.dword;
			break;
		case ZL_R_IT_MINIS_GET:
			--tmpmem->val.dword;
			break;
		}
		ZENGL_RUN_VMEM_OP(memtype,ZL_R_VMOPT_ADDMEM_INT,ZL_R_VMOPT_SETMEM_INT,*tmpmem,src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = tmpmem->val.dword;
		break;
	case ZL_R_RDT_FLOAT:
		switch(type)
		{
		case ZL_R_IT_ADDGET:
			++tmpmem->val.qword;
			break;
		case ZL_R_IT_MINIS_GET:
			--tmpmem->val.qword;
			break;
		}
		ZENGL_RUN_VMEM_OP(memtype,ZL_R_VMOPT_ADDMEM_DOUBLE,ZL_R_VMOPT_SETMEM_DOUBLE,*tmpmem,src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = tmpmem->val.qword;
		break;
	case ZL_R_RDT_STR:
		{
			ZL_CHAR tmpstr[40];
			ZL_LONG tmpint;
			ZL_DOUBLE tmpfloat;
			ZL_BOOL	isfloat;
			if(ZENGL_SYS_STRCHR(tmpmem->val.str,'.') == ZL_NULL)
				isfloat = ZL_FALSE;
			else
				isfloat = ZL_TRUE;
			if(isfloat)
			{
				tmpfloat = ZENGL_SYS_STR_TO_FLOAT(tmpmem->val.str);
				switch(type)
				{
				case ZL_R_IT_ADDGET:
					tmpfloat++;
					break;
				case ZL_R_IT_MINIS_GET:
					tmpfloat--;
					break;
				}
				ZENGL_SYS_SPRINTF(tmpstr,"%.16g",tmpfloat);
			}
			else
			{
				tmpint = ZENGL_SYS_STR_TO_LONG_NUM(tmpmem->val.str);
				switch(type)
				{
				case ZL_R_IT_ADDGET:
					tmpint++;
					break;
				case ZL_R_IT_MINIS_GET:
					tmpint--;
					break;
				}
				ZENGL_SYS_SPRINTF(tmpstr,"%ld",tmpint);
			}
			tmpmem->val.str = tmpstr;
			ZENGL_RUN_VMEM_OP(memtype,ZL_R_VMOPT_ADDMEM_STR,ZL_R_VMOPT_SETMEM_STR,*tmpmem,src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
			ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
			run->RegAssignStr(VM_ARG,ZL_R_RT_AX,tmpstr);
		}
		break; //case ZL_R_RDT_STR:
	} //switch(tmpmem->runType)
}

/*�ӼӼ���������ڱ�����ʾ������ʱ����ȡֵ�������м�һ���һ����*/
ZL_VOID zenglrun_op_getaddminis(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem,ZENGL_RUN_INST_TYPE type,ZENGL_RUN_INST_OP_DATA_TYPE memtype)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VMEM_OP_GET(memtype,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	switch(tmpmem->runType)
	{
	case ZL_R_RDT_NONE: //���û����ʼ����������0�Դ�
		tmpmem->runType = ZL_R_RDT_INT;
		tmpmem->val.dword = 0;
	case ZL_R_RDT_INT:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = tmpmem->val.dword;
		switch(type)
		{
		case ZL_R_IT_GETADD:
			++tmpmem->val.dword;
			break;
		case ZL_R_IT_GET_MINIS:
			--tmpmem->val.dword;
			break;
		}
		ZENGL_RUN_VMEM_OP(memtype,ZL_R_VMOPT_ADDMEM_INT,ZL_R_VMOPT_SETMEM_INT,*tmpmem,src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
		break;
	case ZL_R_RDT_FLOAT:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = tmpmem->val.qword;
		switch(type)
		{
		case ZL_R_IT_GETADD:
			++tmpmem->val.qword;
			break;
		case ZL_R_IT_GET_MINIS:
			--tmpmem->val.qword;
			break;
		}
		ZENGL_RUN_VMEM_OP(memtype,ZL_R_VMOPT_ADDMEM_DOUBLE,ZL_R_VMOPT_SETMEM_DOUBLE,*tmpmem,src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
		break;
	case ZL_R_RDT_STR:
		{
			ZL_CHAR tmpstr[40];
			ZL_LONG tmpint;
			ZL_DOUBLE tmpfloat;
			ZL_BOOL	isfloat;
			ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
			run->RegAssignStr(VM_ARG,ZL_R_RT_AX,tmpmem->val.str);
			if(ZENGL_SYS_STRCHR(tmpmem->val.str,'.') == ZL_NULL)
				isfloat = ZL_FALSE;
			else
				isfloat = ZL_TRUE;
			if(isfloat)
			{
				tmpfloat = ZENGL_SYS_STR_TO_FLOAT(tmpmem->val.str);
				switch(type)
				{
				case ZL_R_IT_GETADD:
					tmpfloat++;
					break;
				case ZL_R_IT_GET_MINIS:
					tmpfloat--;
					break;
				}
				ZENGL_SYS_SPRINTF(tmpstr,"%.16g",tmpfloat);
			}
			else
			{
				tmpint = ZENGL_SYS_STR_TO_LONG_NUM(tmpmem->val.str);
				switch(type)
				{
				case ZL_R_IT_GETADD:
					tmpint++;
					break;
				case ZL_R_IT_GET_MINIS:
					tmpint--;
					break;
				}
				ZENGL_SYS_SPRINTF(tmpstr,"%ld",tmpint);
			}
			tmpmem->val.str = tmpstr;
			ZENGL_RUN_VMEM_OP(memtype,ZL_R_VMOPT_ADDMEM_STR,ZL_R_VMOPT_SETMEM_STR,*tmpmem,src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
		}
		break;
	} //switch(tmpmem->runType)
}

/*���ӼӼ����ͱ��ʽ����ʱ����Ϊ���ʽ����������ڴ�ռ䣬�����޸��ڴ�ռ��ֵ���������������ֱ�ӽ����ʽ�Ľ�����м�һ����һ���ٷ��ؽ����*/
ZL_VOID zenglrun_op_addminisone(ZL_VOID * VM_ARG,ZENGL_RUN_INST_TYPE type)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	switch(ZENGL_RUN_REG(ZL_R_RT_AX).runType)
	{
	case ZL_R_RDT_INT:
		switch(type)
		{
		case ZL_R_IT_ADDONE:
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword++;
			break;
		case ZL_R_IT_MINIS_ONE:
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword--;
			break;
		}
		break;
	case ZL_R_RDT_FLOAT:
		switch(type)
		{
		case ZL_R_IT_ADDONE:
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword++;
			break;
		case ZL_R_IT_MINIS_ONE:
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword--;
			break;
		}
		break;
	case ZL_R_RDT_STR: //������ַ�������תΪ�����򸡵������ٽ��мӼӼ������㣬����ٽ�������ַ�������ʽ���ء�
		{
			ZL_CHAR tmpstr[40];
			ZL_LONG tmpint;
			ZL_DOUBLE tmpfloat;
			ZL_BOOL	isfloat;
			if(ZENGL_SYS_STRCHR(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str,'.') == ZL_NULL)
				isfloat = ZL_FALSE;
			else
				isfloat = ZL_TRUE;
			if(isfloat)
			{
				tmpfloat = ZENGL_SYS_STR_TO_FLOAT(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str);
				switch(type)
				{
				case ZL_R_IT_ADDONE:
					tmpfloat++;
					break;
				case ZL_R_IT_MINIS_ONE:
					tmpfloat--;
					break;
				}
				ZENGL_SYS_SPRINTF(tmpstr,"%.16g",tmpfloat);
			}
			else
			{
				tmpint = ZENGL_SYS_STR_TO_LONG_NUM(ZENGL_RUN_REGVAL(ZL_R_RT_AX).str);
				switch(type)
				{
				case ZL_R_IT_ADDONE:
					tmpint++;
					break;
				case ZL_R_IT_MINIS_ONE:
					tmpint--;
					break;
				}
				ZENGL_SYS_SPRINTF(tmpstr,"%ld",tmpint);
			}
			run->RegAssignStr(VM_ARG,ZL_R_RT_AX,tmpstr);
		}
		break;
	} //switch(ZENGL_RUN_REG(ZL_R_RT_AX).runType)
}

/*ADDR���û��ָ��Ĳ���*/
ZL_VOID zenglrun_op_addr(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_RUNTIME_OP_DATA_TYPE runtype;
	ZL_INT mem;
	switch(ZL_R_CUR_INST.src.type)
	{
	case ZL_R_DT_MEM: //�����ǰADDRָ��Ĳ�������ȫ���ڴ棬�ͽ�runtype��ΪZL_R_RDT_ADDR
		runtype = ZL_R_RDT_ADDR;
		mem = ZL_R_CUR_INST.src.val.mem; //memΪ���õ�ȫ���ڴ������
		break;
	case ZL_R_DT_ARGMEM:
		runtype = ZL_R_RDT_ADDR_LOC;
		mem = ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword + ZL_R_CUR_INST.src.val.mem; //ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword �����в�������ʼջλ�ã�����ZL_R_CUR_INST.src.val.mem����ƫ��ֵ���ܵõ���ȷ�Ĳ���ջλ�á�
		break;
	case ZL_R_DT_LOCMEM:
		runtype = ZL_R_RDT_ADDR_LOC;
		mem = ZENGL_RUN_REGVAL(ZL_R_RT_LOC).dword + ZL_R_CUR_INST.src.val.mem; //ZENGL_RUN_REGVAL(ZL_R_RT_LOC).dword �����оֲ���������ʼջλ�á�
		break; 
	} //switch(ZL_R_CUR_INST.src.type)
	ZENGL_RUN_REG(ZL_R_RT_AX).runType = runtype; //�����õ��ڴ����͸�ֵ��AX��runType�ֶ� ��
	ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = mem; //�������ڴ��������ֵ��AX��dword�ֶΡ�
}

/*��������Ԫ��*/
ZL_VOID zenglrun_op_set_array(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VIRTUAL_MEM_LIST * ptr;
	ZL_INT argcount,index,ptrIndex,i;
	ZENGL_RUN_VMEM_OP_GET(ZL_R_CUR_INST.src.type,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	if(tmpmem->val.memblock == ZL_NULL)
	{
		ptr = run->alloc_memblock(VM_ARG,&ptrIndex);
		tmpmem->val.memblock = (ZL_VOID *)ptr;
		tmpmem->memblk_Index = ptrIndex;
		ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.src.type,ZL_R_VMOPT_ADDMEM_MEMBLOCK,ZL_R_VMOPT_SETMEM_MEMBLOCK,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	}
	else
		ptr = (ZENGL_RUN_VIRTUAL_MEM_LIST *)tmpmem->val.memblock;
	argcount = run->vstack_list.count - ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword;
	if(argcount == 0)
		index = ptr->count;
	else if(argcount == 1)
		index = run->memblock_getindex(VM_ARG,0,tmpmem);
	else
	{
		for(i=0;i < argcount-1;i++)
		{
			index = run->memblock_getindex(VM_ARG,i,tmpmem);
			if(index < 0)
				run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
			ptr = run->realloc_memblock(VM_ARG,ptr,index);
			if(ptr->mem_array[index].val.memblock == ZL_NULL)
			{
				ptr->mem_array[index].runType = ZL_R_RDT_MEM_BLOCK;
				run->assign_memblock(VM_ARG,&(ptr->mem_array[index].val.memblock) ,
					run->alloc_memblock(VM_ARG,&ptr->mem_array[index].memblk_Index));
			}
			ptr = ptr->mem_array[index].val.memblock;
		}
		index = run->memblock_getindex(VM_ARG,i,tmpmem);
	}
	if(index < 0)
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
	ptr = run->realloc_memblock(VM_ARG,ptr,index);
	run->memblock_setval(VM_ARG,ptr,index,tmpmem); //ʹ��memblock_setval��������Ԫ�ء�
	run->vstack_list.count = ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword; //�ָ�ջ�ռ䡣
	(*tmpmem) = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,(*tmpmem),ZL_TRUE);
	if(tmpmem->runType == ZL_R_RDT_INT)
		ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword = tmpmem->val.dword;
	else
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_VSTACK_STRUCT_EXCEPTION,ZL_R_REG_PC);
	if(run->vstack_list.mem_array[run->vstack_list.count - 1].runType == ZL_R_RDT_MEM_BLOCK)
	{
		run->memblock_free(VM_ARG,run->vstack_list.mem_array[run->vstack_list.count - 1].val.memblock,
			&run->vstack_list.mem_array[run->vstack_list.count - 1].memblk_Index);
		run->vstack_list.mem_array[run->vstack_list.count - 1].val.memblock = ZL_NULL;
	}
	run->vstack_list.count--;
}

/*Ϊ�ڴ����䶯̬�ڴ棬ͨ��ZENGL_RUN_VIRTUAL_MEM_LIST�ṹ�������ڴ�飬ZENGL_RUN_VIRTUAL_MEM_LIST�ṹ�е�mem_array�ֶ����·�����ڴ��ָ�롣*/
ZENGL_RUN_VIRTUAL_MEM_LIST * zenglrun_alloc_memblock(ZL_VOID * VM_ARG,ZL_INT * index)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VIRTUAL_MEM_LIST * ptr;
	ptr = (ZENGL_RUN_VIRTUAL_MEM_LIST *)run->memAlloc(VM_ARG,sizeof(ZENGL_RUN_VIRTUAL_MEM_LIST),index);
	ptr->size = ZL_R_MEM_BLOCK_SIZE;
	ptr->count = 0;
	ptr->refcount = 0;
	ptr->mem_array = (ZENGL_RUN_VIRTUAL_MEM_STRUCT *)run->memAlloc(VM_ARG,ptr->size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT),&ptr->mempool_index);
	if(ptr->mem_array == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(ptr->mem_array,0,ptr->size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT));
	return ptr;
}

/*
	��ȡ�����ڴ�������������test[1] ����ô��������1���ȴ����ջ�У�֮���Է���ջ�У�����Ϊ�������������ֵ�����ɱ��ʽ����(����test[a+b],a+bΪ���ʽ)��
	���ʽ�Ľ�����ȷ���AX�У������ѹ��ջ�У�
	��Ӧ��ջλ��ΪARRAY_ITEM�Ĵ�����ֵ����ƫ�����˴�Ϊ0(��Ϊ��һά����)������������в���i����ƫ����0��
	Ȼ��ͨ��VStackListOpsջ������������index(����������ջ������λ��)��Ϊ�������͵õ�tmpmem����ջ�е�ֵ�������о�������1�������͵õ������������ֵ��
*/
ZL_INT zenglrun_memblock_getindex(ZL_VOID * VM_ARG,ZL_INT i,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT index = ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword + i;
	(*tmpmem) = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,index,(*tmpmem),ZL_TRUE);
	switch(tmpmem->runType)
	{
	case ZL_R_RDT_INT:
		return tmpmem->val.dword;
		break;
	case ZL_R_RDT_FLOAT:
		return (ZL_INT)tmpmem->val.qword;
		break;
	case ZL_R_RDT_STR:
		return ZENGL_SYS_STR_TO_NUM((ZL_CHAR *)tmpmem->val.str);
		break;
	}
	return 0;
}

/*Ϊ�ڴ�鶯̬������С��*/
ZENGL_RUN_VIRTUAL_MEM_LIST * zenglrun_realloc_memblock(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_LIST * ptr,ZL_INT index)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT oldsize = ptr->size;
	if(index < ptr->size)
		return ptr;
	while(index >= ptr->size)
	{
		ptr->size += ZL_R_MEM_BLOCK_SIZE;
	}
	ptr->mem_array = (ZENGL_RUN_VIRTUAL_MEM_STRUCT *)run->memReAlloc(VM_ARG,ptr->mem_array,ptr->size * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT),
		&ptr->mempool_index);
	ZENGL_SYS_MEM_SET(ptr->mem_array + oldsize,0,(ptr->size - oldsize) * sizeof(ZENGL_RUN_VIRTUAL_MEM_STRUCT));
	return ptr;
}

/*
	��dest_arg��Ϊsrc_arg��ָ����ڴ�飬ͬʱ����refcount�ڴ������ü�����
	���ͷ��ڴ��ʱ�Ὣrefcount���м�һ��ֱ��refcountΪ0ʱ�Ž����ڴ��ľ����ͷŲ�����
*/
ZL_VOID zenglrun_assign_memblock(ZL_VOID * VM_ARG,ZL_VOID ** dest_arg,ZL_VOID * src_arg)
{
	ZENGL_RUN_VIRTUAL_MEM_LIST ** dest;
	ZENGL_RUN_VIRTUAL_MEM_LIST * src;
	if(src_arg == ZL_NULL || dest_arg == ZL_NULL)
		return;
	dest = (ZENGL_RUN_VIRTUAL_MEM_LIST **)dest_arg;
	src = (ZENGL_RUN_VIRTUAL_MEM_LIST *)src_arg;
	src->refcount++;
	(*dest) = src;
}

/*���������Ӧ�ڴ��ptr��index����λ�õ�ֵ*/
ZL_VOID zenglrun_memblock_setval(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_LIST * ptr,ZL_INT index,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT argindex = ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword - 2; //֮���Լ�2������Ϊ��Ҫ���õ�ֵ������֮�仹ѹ����һ��ԭʼ�����µ�ARRAY_ITEM�Ĵ�����ֵ��ջ�У�����Ҫ��2
	run->is_inMemBlkSetVal = ZL_TRUE; //��is_inMemBlkSetVal��ΪTRUE����������VStackListOps��ȡջ��Ҫ���õ�ֵʱ���ͻ�ֱ�ӷ��ظ�ֵ�������ֵ������Ҳ��ֱ�ӷ��أ������ٵݹ����á�
	(*tmpmem) = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,argindex,(*tmpmem),ZL_TRUE);
	run->is_inMemBlkSetVal = ZL_FALSE;
	switch(tmpmem->runType) //����ֵ������������Ԫ�أ�ͬʱ��ֵҲͬ�����õ�AX�У���������������󣬷���ֵAX��Ҳ�д�ֵ���Ϳ��Լ���ʹ�ø�ֵ���������ı��ʽ�����㡣
	{
	case ZL_R_RDT_INT:
		run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_INT,ptr,index,tmpmem);
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = ptr->mem_array[index].val.dword;
		break;
	case ZL_R_RDT_FLOAT:
		run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_DOUBLE,ptr,index,tmpmem);
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = ptr->mem_array[index].val.qword;
		break;
	case ZL_R_RDT_STR:
		run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_STR,ptr,index,tmpmem);
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_STR;
		run->RegAssignStr(VM_ARG,ZL_R_RT_AX,ptr->mem_array[index].val.str);
		break;
	case ZL_R_RDT_MEM_BLOCK:
		run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_MEMBLOCK,ptr,index,tmpmem);
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_MEM_BLOCK;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).memblock = tmpmem->val.memblock;
		ZENGL_RUN_REG(ZL_R_RT_AX).memblk_Index = tmpmem->memblk_Index;
		break;
	case ZL_R_RDT_ADDR:
	case ZL_R_RDT_ADDR_LOC:
	case ZL_R_RDT_ADDR_MEMBLK:
		run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_ADDR,ptr,index,tmpmem);
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = tmpmem->val.dword;
		if(tmpmem->runType == ZL_R_RDT_ADDR_MEMBLK)
		{
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).memblock = tmpmem->val.memblock;
			ZENGL_RUN_REG(ZL_R_RT_AX).memblk_Index = tmpmem->memblk_Index;
		}
		break;
	} //switch(tmpmem->runType)
}

/*������ptr���index����ָ����ڴ���в���*/
ZENGL_RUN_VIRTUAL_MEM_STRUCT zenglrun_VMemBlockOps(ZL_VOID * VM_ARG,ZENGL_RUN_VMEM_OP_TYPE op,ZENGL_RUN_VIRTUAL_MEM_LIST * ptr,ZL_INT index,
												   ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT len;
	ZL_VOID ** tmptr;
	ZENGL_RUN_VIRTUAL_MEM_STRUCT retmem = (*tmpmem);
	if(index >= ptr->size || index < 0)
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
	if(op != ZL_R_VMOPT_ADDMEM_ADDR && op != ZL_R_VMOPT_SETMEM_ADDR)
	{
		switch(ptr->mem_array[index].runType)
		{
		case ZL_R_RDT_ADDR: //���������Ԫ����ȫ�ֱ������������VMemListOps
			return run->VMemListOps(VM_ARG,op,ptr->mem_array[index].val.dword,(*tmpmem));
			break;
		case ZL_R_RDT_ADDR_LOC: //�ֲ��������������VStackListOps
			return run->VStackListOps(VM_ARG,op,ptr->mem_array[index].val.dword,(*tmpmem),ZL_TRUE);
			break;
		case ZL_R_RDT_ADDR_MEMBLK: //�������һ������Ԫ�ص����ã���ݹ����VMemBlockOps
			return run->VMemBlockOps(VM_ARG,op,ptr->mem_array[index].val.memblock,ptr->mem_array[index].val.dword,tmpmem);
			break;
		}
	}

	if(op != ZL_R_VMOPT_GETMEM)
	{
		if(tmpmem->runType != ZL_R_RDT_STR && ptr->mem_array[index].runType == ZL_R_RDT_STR && 
			ptr->mem_array[index].str_Index > 0) //������ĳ����Ԫ��ʱ���Ƚ�����Ԫ��֮ǰ�洢���ַ����ڴ�ռ��ͷŵ���
		{
			run->memFreeIndex(VM_ARG,ptr->mem_array[index].val.str,&ptr->mem_array[index].str_Index);
			ptr->mem_array[index].val.str = ZL_NULL;
		}
		else if(ptr->mem_array[index].runType == ZL_R_RDT_MEM_BLOCK) //������ĳ����Ԫ��ʱ�����������Ԫ�ر�������һ�����飬�򽫸�����Ԫ�ط�����ڴ���ͷŵ���
		{
			run->memblock_free(VM_ARG,(ZENGL_RUN_VIRTUAL_MEM_LIST *)ptr->mem_array[index].val.memblock,&ptr->mem_array[index].memblk_Index);
			ptr->mem_array[index].val.memblock = ZL_NULL;
		}
	}
	switch(op)
	{
	case ZL_R_VMOPT_SETMEM_INT: //������Ԫ����Ϊ����
	case ZL_R_VMOPT_ADDMEM_INT:
		ptr->mem_array[index].runType = ZL_R_RDT_INT;
		ptr->mem_array[index].val.dword = tmpmem->val.dword;
		if(ptr->mem_array[index].isvalid == ZL_FALSE)
		{
			ptr->count++;
			ptr->mem_array[index].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_DOUBLE: //������Ԫ����Ϊ������
	case ZL_R_VMOPT_ADDMEM_DOUBLE:
		ptr->mem_array[index].runType = ZL_R_RDT_FLOAT;
		ptr->mem_array[index].val.qword = tmpmem->val.qword;
		if(ptr->mem_array[index].isvalid == ZL_FALSE)
		{
			ptr->count++;
			ptr->mem_array[index].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_STR: //������Ԫ����Ϊ�ַ���
	case ZL_R_VMOPT_ADDMEM_STR:
		ptr->mem_array[index].runType = ZL_R_RDT_STR;
		tmptr = &ptr->mem_array[index].val.str;
		len = ZENGL_SYS_STRLEN(tmpmem->val.str);
		(*tmptr) = run->memReUsePtr(VM_ARG,(*tmptr),(len+1) * sizeof(ZL_CHAR),&ptr->mem_array[index].str_Index);
		ZENGL_SYS_STRNCPY(*tmptr,tmpmem->val.str,len);
		(*((ZL_CHAR **)tmptr))[len] = ZL_STRNULL;
		if(ptr->mem_array[index].isvalid == ZL_FALSE)
		{
			ptr->count++;
			ptr->mem_array[index].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_MEMBLOCK: //������Ԫ����Ϊһ���µ�����
	case ZL_R_VMOPT_ADDMEM_MEMBLOCK:
		ptr->mem_array[index].runType = ZL_R_RDT_MEM_BLOCK;
		run->assign_memblock(VM_ARG,&(ptr->mem_array[index].val.memblock),tmpmem->val.memblock);
		ptr->mem_array[index].memblk_Index = tmpmem->memblk_Index; //ͬʱ�����ڴ��ָ�����ڴ���е�����
		if(ptr->mem_array[index].isvalid == ZL_FALSE)
		{
			ptr->count++;
			ptr->mem_array[index].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_ADDR: //������Ԫ����Ϊ��һ���ڴ�Ԫ�ص����á�
	case ZL_R_VMOPT_ADDMEM_ADDR:
		ptr->mem_array[index].runType = tmpmem->runType;
		ptr->mem_array[index].val.dword = tmpmem->val.dword;
		if(tmpmem->runType == ZL_R_RDT_ADDR_MEMBLK)
		{
			ptr->mem_array[index].val.memblock = tmpmem->val.memblock;
			ptr->mem_array[index].memblk_Index = tmpmem->memblk_Index; //ͬʱ�����ڴ��ָ�����ڴ���е�����
		}
		if(ptr->mem_array[index].isvalid == ZL_FALSE)
		{
			ptr->count++;
			ptr->mem_array[index].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_SETMEM_NONE: //������Ԫ����Ϊδ��ʼ��״̬
	case ZL_R_VMOPT_ADDMEM_NONE:
		ptr->mem_array[index].runType = ZL_R_RDT_NONE;
		if(ptr->mem_array[index].isvalid == ZL_FALSE)
		{
			ptr->count++;
			ptr->mem_array[index].isvalid = ZL_TRUE;
		}
		break;
	case ZL_R_VMOPT_GETMEM: //��ȡ����Ԫ�ص�ֵ��
		retmem = ptr->mem_array[index];
		return retmem;
		break;
	} //switch(op)
	return retmem;
}

/*�ͷ�memblock��������ڴ�飬�Ƚ�refcount��һ����refcountС�ڵ���0ʱ����ѭ���ӵݹ齫�ڴ������ڴ��������ڴ��ȫ���ͷŵ�*/
ZL_VOID zenglrun_memblock_free(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_LIST * memblock,ZL_INT * index)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT i;
	if(memblock == ZL_NULL)
		return;
	memblock->refcount--;
	if(memblock->refcount <= 0)
	{
		for(i=0;i<memblock->size;i++)
		{
			if(memblock->mem_array[i].runType == ZL_R_RDT_STR && 
				memblock->mem_array[i].str_Index > 0)
			{
				run->memFreeIndex(VM_ARG,memblock->mem_array[i].val.str,&memblock->mem_array[i].str_Index);
				memblock->mem_array[i].val.str = ZL_NULL;
			}
			else if(memblock->mem_array[i].runType == ZL_R_RDT_MEM_BLOCK &&
				memblock->mem_array[i].val.memblock != ZL_NULL)
			{
				run->memblock_free(VM_ARG,(ZENGL_RUN_VIRTUAL_MEM_LIST *)memblock->mem_array[i].val.memblock,&memblock->mem_array[i].memblk_Index);
				memblock->mem_array[i].val.memblock = ZL_NULL;
			}
		}
		run->memFreeIndex(VM_ARG,memblock->mem_array,&memblock->mempool_index); //û�������ͻ��ڴ�һֱ���󣬵����ڴ�й©����
		run->memFreeIndex(VM_ARG,memblock,index);
	}
}

/*��ȡ�����е�ĳԪ��*/
ZL_VOID zenglrun_op_get_array(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VIRTUAL_MEM_LIST * ptr;
	ZL_INT argcount,index,ptrIndex,i;
	ZENGL_RUN_VMEM_OP_GET(ZL_R_CUR_INST.src.type,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	if(tmpmem->val.memblock == ZL_NULL)
	{
		ptr = run->alloc_memblock(VM_ARG,&ptrIndex);
		tmpmem->val.memblock = (ZL_VOID *)ptr;
		tmpmem->memblk_Index = ptrIndex;
		ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.src.type,ZL_R_VMOPT_ADDMEM_MEMBLOCK,ZL_R_VMOPT_SETMEM_MEMBLOCK,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	}
	else
		ptr = (ZENGL_RUN_VIRTUAL_MEM_LIST *)tmpmem->val.memblock;
	argcount = run->vstack_list.count - ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword;
	if(argcount == 0)
		index = ptr->count;
	else if(argcount == 1)
		index = run->memblock_getindex(VM_ARG,0,tmpmem);
	else
	{
		for(i=0;i < argcount-1;i++)
		{
			index = run->memblock_getindex(VM_ARG,i,tmpmem);
			if(index < 0)
				run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
			ptr = run->realloc_memblock(VM_ARG,ptr,index);
			if(ptr->mem_array[index].val.memblock == ZL_NULL)
			{
				ptr->mem_array[index].runType = ZL_R_RDT_MEM_BLOCK;
				run->assign_memblock(VM_ARG,&(ptr->mem_array[index].val.memblock) ,
					run->alloc_memblock(VM_ARG,&ptr->mem_array[index].memblk_Index));
			}
			ptr = ptr->mem_array[index].val.memblock;
		}
		index = run->memblock_getindex(VM_ARG,i,tmpmem);
	}
	if(index < 0)
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
	ptr = run->realloc_memblock(VM_ARG,ptr,index);
	(*tmpmem) = run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_GETMEM,ptr,index,tmpmem);
	switch(tmpmem->runType)
	{
	case ZL_R_RDT_NONE:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = 0;
		break;
	case ZL_R_RDT_INT:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = tmpmem->val.dword;
		break;
	case ZL_R_RDT_FLOAT:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = tmpmem->val.qword;
		break;
	case ZL_R_RDT_STR:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_STR;
		run->RegAssignStr(VM_ARG,ZL_R_RT_AX,tmpmem->val.str);
		break;
	case ZL_R_RDT_MEM_BLOCK:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_MEM_BLOCK;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).memblock = tmpmem->val.memblock;
		ZENGL_RUN_REG(ZL_R_RT_AX).memblk_Index = tmpmem->memblk_Index;
		break;
	case ZL_R_RDT_ADDR:
	case ZL_R_RDT_ADDR_LOC:
	case ZL_R_RDT_ADDR_MEMBLK:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = tmpmem->val.dword;
		if(tmpmem->runType == ZL_R_RDT_ADDR_MEMBLK)
		{
			ZENGL_RUN_REGVAL(ZL_R_RT_AX).memblock = tmpmem->val.memblock;
			ZENGL_RUN_REG(ZL_R_RT_AX).memblk_Index = tmpmem->memblk_Index;
		}
		break;
	} //switch(tmpmem->runType)
	run->vstack_list.count = ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword; //�ָ�ջ�ռ�
	(*tmpmem) = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,(*tmpmem),ZL_TRUE);
	if(tmpmem->runType == ZL_R_RDT_INT)
		ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword = tmpmem->val.dword;
	else
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_VSTACK_STRUCT_EXCEPTION,ZL_R_REG_PC);
}

/*��ȡ�ڴ��Ԫ�ص����ã���test = &testarray[0];�����*/
ZL_VOID zenglrun_op_get_array_addr(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VIRTUAL_MEM_LIST * ptr;
	ZL_INT argcount,index,ptrIndex,i;
	ZENGL_RUN_VMEM_OP_GET(ZL_R_CUR_INST.src.type,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	if(tmpmem->val.memblock == ZL_NULL)
	{
		ptr = run->alloc_memblock(VM_ARG,&ptrIndex);
		tmpmem->val.memblock = (ZL_VOID *)ptr;
		tmpmem->memblk_Index = ptrIndex;
		ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.src.type,ZL_R_VMOPT_ADDMEM_MEMBLOCK,ZL_R_VMOPT_SETMEM_MEMBLOCK,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	}
	else
	{
		ptr = (ZENGL_RUN_VIRTUAL_MEM_LIST *)tmpmem->val.memblock;
		ptrIndex = tmpmem->memblk_Index;
	}
	argcount = run->vstack_list.count - ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword;
	if(argcount == 0)
		index = ptr->count;
	else if(argcount == 1)
		index = run->memblock_getindex(VM_ARG,0,tmpmem);
	else
	{
		for(i=0;i < argcount-1;i++)
		{
			index = run->memblock_getindex(VM_ARG,i,tmpmem);
			if(index < 0)
				run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
			ptr = run->realloc_memblock(VM_ARG,ptr,index);
			if(ptr->mem_array[index].val.memblock == ZL_NULL)
			{
				ptr->mem_array[index].runType = ZL_R_RDT_MEM_BLOCK;
				run->assign_memblock(VM_ARG,&(ptr->mem_array[index].val.memblock) ,
					run->alloc_memblock(VM_ARG,&ptr->mem_array[index].memblk_Index));
			}
			ptr = ptr->mem_array[index].val.memblock;
			ptrIndex = ptr->mem_array[index].memblk_Index;
		}
		index = run->memblock_getindex(VM_ARG,i,tmpmem);
	}
	if(index < 0)
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
	ptr = run->realloc_memblock(VM_ARG,ptr,index);
	ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_ADDR_MEMBLK; //����AX����ֵ������ʱ����Ϊ ZL_R_RDT_ADDR_MEMBLK ���ڴ������
	ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = index; //dword��������ڴ�������
	ZENGL_RUN_REGVAL(ZL_R_RT_AX).memblock = ptr; //memblock��������ڴ���ָ��
	ZENGL_RUN_REG(ZL_R_RT_AX).memblk_Index = ptrIndex; //memblk_Index��ŵ���memblockָ�����ڴ���е�����
	run->vstack_list.count = ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword; //�ָ�ջ�ռ�
	(*tmpmem) = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,(*tmpmem),ZL_TRUE);
	if(tmpmem->runType == ZL_R_RDT_INT)
		ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword = tmpmem->val.dword;
	else
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_VSTACK_STRUCT_EXCEPTION,ZL_R_REG_PC);
}

/*������Ԫ�ؽ��мӼӣ���������*/
ZL_VOID zenglrun_op_addminis_one_array(ZL_VOID * VM_ARG,ZENGL_RUN_VIRTUAL_MEM_STRUCT * tmpmem,ZENGL_RUN_INST_TYPE op)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_RUN_VIRTUAL_MEM_LIST * ptr;
	ZL_INT argcount,index,ptrIndex,i;
	ZENGL_RUN_VMEM_OP_GET(ZL_R_CUR_INST.src.type,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	if(tmpmem->val.memblock == ZL_NULL) //���û������ڴ����ͨ��alloc_memblockΪ������ڴ��
	{
		ptr = run->alloc_memblock(VM_ARG,&ptrIndex);
		tmpmem->val.memblock = (ZL_VOID *)ptr;
		tmpmem->memblk_Index = ptrIndex;
		ZENGL_RUN_VMEM_OP(ZL_R_CUR_INST.src.type,ZL_R_VMOPT_ADDMEM_MEMBLOCK,ZL_R_VMOPT_SETMEM_MEMBLOCK,(*tmpmem),src,ZL_ERR_RUN_INVALID_VMEM_TYPE)
	}
	else
		ptr = (ZENGL_RUN_VIRTUAL_MEM_LIST *)tmpmem->val.memblock;
	argcount = run->vstack_list.count - ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword; //�õ������������ά������test[1,2,3]��ôargcount����3
	if(argcount == 0) //���û��������Ϣ��test[]����ֱ��������ĩβ���һ��Ԫ��
		index = ptr->count;
	else if(argcount == 1) //���Ϊһά���飬��ֱ��memblock_getindex�õ���������
		index = run->memblock_getindex(VM_ARG,0,tmpmem);
	else
	{
		for(i=0;i < argcount-1;i++) //���ά�ȴ��ڵ���2������ѭ��������Ҫ�õ�����ĸ�ά�ȵ��ڴ�飬����test[1,2,3]��iΪ0ʱΪ����1�����ڴ������test[1]Ҳ��Ϊ��һ�����飬Ȼ��iΪ1ʱΪ����2�����ڴ������test[1,2]Ҳ��Ϊ��һ�����飬test[1,2,3]����test[1,2]�����еĵ�4��Ԫ��(������0��ʼ)
		{
			index = run->memblock_getindex(VM_ARG,i,tmpmem);
			if(index < 0)
				run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
			ptr = run->realloc_memblock(VM_ARG,ptr,index); //Ĭ�Ϸ�����ڴ�鲢���󣬵�index���������ڴ���Сʱ������Ҫ��̬�����ڴ��������index����������
			if(ptr->mem_array[index].val.memblock == ZL_NULL) //�����ά��û�з�����ڴ����alloc_memblockΪ������ڴ�飬ͬʱͨ��assign_memblock�������ڴ������ü���
			{
				ptr->mem_array[index].runType = ZL_R_RDT_MEM_BLOCK;
				run->assign_memblock(VM_ARG,&(ptr->mem_array[index].val.memblock) ,
					run->alloc_memblock(VM_ARG,&ptr->mem_array[index].memblk_Index));
			}
			ptr = ptr->mem_array[index].val.memblock;
		}
		index = run->memblock_getindex(VM_ARG,i,tmpmem); //�õ����һά�ȵ�����
	}
	if(index < 0)
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_INVALID_INDEX);
	ptr = run->realloc_memblock(VM_ARG,ptr,index); //ptrΪ���������ڵ��ڴ��
	(*tmpmem) = run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_GETMEM,ptr,index,tmpmem); //�����ڴ�������ֵ�õ�������ڴ�
	switch(tmpmem->runType) //�����ڴ����ֵ���мӼӣ��������㣬��ͬ������AX����ֵ
	{
	case ZL_R_RDT_NONE:
		tmpmem->runType = ZL_R_RDT_INT;
		tmpmem->val.dword = 0;
	case ZL_R_RDT_INT:
		switch(op)
		{
		case ZL_R_IT_ADDGET_ARRAY:
			tmpmem->val.dword++;
			run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_INT,ptr,index,tmpmem);
			break;
		case ZL_R_IT_MINISGET_ARRAY:
			tmpmem->val.dword--;
			run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_INT,ptr,index,tmpmem);
			break;
		}
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = tmpmem->val.dword;
		switch(op)
		{
		case ZL_R_IT_GETADD_ARRAY:
			tmpmem->val.dword++;
			run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_INT,ptr,index,tmpmem);
			break;
		case ZL_R_IT_GETMINIS_ARRAY:
			tmpmem->val.dword--;
			run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_INT,ptr,index,tmpmem);
			break;
		}
		break; //case ZL_R_RDT_INT:
	case ZL_R_RDT_FLOAT:
		switch(op)
		{
		case ZL_R_IT_ADDGET_ARRAY:
			tmpmem->val.qword++;
			run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_DOUBLE,ptr,index,tmpmem);
			break;
		case ZL_R_IT_MINISGET_ARRAY:
			tmpmem->val.qword--;
			run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_DOUBLE,ptr,index,tmpmem);
			break;
		}
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_FLOAT;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).qword = tmpmem->val.qword;
		switch(op)
		{
		case ZL_R_IT_GETADD_ARRAY:
			tmpmem->val.qword++;
			run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_DOUBLE,ptr,index,tmpmem);
			break;
		case ZL_R_IT_GETMINIS_ARRAY:
			tmpmem->val.qword--;
			run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_DOUBLE,ptr,index,tmpmem);
			break;
		}
		break; //case ZL_R_RDT_FLOAT:
	case ZL_R_RDT_STR:
		{
			ZL_CHAR tmpstr[40];
			ZL_LONG tmpint;
			ZL_DOUBLE tmpfloat;
			ZL_BOOL	isfloat;
			if(ZENGL_SYS_STRCHR(tmpmem->val.str,'.') == ZL_NULL)
			{
				tmpint = ZENGL_SYS_STR_TO_LONG_NUM(tmpmem->val.str);
				isfloat = ZL_FALSE;
			}
			else
			{
				tmpfloat = ZENGL_SYS_STR_TO_FLOAT(tmpmem->val.str);
				isfloat = ZL_TRUE;
			}
			switch(op)
			{
			case ZL_R_IT_ADDGET_ARRAY:
			case ZL_R_IT_MINISGET_ARRAY:
				if(isfloat == ZL_FALSE)
				{
					if(op == ZL_R_IT_ADDGET_ARRAY)
						tmpint++;
					else
						tmpint--;
					ZENGL_SYS_SPRINTF(tmpstr,"%ld",tmpint);
				}
				else
				{
					if(op == ZL_R_IT_ADDGET_ARRAY)
						tmpfloat++;
					else
						tmpfloat--;
					ZENGL_SYS_SPRINTF(tmpstr,"%.16g",tmpfloat);
				}
				tmpmem->val.str = tmpstr;
				run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_STR,ptr,index,tmpmem);
				break;
			default:
				if(isfloat == ZL_FALSE)
					ZENGL_SYS_SPRINTF(tmpstr,"%ld",tmpint);
				else
					ZENGL_SYS_SPRINTF(tmpstr,"%.16g",tmpfloat);
				break;
			}
			ZENGL_RUN_REG(ZL_R_RT_AX).runType = tmpmem->runType;
			run->RegAssignStr(VM_ARG,ZL_R_RT_AX,tmpstr);
			switch(op)
			{
			case ZL_R_IT_GETADD_ARRAY:
			case ZL_R_IT_GETMINIS_ARRAY:
				if(isfloat == ZL_FALSE)
				{
					if(op == ZL_R_IT_ADDGET_ARRAY)
						tmpint++;
					else
						tmpint--;
					ZENGL_SYS_SPRINTF(tmpstr,"%ld",tmpint);
				}
				else
				{
					if(op == ZL_R_IT_ADDGET_ARRAY)
						tmpfloat++;
					else
						tmpfloat--;
					ZENGL_SYS_SPRINTF(tmpstr,"%.16g",tmpfloat);
				}
				tmpmem->val.str = tmpstr;
				run->VMemBlockOps(VM_ARG,ZL_R_VMOPT_SETMEM_STR,ptr,index,tmpmem);
				break;
			}
		}
		break; //case ZL_R_RDT_STR:
	default:
		ZENGL_RUN_REG(ZL_R_RT_AX).runType = ZL_R_RDT_INT;
		ZENGL_RUN_REGVAL(ZL_R_RT_AX).dword = 0;
		break;
	} //switch(tmpmem->runType)
	run->vstack_list.count = ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword; //�ڷ�������Ԫ��ʱ�Ὣ����ѹ��ջ����Ϊ�����Ѿ��������ˣ���������Ϳ��Ը���ARRAY_ITEM�Ĵ�����ֵ�ָ�ջ��������������ֵ
	(*tmpmem) = run->VStackListOps(VM_ARG,ZL_R_VMOPT_GETMEM,-1,(*tmpmem),ZL_TRUE); //��ԭʼ�����е�ARRAY_ITEM�Ĵ���ֵ������VStackListOps����������Ϊ-1��ʾ����ջ�����һ��Ԫ��
	if(tmpmem->runType == ZL_R_RDT_INT)
		ZENGL_RUN_REGVAL(ZL_R_RT_ARRAY_ITEM).dword = tmpmem->val.dword; //ʹ�õ�����ֵ�ָ�ARRAY_ITEM�Ĵ���
	else
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_BLOCK_VSTACK_STRUCT_EXCEPTION,ZL_R_REG_PC);
}

/*�ͷ�ջ�в������ֺ;ֲ��������ֵ������ڴ��*/
ZL_VOID zenglrun_memblock_freeall_local(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT locIndex = ZENGL_RUN_REGVAL(ZL_R_RT_ARG).dword;
	for(;locIndex < run->vstack_list.count;locIndex++)
	{
		switch(run->vstack_list.mem_array[locIndex].runType)
		{
		case ZL_R_RDT_MEM_BLOCK:
			if(run->vstack_list.mem_array[locIndex].val.memblock != ZL_NULL)
			{
				run->memblock_free(VM_ARG,run->vstack_list.mem_array[locIndex].val.memblock,
					&run->vstack_list.mem_array[locIndex].memblk_Index);
				run->vstack_list.mem_array[locIndex].runType = ZL_R_RDT_NONE;
				run->vstack_list.mem_array[locIndex].val.dword = 0;
				run->vstack_list.mem_array[locIndex].val.memblock = ZL_NULL;
			}
			break;
		case ZL_R_RDT_ADDR: //���ֲ������Ͳ����������Ҳ�ͷŵ�
		case ZL_R_RDT_ADDR_LOC:
		case ZL_R_RDT_ADDR_MEMBLK:
			run->vstack_list.mem_array[locIndex].runType = ZL_R_RDT_NONE;
			run->vstack_list.mem_array[locIndex].val.dword = 0;
			run->vstack_list.mem_array[locIndex].val.memblock = ZL_NULL;
			run->vstack_list.mem_array[locIndex].memblk_Index = 0;
			break;
		}
	}
}

/*�����������ʱ���ͷŵ�ȫ�������ڴ棬ջ�ڴ��������ڴ�������*/
ZL_VOID zenglrun_FreeAllForReUse(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT locIndex,globalIndex;
	for(locIndex = 0;locIndex < run->vstack_list.count;locIndex++) //�ͷŵ�ջ�ڴ��е��ڴ�������
	{
		switch(run->vstack_list.mem_array[locIndex].runType)
		{
		case ZL_R_RDT_MEM_BLOCK:
			if(run->vstack_list.mem_array[locIndex].val.memblock != ZL_NULL)
			{
				run->memblock_free(VM_ARG,run->vstack_list.mem_array[locIndex].val.memblock,
					&run->vstack_list.mem_array[locIndex].memblk_Index);
				run->vstack_list.mem_array[locIndex].runType = ZL_R_RDT_NONE;
				run->vstack_list.mem_array[locIndex].val.dword = 0;
				run->vstack_list.mem_array[locIndex].val.memblock = ZL_NULL;
			}
			break;
		case ZL_R_RDT_ADDR: //���ֲ������Ͳ����������Ҳ�ͷŵ�
		case ZL_R_RDT_ADDR_LOC:
		case ZL_R_RDT_ADDR_MEMBLK:
			run->vstack_list.mem_array[locIndex].runType = ZL_R_RDT_NONE;
			run->vstack_list.mem_array[locIndex].val.dword = 0;
			run->vstack_list.mem_array[locIndex].val.memblock = ZL_NULL;
			run->vstack_list.mem_array[locIndex].memblk_Index = 0;
			break;
		default:
			run->vstack_list.mem_array[locIndex].runType = ZL_R_RDT_NONE;
			run->vstack_list.mem_array[locIndex].val.dword = 0;
			break;
		}
	}
	for(globalIndex = 0;globalIndex < run->vmem_list.count;globalIndex++) //�ͷŵ�ȫ�������ڴ��е��ڴ�������
	{
		switch(run->vmem_list.mem_array[globalIndex].runType)
		{
		case ZL_R_RDT_MEM_BLOCK:
			if(run->vmem_list.mem_array[globalIndex].val.memblock != ZL_NULL)
			{
				run->memblock_free(VM_ARG,run->vmem_list.mem_array[globalIndex].val.memblock,
					&run->vmem_list.mem_array[globalIndex].memblk_Index);
				run->vmem_list.mem_array[globalIndex].runType = ZL_R_RDT_NONE;
				run->vmem_list.mem_array[globalIndex].val.dword = 0;
				run->vmem_list.mem_array[globalIndex].val.memblock = ZL_NULL;
			}
			break;
		case ZL_R_RDT_ADDR: //���ֲ������Ͳ����������Ҳ�ͷŵ�
		case ZL_R_RDT_ADDR_LOC:
		case ZL_R_RDT_ADDR_MEMBLK:
			run->vmem_list.mem_array[globalIndex].runType = ZL_R_RDT_NONE;
			run->vmem_list.mem_array[globalIndex].val.dword = 0;
			run->vmem_list.mem_array[globalIndex].val.memblock = ZL_NULL;
			run->vmem_list.mem_array[globalIndex].memblk_Index = 0;
			break;
		default:
			run->vmem_list.mem_array[globalIndex].runType = ZL_R_RDT_NONE;
			run->vmem_list.mem_array[globalIndex].val.dword = 0;
			break;
		}
	}
}

/*SWITCHָ��Ĵ���*/
ZL_VOID zenglrun_op_switch(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_LONG count=0,min=0,max=0,num,golast,table;
	golast = ZL_R_CUR_INST.src.val.num; //SWITCHָ���Դ�������ǵ���switch��ת�����Ҳ�����Ӧ��ֵʱ����Ҫ��ת��Ĭ��λ�û����λ��
	table = ZL_R_CUR_INST.dest.val.num; //Ŀ��������ǵ�ǰswitch...case�ṹ����ת��ַ�����ڵĻ�����λ��
	if(ZENGL_RUN_INST(ZL_R_REG_PC+1).type == ZL_R_IT_LONG)
		count = ZENGL_RUN_INST(ZL_R_REG_PC+1).src.val.num; //SWITCH���LONG�Ĳ���������ת���������
	else
		run->exit(VM_ARG,ZL_ERR_RUN_SWITCH_HAVE_NO_LONG_INST_BELOW);
	if(count == 0) //�����ת��Ϊ�գ���ô��ûʲô�ɱȽϵģ�ֱ������golast
		ZL_R_REG_PC = golast - 1;  //��һ����Ϊ������ָ��ִ��ǰ���Զ���ZL_R_REG_PC��һ
	else
	{
		if(ZENGL_RUN_INST(ZL_R_REG_PC+2).type == ZL_R_IT_LONG) //SWITCH��ĵڶ���LONG��������ת����ɹ��Ƚϵ����ֵ����Сֵ
		{
			min = ZENGL_RUN_INST(ZL_R_REG_PC+2).dest.val.num; //Ŀ�������Ϊ��Сֵ
			max = ZENGL_RUN_INST(ZL_R_REG_PC+2).src.val.num;  //Դ������Ϊ���ֵ
			num = run->getRegInt(VM_ARG,ZL_R_RT_AX); //��Ҫ���бȽϵ�AX���ֵתΪ������ʽ����
			if(num < min || num > max) //���num��min��С�����max��Ҫ�����������golast
			{
				ZL_R_REG_PC = golast - 1;
				return;
			}
			else
			{
				if(count == max - min + 1) //�����ת����Ҫ���бȽϵ����ǵ�����ʽ�� 1,2,3,4,5,6�����ģ��Ϳ���ֱ�Ӵ���ת��ַ���м����Ҫ������ת��Ŀ���ַ
				{
					ZL_R_REG_PC = ZENGL_RUN_INST(table + num - min).dest.val.num - 1; //��ת��ַ�������ת���ÿһ���Ŀ���������
					return;
				}
				else
				{
					if(num == min)	 //���������Сֵ���򷵻���ת���һ�����ת��ַ
					{
						ZL_R_REG_PC = ZENGL_RUN_INST(table).dest.val.num - 1;
						return;
					}
					else if(num == max) //����������ֵ���򷵻���ת�����һ�����ת��ַ
					{
						ZL_R_REG_PC = ZENGL_RUN_INST(table + count - 1).dest.val.num - 1;
						return;
					}
					else //���򣬲������з���ѭ��ȡ�м�����бȽϣ��ҵ�ƥ�����Ŀ�����û�ҵ���������golast
					{
						ZL_INT minIndex=0,maxIndex=count - 1,diff,diffnum;
						do
						{
							diff = (maxIndex - minIndex)/2;
							if(diff == 0)
							{
								ZL_R_REG_PC = golast - 1;
								return;
							}
							else
							{
								diffnum = ZENGL_RUN_INST(table + minIndex + diff).src.val.num;
								if(num == diffnum)
								{
									ZL_R_REG_PC = ZENGL_RUN_INST(table + minIndex + diff).dest.val.num - 1;
									return;
								}
								else if(num > diffnum)
									minIndex += diff;
								else
									maxIndex = minIndex + diff;
							}
						}while(ZL_TRUE);
					}
				} //if(count == max - min + 1) ... else ...
			} //if(num < min || num > max) ... else ...
		} //if(ZENGL_RUN_INST(ZL_R_REG_PC+2).type == ZL_R_IT_LONG)
	} //if(ZENGL_RUN_INST(ZL_R_REG_PC+2).type == ZL_R_IT_LONG) ... else ...
}

/*���ؼĴ���ֵ��������ʽ*/
ZL_LONG zenglrun_getRegInt(ZL_VOID * VM_ARG,ZENGL_RUN_REG_TYPE reg)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	switch(ZENGL_RUN_REG(reg).runType)
	{
	case ZL_R_RDT_INT:
		return ZENGL_RUN_REGVAL(reg).dword;
		break;
	case ZL_R_RDT_FLOAT:
		return (ZL_LONG)(ZENGL_RUN_REGVAL(reg).qword);
		break;
	case ZL_R_RDT_STR:
		return ZENGL_SYS_STR_TO_LONG_NUM((ZL_CHAR *)ZENGL_RUN_REGVAL(reg).str);
		break;
	}
	return 0;
}

/*����������ں���*/
ZL_INT zenglrun_main(ZL_VOID * VM_ARG)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT retcode;
	ZENGL_API_STATES origState;
	run->isinRunning = ZL_TRUE;
	run->start_time = ZENGL_SYS_TIME_CLOCK();
	if((retcode = ZENGL_SYS_JMP_SETJMP(run->jumpBuffer))==0)
	{
		if(VM->vm_main_args->userdef_module_init != ZL_NULL)
		{
			origState = VM->ApiState;
			VM->ApiState = ZL_API_ST_MODULES_INIT;
			((ZL_VM_API_MODS_INIT)VM->vm_main_args->userdef_module_init)(VM_ARG); //�����û��Զ����ģ���ʼ������
			VM->ApiState = origState;
		}
		VM->debug.api_call_pc = ZL_R_REG_PC; //zenglApi_Call����ʱ�ĳ�ʼָ��PCֵ�ͻ����0��������zenglApi_Run֮��ĳ�ʼָ��PCֵΪ0
		VM->debug.api_call_arg = run->reg_list[ZL_R_RT_ARG].val.dword; //zenglApi_Call����ʱ�ĳ�ʼargֵ����0��������zenglApi_Run֮��ĳ�ʼargֵΪ0
		if(VM->debug.break_start == ZL_TRUE && VM->debug.userdef_debug_break != ZL_NULL)
			VM->debug.BreakStart(VM);
		run->RunInsts(VM_ARG);
		if(VM->isUseApiSetErrThenStop == ZL_TRUE) //���ͨ��zenglApi_SetErrThenStop�ӿ���ֹͣ������ģ���ͨ��exit_forApiSetErrThenStop���˳�
			run->exit_forApiSetErrThenStop(VM_ARG);
		else
			run->exit(VM_ARG,ZL_NO_ERR_SUCCESS);
	}
	if(retcode == 1)
		return 0;
	else
		return -1;
	return 0;
}
