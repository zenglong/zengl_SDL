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
	���ļ�Ϊ������������ĸ����������ļ���
*/

#include "zengl_global.h"

/*
	��������ʼ��
*/
ZL_VOID zenglrun_init(ZL_VOID * VM_ARG)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZL_INT increase = 0;
	run->mempool.size = ZL_R_MEM_POOL_SIZE;
	run->mempool.count = 1; //��Ϊ1��������ֻ�д���0������������Ч���ڴ��������
	run->mempool.points = ZENGL_SYS_MEM_ALLOC(run->mempool.size * sizeof(ZENGL_RUN_MEM_POOL_POINT_TYPE)); //Ϊ�ڴ�ط����ڴ�ռ�
	if(run->mempool.points == ZL_NULL)
	{
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_POOL_MALLOC_FAILED);
	}
	else
		ZENGL_SYS_MEM_SET(run->mempool.points,0,run->mempool.size * sizeof(ZENGL_RUN_MEM_POOL_POINT_TYPE));
	increase = run->mempool.size * sizeof(ZENGL_RUN_MEM_POOL_POINT_TYPE);
	run->mempool.totalsize += increase; //�����ڴ���ڴ�ռ����ͳ����
	run->totalsize += increase;
	VM->totalsize += increase;
	run->mempool.isInit = ZL_TRUE;
	run->memfreepool.size = ZL_R_MEM_FREE_POOL_SIZE;//��ʼ���ڴ��ͷųصĶ�̬���顣
	run->memfreepool.count = 0;
	run->memfreepool.frees = run->memAlloc(VM_ARG,run->memfreepool.size * sizeof(ZL_INT) , &run->memfreepool.mempool_index);
	if(run->memfreepool.frees == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_RUN_MEM_FREE_POOL_MALLOC_FAILED);
	else
		ZENGL_SYS_MEM_SET(run->memfreepool.frees,0,run->memfreepool.size * sizeof(ZL_INT));
	run->userdef_run_info = (ZL_VM_API_INFO_FUN_HANDLE)VM->vm_main_args->userdef_run_info;
	run->userdef_run_print = (ZL_VM_API_INFO_FUN_HANDLE)VM->vm_main_args->userdef_run_print;
	run->userdef_run_error = (ZL_VM_API_INFO_FUN_HANDLE)VM->vm_main_args->userdef_run_error;
}

/*
	��Ϊmemfreepool�д�ŵ���֮ǰ���ڴ�����ͷŹ���ָ����Ϣ�����������memfreepool�д������ͷŹ���ָ�룬
	���ҵ���ָ�����ڴ���е������������Ϳ���ֱ������ԭ����������ڴ���Ϊ�µ�ָ��ռ䣬�������ڴ濪����
	ֻ����û�ҵ��ͷŹ����ڴ�ʱ���ŷ����µ��ڴ档
	��һ������size��Ҫ������ڴ��С��
	�ڶ�������index�Ƿ�����ڴ��������
*/
ZL_VOID * zenglrun_memAlloc(ZL_VOID * VM_ARG,ZL_INT size,ZL_INT * index)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZL_INT i = 0;
	ZL_INT increase = 0;
	ZL_VOID * point;
	if(run->mempool.count == run->mempool.size) //����ڴ�س�����Χ����ж�̬���ݡ�
	{
		run->mempool.size += ZL_R_MEM_POOL_SIZE;
		increase += ZL_R_MEM_POOL_SIZE * sizeof(ZENGL_RUN_MEM_POOL_POINT_TYPE);
		run->mempool.points = ZENGL_SYS_MEM_RE_ALLOC(run->mempool.points,run->mempool.size * sizeof(ZENGL_RUN_MEM_POOL_POINT_TYPE));
		ZENGL_SYS_MEM_SET((ZL_VOID *)(run->mempool.points + (run->mempool.size - ZL_R_MEM_POOL_SIZE)),0,
						  (ZL_SIZE_T)increase);
	}
	if(run->memfreepool.count == 0 && run->mempool.points[run->mempool.count].isvalid == ZL_FALSE &&
		run->mempool.points[run->mempool.count].point == ZL_NULL) //���û�ҵ��ͷŹ����ڴ棬��ʹ���ڴ�������һ��Ԫ�صĺ�����Ϊ������run->mempool.count
	{
		i = run->mempool.count;
		goto alloc_point;
	}
	else if(run->memfreepool.count > 0 && 
		   (i = run->memfreepool.frees[run->memfreepool.count - 1]) > 0) //�ҵ��ͷŹ����ڴ棬��ֱ�ӻ������á�
	{
		run->memfreepool.frees[run->memfreepool.count - 1] = 0;
		run->memfreepool.count--;
		if(run->mempool.points[i].point == ZL_NULL && run->mempool.points[i].isvalid == ZL_FALSE) //�����Ԫ����ĳЩδ֪ԭ�򲻿��ã���ֻ�����·��䡣
			goto alloc_point;
		else if(run->mempool.points[i].point != ZL_NULL && run->mempool.points[i].isvalid == ZL_FALSE)
		{
			if(run->mempool.points[i].size < size) //���ԭ���ͷŹ����ڴ��С��������Ҫ����ֻ�����·���
			{
				ZENGL_SYS_MEM_FREE(run->mempool.points[i].point);
				increase -= run->mempool.points[i].size;
				goto alloc_point;
			}
			else if(run->mempool.points[i].size >= size &&
					run->mempool.points[i].size <= (size*3/2)) //���ԭ���ͷŹ����ڴ��С���ں�����Ҫ�Ĵ�С��1��1.5���Ļ�����ֱ�������á�
			{
				(*index) = i;
				run->mempool.points[i].isvalid = ZL_TRUE;
				run->mempool.count++;
				return run->mempool.points[i].point;
			}
			else //���ԭ�ڴ��С���������1.5����������ʱ����ʹ��realloc�����ڴ����Ĵ�С��
			{
				(*index) = i;
				run->mempool.points[i].isvalid = ZL_TRUE;
				run->mempool.points[i].point = ZENGL_SYS_MEM_RE_ALLOC(run->mempool.points[i].point,size);
				increase += size - run->mempool.points[i].size;
				run->mempool.points[i].size = size;
				run->mempool.count++;
				run->mempool.totalsize += increase;
				run->totalsize += increase;
				VM->totalsize += increase;
				return run->mempool.points[i].point;
			}
		}
	}
	for(i=0;i<run->mempool.size;i++) //ֻ��������ķ������Ҳ���ʱ����ѭ�����ҿ����õ��ڴ��������
	{
		if(run->mempool.points[i].point == ZL_NULL && run->mempool.points[i].isvalid == ZL_FALSE)
		{
alloc_point:
			point = ZENGL_SYS_MEM_ALLOC(size);
			if(point != ZL_NULL)
			{
				run->mempool.points[i].point = point;
				run->mempool.points[i].size = size;
				run->mempool.points[i].isvalid = ZL_TRUE;
				(*index) = i;
				run->mempool.count++;
				increase += size;
				run->mempool.totalsize += increase;
				run->totalsize += increase;
				VM->totalsize += increase;
				return point;
			}
			else
				run->exit(VM_ARG,ZL_ERR_RUN_MEM_ALLOC_FUNC_FAILED);
		}
	}
	run->exit(VM_ARG,ZL_ERR_RUN_MEM_ALLOC_FUNC_CAN_NOT_FIND_NULL_POINT);
	return ZL_NULL;
}

