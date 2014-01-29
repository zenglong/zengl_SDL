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
	���ļ�Ϊ�������Ĵ����ļ���
*/

#include "zengl_global.h"

/*
	�齨�����������ʽ
*/
ZL_VOID zengl_buildAsmCode(ZL_VOID * VM_ARG)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_AST_NODE_TYPE * nodes = compile->AST_nodes.nodes;
	ZL_INT tmpNodeNum = compile->AST_nodes.rootnode;
	ZL_INT count = 0;
	compile->gencode_struct.pc = 0;
	if(!(compile->AST_nodes.isInit == ZL_FALSE || compile->AST_nodes.count <= 0)) //����﷨��û��ʼ����Ϊ�գ����ʾ����û���κ�token (������һ���յ��ַ����ű���յĽű��ļ�) ��ֻ���ENDָ��
	{
		while(tmpNodeNum !=0 || count == 0)
		{
			compile->AsmGenCodes(VM_ARG,tmpNodeNum);
			tmpNodeNum = nodes[tmpNodeNum].nextnode;
			count++;
		}
	}
	run->AddInst(VM_ARG,compile->gencode_struct.pc++,compile->AST_nodes.count - 1,
				ZL_R_IT_END,ZL_R_DT_NONE,0,
				ZL_R_DT_NONE,0); //��Ӧ���ָ�� "END"
}

/*
	�ú�������AST�����﷨���Ľڵ�������ĳ�ڵ�תΪ�����롣
	����nodenumΪ�ڵ���AST�﷨����̬������Ľڵ�������
*/
ZL_VOID zengl_AsmGenCodes(ZL_VOID * VM_ARG,ZL_INT nodenum)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_AST_NODE_TYPE * nodes = compile->AST_nodes.nodes;
	ZENGL_RUN_INST_OP_DATA inst_op_data;
	ZL_INT * chnum = ZL_NULL,* extnum = ZL_NULL,i;
	ZL_INT tmpch;
	ZL_INT tmptype;
	ZENGL_STATES state;
	//compile->gencode_struct.state = ZL_ST_START;
	state = ZL_ST_START;
	while(state != ZL_ST_DOWN)
	{
		switch(state)
		{
		case ZL_ST_START:
			switch(nodes[nodenum].toktype)	//����switch...caseЧ�ʸ���Щ��
			{
			case ZL_TK_ASSIGN: //�����ǰ�ڵ��Ǹ�ֵ���ţ��ͽ���ZL_ST_INASSIGN״̬����
				state = ZL_ST_INASSIGN;
				break;
			case ZL_TK_COMMA: //�ж��Ƿ��Ƕ��������
				state = ZL_ST_ASM_CODE_INCOMMA;
				break;
			case ZL_TK_PLUS: //+,-,+=,-=���������ZL_ST_ASM_CODE_INPLUS_MINIS
			case ZL_TK_MINIS:
			case ZL_TK_PLUS_ASSIGN:
			case ZL_TK_MINIS_ASSIGN:
				state = ZL_ST_ASM_CODE_INPLUS_MINIS;
				break;
			case ZL_TK_MOD:
			case ZL_TK_TIMES:
			case ZL_TK_DIVIDE:
			case ZL_TK_MOD_ASSIGN:
			case ZL_TK_TIMES_ASSIGN:
			case ZL_TK_DIVIDE_ASSIGN:
				state = ZL_ST_ASM_CODE_INTIME_DIVIDE;
				break;
			case ZL_TK_GREAT: //���ڣ�С�ڣ����ڣ������ڣ����ڵ��ڣ�С�ڵ�������������ZL_ST_ASM_CODE_INRELATION״̬
			case ZL_TK_LESS:
			case ZL_TK_EQUAL:
			case ZL_TK_NOT_EQ:
			case ZL_TK_GREAT_EQ:
			case ZL_TK_LESS_EQ:
				state = ZL_ST_ASM_CODE_INRELATION;
				break;
			case ZL_TK_AND: //�߼��룬�߼������ZL_ST_ASM_CODE_INAND_OR״̬
			case ZL_TK_OR:
				state = ZL_ST_ASM_CODE_INAND_OR;
				break;
			case ZL_TK_NEGATIVE: //���ŵ�Ŀ�����
				state = ZL_ST_ASM_CODE_INNEGATIVE;
				break;
			case ZL_TK_BIT_REVERSE: //��λȡ�������
				state = ZL_ST_ASM_CODE_INBIT_REVERSE;
				break;
			case ZL_TK_REVERSE: //ȡ�������
				state = ZL_ST_ASM_CODE_INREVERSE;
				break;
			case ZL_TK_ADDRESS: //�����������
				state = ZL_ST_ASM_CODE_INADDRESS;
				break;
			case ZL_TK_FUNCALL: //��������
				state = ZL_ST_ASM_CODE_INFUNCALL;
				break;
			case ZL_TK_ARRAY_ITEM: //�����е�ĳԪ�ء�
				state = ZL_ST_ASM_CODE_INARRAY_ITEM;
				break;
			case ZL_TK_CLASS_STATEMENT: //�������������Poker��һ��class�࣬��ôPoker test; ��������������������ڸ�������������һ��Poker���͵�test��������������е�Poker����CLASS_STATEMENT
				state = ZL_ST_ASM_CODE_INCLASS_STATEMENT;
				break;
			case ZL_TK_DOT: //�����������test.name�������Ա�����þ����õĵ������
				state = ZL_ST_ASM_CODE_INDOT;
				break;
			case ZL_TK_COLON: //�ж��Ƿ���... ? ... : ... �ṹ�е�ð��
				state = ZL_ST_ASM_CODE_INCOLON;
				break;
			case ZL_TK_QUESTION_MARK: //�ж��Ƿ���... ? ... : ... �ṹ�е��ʺ�
				state = ZL_ST_ASM_CODE_INQUESTION;
				break;
			case ZL_TK_BIT_AND: //��λ�룬�����&= ��˫Ŀλ�����
			case ZL_TK_BIT_AND_ASSIGN:
			case ZL_TK_BIT_OR:
			case ZL_TK_BIT_OR_ASSIGN:
			case ZL_TK_BIT_XOR:
			case ZL_TK_BIT_XOR_ASSIGN:
			case ZL_TK_BIT_RIGHT:
			case ZL_TK_BIT_RIGHT_ASSIGN:
			case ZL_TK_BIT_LEFT:
			case ZL_TK_BIT_LEFT_ASSIGN:
				state = ZL_ST_ASM_CODE_INBITS;
				break;
			default:
				if(nodes[nodenum].tokcategory == ZL_TKCG_OP_PP_MM) //�Ӽӣ����������
					state = ZL_ST_ASM_CODE_IN_PP_MM;
				else if(nodes[nodenum].tokcategory == ZL_TKCG_OP_FACTOR) //id������num���֣�float��������str�ַ����Ĳ������ӡ�
				{
					switch(nodes[nodenum].toktype)
					{
					case ZL_TK_ID:
						state = ZL_ST_INID;
						break;
					case ZL_TK_NUM:
						state = ZL_ST_INNUM;
						break;
					case ZL_TK_FLOAT:
						state = ZL_ST_INFLOAT;
						break;
					case ZL_TK_STR:
						state = ZL_ST_INSTR;
						break;
					default:
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_OP_FACTOR);
						break;
					}
				}
				else if(nodes[nodenum].toktype == ZL_TK_RESERVE) //���ֹؼ��֣������֣�
				{
					switch(nodes[nodenum].reserve)
					{
					case ZL_RSV_PRINT: //��ӡ���
						state = ZL_ST_ASM_CODE_INPRINT;
						break;
					case ZL_RSV_IF: //if���
						state = ZL_ST_ASM_CODE_INIF;
						break;
					case ZL_RSV_FOR: //for���
						state = ZL_ST_ASM_CODE_INFOR;
						break;
					case ZL_RSV_FUN: //fun���
						state = ZL_ST_ASM_CODE_INFUN;
						break;
					case ZL_RSV_CLASS: //class���
						state = ZL_ST_ASM_CODE_INCLASS;
						break;
					case ZL_RSV_USE: //use���
						state = ZL_ST_ASM_CODE_INUSE;
						break;
					case ZL_RSV_BREAK: //break���
						state = ZL_ST_ASM_CODE_INBREAK;
						break;
					case ZL_RSV_CONTINUE: //continue���
						state = ZL_ST_ASM_CODE_INCONTINUE;
						break;
					case ZL_RSV_GLOBAL: //global���
						state = ZL_ST_ASM_CODE_INGLOBAL;
						break;
					case ZL_RSV_RETURN: //return���
						state = ZL_ST_ASM_CODE_INRETURN;
						break;
					case ZL_RSV_SWITCH: //switch...case��䣬�ͽ���ZL_ST_ASM_CODE_INSWITCH״̬
						state = ZL_ST_ASM_CODE_INSWITCH;
						break;
					case ZL_RSV_WHILE: //while...endwhileѭ���ṹ
						state = ZL_ST_ASM_CODE_INWHILE;
						break;
					case ZL_RSV_DO: //do...dowhileѭ���ṹ
						state = ZL_ST_ASM_CODE_INDOWHILE;
						break;
					default:
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_RESERVE);
						break;
					}
				}
				else
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_TOKEN_CAN_NOT_GEN_CODE);
				}
				break;
			}	//switch(nodes[nodenum].toktype)
			break;	//case ZL_ST_START:
		case ZL_ST_INID: //fun��������Ĳ��������� fun test(a,b) a��b����id��ʶ�����������ŷָ�����a��b���ᾭ��AsmGenCodes�����������������Ҫ��a��b��ֵ�ȸ�ֵ��AX,��AsmGenCodes��������ʱ�ٽ�AXѹ��ջ���Ӷ�ʹ��a,b��ֵ�ܳ�Ϊfun test�����Ĳ�����
			inst_op_data = compile->SymLookupID(VM_ARG,nodenum);
			run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
							ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
							inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MOV AX (%d)"
			state = ZL_ST_DOWN;
			break;
		case ZL_ST_INNUM: //����ͬ��
			inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[nodenum].strindex));
			run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
							ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
							ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV AX 123"
			state = ZL_ST_DOWN;
			break;
		case ZL_ST_INFLOAT: //����ͬ��
			inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[nodenum].strindex));
			run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
							ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
							ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV AX 3.1415926"
			state = ZL_ST_DOWN;
			break;
		case ZL_ST_INSTR: //����ͬ��
			inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[nodenum].strindex);
			run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
							ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
							ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV AX "hello world"]
			state = ZL_ST_DOWN;
			break;
		case ZL_ST_INASSIGN: //��ֵ�����������
			if(nodes[nodenum].childs.count != 2)
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_ASSIGN_MUST_HAVE_TWO_CHILD);
			}
			chnum = nodes[nodenum].childs.childnum; //��ȡ�ӽڵ����������
			switch(nodes[chnum[1]].toktype) //����ڶ����ӽڵ�ΪID��ʾ�������������MOV AX (�����ڴ��ַ) �� ����0��ʾ��һ���ӽڵ㣬1���ǵڶ����ӽڵ㣬���ýڵ�����ڴ�ռ��ֵ����AX�Ĵ���
			{
			case ZL_TK_ID:
				inst_op_data = compile->SymLookupID(VM_ARG,chnum[1]);
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[1],
								ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
								inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MOV AX (%d)"
				break;
			case ZL_TK_NUM:
				inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex));
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[1],
								ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
								ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV AX 123"
				break;
			case ZL_TK_FLOAT:
				inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex));
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[1],
								ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
								ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV AX 3.1415926"
				break;
			case ZL_TK_STR:
				inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex);
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[1],
								ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
								ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV AX "hello world"]
				break;
			default:
				if(ZENGL_AST_ISTOKEXPRESS(chnum[1]))
					compile->AsmGenCodes(VM_ARG,chnum[1]); //�ݹ����AsmGenCodes���ɵڶ����ӽڵ��Ӧ�ı��ʽ�Ļ����룬�öλ�����ִ�����һ��Ὣ���������AX�Ĵ����У���֮��Ļ��ָ��ʹ�á�
				else
				{
					compile->parser_curnode = chnum[1];
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_ASSIGN_SECOND_NODE_IS_INVALID_TYPE);
				}
				break;
			} //switch(nodes[chnum[1]].toktype)

