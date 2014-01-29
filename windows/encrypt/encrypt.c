#define ZL_EXP_OS_IN_WINDOWS //�ڼ��������zenglͷ�ļ�֮ǰ��windowsϵͳ�붨��ZL_EXP_OS_IN_WINDOWS��linux , macϵͳ�붨��ZL_EXP_OS_IN_LINUX
#include "zengl_exportfuns.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>

enum ENCRYPT_TYPE{
	ENC_TYPE_NONE = 0,
	ENC_TYPE_XOR = 1,
	ENC_TYPE_RC4 = 2,
};

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
	enum ENCRYPT_TYPE isRC4 = ENC_TYPE_NONE;
	if(argc != 6)
	{
		printf("usage: %s <src> <dest> <key> -t <xor|rc4> (�÷�����Ӧ���ǳ�������"
				"��srcԴ�ļ�����destҪ���ɵ�Ŀ���ļ�����key������Կ��\n"
				"-t����ָ�����ܷ�ʽ(-t�����Ͳ���ֵ�ŵ����������ĺ���)��xor��ʾ��ͨ�����ܣ�rc4��ʾRC4����)",argv[0]);
		#ifdef ZL_EXP_OS_IN_WINDOWS
			system("pause");
		#endif
		exit(-1);
	}

	if(strcmp(argv[4],"-t") == 0)
	{
		if(strcmp(argv[5],"xor") == 0)
			isRC4 = ENC_TYPE_XOR;
		else if(strcmp(argv[5],"rc4") == 0)
			isRC4 = ENC_TYPE_RC4;
		else
		{
			printf("���ܷ�ʽֻ������ͨ�����ܺ�RC4����ʹ��-t xor��������ʾ��ͨ�����ܷ�ʽ��-t rc4��������ʾRC4��ʽ\n");
			#ifdef ZL_EXP_OS_IN_WINDOWS
				system("pause");
			#endif
			exit(-1);
		}
	}

	if(isRC4 == ENC_TYPE_XOR)
		main_output_xor_source(argv[1],argv[2],argv[3]);
	else if(isRC4 == ENC_TYPE_RC4)
		main_output_rc4_source(argv[1],argv[2],argv[3]);
	else
		printf("��û��ָ�����ܷ�ʽ����ʹ��-t xor��������ʾ��ͨ�����ܷ�ʽ��-t rc4��������ʾRC4��ʽ��-t������ŵ����������ĺ���");

	#ifdef ZL_EXP_OS_IN_WINDOWS
		system("pause");
	#endif

	return 0;
}