/*
	����pointָ��Ĵ�СΪsize��ͬʱ����indexΪ��ָ�����ڴ���е�������
*/
ZL_VOID * zenglrun_memReAlloc(ZL_VOID * VM_ARG , ZL_VOID * point , ZL_INT size, ZL_INT * index)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZL_INT i;
	ZL_INT increase = 0;
	if(point == ZL_NULL || (i = (*index)) <= 0) //��pointΪNULL������С�ڵ���0�����·���һ��ָ����ڴ档
	{
		point = run->memAlloc(VM_ARG,size,index);
		return point;
	}

	if(i > 0 && run->mempool.points[i].point == point && run->mempool.points[i].isvalid == ZL_TRUE) //���index����ָ����ڴ��ָ�����point��������ѭ��������ֱ�ӽ���realloc������
		goto realloc_point;
	for(i=0;i<run->mempool.size;i++) //���򣬾���Ҫѭ�������ڴ��
	{
		if(run->mempool.points[i].point == point)
		{
			if(run->mempool.points[i].isvalid == ZL_FALSE)
				run->exit(VM_ARG,ZL_ERR_RUN_MEM_RE_ALLOC_INVALID_POINT);
realloc_point:
			point = ZENGL_SYS_MEM_RE_ALLOC(point,size);
			if(point != ZL_NULL)
			{
				run->mempool.points[i].point = point;
				increase += size - run->mempool.points[i].size;
				run->mempool.points[i].size = size;
				(*index) = i;
				run->mempool.totalsize += increase;
				run->totalsize += increase;
				VM->totalsize += increase;
				return point;
			}
			else
				run->exit(VM_ARG,ZL_ERR_RUN_MEM_RE_ALLOC_FUNC_FAILED);
		}
	}
	point = run->memAlloc(VM_ARG,size,index); //���û�ҵ��������·��䡣
	return point;
}

/*����pointָ����ڴ���в���ָ���Ӧ������ֵ*/
ZL_INT zenglrun_memFindPtrIndex(ZL_VOID * VM_ARG , ZL_VOID * point)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZL_INT i;

	for(i=0;i<run->mempool.size;i++)
	{
		if(run->mempool.points[i].point == point)
			return i;
	}
	return -1;
}

/*
	���������size��С�����ڴ�ص�ָ����е�����С�������ò�������memReAlloc�������ڣ�������ֻ������С��û���ڴ�copy�Ŀ���
*/
ZL_VOID * zenglrun_memReUsePtr(ZL_VOID * VM_ARG,ZL_VOID * point,ZL_INT size,ZL_INT * index)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZL_INT i;
	ZL_INT increase = 0;
	if(point == ZL_NULL || (i = (*index)) <= 0) //��pointΪNULL������С�ڵ���0�����·���һ��ָ����ڴ档
	{
		point = run->memAlloc(VM_ARG,size,index);
		return point;
	}

	if(i > 0 && run->mempool.points[i].point == point && run->mempool.points[i].isvalid == ZL_TRUE) //���index����ָ����ڴ��ָ�����point����ֱ�ӵ���
	{
		if(run->mempool.points[i].size < size) //ԭʼ��СС�����������·���
		{
			ZENGL_SYS_MEM_FREE(run->mempool.points[i].point);
			point = ZENGL_SYS_MEM_ALLOC(size);
			if(point != ZL_NULL)
			{
				run->mempool.points[i].point = point;
				increase += size - run->mempool.points[i].size;
				run->mempool.points[i].size = size;
				run->mempool.totalsize += increase;
				run->totalsize += increase;
				VM->totalsize += increase;
				return run->mempool.points[i].point;
			}
			else
				run->exit(VM_ARG,ZL_ERR_RUN_MEM_RE_USE_PTR_MALLOC_FAILED);
		}
		else if(run->mempool.points[i].size >= size && 
				run->mempool.points[i].size <= (size*3/2)) //���������1��1.5��������Ҫ����ֱ�ӷ��ء�
		{
			return run->mempool.points[i].point;
		}
		else //������realloc�����ڴ��С��realloc�ڴӴ�ĵ�Сʱ��������������ķ��䣬ֻ���޸��ڴ�Ĵ�С������һ���ڴ�ָ�벻�ᷢ���仯��Ҳ������copy������
		{
			run->mempool.points[i].point = ZENGL_SYS_MEM_RE_ALLOC(run->mempool.points[i].point , size);
			increase += size - run->mempool.points[i].size;
			run->mempool.points[i].size = size;
			run->mempool.totalsize += increase;
			run->totalsize += increase;
			VM->totalsize += increase;
			return run->mempool.points[i].point;
		}
	}
	point = run->memAlloc(VM_ARG,size,index);
	return point;
}