assign: //�Ӹ�ֵ������ֵ������������ɼӼ��Ȼ�����󣬾ͻ������˴������ɸ�ֵ���Ļ�����

			switch(nodes[chnum[0]].toktype)
			{
			case ZL_TK_ID: //�жϸ�ֵ�������һ���ӽڵ��Ƿ��Ǳ�����ʾ��
				inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
				run->AddInst(VM_ARG, compile->gencode_struct.pc++, chnum[0],
							ZL_R_IT_MOV , inst_op_data.type , inst_op_data.val.mem,
							ZL_R_DT_REG , ZL_R_RT_AX); //��Ӧ���ָ�� ���� "MOV (%d) AX"
				break;
			case ZL_TK_ARRAY_ITEM: //�жϵ�һ���ӽڵ��Ƿ�������Ԫ�ػ�������ĳ�Ա��
			case ZL_TK_DOT:
				compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_ASM_AI_OP_IN_MOV,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE); //��Ϊ����Ԫ���ڲ�������Ƕ��������Ԫ����test[test2[0]]��������ʽ������ͨ��ѹջ�ķ�ʽ���Է�ֹǶ��ʱ���ڲ�������Ԫ��Ӱ���ⲿ������Ԫ�ص�����жϡ�
				run->AddInst(VM_ARG, compile->gencode_struct.pc++, chnum[0],
							ZL_R_IT_PUSH , ZL_R_DT_NONE , 0,
							ZL_R_DT_REG , ZL_R_RT_AX); //��Ӧ���ָ�� "PUSH AX" �����AsmGenCodes���ɵı��ʽ�Ľ��Ĭ��Ҳ�Ǵ����AX�еģ����������Ƚ�AXѹջ���档
				compile->AsmGenCodes(VM_ARG,chnum[0]); //��������Ԫ�ػ����Ա�Ļ����롣
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_TRUE); //��ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE����ջ��
				break;
			default:
				compile->parser_curnode = chnum[0];
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_ASSIGN_FIRST_NODE_IS_INVALID_TYPE);
				break;
			}
			state = ZL_ST_DOWN; //��ֵ�Ĵ���������ϣ�state��ΪZL_ST_DOWN�������˳�ѭ�������һ��AsmGenCodes������
			break; //case ZL_ST_INASSIGN: //��ֵ�����������
		case ZL_ST_ASM_CODE_INBITS: //��λ�룬������˫Ŀλ�����
		case ZL_ST_ASM_CODE_INRELATION:	//���ڵ���֮��Ĺ�ϵ�Ƚ������
		case ZL_ST_ASM_CODE_INAND_OR:	//��������
		case ZL_ST_ASM_CODE_INTIME_DIVIDE://�˳�ȡ�������������
		case ZL_ST_ASM_CODE_INPLUS_MINIS: //�Ӽ������������
			if(nodes[nodenum].toktype == ZL_TK_AND) //AND��������ӽڵ��п��ܻ���AND�����߼����ڲ���Ƕ�����߼������㣬����Ҳ��Ҫѹջ��
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_AND_ADDR);
			else if(nodes[nodenum].toktype == ZL_TK_OR) //OR��������ӽڵ��п��ܻ���OR�����߼����ڲ���Ƕ�����߼������㣬����Ҳ��Ҫѹջ��
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_OR_ADDR);
			if(nodes[nodenum].childs.count == 2) //�жϽڵ��Ƿ��������ӽڵ�
			{
				chnum = nodes[nodenum].childs.childnum; //��ȡ�ӽڵ����������
				switch(nodes[chnum[0]].toktype) //����Ϊ�Ե�һ���ӽڵ���жϲ������Ӧ�Ļ����룬���������AX�Ĵ����С�
				{
				case ZL_TK_ID:
					inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MOV AX (%d)"
					break;
				case ZL_TK_NUM:
					inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV AX 123"
					break;
				case ZL_TK_FLOAT:
					inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV AX 3.1415926"
					break;
				case ZL_TK_STR:
					inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV AX "hello world"]
					break;
				default:
					if(ZENGL_AST_ISTOKEXPRESS(chnum[0]))
						compile->AsmGenCodes(VM_ARG,chnum[0]);
					else
					{
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
							compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
							nodes[chnum[0]].line_no,
							nodes[chnum[0]].col_no,
							nodes[chnum[0]].filename);
					}
					break;
				} //switch(nodes[chnum[0]].toktype)
				
				if(nodes[nodenum].toktype == ZL_TK_AND)
				{
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JE,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_AND_ADDR,ZL_FALSE)); //��Ӧ���ָ�� "JE adr%d" ����%dΪ�������͵�α��ֵַ
				}
				else if(nodes[nodenum].toktype == ZL_TK_OR)
				{
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JNE,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_OR_ADDR,ZL_FALSE)); //��Ӧ���ָ�� "JNE adr%d" ����%dΪ�������͵�α��ֵַ
				}

				switch(nodes[chnum[1]].toktype) //����Ϊ�Եڶ����ӽڵ���жϲ������Ӧ�Ļ����룬���������BX�Ĵ����С�
				{
				case ZL_TK_ID:
					inst_op_data = compile->SymLookupID(VM_ARG,chnum[1]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[1],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_BX,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MOV BX (%d)"
					break;
				case ZL_TK_NUM:
					inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[1],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_BX,
									ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV BX 123"
					break;
				case ZL_TK_FLOAT:
					inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[1],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_BX,
									ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV BX 3.1415926"
					break;
				case ZL_TK_STR:
					inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[1],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_BX,
									ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV BX "hello world"]
					break;
				default:
					if(ZENGL_AST_ISTOKEXPRESS(chnum[1]))
					{
						run->AddInst(VM_ARG, compile->gencode_struct.pc++, chnum[1],
								ZL_R_IT_PUSH , ZL_R_DT_NONE , 0,
								ZL_R_DT_REG , ZL_R_RT_AX); //��Ӧ���ָ�� "PUSH AX" �Ƚ�ǰ���AX�Ĵ������ֵѹ��ջ�������AsmGenCodes�Ὣ��ֵ����AX
						compile->AsmGenCodes(VM_ARG,chnum[1]);
						run->AddInst(VM_ARG, compile->gencode_struct.pc++, chnum[1],
								ZL_R_IT_MOV , ZL_R_DT_REG , ZL_R_RT_BX,
								ZL_R_DT_REG , ZL_R_RT_AX); //��Ӧ���ָ�� "MOV BX AX" ��AX���ŵı��ʽ�Ľ��ת�浽BX�Ĵ����С�
						run->AddInst(VM_ARG, compile->gencode_struct.pc++, chnum[1],
								ZL_R_IT_POP , ZL_R_DT_NONE , 0,
								ZL_R_DT_REG , ZL_R_RT_AX); //��Ӧ���ָ�� "POP AX" ������ջ�е���AXֵ������ͻ�����AX��BX��ֵ���мӼ��˳����㡣
					}
					else
					{
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
							compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex),
							nodes[chnum[1]].line_no,
							nodes[chnum[1]].col_no,
							nodes[chnum[1]].filename);
					}
					break;
				} //switch(nodes[chnum[1]].toktype)

				switch(nodes[nodenum].toktype) //����������ڵ�����������ͬ�Ļ����롣
				{
				case ZL_TK_PLUS_ASSIGN:
				case ZL_TK_PLUS:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_PLUS , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "PLUS" �ӷ�����룬PLUSָ��ὫAX �� BX�Ĵ�����ֵ��ӣ���������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_PLUS_ASSIGN) //�����+=�������������ӷ�����������assign���ɸ�ֵ���Ļ�ࡣ
						goto assign;
					break;
				case ZL_TK_MINIS_ASSIGN:
				case ZL_TK_MINIS:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_MINIS , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "MINIS" ��������룬MINISָ��ὫAX �� BX�Ĵ�����ֵ�������������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_MINIS_ASSIGN)
						goto assign;
					break;
				case ZL_TK_MOD_ASSIGN:
				case ZL_TK_MOD: //ȡ������������MOD���ָ�
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_MOD , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "MOD" ȡ������
					if(nodes[nodenum].toktype == ZL_TK_MOD_ASSIGN)
						goto assign;
					break;
				case ZL_TK_TIMES_ASSIGN:
				case ZL_TK_TIMES:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_TIMES , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "TIMES" �˷������ TIMESָ��ὫAX �� BX�Ĵ�����ֵ��ˣ���������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_TIMES_ASSIGN)
						goto assign;
					break;
				case ZL_TK_DIVIDE_ASSIGN:
				case ZL_TK_DIVIDE:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_DIVIDE , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "DIVIDE" ��������� DIVIDEָ��ὫAX �� BX�Ĵ�����ֵ�������������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_DIVIDE_ASSIGN)
						goto assign;
					break;
				case ZL_TK_BIT_AND_ASSIGN:
				case ZL_TK_BIT_AND:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_BIT_AND , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "BIT_AND" ��λ������ BIT_ANDָ��ὫAX �� BX�Ĵ�����ֵ���а�λ�����㣬��������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_BIT_AND_ASSIGN)
						goto assign;
					break;
				case ZL_TK_BIT_OR_ASSIGN:
				case ZL_TK_BIT_OR:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_BIT_OR , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "BIT_OR" ��λ������ BIT_ORָ��ὫAX �� BX�Ĵ�����ֵ���а�λ�����㣬��������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_BIT_OR_ASSIGN)
						goto assign;
					break;
				case ZL_TK_BIT_XOR_ASSIGN:
				case ZL_TK_BIT_XOR:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_BIT_XOR , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "BIT_XOR" ��λ������� BIT_XORָ��ὫAX �� BX�Ĵ�����ֵ���а�λ������㣬��������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_BIT_XOR_ASSIGN)
						goto assign;
					break;
				case ZL_TK_BIT_RIGHT_ASSIGN:
				case ZL_TK_BIT_RIGHT:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_BIT_RIGHT , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "BIT_RIGHT" ���ƻ���� BIT_RIGHTָ��ὫAX���ֵ����BX���ֵ����>>�������㣬��������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_BIT_RIGHT_ASSIGN)
						goto assign;
					break;
				case ZL_TK_BIT_LEFT_ASSIGN:
				case ZL_TK_BIT_LEFT:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_BIT_LEFT , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "BIT_LEFT" ���ƻ���� BIT_LEFTָ��ὫAX���ֵ����BX���ֵ����<<�������㣬��������AX�С�
					if(nodes[nodenum].toktype == ZL_TK_BIT_LEFT_ASSIGN)
						goto assign;
					break;
				case ZL_TK_GREAT:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_GREAT , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "GREAT" �������������룬GREATָ��ὫAX��BX���д�С�Ƚϣ����ͨ����0��1��ֵ�����AX�С�0��ʾFALSE��1��ʾTRUE
					break;
				case ZL_TK_LESS:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_LESS , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "LESS" С�����������룬LESSָ��ὫAX��BX���д�С�Ƚϣ����ͨ����0��1��ֵ�����AX�С�
					break;
				case ZL_TK_EQUAL:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_EQUAL , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "EQUAL" ��������������
					break;
				case ZL_TK_NOT_EQ:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_NOT_EQ , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "NOT_EQ" ����������������
					break;
				case ZL_TK_GREAT_EQ:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_GREAT_EQ , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "GREAT_EQ" ���ڵ��ڻ����
					break;
				case ZL_TK_LESS_EQ:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_LESS_EQ , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "LESS_EQ" С�ڵ��ڻ����
					break;
				case ZL_TK_AND:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_AND , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "AND" �����������룬������һ�����������жϣ������PHP��C��&&�������
					break;
				case ZL_TK_OR:
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
							ZL_R_IT_OR , ZL_R_DT_NONE , 0,
							ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "OR" �����������룬�൱��PHP��C��||�����
					break;
				} //switch(nodes[nodenum].toktype) //����������ڵ�����������ͬ�Ļ����롣
				if(nodes[nodenum].toktype == ZL_TK_AND) //����and�߼������ת����ַ������ZL_ASM_STACK_ENUM_AND_ADDR��ջ�е���
				{
					compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_AND_ADDR,ZL_TRUE),compile->gencode_struct.pc);
				}
				else if(nodes[nodenum].toktype == ZL_TK_OR) //����or�߼������ת����ַ������ZL_ASM_STACK_ENUM_OR_ADDR��ջ�е���
				{
					compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_OR_ADDR,ZL_TRUE),compile->gencode_struct.pc);
				}
				state = ZL_ST_DOWN; //�Ӽ��˳��Ȳ����Ĵ���������ϣ�state��ΪZL_ST_DOWN�������˳�ѭ�������һ��AsmGenCodes������
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_TWO_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INPLUS_MINIS: //�Ӽ������������
		case ZL_ST_ASM_CODE_INNEGATIVE: //���ŵ�Ŀ������Ļ�����
			if(nodes[nodenum].childs.count == 1)
			{
				chnum = nodes[nodenum].childs.childnum; //��ȡ�ӽڵ����������
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
							ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
							ZL_R_DT_NUM,0); //��Ӧ���ָ�� "MOV AX 0" ��0��ΪAX����������0����һ������������Ǹ����ĸ����ˡ�
				switch(nodes[chnum[0]].toktype) //����Ϊ�Ե�һ���ӽڵ���жϲ������Ӧ�Ļ����룬���������BX�Ĵ����С�
				{
				case ZL_TK_ID:
					inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_BX,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MOV BX (%d)"
					break;
				case ZL_TK_NUM:
					inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_BX,
									ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV BX 123"
					break;
				case ZL_TK_FLOAT:
					inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_BX,
									ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV BX 3.1415926"
					break;
				case ZL_TK_STR:
					inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_BX,
									ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV BX "hello world"]
					break;
				default:
					if(ZENGL_AST_ISTOKEXPRESS(chnum[0]))
					{
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
								ZL_R_IT_PUSH , ZL_R_DT_NONE , 0,
								ZL_R_DT_REG , ZL_R_RT_AX); //��Ӧ���ָ�� "PUSH AX"
						compile->AsmGenCodes(VM_ARG,chnum[0]);
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
								ZL_R_IT_MOV , ZL_R_DT_REG , ZL_R_RT_BX,
								ZL_R_DT_REG , ZL_R_RT_AX); //��Ӧ���ָ�� "MOV BX AX"
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
								ZL_R_IT_POP , ZL_R_DT_NONE , 0,
								ZL_R_DT_REG , ZL_R_RT_AX); //��Ӧ���ָ�� "POP AX"
					}
					else
					{
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
							compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
							nodes[chnum[0]].line_no,
							nodes[chnum[0]].col_no,
							nodes[chnum[0]].filename);
					}
					break;
				} //switch(nodes[chnum[0]].toktype)
				run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
						ZL_R_IT_MINIS , ZL_R_DT_NONE , 0,
						ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "MINIS" 
				state = ZL_ST_DOWN; 
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break;
		case ZL_ST_ASM_CODE_INCOMMA: //�����������c���ԵĶ��������������һ���ģ����Ǵ������ң�����ִ�б��ʽ���������һ�����ʽ�Ľ����Ϊ���ؽ����
			if(nodes[nodenum].childs.count == 2)
			{
				chnum = nodes[nodenum].childs.childnum;
				if(nodes[chnum[0]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[0]))
					compile->AsmGenCodes(VM_ARG,chnum[0]);
				else
					compile->exit(VM_ARG,ZL_ERR_CP_SYNTAX_INVALID_TOKEN,
						nodes[chnum[0]].line_no,
						nodes[chnum[0]].col_no,
						nodes[chnum[0]].filename,
						compile->getTokenStr(VM_ARG,nodes,chnum[0]));
				if(nodes[chnum[1]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[1]))
					compile->AsmGenCodes(VM_ARG,chnum[1]);
				else
					compile->exit(VM_ARG,ZL_ERR_CP_SYNTAX_INVALID_TOKEN,
						nodes[chnum[1]].line_no,
						nodes[chnum[1]].col_no,
						nodes[chnum[1]].filename,
						compile->getTokenStr(VM_ARG,nodes,chnum[1]));
				state = ZL_ST_DOWN; 
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_TWO_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INCOMMA: //�����������c���ԵĶ��������������һ����
		case ZL_ST_ASM_CODE_INBIT_REVERSE: //��λȡ��������Ĵ���
		case ZL_ST_ASM_CODE_INREVERSE: //ȡ��������Ĵ���
			if(nodes[nodenum].childs.count == 1)
			{
				chnum = nodes[nodenum].childs.childnum;
				switch(nodes[chnum[0]].toktype) //����Ϊ�Ե�һ���ӽڵ���жϲ������Ӧ�Ļ����룬���������AX�Ĵ����С�
				{
				case ZL_TK_ID:
					inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MOV AX (%d)"
					break;
				case ZL_TK_NUM:
					inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV AX 123"
					break;
				case ZL_TK_FLOAT:
					inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV AX 3.1415926"
					break;
				case ZL_TK_STR:
					inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV AX "hello world"]
					break;
				default:
					if(ZENGL_AST_ISTOKEXPRESS(chnum[0]))
						compile->AsmGenCodes(VM_ARG,chnum[0]);
					else
					{
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
							compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
							nodes[chnum[0]].line_no,
							nodes[chnum[0]].col_no,
							nodes[chnum[0]].filename);
					}
					break;
				} //switch(nodes[chnum[0]].toktype)
				if(state == ZL_ST_ASM_CODE_INREVERSE)
				{
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_REVERSE,ZL_R_DT_NONE , 0,
									ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "REVERSE" ȡ��������Ļ���룬�ὫAX���ֵȡ������������AX�С�
				}
				else
				{
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_BIT_REVERSE,ZL_R_DT_NONE , 0,
									ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "BIT_REVERSE" ��λȡ��������Ļ���룬�ὫAX���ֵ���а�λȡ������������AX�С�
				}
				state = ZL_ST_DOWN; 
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INREVERSE: //ȡ���������λȡ��������Ĵ���
		case ZL_ST_ASM_CODE_INADDRESS: //����������Ĵ���
			if(nodes[nodenum].childs.count == 1)
			{
				chnum = nodes[nodenum].childs.childnum;
				switch(nodes[chnum[0]].toktype)
				{
				case ZL_TK_ID:
					inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
								ZL_R_IT_ADDR ,ZL_R_DT_NONE , 0,
								inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "ADDR (%d)"
					break;
				case ZL_TK_ARRAY_ITEM:
				case ZL_TK_DOT:
					compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_ASM_AI_OP_IN_ADDR,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE);
					compile->AsmGenCodes(VM_ARG,chnum[0]);
					compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_TRUE);
					break;
				default:
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
						compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
						nodes[chnum[0]].line_no,
						nodes[chnum[0]].col_no,
						nodes[chnum[0]].filename);
					break;
				}
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INADDRESS: //����������Ĵ���
		case ZL_ST_ASM_CODE_IN_PP_MM: //++��--������������������������Ҳ��C���Ե�һ�������ǶԱ�������ʽ�����һ���һ����++��ǰ��ʱ���ȼ�һ��ȡֵ����++�ں���ʱ����ȡֵ���һ��--ͬ��
			if(nodes[nodenum].childs.count == 1)
			{
				chnum = nodes[nodenum].childs.childnum;
				switch(nodes[chnum[0]].toktype)
				{
				case ZL_TK_ID:
					switch(nodes[nodenum].leftOrRight)
					{
					case ZL_OP_POS_IN_LEFT: //�ӼӼ�������ִ࣬��ADDGET,MINIS_GET���ȼӼ�����ȡֵ��
						switch(nodes[nodenum].toktype)
						{
						case ZL_TK_PLUS_PLUS:
							inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
							run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_ADDGET, ZL_R_DT_NONE , 0,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "ADDGET (%d)"
							break;
						case ZL_TK_MINIS_MINIS:
							inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
							run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_MINIS_GET, ZL_R_DT_NONE , 0,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MINIS_GET (%d)"
							break;
						}
						break;
					case ZL_OP_POS_IN_RIGHT: //GETADD,GET_MINIS,��ȡֵ��Ӽ�
						switch(nodes[nodenum].toktype)
						{
						case ZL_TK_PLUS_PLUS:
							inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
							run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_GETADD, ZL_R_DT_NONE , 0,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "GETADD (%d)"
							break;
						case ZL_TK_MINIS_MINIS:
							inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
							run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_GET_MINIS, ZL_R_DT_NONE , 0,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "GET_MINIS (%d)"
							break;
						}
						break;
					}
					break; //case ZL_TK_ID:
				case ZL_TK_ARRAY_ITEM: //�ӼӼ�����������Ԫ�ػ�����ĳ�Աʱ������AsmGenCodes����������Ԫ�صĻ����룬���������ɹ����л����ZL_ASM_AI_OP_IN_ADDGET֮����������ɶ�Ӧ�Ĵ��롣
				case ZL_TK_DOT:
					switch(nodes[nodenum].leftOrRight)
					{
					case ZL_OP_POS_IN_LEFT: //�ӼӼ��������
						switch(nodes[nodenum].toktype)
						{
						case ZL_TK_PLUS_PLUS:
							compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_ASM_AI_OP_IN_ADDGET,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE);
							compile->AsmGenCodes(VM_ARG,chnum[0]);
							compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_TRUE);
							break;
						case ZL_TK_MINIS_MINIS:
							compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_ASM_AI_OP_IN_MINISGET,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE);
							compile->AsmGenCodes(VM_ARG,chnum[0]);
							compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_TRUE);
							break;
						}
						break; //case ZL_OP_POS_IN_LEFT: //�ӼӼ��������
					case ZL_OP_POS_IN_RIGHT: //GETADD,GET_MINIS,��ȡֵ��Ӽ� 
						switch(nodes[nodenum].toktype)
						{
						case ZL_TK_PLUS_PLUS:
							compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_ASM_AI_OP_IN_GETADD,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE);
							compile->AsmGenCodes(VM_ARG,chnum[0]);
							compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_TRUE);
							break;
						case ZL_TK_MINIS_MINIS:
							compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_ASM_AI_OP_IN_GETMINIS,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE);
							compile->AsmGenCodes(VM_ARG,chnum[0]);
							compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_TRUE);
							break;
						}
						break;
					}
					break; //case ZL_TK_DOT: case ZL_TK_ARRAY_ITEM: //�ӼӼ�����������Ԫ�ػ�����ĳ�Աʱ�Ĵ���
				case ZL_TK_NUM: //���ֱ�Ӷ�����ʹ��++,--��ֱ�ӶԽ�����м�һ����һ��
					inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV AX 123"
					switch(nodes[nodenum].toktype)
					{
					case ZL_TK_PLUS_PLUS:
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_ADDONE, ZL_R_DT_NONE , 0,
								ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "ADDONE"
						break;
					case ZL_TK_MINIS_MINIS:
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_MINIS_ONE, ZL_R_DT_NONE , 0,
								ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "MINIS_ONE"
						break;
					}
					break; //case ZL_TK_NUM:
				case ZL_TK_FLOAT: //���ֱ�ӶԸ�����ʹ��++,--��ֱ�ӶԽ�����м�һ����һ��
					inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV AX 3.1415926"
					switch(nodes[nodenum].toktype)
					{
					case ZL_TK_PLUS_PLUS:
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_ADDONE, ZL_R_DT_NONE , 0,
								ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "ADDONE"
						break;
					case ZL_TK_MINIS_MINIS:
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_MINIS_ONE, ZL_R_DT_NONE , 0,
								ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "MINIS_ONE"
						break;
					}
					break; //case ZL_TK_FLOAT:
				case ZL_TK_STR:
					inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV AX "hello world"]
					switch(nodes[nodenum].toktype)
					{
					case ZL_TK_PLUS_PLUS:
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_ADDONE, ZL_R_DT_NONE , 0,
								ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "ADDONE"
						break;
					case ZL_TK_MINIS_MINIS:
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_MINIS_ONE, ZL_R_DT_NONE , 0,
								ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "MINIS_ONE"
						break;
					}
					break; //case ZL_TK_STR:
				default:
					if(ZENGL_AST_ISTOKEXPRESS(chnum[0])) //ֱ�ӶԱ��ʽ�����һ����һ
					{
						compile->AsmGenCodes(VM_ARG,chnum[0]);
						switch(nodes[nodenum].toktype)
						{
						case ZL_TK_PLUS_PLUS:
							run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_ADDONE, ZL_R_DT_NONE , 0,
									ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "ADDONE"
							break;
						case ZL_TK_MINIS_MINIS:
							run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_MINIS_ONE, ZL_R_DT_NONE , 0,
									ZL_R_DT_NONE , 0); //��Ӧ���ָ�� "MINIS_ONE"
							break;
						}
					}
					else
					{
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
							compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
							nodes[chnum[0]].line_no,
							nodes[chnum[0]].col_no,
							nodes[chnum[0]].filename);
					}
					break; //default:
				} //switch(nodes[chnum[0]].toktype)
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_IN_PP_MM: //++��--������������������������Ҳ��C���Ե�һ�������ǶԱ�������ʽ�����һ���һ
		case ZL_ST_ASM_CODE_INPRINT: //��ӡ���Ļ��������
			if(nodes[nodenum].childs.count == 1)
			{
				chnum = nodes[nodenum].childs.childnum;
				switch(nodes[chnum[0]].toktype)
				{
				case ZL_TK_ID: //��ӡ�����ȱ�ʾ��
					inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
					run->AddInst(VM_ARG, compile->gencode_struct.pc++, nodenum,
								ZL_R_IT_PRINT , ZL_R_DT_NONE , 0,
								inst_op_data.type , inst_op_data.val.mem); //��Ӧ���ָ�� ���� "PRINT (%d)"
					break;
				case ZL_TK_NUM:
					inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_PRINT , ZL_R_DT_NONE , 0,
								ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "PRINT 123"
					break;
				case ZL_TK_FLOAT:
					inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_PRINT , ZL_R_DT_NONE , 0,
								ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "PRINT 3.1415926"
					break;
				case ZL_TK_STR:
					inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_PRINT , ZL_R_DT_NONE , 0,
								ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [PRINT "hello world"]
					break;
				default:
					if(ZENGL_AST_ISTOKEXPRESS(chnum[0]))
					{
						compile->AsmGenCodes(VM_ARG,chnum[0]);
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_PRINT , ZL_R_DT_NONE , 0,
								ZL_R_DT_REG,ZL_R_RT_AX); //��Ӧ���ָ�� "PRINT AX"
					}
					else
					{
						compile->parser_curnode = chnum[0];
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_TOKEN_CAN_NOT_GEN_CODE);
					}
					break;
				} //switch(nodes[chnum[0]].toktype)
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_PRINT_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INPRINT: //��ӡ���Ļ��������
		case ZL_ST_ASM_CODE_INIF: //if-elif-else �������Ļ��������
			if(nodes[nodenum].childs.count >= 2)
			{
				ZL_INT LastNodeNum;
				chnum = nodes[nodenum].childs.childnum;
				if(chnum[0] == -1)
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_IF_HAVE_NO_EXPRESS);
				}
				else if(nodes[chnum[0]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[0])) //�������ӻ���ʽ��������Ϊ�жϱ��ʽ��
				{
					compile->AsmGenCodes(VM_ARG,chnum[0]); //����if��һ���жϱ��ʽ��Ļ�����
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JE,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_IF_ADDR)); //��Ӧ���ָ�� "JE adr%d" �����жϱ��ʽ�Ľ���ж�����ת��elif��else���β�������Ǽ���ִ��
					i = chnum[1];
					while(i>0)  //ѭ������if��һ��ִ�п�����������Ļ����롣
					{
						LastNodeNum = i;
						compile->AsmGenCodes(VM_ARG,i);
						i = nodes[i].nextnode;
					}
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,LastNodeNum,
						ZL_R_IT_JMP,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_IF_END)); //��Ӧ���ָ�� "JMP adr%d" ������ִ�п���Ĵ���󣬾�JMP��������ת��if-elif-else���Ľ���λ�á�
					compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_IF_ADDR,ZL_TRUE),compile->gencode_struct.pc); //��¼��IF_ADDR �����ʾ����ת��ַ��Ӧ�Ļ��ָ���λ�á������һ���ж�Ϊfalse����ת�����
				}
				else
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
						compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
						nodes[chnum[0]].line_no,
						nodes[chnum[0]].col_no,
						nodes[chnum[0]].filename);
				}
				i = 2;
				while(i < nodes[nodenum].childs.count)
				{
					if(i >= ZL_AST_CHILD_NODE_SIZE) //�ж��Ƿ���Ҫ��ѯ��չ�ӽڵ㣬Ĭ�ϳ���3�����ӽڵ㶼�������չ�ӽڵ��
					{
						tmpch = nodes[nodenum].childs.extchilds[i - ZL_AST_CHILD_NODE_SIZE];
					}
					else
						tmpch = chnum[i];
					if(nodes[tmpch].reserve == ZL_RSV_ELIF) //�����elif��ͨ��AsmGCElif����������elif���ֵĻ��ָ��
					{
						compile->AsmGCElif(VM_ARG,&nodes[nodenum].childs,i);
						i +=2;
					}
					else if(nodes[tmpch].reserve == ZL_RSV_ELSE) //�����else,��ѭ����else�����������ɶ�Ӧ�Ļ��ָ�
					{
						if(i + 1 < ZL_AST_CHILD_NODE_SIZE)
							i = chnum[i + 1];
						else
							i = nodes[nodenum].childs.extchilds[i + 1 - ZL_AST_CHILD_NODE_SIZE];
						while(i > 0)
						{
							compile->AsmGenCodes(VM_ARG,i);
							i = nodes[i].nextnode;
						}
						break;
					}
				}
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_IF_END,ZL_TRUE),compile->gencode_struct.pc); //����if�ṹ����λ�õĻ��λ�á�
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_AT_LEAST_TWO_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INIF: //if-elif-else �������Ļ��������
		case ZL_ST_ASM_CODE_INFOR: //for...endfor..ѭ�����������������
			if(nodes[nodenum].childs.count == 4)
			{
				chnum = nodes[nodenum].childs.childnum; //chnum�����ʼ��3���ӽڵ㡣
				extnum = nodes[nodenum].childs.extchilds; //extnum��������3������չ�ӽڵ㡣
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_FOR_ADDR); //��Ϊfor�ڲ�������Ƕ��������for��䣬������Ҫ����for�йصĵ�ַ����ѹջ������
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_FOR_END);
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_FOR_CONTINUE);
				if(chnum[0] != -1 && ZENGL_AST_ISTOKEXPRESS(chnum[0])) //����for(i=1;i<3;i++)���е�i=1��ʼ�����־��ǵ�һ���ӽڵ㡣
					compile->AsmGenCodes(VM_ARG,chnum[0]); //���ɵ�һ���ӽڵ��Ӧ�ı��ʽ�Ļ����롣
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FOR_ADDR,ZL_FALSE),compile->gencode_struct.pc); //��¼�µڶ����Ƚ��ӽڵ�Ŀ�ʼ������λ�á�ѭ������ִ�е�����ʱ����ת����������ж��Ƿ���Ҫ����ѭ����
				if(chnum[1] != -1 && ZENGL_AST_ISTOKEXPRESS(chnum[1]))
				{
					compile->AsmGenCodes(VM_ARG,chnum[1]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JE,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FOR_END,ZL_FALSE)); //��Ӧ���ָ�� ���� "JE adr%d" ����ж�Ϊfalse������ѭ����
				}
				i = extnum[0]; //��չ�ӽڵ㲿�־���for...endfor֮���ѭ���岿�ֵĴ��롣
				while(i > 0) //����ڵ��С��0��˵����������䡣
				{
					compile->AsmGenCodes(VM_ARG,i); //ѭ������for...endfor֮�����������Ӧ�Ļ����롣
					i = nodes[i].nextnode;
				}
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FOR_CONTINUE,ZL_FALSE),compile->gencode_struct.pc); //ѭ�����ĩβ����continue��Ҫ��ת��λ�á�
				if(chnum[2] != -1 && ZENGL_AST_ISTOKEXPRESS(chnum[2])) //�������ӽڵ���������е�i++��ѭ����ִ�������Ҫִ�еĴ��롣
					compile->AsmGenCodes(VM_ARG,chnum[2]);
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JMP,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FOR_ADDR,ZL_FALSE)); //��Ӧ���ָ�� ���� "JMP adr%d" ִ����������ӽڵ�Ĵ������ת���ڶ����ӽڵ�Ĵ��봦�����ж��Ƿ���Ҫ����ѭ����
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FOR_END,ZL_FALSE),compile->gencode_struct.pc); //for...endfor����λ��
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FOR_ADDR,ZL_TRUE); //������for�Ļ�����󣬽�for��صĵ�ַ����ջ�� 
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FOR_END,ZL_TRUE);
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FOR_CONTINUE,ZL_TRUE); 
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_FOUR_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INFOR: //for...endfor..ѭ�����������������
		case ZL_ST_ASM_CODE_INFUN: //fun...endfun ��������������
			if(nodes[nodenum].childs.count == 3)
			{
				ZL_INT tmpFunid;	//��ŵ�ǰ�����ĺ���ID����ʱ����
				ZL_INT tmpFunEnd = compile->AsmGCAddrNum++;		//��ŵ�ǰ��������λ�õ�α��ֵַ
				ZL_INT tmpFunAddr = compile->AsmGCAddrNum++;	//��ŵ�ǰ�������λ�õ�α��ֵַ
				ZL_INT tmpFunArgPC; //���FUNARGָ�������λ�õ���ʱ����
				ZL_INT tmpClassID = 0; //����ຯ��ID��Ϣ
				if(compile->gencode_struct.is_inFun)
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_FUN_CAN_NOT_DEFINED_IN_OTHER_FUN);
				}
				compile->gencode_struct.is_inFun = ZL_TRUE;
				chnum = nodes[nodenum].childs.childnum;
				if(compile->AsmGCIsInClass == ZL_TRUE) //�ຯ��ʱ���Ȼ�ȡ��ID��Ϣ
				{
					tmpClassID = compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FUN_CLASSID,ZL_FALSE);
					if(tmpClassID < 0)
						tmpClassID = 0;
				}
				compile->SymFunTable.global_funid = tmpFunid = compile->SymLookupFun(VM_ARG,chnum[0],tmpClassID); //fun��һ���ӽڵ��Ǻ�����
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JMP,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,tmpFunEnd); //��Ӧ���ָ�� ���� "JMP adr%d" ��ת����������λ�ã������Ǻ������õ�����¾Ͳ���ִ�к�����
				compile->LDAddrListSet(VM_ARG,tmpFunAddr,compile->gencode_struct.pc); //���õ�ǰ��������ڵ�ַ��Ӧ��ʵ�ʻ��λ��
				compile->SymFunTable.funs[tmpFunid].LDAdr = tmpFunAddr; //���ú��������α��ַ
				tmpFunArgPC = compile->gencode_struct.pc;
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_FUNARG,ZL_R_DT_NONE,0,
						ZL_R_DT_NUM , 0); //��Ӧ���ָ�� "FUNARG %d" ������FUNARG��Դ�������������ã��õ����������Ŀ��FUNARG����ݲ�����Ŀ��������ʱ�������ջ�з����Ӧ�������ڴ�ռ����Դ�����еĲ�����
				compile->gencode_struct.localID = 0;
				compile->SymScanFunArg(VM_ARG,chnum[1]); //���ݵڶ����ӽڵ�ɨ�躯���Ĳ�����
				run->inst_list.insts[tmpFunArgPC].src.val.num = compile->gencode_struct.localID;
				compile->gencode_struct.localID = 0;
				compile->SymScanFunLocal(VM_ARG,chnum[2]); //���ݵ������ӽڵ�ɨ�躯�����ڵľֲ�������global�ؼ������ε�ȫ�ֱ���(SymScanFunLocal��ɨ��ʱ������global�ؼ��־ͻ����SymScanFunGlobal������ȫ�ֱ���)������ÿ���ֲ���������һ��PUSH_LOC�Ļ����룬������Ϊÿ���ֲ�����������һ��ջ�ռ䡣
				i = chnum[2];
				compile->AsmGCStackPush(VM_ARG,0,ZL_ASM_STACK_ENUM_FUN_CLASSID); //��ֹ������ID��ϢӰ�쵽fun�������ڲ���funcall�������ã�û���������룬��fun�����ͨ��funcallҲ������ຯ�������ˣ�
				while(i > 0) //ѭ������fun���������ÿ�����ʽ�Ļ����롣
				{
					compile->AsmGenCodes(VM_ARG,i);
					i = nodes[i].nextnode;
				}
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FUN_CLASSID,ZL_TRUE); //��ǰ��ѹ���0��classid����ȥ
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_RET,ZL_R_DT_NONE,0,
						ZL_R_DT_NONE,0); //��Ӧ���ָ�� "RET"
				compile->LDAddrListSet(VM_ARG,tmpFunEnd,compile->gencode_struct.pc); //���ú����Ľ�����ַ
				compile->gencode_struct.is_inFun = ZL_FALSE;
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_THREE_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INFUN: //fun...endfun ��������������
		case ZL_ST_ASM_CODE_INFUNCALL: //����Ǻ������ã��������Ӧ�Ļ����롣
			if(nodes[nodenum].childs.count == 1)
			{
				ZL_INT tmpReturnPC,tmpFunID,tmpClassID; //���������÷���ʱҪִ�е���һ������Ļ�����λ��
				compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_TK_FUNCALL,ZL_ASM_STACK_ENUM_IS_IN_ARRAYITEM_OR_FUNCALL); //����test(a+b)��䣬a+b���ɴ������Ϊ��FUNCALL������������Ի�PUSH��a+b��ֵѹ��ջ���Ӷ���Ϊtest�����Ĳ���
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
					ZL_R_IT_PUSH,ZL_R_DT_NONE,0,
					ZL_R_DT_REG,ZL_R_RT_ARG); //��Ӧ���ָ�� "PUSH ARG" ����ǰ��ARG�����Ĵ���ѹ��ջ��
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
					ZL_R_IT_PUSH,ZL_R_DT_NONE,0,
					ZL_R_DT_REG,ZL_R_RT_LOC); //��Ӧ���ָ�� "PUSH LOC" ����ǰ��LOC�ֲ������Ĵ���ѹ��ջ��
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
					ZL_R_IT_PUSH,ZL_R_DT_NONE,0,
					ZL_R_DT_REG,ZL_R_RT_ARGTMP); //��Ӧ���ָ�� "PUSH ARGTMP" ����ǰ��ARGTMP��ʱ�����Ĵ���Ҳѹ��ջ��
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
					ZL_R_IT_RESET,ZL_R_DT_NONE,0,
					ZL_R_DT_REG,ZL_R_RT_ARGTMP); //��Ӧ���ָ�� "RESET ARGTMP" ARGTMP��ʱ��ARG�����Ĵ������������һ����ֱ��RESET ARG�Ļ���������PUSH ����ʱ����Ϊ���������Ǹ����ʽ�������ʽ��ܿ���Ҫ�õ���ǰ��ARG�����Ĵ�����������RESET ARGTMP�Ĵ�������ARGTMP�Ĵ�����¼�µ�ǰ�������ջ��λ�ã��Ȳ�����������ٽ�ARGTMP ��ֵ��ARG�Ĵ�����
				chnum = nodes[nodenum].childs.childnum;
				if(chnum[0] == -1) //����������õĲ���Ϊ����test()��䣬���ӽڵ�Ϊ-1����ʱ�������������
					;
				else if(nodes[chnum[0]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[0])) //�������õĲ�������Ҫô�ǵ����ı�ʶ��֮��Ĳ������ӣ�Ҫô�Ƕ��ŷָ����ı��ʽ������ֱ��AsmGenCodes���ɸýڵ�Ļ����뼴�ɣ�AsmGenCodes��������Ȼ��Զ��Ž��д���
				{
					compile->AsmGCStackPush(VM_ARG,0,ZL_ASM_STACK_ENUM_FUN_CLASSID);
					compile->AsmGenCodes(VM_ARG,chnum[0]);
					compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FUN_CLASSID,ZL_TRUE);
				}
				else
					compile->exit(VM_ARG,ZL_ERR_CP_SYNTAX_INVALID_TOKEN,
						nodes[chnum[0]].line_no,
						nodes[chnum[0]].col_no,
						nodes[chnum[0]].filename,
						compile->getTokenStr(VM_ARG,nodes,chnum[0]));
				tmpReturnPC = compile->gencode_struct.pc + 4;
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
					ZL_R_IT_PUSH,ZL_R_DT_NONE,0,
					ZL_R_DT_NUM,tmpReturnPC); //��Ӧ���ָ�� ���� "PUSH %d" ���������÷���ʱҪִ�е���һ������Ļ�����λ��ѹ��ջ�С�
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
					ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_ARG,
					ZL_R_DT_REG,ZL_R_RT_ARGTMP); //��Ӧ���ָ�� "MOV ARG ARGTMP" ��ARGTMP��ֵ��ARG�Ĵ�����
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
					ZL_R_IT_RESET,ZL_R_DT_NONE,0,
					ZL_R_DT_REG,ZL_R_RT_LOC); //��Ӧ���ָ�� "RESET LOC" ����ǰ�����ջ��λ��ֵͨ��RESETָ�ֵ��LOC�Ĵ�����
				tmpClassID = compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FUN_CLASSID,ZL_FALSE);
				if(tmpClassID < 0)
					tmpClassID = 0;
				if((tmpFunID = compile->SymLookupFun(VM_ARG,nodenum,tmpClassID)) == 0)
				{
					inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[nodenum].strindex);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_CALL,ZL_R_DT_NONE,0,
						ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� "CALL %s" //����ڵ�ǰ���û��Զ���ű�������ϣ����û���ҵ���������Ϣ����˵���ú��������û��Զ���ĺ���������use�ؼ��������ģ����ĺ��������Ծ����CALL "������"���ֻ���ʽ�����������������������ʱ�ͻ���use�����ģ���в��Ҳ����ú�����
				}
				else
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JMP,ZL_R_DT_NONE,0,
						ZL_R_DT_LDFUNID,tmpFunID); //��Ӧ���ָ�� ���� "JMP funid%d" ����ҵ���������Ϣ��˵�����û��Զ���Ľű�����������ת��Ŀ�꺯���Ŀ�ִ�д�����ڴ�������ʹ�õ��Ǻ���ID���������滻ʱ�����ɺ���ID�õ�����α��ַ������α��ַ�õ���ʵ���λ�ã����������Ϳ��Զ����ڽű�������Ϸ�λ�ã����ֱ��ʹ��α��ַ����ô������ֻ�ܶ����ں�������֮ǰ
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_IS_IN_ARRAYITEM_OR_FUNCALL,ZL_TRUE); //FUNCALL������ϣ����Խ�֮ǰ��ѹջ������
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INFUNCALL: //����Ǻ������ã��������Ӧ�Ļ����롣
		case ZL_ST_ASM_CODE_INARRAY_ITEM: //��������Ԫ�صĻ����롣����test[a,b+1,c] ����һ����ά�����飬���Դ˽�a,b+1,c��ֵѹ��ջ��ARRAY_ITEM�Ĵ���ָ���һ��a��ջλ�ã���a�õ���һά����b+1��ֵ�õ��ڶ�ά����c�õ�����ά������test[a,b+1,c]����php��test[a][b+1][c]����Ȼa,b+1��c������������php�����������
			if(nodes[nodenum].childs.count == 1)
			{
				ZENGL_ASM_ARRAY_ITEM_OP_TYPE array_item;
				compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_TK_ARRAY_ITEM,ZL_ASM_STACK_ENUM_IS_IN_ARRAYITEM_OR_FUNCALL); //��ZL_TK_ARRAY_ITEMѹ��ջ�������ڲ�Ƕ����������Ԫ�أ�ͬʱ������AsmGenCodes���ɱ��ʽ�󻹻����PUSH AX����������Ԫ����ı��ʽ��ֵ�Ϳ�����Ϊ����������
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_PUSH,ZL_R_DT_NONE,0,
						ZL_R_DT_REG,ZL_R_RT_ARRAY_ITEM); //��Ӧ���ָ�� "PUSH ARRAY_ITEM" �Ƚ�ԭ����ARRAY_ITEM�Ĵ���ѹ��ջ��
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_RESET,ZL_R_DT_NONE,0,
						ZL_R_DT_REG,ZL_R_RT_ARRAY_ITEM); //��Ӧ���ָ�� "RESET ARRAY_ITEM" ���õ�ǰ��ARRAY_ITEM�Ĵ���Ϊ��ǰ��ջ����
				chnum = nodes[nodenum].childs.childnum;
				if(chnum[0] == -1) //����-1ʱ��ʾ�սڵ㣬��test[]
					;
				else if(nodes[chnum[0]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[0]))
				{
					compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_ASM_AI_OP_NONE,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE); //��ֹAsmGenCodes�����ɱ��ʽʱ���ܵ����ʽ�ڲ�Ƕ�׵�Ӱ��
					compile->AsmGenCodes(VM_ARG,chnum[0]);
					compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_TRUE);
				}
				else
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
						compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
						nodes[chnum[0]].line_no,
						nodes[chnum[0]].col_no,
						nodes[chnum[0]].filename);
				}
				array_item = compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_FALSE);
				switch(array_item)
				{
				case ZL_ASM_AI_OP_IN_MOV: //���������test[0] = 5�����ĸ�ֵ��䣬�����SET_ARRAYָ���ָ�����AX���ֵ�������������Ԫ�ء�
					inst_op_data = compile->SymLookupID(VM_ARG,nodenum);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_SET_ARRAY,ZL_R_DT_NONE,0,
								inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "SET_ARRAY (%d)"
					break;
				case ZL_ASM_AI_OP_IN_ADDR: //��������� &test[0] ��������������Ԫ�ص���䣬�����GET_ARRAY_ADDRָ���ָ���õ�����Ԫ�ص�������Ϣ��
					inst_op_data = compile->SymLookupID(VM_ARG,nodenum);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_GET_ARRAY_ADDR,ZL_R_DT_NONE,0,
								inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "GET_ARRAY_ADDR (%d)"
					break;
				case ZL_ASM_AI_OP_IN_ADDGET: //��������� ++test[0] �������ȼӼӺ�ȡֵ����䣬�����ADDGET_ARRAYָ�������Ԫ�ؽ��м�һ�����ٷ���ֵ��
					inst_op_data = compile->SymLookupID(VM_ARG,nodenum);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_ADDGET_ARRAY,ZL_R_DT_NONE,0,
								inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "ADDGET_ARRAY (%d)"
					break;
				case ZL_ASM_AI_OP_IN_MINISGET: //--test[0]֮������
					inst_op_data = compile->SymLookupID(VM_ARG,nodenum);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_MINISGET_ARRAY,ZL_R_DT_NONE,0,
								inst_op_data.type,inst_op_data.val.mem); ///��Ӧ���ָ�� ���� "MINISGET_ARRAY (%d)"
					break;
				case ZL_ASM_AI_OP_IN_GETADD: //test[0]++֮������
					inst_op_data = compile->SymLookupID(VM_ARG,nodenum);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_GETADD_ARRAY,ZL_R_DT_NONE,0,
								inst_op_data.type,inst_op_data.val.mem); ///��Ӧ���ָ�� ���� "GETADD_ARRAY (%d)"
					break;
				case ZL_ASM_AI_OP_IN_GETMINIS: //test[0]--֮������
					inst_op_data = compile->SymLookupID(VM_ARG,nodenum);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_GETMINIS_ARRAY,ZL_R_DT_NONE,0,
								inst_op_data.type,inst_op_data.val.mem); ///��Ӧ���ָ�� ���� "GETMINIS_ARRAY (%d)"
					break;
				default: //a = test[0]֮��Ļ�ȡ����Ԫ�ص�ֵ����䡣
					inst_op_data = compile->SymLookupID(VM_ARG,nodenum);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_GET_ARRAY,ZL_R_DT_NONE,0,
								inst_op_data.type,inst_op_data.val.mem); ///��Ӧ���ָ�� ���� "GET_ARRAY (%d)"
					break;
				} //switch(array_item)
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_IS_IN_ARRAYITEM_OR_FUNCALL,ZL_TRUE); //���������Ԫ�صĻ��ָ��󣬾Ϳ��Ե���֮ǰ��ѹջ�ˡ�
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INARRAY_ITEM: //��������Ԫ�صĻ����롣
		case ZL_ST_ASM_CODE_INUSE: //use���Ļ���������
			if(nodes[nodenum].childs.count == 1)
			{
				chnum = nodes[nodenum].childs.childnum;
				compile->SymScanUseRsv(VM_ARG,chnum[0],nodenum);
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INUSE: //use���Ļ���������
		case ZL_ST_ASM_CODE_INBREAK: //break�ؼ��ֵĻ�����
			compile->AsmGCBreak_Codes(VM_ARG,nodenum);
			state = ZL_ST_DOWN;
			break;
		case ZL_ST_ASM_CODE_INCONTINUE: //continue�ؼ��ֵĻ�����
			compile->AsmGCContinue_Codes(VM_ARG,nodenum);
			state = ZL_ST_DOWN;
			break;
		case ZL_ST_ASM_CODE_INGLOBAL: //global��������
			if(compile->gencode_struct.is_inFun == ZL_TRUE) //�ж�global�ؼ����Ƿ�ʹ���ں������Ϊglobal��ǰ���ScanFunArg_Global��������й�ɨ�裬��������ͽ�state��Ϊdown��ֱ�ӷ��ء�
				state = ZL_ST_DOWN;
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_UNEXPECT_LOC_OF_GLOBAL);
			}
			break;
		case ZL_ST_ASM_CODE_INRETURN: //return��������
			if(compile->gencode_struct.is_inFun != ZL_TRUE)
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_UNEXPECT_LOC_OF_RETURN);
			}
			if(nodes[nodenum].childs.count == 1) //return�ؼ��ֺ��������Ҫ���ص�ֵ������Ǳ�����ʶ�������ֻ��ַ�������ֱ�ӽ�����,����,�ַ�����ֵ��ֵ��AX�Ĵ�����Ϊ����ֵ������Ǳ��ʽ�ͼ�������ʽ��ֵ����Ϊ���ʽ�Ľ��Ĭ�Ͼ���AX�У����Կ���ֱ����Ϊ����ֵ�������RETָ�������ű�������
			{
				chnum = nodes[nodenum].childs.childnum;
				if(chnum[0] == -1) //���ӽڵ�Ϊ-1����return;��䣬return��û�б��ʽ�������ӽڵ�Ϊ-1������Ҫ�ڴ˴����������������������-1�������ͻ�����ڴ����
					;
				else
				{
					switch(nodes[chnum[0]].toktype) //����Ϊ�Ե�һ���ӽڵ���жϲ������Ӧ�Ļ����룬���������AX�Ĵ����С�
					{
					case ZL_TK_ID:
						inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
										ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
										inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MOV AX (%d)"
						break;
					case ZL_TK_NUM:
						inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
										ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
										ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV AX 123"
						break;
					case ZL_TK_FLOAT:
						inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
										ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
										ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV AX 3.1415926"
						break;
					case ZL_TK_STR:
						inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex);
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
										ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
										ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV AX "hello world"]
						break;
					default:
						if(ZENGL_AST_ISTOKEXPRESS(chnum[0]))
							compile->AsmGenCodes(VM_ARG,chnum[0]);
						else if(chnum[0] != -1)
						{
							compile->parser_curnode = nodenum;
							compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
								compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
								nodes[chnum[0]].line_no,
								nodes[chnum[0]].col_no,
								nodes[chnum[0]].filename);
						}
						break;
					} //switch(nodes[chnum[0]].toktype)
				} //else
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_RET,ZL_R_DT_NONE,0,
						ZL_R_DT_NONE,0); //��Ӧ���ָ�� "RET"
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INRETURN: //return��������
		case ZL_ST_ASM_CODE_INCLASS: //class���Ļ�����
			if(nodes[nodenum].childs.count == 2)
			{
				ZL_INT tmpNodeNum,classid;
				chnum = nodes[nodenum].childs.childnum;
				compile->AsmGCIsInClass = ZL_TRUE;
				classid = compile->SymLookupClass(VM_ARG,chnum[0]);
				compile->AsmGCStackPush(VM_ARG,classid,ZL_ASM_STACK_ENUM_FUN_CLASSID);
				tmpNodeNum = chnum[1];
				while(tmpNodeNum !=0)
				{
					switch(nodes[tmpNodeNum].reserve)
					{
					case ZL_RSV_FUN:
						compile->AsmGenCodes(VM_ARG,tmpNodeNum);
						break;
					}
					tmpNodeNum = nodes[tmpNodeNum].nextnode;
				}
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FUN_CLASSID,ZL_TRUE);
				compile->AsmGCIsInClass = ZL_FALSE;
			}
			state = ZL_ST_DOWN;
			break; //case ZL_ST_ASM_CODE_INCLASS:
		case ZL_ST_ASM_CODE_INCLASS_STATEMENT: //�������Poker��һ���࣬��ô�������Poker test; �ͻὫPoker���classid���뵽test�������ڵ�ȫ�ֻ�ֲ���ϣ���classid��Ա��
			if(nodes[nodenum].childs.count == 1)
			{
				chnum = nodes[nodenum].childs.childnum;
				compile->SymScanClassStatement(VM_ARG,chnum[0],nodenum);
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INCLASS_STATEMENT:
		case ZL_ST_ASM_CODE_INDOT: //ͨ���������������ĳ�Ա
			if(nodes[nodenum].childs.count == 2)
			{
				chnum = nodes[nodenum].childs.childnum;
				if(nodes[chnum[1]].toktype != ZL_TK_FUNCALL)
				{
					ZENGL_ASM_ARRAY_ITEM_OP_TYPE array_item;
					compile->AsmGCStackPush(VM_ARG,(ZL_INT)ZL_TK_ARRAY_ITEM,ZL_ASM_STACK_ENUM_IS_IN_ARRAYITEM_OR_FUNCALL); //��ZL_TK_ARRAY_ITEMѹ��ջ�������ڲ�Ƕ����������Ԫ�أ�ͬʱ������AsmGenCodes���ɱ��ʽ�󻹻����PUSH AX����������Ԫ����ı��ʽ��ֵ�Ϳ�����Ϊ����������
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
							ZL_R_IT_PUSH,ZL_R_DT_NONE,0,
							ZL_R_DT_REG,ZL_R_RT_ARRAY_ITEM); //��Ӧ���ָ�� "PUSH ARRAY_ITEM" �Ƚ�ԭ����ARRAY_ITEM�Ĵ���ѹ��ջ��
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
							ZL_R_IT_RESET,ZL_R_DT_NONE,0,
							ZL_R_DT_REG,ZL_R_RT_ARRAY_ITEM); //��Ӧ���ָ�� "RESET ARRAY_ITEM" ���õ�ǰ��ARRAY_ITEM�Ĵ���Ϊ��ǰ��ջ����
					compile->SymScanDotForClass(VM_ARG,nodenum);
					array_item = compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_ARRAY_ITEM_OP_TYPE,ZL_FALSE);
					switch(array_item)
					{
					case ZL_ASM_AI_OP_IN_MOV: //���������test[0] = 5�����ĸ�ֵ��䣬�����SET_ARRAYָ���ָ�����AX���ֵ�������������Ԫ�ء�
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_SET_ARRAY,ZL_R_DT_NONE,0,
									compile->memDataForDot.type,compile->memDataForDot.val.mem); //��Ӧ���ָ�� ���� "SET_ARRAY (%d)"
						break;
					case ZL_ASM_AI_OP_IN_ADDR: //��������� &test[0] ��������������Ԫ�ص���䣬�����GET_ARRAY_ADDRָ���ָ���õ�����Ԫ�ص�������Ϣ��
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_GET_ARRAY_ADDR,ZL_R_DT_NONE,0,
									compile->memDataForDot.type,compile->memDataForDot.val.mem); //��Ӧ���ָ�� ���� "GET_ARRAY_ADDR (%d)"
						break;
					case ZL_ASM_AI_OP_IN_ADDGET: //��������� ++test[0] �������ȼӼӺ�ȡֵ����䣬�����ADDGET_ARRAYָ�������Ԫ�ؽ��м�һ�����ٷ���ֵ��
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_ADDGET_ARRAY,ZL_R_DT_NONE,0,
									compile->memDataForDot.type,compile->memDataForDot.val.mem); //��Ӧ���ָ�� ���� "ADDGET_ARRAY (%d)"
						break;
					case ZL_ASM_AI_OP_IN_MINISGET: //--test[0]֮������
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_MINISGET_ARRAY,ZL_R_DT_NONE,0,
									compile->memDataForDot.type,compile->memDataForDot.val.mem); ///��Ӧ���ָ�� ���� "MINISGET_ARRAY (%d)"
						break;
					case ZL_ASM_AI_OP_IN_GETADD: //test[0]++֮������
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_GETADD_ARRAY,ZL_R_DT_NONE,0,
									compile->memDataForDot.type,compile->memDataForDot.val.mem); ///��Ӧ���ָ�� ���� "GETADD_ARRAY (%d)"
						break;
					case ZL_ASM_AI_OP_IN_GETMINIS: //test[0]--֮������
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_GETMINIS_ARRAY,ZL_R_DT_NONE,0,
									compile->memDataForDot.type,compile->memDataForDot.val.mem); ///��Ӧ���ָ�� ���� "GETMINIS_ARRAY (%d)"
						break;
					default: //a = test[0]֮��Ļ�ȡ����Ԫ�ص�ֵ����䡣
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
									ZL_R_IT_GET_ARRAY,ZL_R_DT_NONE,0,
									compile->memDataForDot.type,compile->memDataForDot.val.mem); ///��Ӧ���ָ�� ���� "GET_ARRAY (%d)"
						break;
					}
					compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_IS_IN_ARRAYITEM_OR_FUNCALL,ZL_TRUE); //���������Ԫ�صĻ��ָ��󣬾Ϳ��Ե���֮ǰ��ѹջ�ˡ�
				} //if(nodes[chnum[1]].toktype != ZL_TK_FUNCALL)
				else
				{
					ZL_INT classid = 0;
					if(nodes[chnum[0]].toktype != ZL_TK_ID)
					{
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
							compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
							nodes[chnum[0]].line_no,
							nodes[chnum[0]].col_no,
							nodes[chnum[0]].filename);
					}
					classid = compile->SymLookupClass(VM_ARG,chnum[0]);
					compile->AsmGCStackPush(VM_ARG,classid,ZL_ASM_STACK_ENUM_FUN_CLASSID);
					compile->AsmGenCodes(VM_ARG,chnum[1]);
					compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_FUN_CLASSID,ZL_TRUE);
				}
				state = ZL_ST_DOWN;
			} //if(nodes[nodenum].childs.count == 2)
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_TWO_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INDOT: 
		case ZL_ST_ASM_CODE_INSWITCH: //switch...case�ṹ������
			if(nodes[nodenum].childs.count >= 1)
			{
				ZL_BOOL hasminmax,hasdefault;
				ZL_INT max,min,num,j;
				ZENGL_ASM_CASE_JMP_TABLE casejmptable = {0}; //switch...case����ת��
				chnum = nodes[nodenum].childs.childnum;
				extnum = nodes[nodenum].childs.extchilds;
				switch(nodes[chnum[0]].toktype) //�Ƚ�Ҫ�Ƚϵ�ֵ��ֵ��AX
				{
				case ZL_TK_ID:
					inst_op_data = compile->SymLookupID(VM_ARG,chnum[0]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									inst_op_data.type,inst_op_data.val.mem); //��Ӧ���ָ�� ���� "MOV AX (%d)"
					break;
				case ZL_TK_NUM:
					inst_op_data.val.num = ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_NUM,inst_op_data.val.num); //��Ӧ���ָ�� ���� "MOV AX 123"
					break;
				case ZL_TK_FLOAT:
					inst_op_data.val.floatnum = ZENGL_SYS_STR_TO_FLOAT(compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex));
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_FLOAT,inst_op_data.val.floatnum); //��Ӧ���ָ�� ���� "MOV AX 3.1415926"
					break;
				case ZL_TK_STR:
					inst_op_data.val.num = (ZL_LONG)compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,chnum[0],
									ZL_R_IT_MOV,ZL_R_DT_REG,ZL_R_RT_AX,
									ZL_R_DT_STR,inst_op_data.val.num); //��Ӧ���ָ�� ���� [MOV AX "hello world"]
					break;
				default:
					if(ZENGL_AST_ISTOKEXPRESS(chnum[0])) //���ʽ�Ľ��Ĭ�Ͼ��Ǵ����AX�еģ�����ֱ��AsmGenCodes���ɱ��ʽ�Ļ����뼴��
						compile->AsmGenCodes(VM_ARG,chnum[0]);
					else
					{
						compile->parser_curnode = nodenum;
						compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
							compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
							nodes[chnum[0]].line_no,
							nodes[chnum[0]].col_no,
							nodes[chnum[0]].filename);
					}
					break;
				} //switch(nodes[chnum[0]].toktype)
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_SWITCH_TABLE);
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_SWITCH_DEFAULT);
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_SWITCH_END);
				compile->AsmScanCaseMinMax(VM_ARG,nodenum,&hasminmax,&min,&max,&hasdefault,&casejmptable);
				if(hasdefault == ZL_TRUE) //�����default�����SWITCH adr%d(��ת���ַ) adr%d(default����ַ)�Ļ���ʽ
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_SWITCH,ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_TABLE,ZL_FALSE),
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_DEFAULT,ZL_FALSE));
				else //���û��default�����SWITCH adr%d(��ת���ַ) adr%d(switch�ṹ�Ľ���λ��)�Ļ���ʽ
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_SWITCH,ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_TABLE,ZL_FALSE),
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_END,ZL_FALSE));
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_LONG,ZL_R_DT_NUM,casejmptable.count,
						ZL_R_DT_NUM,casejmptable.count); //��Ӧ���ָ�� ���� "LONG %d %d" LONG������������������ǵ�ǰswitch...case��ת���е�case����
				if(hasminmax == ZL_TRUE)
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_LONG,ZL_R_DT_NUM,min,
						ZL_R_DT_NUM,max); //��Ӧ���ָ�� ���� "LONG %d %d" LONG����ֱ���case�е���Сֵ�����ֵ
				i = 1;
				j = 0;
				while(i < nodes[nodenum].childs.count)
				{
					if(i >= ZL_AST_CHILD_NODE_SIZE) //������������ӽڵ�����ʹ����չ�ӽڵ�
						tmpch = extnum[i - ZL_AST_CHILD_NODE_SIZE];
					else
						tmpch = chnum[i];
					if(nodes[tmpch].reserve == ZL_RSV_CASE) //����case�鲿�ֵ�ִ�д���
					{
						casejmptable.member[j++].caseAddr = compile->gencode_struct.pc; //����case���ִ�д���Ļ��λ�����õ���ת���caseAddr�ֶ�
						i++;
						if(i < nodes[nodenum].childs.count)
						{
							if(i >= ZL_AST_CHILD_NODE_SIZE) //������������ӽڵ�����ʹ����չ�ӽڵ�
								tmpch = extnum[i - ZL_AST_CHILD_NODE_SIZE];
							else
								tmpch = chnum[i];
							switch(nodes[tmpch].reserve)
							{
							case ZL_RSV_CASE:
							case ZL_RSV_DEFAULT:
								break;
							default:
								while(tmpch > 0) //ѭ�����ɱ�case��Ļ��ִ�д���
								{
									compile->AsmGenCodes(VM_ARG,tmpch);
									tmpch = nodes[tmpch].nextnode;
								}
								i++;
								break;
							}
						}
						if(i >= nodes[nodenum].childs.count)
							run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_JMP,ZL_R_DT_NONE,0,
								ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_END,ZL_FALSE)); //��Ӧ���ָ�� ���� "JMP adr%d" �����ǰ�����һ��case����case����default��JMP��ת��switch...case����λ�ã���switch...case����λ��ǰ��ŵ�����ת�������������JMP������ת
					} //if(nodes[tmpch].reserve == ZL_RSV_CASE) 
					else if(nodes[tmpch].reserve == ZL_RSV_DEFAULT) //����default�鲿�ֵ�ִ�д���
					{
						compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_DEFAULT,ZL_FALSE),compile->gencode_struct.pc);
						i++;
						if(i < nodes[nodenum].childs.count)
						{
							if(i >= ZL_AST_CHILD_NODE_SIZE) //������������ӽڵ�����ʹ����չ�ӽڵ�
								tmpch = extnum[i - ZL_AST_CHILD_NODE_SIZE];
							else
								tmpch = chnum[i];
							while(tmpch > 0) //����default�鲿�ֵĻ�����
							{
								compile->AsmGenCodes(VM_ARG,tmpch);
								tmpch = nodes[tmpch].nextnode;
							}
						}
						run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_JMP,ZL_R_DT_NONE,0,
								ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_END,ZL_FALSE)); //��Ӧ���ָ�� ���� "JMP adr%d" JMP��ת��switch...case�Ľ���λ��
						break;
					} //else if(nodes[tmpch].reserve == ZL_RSV_DEFAULT)
				} //while(i < nodes[nodenum].childs.count)
				if(casejmptable.count > 0)
					compile->AsmSortCaseJmpTable(VM_ARG,&casejmptable,nodenum); //��switch...case����ת����С�����˳���������Ϊ����������ת��Ļ�������׼��
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_TABLE,ZL_FALSE),compile->gencode_struct.pc); 
				if(hasminmax == ZL_TRUE) //�����case��Сֵ�����ֵ��˵��switch...case����case�ڵ㣬���б�Ҫ������ת��Ļ�����
				{
					num = casejmptable.count;
					for(i=0;i<num;i++)	//ѭ��������ת��Ļ�����
					{
						if(casejmptable.member[i].caseAddr != 0)
							run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
								ZL_R_IT_LONG,ZL_R_DT_NUM,casejmptable.member[i].caseAddr,
								ZL_R_DT_NUM,casejmptable.member[i].caseNum); //��Ӧ���ָ�� ���� "LONG %d %d" LONG����ķֱ�����ת���д�ŵ�case��ִ�д����λ�ú�case�ıȽ���
						else
							compile->exit(VM_ARG,ZL_ERR_CP_ASM_CASE_JMP_TABLE_CASEADDR_CAN_NOT_BE_ZERO);
					}
				}
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_END,ZL_FALSE),compile->gencode_struct.pc); //��switch...case�Ľ���λ�ü��뵽��������
				if(hasminmax == ZL_TRUE && casejmptable.member != ZL_NULL)
					compile->memFreeOnce(VM_ARG,casejmptable.member); //�ͷŵ���ǰswitch...case����ת��
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_TABLE,ZL_TRUE);
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_DEFAULT,ZL_TRUE);
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_SWITCH_END,ZL_TRUE);
				state = ZL_ST_DOWN;
			} //if(nodes[nodenum].childs.count >= 1)
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_AT_LEAST_ONE_CHILD);
			}
			break; //case ZL_ST_ASM_CODE_INSWITCH:
		case ZL_ST_ASM_CODE_INWHILE: //while...endwhileѭ���ṹ����������
			if(nodes[nodenum].childs.count == 2)
			{
				chnum = nodes[nodenum].childs.childnum;
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_WHILE_ADDR);
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_WHILE_END);
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_WHILE_ADDR,ZL_FALSE),compile->gencode_struct.pc); //����ͷ�����жϵĻ��λ�ü������Ӷ�̬������
				if(chnum[0] != -1 && ZENGL_AST_ISTOKEXPRESS(chnum[0])) //������while������������жϱ��ʽ�Ļ����
				{
					compile->AsmGenCodes(VM_ARG,chnum[0]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
							ZL_R_IT_JE,ZL_R_DT_NONE,0,
							ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_WHILE_END,ZL_FALSE)); //��Ӧ���ָ�� ���� "JE adr%d" ���������ж��Ƿ���Ҫ����ѭ��
				}
				i = chnum[1];
				while(i > 0) //ѭ������while...endwhile��Ĵ����Ļ����
				{
					compile->AsmGenCodes(VM_ARG,i);
					i = nodes[i].nextnode;
				}
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JMP,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_WHILE_ADDR,ZL_FALSE)); //��Ӧ���ָ�� ���� "JMP adr%d" ѭ����ִ�����������ͷ���������ж�
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_WHILE_END,ZL_FALSE),compile->gencode_struct.pc); //��while����λ�ü��뵽���Ӷ�̬������
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_WHILE_ADDR,ZL_TRUE);
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_WHILE_END,ZL_TRUE);
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_TWO_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INWHILE: //while...endwhileѭ���ṹ����������
		case ZL_ST_ASM_CODE_INDOWHILE: //do...dowhileѭ���ṹ�Ļ���������
			if(nodes[nodenum].childs.count == 2)
			{
				chnum = nodes[nodenum].childs.childnum;
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_DO_ADDR);
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_DO_CONTINUE);
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_DO_END);
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_DO_ADDR,ZL_FALSE),compile->gencode_struct.pc); //��do...dowhile�ṹ��ͷ���λ�ü��뵽���Ӷ�̬������
				i = chnum[0];
				while(i > 0) //ѭ������do...dowhile��Ĵ����Ļ����
				{
					compile->AsmGenCodes(VM_ARG,i);
					i = nodes[i].nextnode;
				}
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_DO_CONTINUE,ZL_FALSE),compile->gencode_struct.pc); //��continue��Ҫ��ת��λ�ü��뵽���Ӷ�̬������
				if(chnum[1] != -1 && ZENGL_AST_ISTOKEXPRESS(chnum[1])) //����dowhile�����������жϱ��ʽ�Ļ�����
				{
					compile->AsmGenCodes(VM_ARG,chnum[1]);
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JNE,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_DO_ADDR,ZL_FALSE)); //��Ӧ���ָ�� ���� "JNE adr%d"
				}
				else
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JMP,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_DO_ADDR,ZL_FALSE)); //��������ж�����ǿյģ���JMP����ѭ������ʱֻ��break���ſ�������ѭ��
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_DO_END,ZL_FALSE),compile->gencode_struct.pc); //��do...dowhile�Ľ���λ�ü��뵽�������Ķ�̬������
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_DO_ADDR,ZL_TRUE);
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_DO_CONTINUE,ZL_TRUE);
				compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_DO_END,ZL_TRUE);
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_TWO_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INDOWHILE: //do...dowhileѭ���ṹ�Ļ���������
		case ZL_ST_ASM_CODE_INCOLON: //����� ... ? ... : ... �е�ð�������
			if(nodes[nodenum].childs.count == 2)
			{
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_QUESTION_END);
				chnum = nodes[nodenum].childs.childnum;
				if(nodes[chnum[0]].toktype == ZL_TK_QUESTION_MARK)
					compile->AsmGenCodes(VM_ARG,chnum[0]);
				else
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
						compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
						nodes[chnum[0]].line_no,
						nodes[chnum[0]].col_no,
						nodes[chnum[0]].filename);
				}
				if(nodes[chnum[1]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[1]))
					compile->AsmGenCodes(VM_ARG,chnum[1]);
				else
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
						compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex),
						nodes[chnum[1]].line_no,
						nodes[chnum[1]].col_no,
						nodes[chnum[1]].filename);
				}
				/*����... ? ... : ...�������ʽ��������ת����ַ������ZL_ASM_STACK_ENUM_QUESTION_END��ջ�е�����
				��Ϊ����ʺ�ǰ�ı��ʽΪTRUE��ʱ����ִ�����ʺź�ı��ʽ�󣬾�Ӧ������ִ�У�����a > b ? test(a) : test(b);���a>b��ִ����test(a)��Ӧ��������
				����Ӧ�ü���ִ��test(b)��*/
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_QUESTION_END,ZL_TRUE),compile->gencode_struct.pc);
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_TWO_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INCOLON: 
		case ZL_ST_ASM_CODE_INQUESTION: //����� ... ? ... : ... �е��ʺ������
			if(nodes[nodes[nodenum].parentnode].toktype != ZL_TK_COLON)
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_QUESTION_NO_COLON_RIGHT);
			}
			if(nodes[nodenum].childs.count == 2) //�жϽڵ��Ƿ��������ӽڵ�
			{
				compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_QUESTION_ADDR);
				chnum = nodes[nodenum].childs.childnum;  //��ȡ�ӽڵ����������
				if(nodes[chnum[0]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[0]))
					compile->AsmGenCodes(VM_ARG,chnum[0]);
				else
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
						compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
						nodes[chnum[0]].line_no,
						nodes[chnum[0]].col_no,
						nodes[chnum[0]].filename);
				}
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JE,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_QUESTION_ADDR,ZL_FALSE)); //��Ӧ���ָ�� ���� "JE adr%d" ����жϽ��ΪFALSE(���е���0����ַ���������FALSE)������תִ�еڶ������ʽ
				if(nodes[chnum[1]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[1]))
					compile->AsmGenCodes(VM_ARG,chnum[1]);
				else
				{
					compile->parser_curnode = nodenum;
					compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
						compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[1]].strindex),
						nodes[chnum[1]].line_no,
						nodes[chnum[1]].col_no,
						nodes[chnum[1]].filename);
				}
				run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_JMP,ZL_R_DT_NONE,0,
						ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_QUESTION_END,ZL_FALSE)); //��Ӧ���ָ�� ���� "JMP adr%d" 
				compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_QUESTION_ADDR,ZL_TRUE),compile->gencode_struct.pc); //�����ʺ��ұ߱��ʽ��β������ת����ַ������ZL_ASM_STACK_ENUM_QUESTION_ADDR��ջ�е���
				state = ZL_ST_DOWN;
			}
			else
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_CURRENT_NODE_MUST_HAVE_TWO_CHILDS);
			}
			break; //case ZL_ST_ASM_CODE_INQUESTION:
		}	//switch(state)
	}	//while(state != ZL_ST_DOWN)
	tmptype = compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_IS_IN_ARRAYITEM_OR_FUNCALL,ZL_FALSE);
	switch(tmptype)
	{
	case ZL_TK_FUNCALL: //������ں��������У���ô�����������ÿ�������������ʽ��ֵ������ѹ��ջ�У������������ܴ�ջ���ҵ���Ҫ�Ĳ�����
		{
			ZL_INT parent = nodes[nodenum].parentnode;
			switch(nodes[parent].toktype)
			{
			case ZL_TK_COMMA:
			case ZL_TK_FUNCALL:
				if(nodes[nodenum].toktype != ZL_TK_COMMA)
				{
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_PUSH,ZL_R_DT_NONE,0,
						ZL_R_DT_REG,ZL_R_RT_AX); //��Ӧ���ָ�� "PUSH AX" 
				}
				break;
			}
		}
		break;
	case ZL_TK_ARRAY_ITEM: //���������������Ԫ������������ʽʱ�������PUSH AX�����ʽ��ֵѹջ��Ϊ���������ֵ����test[a+b]��䣬a+bִ�����ͻ�PUSH AX��a+b�Ľ��ѹ��ջ������Ϊtest���������ֵ��
		{
			ZL_INT parent = nodes[nodenum].parentnode;
			switch(nodes[parent].toktype)
			{
			case ZL_TK_COMMA:
			case ZL_TK_ARRAY_ITEM:
				if(nodes[nodenum].toktype != ZL_TK_COMMA)
				{
					run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
						ZL_R_IT_PUSH,ZL_R_DT_NONE,0,
						ZL_R_DT_REG,ZL_R_RT_AX); //��Ӧ���ָ�� "PUSH AX" 
				}
				break;
			}
		}
		break;
	} //switch(tmptype)
} //ZL_VOID zengl_AsmGenCodes(ZL_VOID * VM_ARG,ZL_INT nodenum)

