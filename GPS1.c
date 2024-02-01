#include "zf_common_headfile.h"
#include "GPS1.h"

//1.��ȷ�������
//2.������������Ƕȼ��㺯��--�����
//3.IMU--�����
//4.�÷����-�����=�Ƕ����
//��GPS�����--FLASH/����

#if Double_Record_FLAG

int      Number=0;//���ʱ���

double   Target_point[2][150];//���ڴ���ɼ���Ŀ�����Ϣ�����ں�����λ�ü���(��һ����γ�ȣ��ڶ����Ǿ���)
double   Work_target_array[2][150];//���ڽ���flash�ж�ȡ��Ŀ�����������ݸ��ƹ�������������鱻��ֵʱ��������һ�к�Flash���޹�ϵ
double   lat; //γ��(Ӣ�����FLASHֻ�ܴ�32λ���ݣ�������double���Ͷ���ı����浽������ֻ��ת��float)
double   lot; //����(Ӣ�����FLASHֻ�ܴ�32λ���ݣ�������double���Ͷ���ı����浽������ֻ��ת��float)

uint32   I_lat;//double����γ��������������
float    F_lat;//double����γ������С������(ǿ��ת��Ϊfloat)(������float���Ͷ�������Ϊ����������λ0ʱ��float���Ϳ��Դ浽��7λ)

uint32   I_lot;//double���;���������������
float    F_lot;//double���;�������С������(ǿ��ת��Ϊfloat)

void GPS_Record_flash()    //���ɼ��ĵ�λ��¼����������������GPS_FLASH
{
    static int NUM=0;       //�ɼ���Ĵ���
    {
        lat=gnss.latitude;   //γ��(ԭʼ������double���ͱ����洢,14λ)
        lot=gnss.longitude;  //����(ԭʼ������double���ͱ����洢,13λ)

        I_lat=(uint32)(lat);//����ǿת��latȡ���õ���������
        F_lat=lat-I_lat; //��double��������-�������ֵõ�С�����ֲ�ǿ��ת��Ϊfloat����

        I_lot=(uint32)(lot);//����ǿת��lotȡ���õ���������
        F_lot=lot-I_lot; //��double��������-�������ֵõ�С�����ֲ�ǿ��ת��Ϊfloat����

        ips_show_string(0, 16*0,"R:");

        {
            flash_union_buffer[Number].uint32_type=I_lat;  //������ά������ǿ��ת���󴢴���FLASH ���������ݻ�����
            ips_show_uint (50, 16*0, flash_union_buffer[Number].uint32_type, 5);  //��ʾ��������
            ips_show_uint(150, 16*0, Number, 3);//����������λ��

            Number++;

            flash_union_buffer[Number].float_type=F_lat; //��������ά������ǿ��ת���󴢴���FLASH ���������ݻ�����
            ips_show_float(50, 16*1, flash_union_buffer[Number].float_type, 3, 6);//��ʾС������
            ips_show_uint(150, 16*1, Number, 3);//����������λ��

            ips_show_float(50, 16*2, gnss.latitude, 3, 6);//��ʾ��������

        }

         Number++;//�����±�+1,�л����澭��

        {
            flash_union_buffer[Number].uint32_type=I_lot;  //��������������ǿ��ת���󴢴���FLASH ���������ݻ�����
            ips_show_uint (50, 16*4, flash_union_buffer[Number].uint32_type, 5);  //��ʾ��������
            ips_show_uint(150, 16*4, Number, 3);//����������λ��

            Number++;

            flash_union_buffer[Number].float_type=F_lot; //����������������ǿ��ת���󴢴���FLASH ���������ݻ�����
            ips_show_float(50, 16*5, flash_union_buffer[Number].float_type, 3, 6);//��ʾС������
            ips_show_uint(150, 16*5, Number, 3);//����������λ��

            ips_show_float(50, 16*6, gnss.longitude, 3, 6);//��ʾ��������
        }


        Number++;//�����±�+1,�л�����γ��

                //�����û���ֶ����GPS_FLASH����������������Զ����GPS_FLASH������������:ÿ�βɼ���λ֮ǰ�����GPS_FLASH
        if(flash_check(FLASH_SECTION_INDEX, GPS_PAGE_INDEX))                      //�ж�Flash�Ƿ������� : �����ݷ���1�������ݷ���0 (�ú���ÿ������ִ��һ�Σ���ִ�б�����ʱ,GPS_FLASHû�ж��������Բ���Ҳ�޷�)                                                               //
        {
          flash_erase_page(FLASH_SECTION_INDEX, GPS_PAGE_INDEX);                //����Flash����
        }

          flash_write_page_from_buffer(FLASH_SECTION_INDEX, GPS_PAGE_INDEX);        //��������������д�뵽ָ��Flash ������ҳ��

    }

    NUM++;

       if(NUM>RP_MAX)                                                               //����ɼ���������ڹ涨���������ֵ������������1
           {
             NUM=1;
           }

       ips_show_uint(210, 0, NUM, 3);                                              //��ʾ�Ѳɼ��ĵ���


}


void GPS_Flash_use()//��GPS_FLASH���������¶��ػ���������ֵ������
{

    flash_buffer_clear();//��ջ�����

    if(flash_check(FLASH_SECTION_INDEX, GPS_PAGE_INDEX))                      //�ж�Flash�Ƿ������� : �����ݷ���1�������ݷ���0
    {
        flash_read_page_to_buffer(FLASH_SECTION_INDEX, GPS_PAGE_INDEX);       //�����ݴ�FLASHָ������ҳ����뵽������

        int TG=0;//�����л���ά�����±�

        for(int data=0;data<200;data++)
          {

            Target_point[0][TG]=flash_union_buffer[data].uint32_type+flash_union_buffer[data+1].float_type; //γ(�����ƺ�Ҫ��double���͵�����,��Ϊ����������������һ��float���͵�С������)

            data=data+2;

            Target_point[1][TG]=flash_union_buffer[data].uint32_type+flash_union_buffer[data+1].float_type; //��

            data=data+1;
            TG++;//�±�+1

          }


    }

}


#endif