/*
	�������˳�����
*/
ZL_VOID zenglrun_exit(ZL_VOID * VM_ARG,ZENGL_ERRORNO errorno, ...)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_COMPILE_TYPE * compile = &VM->compile;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZENGL_SYS_ARG_LIST arg;
	ZL_INT error_nodenum = 0;
	ZL_INT isNeedDebugInfo = ZL_EXP_CP_AF_IN_DEBUG_MODE | ZL_EXP_CP_AF_OUTPUT_DEBUG_INFO;
	if(compile->isinCompiling) //������ڱ����������У�����������������������ָ��ʱ����ʱ����Ҫ���ͷŽ����������Դ����Ҫ���ñ��������˳����������ͷű���������Դ
	{
		VM->errorno = errorno;
		if(VM->errorno != ZL_NO_ERR_SUCCESS)
		{
			ZENGL_SYS_ARG_START(arg,errorno);
			compile->makeInfoString(VM_ARG,&compile->errorFullString, VM->errorString[VM->errorno] , arg);
			compile->makeInfoString(VM_ARG,&compile->errorFullString, ": " , arg);
			ZENGL_SYS_ARG_END(arg);
		}
		if(VM->isinApiRun == ZL_FALSE) //�����zenglApi_Run���еĽű������ڴ��ͷ���Դ������ͨ��zenglApi_Close���ͷ�
		{
			run->memFreeAll(VM_ARG);
			run->isDestroyed = ZL_TRUE;
			compile->exit(VM_ARG,ZL_ERR_RUN_ERROR_EXIT_WHEN_IN_COMPILE);
		}
	}
	else
	{
		VM->errorno = errorno;
		if(VM->errorno != ZL_NO_ERR_SUCCESS)
		{
			ZENGL_SYS_ARG_START(arg,errorno);
			run->makeInfoString(VM_ARG,&run->errorFullString , VM->errorString[VM->errorno] , arg);
			if(run->isinRunning == ZL_FALSE) //���û��������ִ��ǰ��ͨ��API�ӿ�����һЩ���������ʱ���������������ô��ʱ��run->isinRunning����ZL_FALSE״̬����ʱֱ����������λ��
			{
				ZENGL_SYS_ARG_END(arg);
				goto zenglapi_goto_end;
			}
			if((VM->vm_main_args->flags & ZL_EXP_CP_AF_IN_DEBUG_MODE) != 0) //�û��Զ���ĵ���ģʽ�£���ӡ���ڵ�����к���Ϣ
			{
				error_nodenum = run->inst_list.insts[ZL_R_REG_PC].nodenum;
				run->ExitPrintSourceInfo(VM_ARG,ZL_ERR_RUN_SOURCE_CODE_INFO,
					compile->getTokenStr(VM_ARG,compile->AST_nodes.nodes,error_nodenum),
					compile->AST_nodes.nodes[error_nodenum].line_no,
					compile->AST_nodes.nodes[error_nodenum].col_no,
					compile->AST_nodes.nodes[error_nodenum].filename);
				if(!compile->isDestroyed && VM->isinApiRun == ZL_FALSE)
				{
					compile->memFreeAll(VM_ARG);
					compile->infoFullString.str = compile->errorFullString.str = ZL_NULL;
					compile->isDestroyed = ZL_TRUE;
				}
			}
			if(run->userdef_run_error != ZL_NULL)
				run->userdef_run_error(run->errorFullString.str,run->errorFullString.count,VM_ARG);
			if(VM->isinApiRun == ZL_FALSE)
				run->freeInfoString(VM_ARG,&run->errorFullString);
			ZENGL_SYS_ARG_END(arg);
		}
		else
		{
			if(!compile->isDestroyed && VM->isinApiRun == ZL_FALSE)
			{
				compile->memFreeAll(VM_ARG);
				compile->infoFullString.str = compile->errorFullString.str = ZL_NULL;
				compile->isDestroyed = ZL_TRUE;
			}
		}
		run->end_time = ZENGL_SYS_TIME_CLOCK();
		run->total_time = run->end_time - run->start_time;
		if(VM->errorno != ZL_NO_ERR_SUCCESS)
			VM->isRunError = ZL_TRUE;
		if((VM->vm_main_args->flags & isNeedDebugInfo) == isNeedDebugInfo) //�û��Զ���ĵ���ģʽ��
		{
			ZL_INT run_totalsize;
			ZL_INT vm_totalsize;
			run_totalsize = VM->debug.orig_run_totalsize != 0 ? VM->debug.orig_run_totalsize : run->totalsize;
			vm_totalsize = VM->debug.orig_vm_totalsize != 0 ? VM->debug.orig_vm_totalsize : VM->totalsize;
			VM->end_time = ZENGL_SYS_TIME_CLOCK();
			VM->total_time = VM->end_time - VM->start_time; //�õ�������ܵ�ִ��ʱ��
			run->info(VM_ARG,"\n run time:%.16g s totalsize: %.16g Kbyte\n VM time:%.16g s totalsize: %.16g Kbyte\n",(ZL_DOUBLE)run->total_time / CLOCKS_PER_SEC,
			(ZL_FLOAT)run_totalsize / 1024,
			(ZL_DOUBLE)VM->total_time / CLOCKS_PER_SEC,
			(ZL_FLOAT)vm_totalsize / 1024); //for debug
		}
		if(VM->isinApiRun == ZL_FALSE)
		{
			run->memFreeAll(VM_ARG);
			run->isDestroyed = ZL_TRUE;
			run->infoFullString.str = run->errorFullString.str = run->printFullString.str = ZL_NULL;
		}
		run->isinRunning = ZL_FALSE;
zenglapi_goto_end: //API�ӿڳ���ʱ��ֱ����ת���������
		if(VM->errorno == ZL_NO_ERR_SUCCESS)
			ZENGL_SYS_JMP_LONGJMP_TO(run->jumpBuffer,1);
		else
			ZENGL_SYS_JMP_LONGJMP_TO(run->jumpBuffer,-1);
	}
}