/*
	������ѹ�����ջ
*/
ZL_INT zengl_AsmGCStackPush(ZL_VOID * VM_ARG,ZL_INT num,ZENGL_ASM_STACK_ENUM type)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZL_INT i,index=-1;
	if(!compile->AsmGCStackList.isInit)
		compile->AsmGCStackInit(VM_ARG);
	i=0;
find_index:
	for(;i < compile->AsmGCStackList.size - 1;i++)
	{
		if(compile->AsmGCStackList.stacks[i].isvalid == ZL_FALSE)
		{
			index = i;
			break;
		}
	}
	if(index == -1)
	{
		compile->AsmGCStackList.size += ZL_ASM_STACK_LIST_SIZE;
		compile->AsmGCStackList.stacks = compile->memReAlloc(VM_ARG,compile->AsmGCStackList.stacks,compile->AsmGCStackList.size * sizeof(ZENGL_ASM_STACK_TYPE));
		ZENGL_SYS_MEM_SET(compile->AsmGCStackList.stacks + (compile->AsmGCStackList.size - ZL_ASM_STACK_LIST_SIZE),0,
			ZL_ASM_STACK_LIST_SIZE * sizeof(ZENGL_ASM_STACK_TYPE));
		i = compile->AsmGCStackList.size - ZL_ASM_STACK_LIST_SIZE;
		goto find_index;
	}
	compile->AsmGCStackList.stacks[index].addrnum = num;
	compile->AsmGCStackList.stacks[index].addrtype = type;
	compile->AsmGCStackList.stacks[index].before_index = compile->AsmGCStackList.ends[type];
	compile->AsmGCStackList.stacks[index].isvalid = ZL_TRUE;
	compile->AsmGCStackList.ends[type] = index;
	compile->AsmGCStackList.count++;
	return num;
}

/*
	�������ջ
*/
ZL_INT zengl_AsmGCStackPop(ZL_VOID * VM_ARG,ZENGL_ASM_STACK_ENUM type,ZL_BOOL isremove)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZL_INT end=-1,retnum;
	if(!compile->AsmGCStackList.isInit) //��ѹջ�͵���ջʱ��Ҫ�ж��Ƿ��ʼ�������û�г�ʼ����Ҫ���г�ʼ����
		compile->AsmGCStackInit(VM_ARG);
	if(compile->AsmGCStackList.count <= 0)
	{
		return -1;
	}
	end = compile->AsmGCStackList.ends[type];
	if(end == -1)
		return -1;
	if(compile->AsmGCStackList.stacks[end].isvalid == ZL_TRUE && compile->AsmGCStackList.stacks[end].addrtype == type)
	{
		if(isremove == ZL_TRUE)
		{
			retnum = compile->AsmGCStackList.stacks[end].addrnum;
			compile->AsmGCStackList.ends[type] = compile->AsmGCStackList.stacks[end].before_index;
			compile->AsmGCStackList.stacks[end].isvalid = ZL_FALSE;
			compile->AsmGCStackList.count--;
			return retnum;
		}
		else
			return compile->AsmGCStackList.stacks[end].addrnum;
	}
	return -1;
}