/*ר��ΪzenglApi_SetErrThenStop���API�ӿڶ��Ƶ��˳�����*/
ZL_VOID zenglrun_exit_forApiSetErrThenStop(ZL_VOID * VM_ARG)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_COMPILE_TYPE * compile = &VM->compile;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZL_INT isNeedDebugInfo = ZL_EXP_CP_AF_IN_DEBUG_MODE | ZL_EXP_CP_AF_OUTPUT_DEBUG_INFO;

	if((VM->vm_main_args->flags & ZL_EXP_CP_AF_IN_DEBUG_MODE) != 0) //�û��Զ���ĵ���ģʽ�£���ӡ���ڵ�����к���Ϣ
	{
		if(!compile->isDestroyed && VM->isinApiRun == ZL_FALSE)
		{
			compile->memFreeAll(VM_ARG);
			compile->infoFullString.str = compile->errorFullString.str = ZL_NULL;
			compile->isDestroyed = ZL_TRUE;
		}
	}
	
	run->end_time = ZENGL_SYS_TIME_CLOCK();
	run->total_time = run->end_time - run->start_time;
	VM->isRunError = ZL_TRUE;
	if((VM->vm_main_args->flags & isNeedDebugInfo) == isNeedDebugInfo) //�û��Զ���ĵ���ģʽ��
	{
		ZL_INT run_totalsize;
		ZL_INT vm_totalsize;
		run_totalsize = VM->debug.orig_run_totalsize != 0 ? VM->debug.orig_run_totalsize : run->totalsize;
		vm_totalsize = VM->debug.orig_vm_totalsize != 0 ? VM->debug.orig_vm_totalsize : VM->totalsize;
		VM->end_time = ZENGL_SYS_TIME_CLOCK();
		VM->total_time = VM->end_time - VM->start_time; //�õ�������ܵ�ִ��ʱ��
		run->info(VM_ARG,"\n run time:%.16g s totalsize: %.16g Kbyte\n VM time:%.16g s totalsize: %.16g Kbyte\n",(ZL_DOUBLE)run->total_time / CLOCKS_PER_SEC,
		(ZL_FLOAT)run_totalsize / 1024,
		(ZL_DOUBLE)VM->total_time / CLOCKS_PER_SEC,
		(ZL_FLOAT)vm_totalsize / 1024); //for debug
	}
	if(VM->isinApiRun == ZL_FALSE)
	{
		run->memFreeAll(VM_ARG);
		run->isDestroyed = ZL_TRUE;
		run->infoFullString.str = run->errorFullString.str = run->printFullString.str = ZL_NULL;
	}
	run->isinRunning = ZL_FALSE;
	ZENGL_SYS_JMP_LONGJMP_TO(run->jumpBuffer,-1);
}

/*
	�������������˳�ʱ����ӡ����ǰ�������Ӧ��AST�ڵ�����к��ļ�����Ϣ
*/
ZL_VOID zenglrun_ExitPrintSourceInfo(ZL_VOID * VM_ARG,ZENGL_ERRORNO errorno, ...)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_RUN_TYPE * run = &VM->run;
	ZENGL_SYS_ARG_LIST arg;
	if(errorno != ZL_NO_ERR_SUCCESS)
	{
		ZENGL_SYS_ARG_START(arg,errorno);
		run->makeInfoString(VM_ARG,&run->errorFullString , VM->errorString[errorno] , arg);
		ZENGL_SYS_ARG_END(arg);
	}
}

/*�������ͷ��ڴ����Դ*/
ZL_VOID zenglrun_memFreeAll(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT i;
	if(run->mempool.isInit)
	{
		for(i=0;i<run->mempool.size;i++)
		{
			if(run->mempool.points[i].point != ZL_NULL)
				ZENGL_SYS_MEM_FREE(run->mempool.points[i].point);
		}
		ZENGL_SYS_MEM_FREE(run->mempool.points);
		run->mempool.isInit = ZL_FALSE;
	}
}

/*ר����������API������Ϣ����չ����*/
ZL_CHAR * zenglrun_SetApiErrorEx(ZL_VOID * VM_ARG,ZENGL_ERRORNO errorno , ...)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	ZENGL_SYS_ARG_LIST arglist;
	ZENGL_SYS_ARG_START(arglist,errorno);
	VM->run.SetApiError(VM_ARG,errorno,arglist);
	ZENGL_SYS_ARG_END(arglist);
	return VM->ApiError;
}

/*ר����������API������Ϣ�ĺ���*/
ZL_CHAR * zenglrun_SetApiError(ZL_VOID * VM_ARG,ZENGL_ERRORNO errorno ,ZENGL_SYS_ARG_LIST arglist)
{
	ZENGL_VM_TYPE * VM = (ZENGL_VM_TYPE *)VM_ARG;
	if(VM->ApiError != ZL_NULL)
		VM->run.memFreeOnce(VM_ARG,VM->ApiError);
	VM->errorno = errorno;
	VM->run.makeInfoString(VM_ARG,&VM->run.errorFullString , VM->errorString[VM->errorno] , arglist);
	VM->ApiError = zenglApi_AllocMemForString(VM_ARG, VM->run.errorFullString.str);
	VM->run.freeInfoString(VM_ARG,&VM->run.errorFullString);
	return VM->ApiError;
}

/*���ɸ�ʽ����Ϣ�ַ���*/
ZL_CHAR * zenglrun_makeInfoString(ZL_VOID * VM_ARG,ZENGL_RUN_INFO_STRING_TYPE * infoStringPtr , ZL_CONST ZL_CHAR * format , ZENGL_SYS_ARG_LIST arglist)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT retcount = -1;
	if(infoStringPtr->str == ZL_NULL)
	{
		infoStringPtr->size = ZL_INFO_STRING_SIZE;
		infoStringPtr->str = run->memAlloc(VM_ARG,infoStringPtr->size * sizeof(ZL_CHAR),&infoStringPtr->mempool_index);
	}
	while(ZL_TRUE)
	{
		retcount = ZENGL_SYS_SPRINTF_ARG_NUM((infoStringPtr->str + infoStringPtr->cur),
											 (infoStringPtr->size - infoStringPtr->count),format,arglist);

		if(retcount >= 0 && retcount < (infoStringPtr->size - infoStringPtr->count))
		{
			infoStringPtr->count += retcount;
			infoStringPtr->cur = infoStringPtr->count;
			infoStringPtr->str[infoStringPtr->cur] = ZL_STRNULL;
			return infoStringPtr->str;
		}

		infoStringPtr->size += ZL_INFO_STRING_SIZE;
		infoStringPtr->str = run->memReAlloc(VM_ARG,infoStringPtr->str,infoStringPtr->size * sizeof(ZL_CHAR),&infoStringPtr->mempool_index);
	}
	return ZL_NULL;
}

/*�ͷŸ�ʽ����Ϣ�ַ������α�ʹ�С��Ϣ*/
ZL_VOID zenglrun_freeInfoString(ZL_VOID * VM_ARG,ZENGL_RUN_INFO_STRING_TYPE * infoStringPtr)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	infoStringPtr->cur = infoStringPtr->count = 0;
	if(infoStringPtr->size > ZL_INFO_STRING_SIZE * 3) //�������������С3������ô������3��������ռ�ù�����Դ
	{
		infoStringPtr->size = ZL_INFO_STRING_SIZE * 3;
		infoStringPtr->str = run->memReAlloc(VM_ARG,infoStringPtr->str,infoStringPtr->size * sizeof(ZL_CHAR),&infoStringPtr->mempool_index);
	}
}

/*
	����infoFullString��errorFullString�ĸ�ʽ���ַ�����Ϣ����ʹ���û��Զ��庯�����������ʾ
*/
ZL_VOID zenglrun_info(ZL_VOID * VM_ARG , ZL_CONST ZL_CHAR * format , ...)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_SYS_ARG_LIST arg;
	ZENGL_SYS_ARG_START(arg,format);
	if(run->errorFullString.count > 0 && ((ZENGL_VM_TYPE *)VM_ARG)->isRunError == ZL_FALSE) //��exit�����˳�ʱ�����������������Ϣʱ��ʹ��errorFullString�������
	{
		run->makeInfoString(VM_ARG,&run->errorFullString,format,arg);
	}
	else //����ʹ����ͨ��ʽ�������
	{
		run->makeInfoString(VM_ARG,&run->infoFullString,format,arg);
		if(run->userdef_run_info != ZL_NULL)
			run->userdef_run_info(run->infoFullString.str , run->infoFullString.count , VM_ARG);
		run->freeInfoString(VM_ARG,&run->infoFullString);
	}
	ZENGL_SYS_ARG_END(arg);
}

/*
	����printFullString�ĸ�ʽ���ַ�����Ϣ����ʹ���û��Զ��庯�������ʾ
*/
ZL_VOID zenglrun_print(ZL_VOID * VM_ARG , ZL_CONST ZL_CHAR * format , ...)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_SYS_ARG_LIST arg;
	ZENGL_SYS_ARG_START(arg,format);
	run->makeInfoString(VM_ARG,&run->printFullString,format,arg);
	if(run->userdef_run_print != ZL_NULL)
		run->userdef_run_print(run->printFullString.str , run->printFullString.count , VM_ARG);
	run->freeInfoString(VM_ARG,&run->printFullString);
	ZENGL_SYS_ARG_END(arg);
}

/*
	ָ��������ַ���������ַ���
*/
ZL_INT zenglrun_InstDataStringPoolAdd(ZL_VOID * VM_ARG , ZL_CHAR * src)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT len;
	ZL_INT i,j;
	if(!run->InstData_StringPool.isInit)
		run->initInstDataStringPool(VM_ARG);
	if(src == ZL_NULL)
		return -1;
	len = ZENGL_SYS_STRLEN(src);
	if(run->InstData_StringPool.count == run->InstData_StringPool.size ||
		run->InstData_StringPool.count + len + 1 > run->InstData_StringPool.size)
	{
		run->InstData_StringPool.size += ZL_R_INST_DATA_STRING_POOL_SIZE;
		run->InstData_StringPool.ptr = run->memReAlloc(VM_ARG,run->InstData_StringPool.ptr,
												run->InstData_StringPool.size * sizeof(ZL_CHAR),
												&run->InstData_StringPool.mempool_index);
		ZENGL_SYS_MEM_SET(run->InstData_StringPool.ptr + (run->InstData_StringPool.size - ZL_R_INST_DATA_STRING_POOL_SIZE),0,
						ZL_R_INST_DATA_STRING_POOL_SIZE * sizeof(ZL_CHAR));
	}
	for(i=run->InstData_StringPool.count,j=0;
		i<run->InstData_StringPool.size && j<len;i++,j++)
	{
		run->InstData_StringPool.ptr[i] = src[j];
	}
	if(i >= run->InstData_StringPool.size)
		run->exit(VM_ARG,ZL_ERR_RUN_INST_DATA_STR_POOL_ADD_I_OUT_OF_BOUNDS);
	else
		run->InstData_StringPool.ptr[i] = ZL_STRNULL;
	i = run->InstData_StringPool.count;
	run->InstData_StringPool.count += len + 1;
	return i;
}