/*
	����ջ��ʼ��
*/
ZL_VOID zengl_AsmGCStackInit(ZL_VOID * VM_ARG)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZL_INT i;
	if(compile->AsmGCStackList.isInit)
		return;
	compile->AsmGCStackList.count = 0;
	compile->AsmGCStackList.size = ZL_ASM_STACK_LIST_SIZE;
	compile->AsmGCStackList.stacks = compile->memAlloc(VM_ARG,compile->AsmGCStackList.size * sizeof(ZENGL_ASM_STACK_TYPE));
	if(compile->AsmGCStackList.stacks == ZL_NULL)
		compile->exit(VM_ARG,ZL_ERR_CP_ASM_GCSTACK_LIST_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(compile->AsmGCStackList.stacks, 0 , compile->AsmGCStackList.size * sizeof(ZENGL_ASM_STACK_TYPE));
	for(i=0;i<ZL_ASM_ADDR_TYPE_NUM;i++)
		compile->AsmGCStackList.ends[i] = -1;
	compile->AsmGCStackList.isInit = ZL_TRUE;
}

/*
	zengl_AsmGCElif������������elif������Ӧ�Ļ��ָ��
*/
ZL_VOID zengl_AsmGCElif(ZL_VOID * VM_ARG,ZENGL_AST_CHILD_NODE_TYPE * ifchnum,ZL_INT num)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZENGL_AST_NODE_TYPE * nodes = compile->AST_nodes.nodes;
	ZL_INT * chnum = ZL_NULL;
	ZL_INT nodenum;
	ZL_INT lastNodeNum;
	if(num < ZL_AST_CHILD_NODE_SIZE)
		nodenum = ifchnum->childnum[num];
	else
		nodenum = ifchnum->extchilds[num - ZL_AST_CHILD_NODE_SIZE];
	if(nodes[nodenum].childs.count == 1)  //����elif��Ӧ���жϱ��ʽ�Ļ��ָ��
	{
		chnum = nodes[nodenum].childs.childnum;
		if(chnum[0] == -1)
		{
			compile->parser_curnode = nodenum;
			compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_IF_HAVE_NO_EXPRESS);
		}
		else if(nodes[chnum[0]].tokcategory == ZL_TKCG_OP_FACTOR || ZENGL_AST_ISTOKEXPRESS(chnum[0])) //�����ı��������֣��ַ���֮��Ĳ������ӻ��߱��ʽ��������Ϊif...elif���жϱ��ʽ
		{
			compile->AsmGenCodes(VM_ARG,chnum[0]);
			run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
				ZL_R_IT_JE,ZL_R_DT_NONE,0,
				ZL_R_DT_LDADDR,compile->AsmGCStackPush(VM_ARG,compile->AsmGCAddrNum++,ZL_ASM_STACK_ENUM_IF_ADDR)); //��Ӧ���ָ�� "JE adr%d"
		}
		else
		{
			compile->parser_curnode = nodenum;
			compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_INVALID_CHILD_NODE_TYPE,
				compile->TokenStringPoolGetPtr(VM_ARG,nodes[chnum[0]].strindex),
				nodes[chnum[0]].line_no,
				nodes[chnum[0]].col_no,
				nodes[chnum[0]].filename);
		}
	}
	else
	{
		compile->parser_curnode = nodenum;
		compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_PRINT_MUST_HAVE_ONE_CHILD);
	}
	if(num + 1 < ZL_AST_CHILD_NODE_SIZE)
		nodenum = ifchnum->childnum[num + 1];
	else
		nodenum = ifchnum->extchilds[num + 1 - ZL_AST_CHILD_NODE_SIZE];
	while(nodenum > 0)  //ѭ������elif����Ĵ����Ļ��ָ��
	{
		lastNodeNum = nodenum;
		compile->AsmGenCodes(VM_ARG,nodenum);
		nodenum = nodes[nodenum].nextnode;
	}
	run->AddInst(VM_ARG,compile->gencode_struct.pc++,lastNodeNum,
			ZL_R_IT_JMP,ZL_R_DT_NONE,0,
			ZL_R_DT_LDADDR,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_IF_END,ZL_FALSE)); //��Ӧ���ָ�� "JMP adr%d" elif�����ִ����Ϻ�JMP��������ת��if-elif-else�Ľ���λ��
	compile->LDAddrListSet(VM_ARG,compile->AsmGCStackPop(VM_ARG,ZL_ASM_STACK_ENUM_IF_ADDR,ZL_TRUE),compile->gencode_struct.pc); //elif�жϱ��ʽΪfalseʱ����תλ��
}