/*
	ָ��������ַ����صĳ�ʼ��
*/
ZL_VOID zenglrun_initInstDataStringPool(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(run->InstData_StringPool.isInit)
		return;
	run->InstData_StringPool.count = 2;
	run->InstData_StringPool.size = ZL_R_INST_DATA_STRING_POOL_SIZE;
	run->InstData_StringPool.ptr = run->memAlloc(VM_ARG,run->InstData_StringPool.size * sizeof(ZL_CHAR),
													&run->InstData_StringPool.mempool_index);
	if(run->InstData_StringPool.ptr == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_RUN_INST_DATA_STRING_POOL_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(run->InstData_StringPool.ptr, 0 , run->InstData_StringPool.size * sizeof(ZL_CHAR));
	run->InstData_StringPool.isInit = ZL_TRUE;
}

/*
	��������ֵ���ַ������в���ָ��������ַ�����ָ����Ϣ
*/
ZL_CHAR * zenglrun_InstDataStringPoolGetPtr(ZL_VOID * VM_ARG , ZL_INT index)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(index < 2 || index > run->InstData_StringPool.count -1)
		return ZL_NULL;
	return run->InstData_StringPool.ptr + index;
}

/*
	ʹ��C���strcat���ƻ��ڴ�أ����Ծ��Խ�һ��zenglrun_strcat�������ַ����õģ�ͬʱ�ڱ�Ҫʱ������ڴ��ָ��Ӷ����ƻ��ڴ�ء�
*/
ZL_VOID * zenglrun_strcat(ZL_VOID * VM_ARG,ZL_CHAR * dest,ZL_INT * destIndex,ZL_CHAR * src)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT lensrc;
	ZL_INT lendest;
	ZL_INT index,i;

	if(dest == ZL_NULL)
		lendest = 0;
	else
		lendest = ZENGL_SYS_STRLEN(dest);
	if(src == ZL_NULL)
		lensrc = 0;
	else
		lensrc = ZENGL_SYS_STRLEN(src);
	dest = run->memReAlloc(VM_ARG,dest,(lendest+lensrc+1) * sizeof(ZL_CHAR),destIndex);
	for(index = lendest,i=0;index< (lendest + lensrc) && i<lensrc ;index++,i++)
	{
		dest[index] = src[i];
	}
	dest[index] = ZL_STRNULL;
	return dest;
}

/*
	�ͷ��ڴ����ָ��������Ӧ��ָ�롣
	���ͷŹ���������ŵ�memfreepool��frees��̬������(�Ա㽫����������)��
*/
ZL_VOID zenglrun_memFreeIndex(ZL_VOID * VM_ARG,ZL_VOID * point,ZL_INT * srcindex)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT index = (*srcindex);
	if(run->mempool.points[index].isvalid == ZL_TRUE && run->mempool.points[index].point == point)
	{
		run->mempool.points[index].isvalid = ZL_FALSE; //���ڴ�ظ���������Чλ��ΪZL_FALSE
		run->mempool.count--;
		run->mempool.count = run->mempool.count < 1 ? 1 : run->mempool.count; //����0������Ч�Ľ������ڴ������������СΪ1
		if(run->memfreepool.count == run->memfreepool.size)
		{
			run->memfreepool.size += ZL_R_MEM_FREE_POOL_SIZE;
			run->memfreepool.frees = run->memReAlloc(VM_ARG,run->memfreepool.frees,run->memfreepool.size * sizeof(ZL_INT),
													&run->memfreepool.mempool_index);
			ZENGL_SYS_MEM_SET((ZL_VOID *)(run->memfreepool.frees + (run->memfreepool.size - ZL_R_MEM_FREE_POOL_SIZE)),0,
					(ZL_SIZE_T)(ZL_R_MEM_FREE_POOL_SIZE * sizeof(ZL_INT)));
		}
		run->memfreepool.frees[run->memfreepool.count++] = index;
		(*srcindex) = 0;
		return;
	}
	else //�������ͨ��indexֱ�Ӷ�λ���ڴ�صľ���λ�ã���ֻ��ͨ��memFreeOnce��������ѭ�������ڴ�ء�
	{
		run->memFreeOnce(VM_ARG,point);
		(*srcindex) = 0;
	}
}

/*
	�ͷ��ڴ���е�ĳ��ָ�룬������ָ���������ŵ�memfreepool��frees��̬������(�Ա㽫����������)��
*/
ZL_VOID zenglrun_memFreeOnce(ZL_VOID * VM_ARG,ZL_VOID * point)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT i = 0;
	for(;i < run->mempool.size;i++)
	{
		if(run->mempool.points[i].isvalid == ZL_TRUE && run->mempool.points[i].point == point)
		{
			run->mempool.points[i].isvalid = ZL_FALSE;
			run->mempool.count--;
			run->mempool.count = run->mempool.count < 1 ? 1 : run->mempool.count; //����0������Ч�Ľ������ڴ������������СΪ1
			if(run->memfreepool.count == run->memfreepool.size)
			{
				run->memfreepool.size += ZL_R_MEM_FREE_POOL_SIZE;
				run->memfreepool.frees = run->memReAlloc(VM_ARG,run->memfreepool.frees,run->memfreepool.size * sizeof(ZL_INT),
														&run->memfreepool.mempool_index);
				ZENGL_SYS_MEM_SET((ZL_VOID *)(run->memfreepool.frees + (run->memfreepool.size - ZL_R_MEM_FREE_POOL_SIZE)),0,
						(ZL_SIZE_T)(ZL_R_MEM_FREE_POOL_SIZE * sizeof(ZL_INT)));
			}
			run->memfreepool.frees[run->memfreepool.count++] = i;
			break;
		}
	}
}

/*
	��src1��src2���ַ���������һ������dest��
*/
ZL_VOID * zenglrun_strcat2(ZL_VOID * VM_ARG,ZL_CHAR * dest,ZL_INT * destIndex,ZL_CHAR * src1,ZL_CHAR * src2)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT lensrc1,lensrc2;
	ZL_CHAR * tmpdest;
	if(src1 == ZL_NULL)
		lensrc1 = 0;
	else
		lensrc1 = ZENGL_SYS_STRLEN(src1);
	if(src2 == ZL_NULL)
		lensrc2 = 0;
	else
		lensrc2 = ZENGL_SYS_STRLEN(src2);
	dest = run->memReUsePtr(VM_ARG,dest,(lensrc1+lensrc2+1) * sizeof(ZL_CHAR),destIndex);
	ZENGL_SYS_STRNCPY(dest,src1,lensrc1);
	tmpdest = dest + lensrc1;
	ZENGL_SYS_STRNCPY(tmpdest,src2,lensrc2);
	dest[lensrc1+lensrc2] = ZL_STRNULL;
	return dest;
}