/*
	break���Ļ��������ɣ�break�����for��������for�������switch��������switch
	�����while�ṹ��������while�������do...dowhile��������do...dowhile
*/
ZL_VOID zengl_AsmGCBreak_Codes(ZL_VOID * VM_ARG,ZL_INT nodenum)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT asm_ends[4] = {ZL_ASM_STACK_ENUM_FOR_END,ZL_ASM_STACK_ENUM_SWITCH_END,ZL_ASM_STACK_ENUM_WHILE_END,ZL_ASM_STACK_ENUM_DO_END};
	ZL_INT i,maxend,adrnum;
	for(i=1,maxend=0;i<4;i++)
	{
		if(compile->AsmGCStackList.ends[asm_ends[maxend]] < compile->AsmGCStackList.ends[asm_ends[i]])
			maxend = i;
	}
	adrnum = compile->AsmGCStackPop(VM_ARG,asm_ends[maxend],ZL_FALSE);
	if(adrnum == -1)
	{
		compile->parser_curnode = nodenum;
		compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_UNEXPECT_LOC_OF_BREAK_CONTINUE);
	}
	else
		run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
			ZL_R_IT_JMP,ZL_R_DT_NONE,0,
			ZL_R_DT_LDADDR,adrnum); //��Ӧ���ָ�� "JMP adr%d" ��ת��for��switch��while��do...dowhile��β��
}