/*
	���ڼ���������еĹ�ϣ������
*/
ZL_INT zenglrun_Hash(ZL_VOID * VM_ARG,ZL_CHAR * key)
{
	ZL_INT temp = 0;
	ZL_INT i = 0;
	while (key[i] != ZL_STRNULL)
	{
		temp = ((temp << ZL_R_HASH_SHIFT) + key[i]) % ZL_R_HASH_SIZE;
		++i;
	}
	return temp;
}

/*
	��ĳģ�������뵽ģ�鶯̬������
*/
ZL_INT zenglrun_InsertModuleTable(ZL_VOID * VM_ARG,ZL_CHAR * moduleName,ZL_VM_API_MOD_INIT_FUNC module_init_function)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT index;
	if(!run->moduleTable.isInit)
		run->initModuleTable(VM_ARG);
	if(run->moduleTable.count == run->moduleTable.size)
	{
		run->moduleTable.size += ZL_R_MODULE_TABLE_SIZE;
		run->moduleTable.modules = run->memReAlloc(VM_ARG,run->moduleTable.modules,
			run->moduleTable.size * sizeof(ZENGL_RUN_MODULE_TABLE_MEMBER),&run->moduleTable.mempool_index);
		ZENGL_SYS_MEM_SET(run->moduleTable.modules + (run->moduleTable.size - ZL_R_MODULE_TABLE_SIZE),0,
			ZL_R_MODULE_TABLE_SIZE * sizeof(ZENGL_RUN_MODULE_TABLE_MEMBER));
	}
	index = run->moduleTable.count;
	if(run->moduleTable.modules[index].isvalid == ZL_FALSE)
	{
		run->moduleTable.modules[index].ID = index;
		run->moduleTable.modules[index].init_func = module_init_function;
		run->moduleTable.modules[index].strIndex = run->InstDataStringPoolAdd(VM_ARG,moduleName);
		run->moduleTable.modules[index].next = 0;
		run->moduleTable.modules[index].isvalid = ZL_TRUE;
		run->moduleTable.count++;
		return index;
	}
	else
		run->exit(VM_ARG,ZL_ERR_RUN_MODULE_TALBE_NO_NOT_VALID_INDEX);
	return -1;
}

/*
	ģ�鶯̬�����ʼ��
*/
ZL_VOID zenglrun_initModuleTable(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(run->moduleTable.isInit)
		return;
	run->moduleTable.size = ZL_R_MODULE_TABLE_SIZE;
	run->moduleTable.count = 1;
	run->moduleTable.modules = run->memAlloc(VM_ARG,run->moduleTable.size * sizeof(ZENGL_RUN_MODULE_TABLE_MEMBER),&run->moduleTable.mempool_index);
	if(run->moduleTable.modules == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_RUN_MODULE_TALBE_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(run->moduleTable.modules,0,run->moduleTable.size * sizeof(ZENGL_RUN_MODULE_TABLE_MEMBER));
	run->moduleTable.isInit = ZL_TRUE;
}

/*
	��ģ�麯����Ϣ���뵽ģ�麯����̬������
*/
ZL_INT zenglrun_InsertModFunTable(ZL_VOID * VM_ARG,ZL_INT moduleID,ZL_CHAR * functionName,ZL_VM_API_MOD_FUN_HANDLE handle)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT index;
	if(!run->ModFunTable.isInit)
		run->initModFunTable(VM_ARG);
	if(run->ModFunTable.count == run->ModFunTable.size)
	{
		run->ModFunTable.size += ZL_R_MOD_FUN_TABLE_SIZE;
		run->ModFunTable.mod_funs = run->memReAlloc(VM_ARG,run->ModFunTable.mod_funs,
			run->ModFunTable.size * sizeof(ZENGL_RUN_MOD_FUN_TABLE_MEMBER),&run->ModFunTable.mempool_index);
		ZENGL_SYS_MEM_SET(run->ModFunTable.mod_funs + (run->ModFunTable.size - ZL_R_MOD_FUN_TABLE_SIZE),0,
			ZL_R_MOD_FUN_TABLE_SIZE * sizeof(ZENGL_RUN_MOD_FUN_TABLE_MEMBER));
	}
	index = run->ModFunTable.count;
	if(run->ModFunTable.mod_funs[index].isvalid == ZL_FALSE)
	{
		run->ModFunTable.mod_funs[index].moduleID = moduleID;
		run->ModFunTable.mod_funs[index].strIndex = run->InstDataStringPoolAdd(VM_ARG,functionName);
		run->ModFunTable.mod_funs[index].handle = handle;
		run->ModFunTable.mod_funs[index].next = 0;
		run->ModFunTable.mod_funs[index].isvalid = ZL_TRUE;
		run->ModFunTable.count++;
		return index;
	}
	else
		run->exit(VM_ARG,ZL_ERR_RUN_MOD_FUN_TABLE_NO_NOT_VALID_INDEX);
	return -1;
}