/*
	continue���Ļ��������ɣ��ж�����for������while������do...dowhile�Ȼ��
	�����ж�������Ӧ�ṹ�Ŀ�ͷ
*/
ZL_VOID zengl_AsmGCContinue_Codes(ZL_VOID * VM_ARG,ZL_INT nodenum)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZENGL_RUN_TYPE * run = &((ZENGL_VM_TYPE *)VM_ARG)->run;
	ZL_INT asm_ends[3] = {ZL_ASM_STACK_ENUM_FOR_CONTINUE,ZL_ASM_STACK_ENUM_WHILE_ADDR,ZL_ASM_STACK_ENUM_DO_CONTINUE};
	ZL_INT i,maxend,adrnum;
	for(i=1,maxend=0;i<3;i++)
	{
		if(compile->AsmGCStackList.ends[asm_ends[maxend]] < compile->AsmGCStackList.ends[asm_ends[i]])
			maxend = i;
	}
	adrnum = compile->AsmGCStackPop(VM_ARG,asm_ends[maxend],ZL_FALSE);
	if(adrnum == -1)
	{
		compile->parser_curnode = nodenum;
		compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_UNEXPECT_LOC_OF_BREAK_CONTINUE);
	}
	else
		run->AddInst(VM_ARG,compile->gencode_struct.pc++,nodenum,
			ZL_R_IT_JMP,ZL_R_DT_NONE,0,
			ZL_R_DT_LDADDR,adrnum); //��Ӧ���ָ�� "JMP adr%d" ��ת��for��while��do...dowhile��continue��Ӧλ��
}