/*
	ģ�麯����̬�����ʼ��
*/
ZL_VOID zenglrun_initModFunTable(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(run->ModFunTable.isInit)
		return;
	run->ModFunTable.size = ZL_R_MOD_FUN_TABLE_SIZE;
	run->ModFunTable.count = 1;
	run->ModFunTable.mod_funs = run->memAlloc(VM_ARG,run->ModFunTable.size * sizeof(ZENGL_RUN_MOD_FUN_TABLE_MEMBER),&run->ModFunTable.mempool_index);
	if(run->ModFunTable.mod_funs == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_RUN_MOD_FUN_TABLE_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(run->ModFunTable.mod_funs,0,run->ModFunTable.size * sizeof(ZENGL_RUN_MOD_FUN_TABLE_MEMBER));
	run->ModFunTable.isInit = ZL_TRUE;
}

/*
	����ĳģ�����Ϣ�����ظ�ģ����ģ�鶯̬�����е�����
*/
ZL_INT zenglrun_LookupModuleTable(ZL_VOID * VM_ARG,ZL_CHAR * moduleName)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT h = run->Hash(VM_ARG,moduleName) + ZL_R_HASH_SIZE * ZL_R_HASH_TYPE_MODULE_TABLE;
	ZL_INT tmpindex = run->HashTable[h];

	while(tmpindex != 0 && run->moduleTable.modules[tmpindex].isvalid == ZL_TRUE && 
		ZENGL_SYS_STRCMP(moduleName,run->InstDataStringPoolGetPtr(VM_ARG,run->moduleTable.modules[tmpindex].strIndex)) != 0)
		tmpindex = run->moduleTable.modules[tmpindex].next;
	if(tmpindex == 0)
		run->exit(VM_ARG,ZL_ERR_RUN_MODULE_IS_INVALID,moduleName,ZL_R_REG_PC,moduleName);
	else if(run->moduleTable.modules[tmpindex].isvalid == ZL_FALSE)
		run->exit(VM_ARG,ZL_ERR_RUN_MODULE_TABLE_FIND_INVALID_INDEX);
	else
		return tmpindex;
	return -1;
}

/*
	����ĳģ�麯������Ϣ�����ظ�ģ�麯���ڶ�̬�����е�����
*/
ZL_INT zenglrun_LookupModFunTable(ZL_VOID * VM_ARG,ZL_CHAR * functionName)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT h = run->Hash(VM_ARG,functionName) + ZL_R_HASH_SIZE * ZL_R_HASH_TYPE_MOD_FUN_TABLE;
	ZL_INT tmpindex = run->HashTable[h];

	while(tmpindex != 0 && run->ModFunTable.mod_funs[tmpindex].isvalid == ZL_TRUE && 
		ZENGL_SYS_STRCMP(functionName,run->InstDataStringPoolGetPtr(VM_ARG,run->ModFunTable.mod_funs[tmpindex].strIndex)) != 0)
		tmpindex = run->ModFunTable.mod_funs[tmpindex].next;
	if(tmpindex == 0)
		run->exit(VM_ARG,ZL_ERR_RUN_FUNCTION_IS_INVALID,functionName,ZL_R_REG_PC,functionName);
	else if(run->ModFunTable.mod_funs[tmpindex].isvalid == ZL_FALSE)
		run->exit(VM_ARG,ZL_ERR_RUN_MOD_FUN_TABLE_FIND_INVALID_INDEX);
	else
		return tmpindex;
	return -1;
}

/*��ʼ���û��������ݶ�̬����*/
ZL_VOID zenglrun_initExtraDataTable(ZL_VOID * VM_ARG)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	if(run->ExtraDataTable.isInit)
		return;
	run->ExtraDataTable.size = ZL_R_EXTRA_DATA_TABLE_SIZE;
	run->ExtraDataTable.count = 1;
	run->ExtraDataTable.extras = run->memAlloc(VM_ARG,run->ExtraDataTable.size * sizeof(ZENGL_RUN_EXTRA_DATA_TABLE_MEMBER),&run->ExtraDataTable.mempool_index);
	if(run->ExtraDataTable.extras == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_VM_API_INIT_EXTRA_DATA_TABLE_FAILED);
	ZENGL_SYS_MEM_SET(run->ExtraDataTable.extras,0,run->ExtraDataTable.size * sizeof(ZENGL_RUN_EXTRA_DATA_TABLE_MEMBER));
	run->ExtraDataTable.isInit = ZL_TRUE;
}

/*�����ݲ��뵽�������ݶ�̬������*/
ZL_INT zenglrun_InsertExtraDataTable(ZL_VOID * VM_ARG,ZL_CHAR * extraDataName,ZL_VOID * point)
{
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT index;
	if(!run->ExtraDataTable.isInit)
		run->initExtraDataTable(VM_ARG);
	if(run->ExtraDataTable.count == run->ExtraDataTable.size)
	{
		run->ExtraDataTable.size += ZL_R_EXTRA_DATA_TABLE_SIZE;
		run->ExtraDataTable.extras = run->memReAlloc(VM_ARG,run->ExtraDataTable.extras,
			run->ExtraDataTable.size * sizeof(ZENGL_RUN_EXTRA_DATA_TABLE_MEMBER),&run->ExtraDataTable.mempool_index);
		ZENGL_SYS_MEM_SET(run->ExtraDataTable.extras + (run->ExtraDataTable.size - ZL_R_EXTRA_DATA_TABLE_SIZE),0,
			ZL_R_EXTRA_DATA_TABLE_SIZE * sizeof(ZENGL_RUN_EXTRA_DATA_TABLE_MEMBER));
	}
	index = run->ExtraDataTable.count;
	if(extraDataName == ZL_NULL || point == ZL_NULL)
		run->exit(VM_ARG,ZL_ERR_VM_API_INVALID_EXTRA_DATA_NAME_OR_POINT);
	if(run->ExtraDataTable.extras[index].isvalid == ZL_FALSE)
	{
		run->ExtraDataTable.extras[index].strIndex = run->InstDataStringPoolAdd(VM_ARG,extraDataName);
		run->ExtraDataTable.extras[index].point = point;
		run->ExtraDataTable.extras[index].next = 0;
		run->ExtraDataTable.extras[index].isvalid = ZL_TRUE;
		run->ExtraDataTable.count++;
		return index;
	}
	else
		run->exit(VM_ARG,ZL_ERR_VM_API_CAN_NOT_FIND_NOT_VALID_INDEX_IN_EXTRA_DATA_TABLE);
	return -1;
}