/*
    ɨ��switch...case ���ҳ����е�case�����ֵ����Сֵ���Լ��ж��Ƿ���defaultĬ�Ͻڵ㡣
	����nodenumΪswitch�ڵ�Ľڵ�š�
	����hasminmax�����ж��Ƿ���������Сֵ��
	����minarg���ڱ���case�е���Сֵ��
	����maxarg���ڱ���case�е����ֵ��
	����hasdefault�����ж��Ƿ���default�ڵ㡣
	����table��switch...case����ת��
*/
ZL_VOID zengl_AsmScanCaseMinMax(ZL_VOID * VM_ARG,ZL_INT nodenum,ZL_BOOL * hasminmax,ZL_INT * minarg,ZL_INT * maxarg,ZL_BOOL * hasdefault,
								ZENGL_ASM_CASE_JMP_TABLE * table)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZENGL_AST_NODE_TYPE * nodes = compile->AST_nodes.nodes;
	ZL_INT * chnum = ZL_NULL,* extnum = ZL_NULL,i;
	ZL_LONG num,min=0,max=0;
	ZENGL_AST_CHILD_NODE_TYPE * childs = &nodes[nodenum].childs;
	(*hasminmax) = ZL_FALSE;
	(*hasdefault) = ZL_FALSE;
	if(childs->count == 1)
		return;
	chnum = childs->childnum;
	extnum = childs->extchilds;
	for(i=1;i<nodes[nodenum].childs.count;i++)
	{
		if(i < ZL_AST_CHILD_NODE_SIZE)
			num = chnum[i];
		else
			num = extnum[i - ZL_AST_CHILD_NODE_SIZE];
		switch(nodes[num].reserve)
		{
		case ZL_RSV_CASE:
			num = compile->GetNodeInt(VM_ARG,nodes[num].childs.childnum[0]);
			compile->AsmAddCaseJmpTable(VM_ARG,table,num);
			if(i == 1)
				min = max = num;
			else
			{
				min = num < min ? num : min;
				max = num > max ? num : max;
			}
			(*hasminmax) = ZL_TRUE;
			break;
		case ZL_RSV_DEFAULT:
			(*hasdefault) = ZL_TRUE;
			break;
		}
	}
	(*minarg) = min;
	(*maxarg) = max;
}

/*���ؽڵ���ַ�����Ϣ��������ʽ*/
ZL_LONG zengl_GetNodeInt(ZL_VOID * VM_ARG,ZL_INT nodenum)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZENGL_AST_NODE_TYPE * nodes = compile->AST_nodes.nodes;
	switch(nodes[nodenum].toktype)
	{
	case ZL_TK_NUM:
	case ZL_TK_FLOAT:
	case ZL_TK_STR:
		return ZENGL_SYS_STR_TO_LONG_NUM(compile->TokenStringPoolGetPtr(VM_ARG,nodes[nodenum].strindex));
		break;
	}
	return 0;
}

/*��case��Ӧ�ıȽ�������ӵ���ת����*/
ZL_VOID zengl_AsmAddCaseJmpTable(ZL_VOID * VM_ARG,ZENGL_ASM_CASE_JMP_TABLE * table,ZL_LONG num)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	if(!table->isInit)
		compile->AsmInitCaseJmpTable(VM_ARG,table);
	if(table->count == table->size)
	{
		table->size += ZL_ASM_CASE_JMP_TABLE_SIZE;
		table->member = compile->memReAlloc(VM_ARG,table->member,table->size * sizeof(ZENGL_ASM_CASE_JMP_TABLE_MEMBER));
		if(table->member == ZL_NULL)
			compile->exit(VM_ARG,ZL_ERR_CP_ASM_CASE_JMP_TABLE_REALLOC_FAILED);
		ZENGL_SYS_MEM_SET(table->member + (table->size - ZL_ASM_CASE_JMP_TABLE_SIZE),0,
			ZL_ASM_CASE_JMP_TABLE_SIZE * sizeof(ZENGL_ASM_CASE_JMP_TABLE_MEMBER));
	}
	table->member[table->count].caseNum = num;
	table->count++;
}

/*��ʼ��switch case����ת��*/
ZL_VOID zengl_AsmInitCaseJmpTable(ZL_VOID * VM_ARG,ZENGL_ASM_CASE_JMP_TABLE * table)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	if(table->isInit)
		return;
	table->count = 0;
	table->size = ZL_ASM_CASE_JMP_TABLE_SIZE;
	table->member = compile->memAlloc(VM_ARG,table->size * sizeof(ZENGL_ASM_CASE_JMP_TABLE_MEMBER));
	if(table->member == ZL_NULL)
		compile->exit(VM_ARG,ZL_ERR_CP_ASM_CASE_JMP_TABLE_MALLOC_FAILED);
	ZENGL_SYS_MEM_SET(table->member,0,table->size * sizeof(ZENGL_ASM_CASE_JMP_TABLE_MEMBER));
	table->isInit = ZL_TRUE;
}

/*��switch...case��ת����д�С���������*/
ZL_VOID zengl_AsmSortCaseJmpTable(ZL_VOID * VM_ARG,ZENGL_ASM_CASE_JMP_TABLE * table,ZL_INT nodenum)
{
	ZENGL_COMPILE_TYPE * compile = &((ZENGL_VM_TYPE *)VM_ARG)->compile;
	ZENGL_AST_NODE_TYPE * nodes = compile->AST_nodes.nodes;
	ZL_INT i,j,k;
	ZENGL_ASM_CASE_JMP_TABLE_MEMBER temp;
	for(i=0;i<table->count-1;i++)
	{
		k = i;
		for(j=i+1;j<table->count;j++)
		{
			if(table->member[k].caseNum > table->member[j].caseNum)
				k = j;
			else if(table->member[k].caseNum == table->member[j].caseNum)
			{
				compile->parser_curnode = nodenum;
				compile->parser_errorExit(VM_ARG,ZL_ERR_CP_SYNTAX_ASM_SWITCH_FIND_SAME_CASE_VAL);
			}
		}
		if(k != i)
		{
			temp = table->member[i];
			table->member[i] = table->member[k];
			table->member[k] = temp;
		}
	}
}
