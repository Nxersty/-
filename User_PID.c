#include "User_PID.h"

PIDError ErrorInc = {0};
PIDError ErrorLoc = {0};
float KpInc = 2000,KiInc = 200,KdInc = 0;
float KpLoc = 30, KiLoc = 10, KdLoc = -10;
int DiffMax = 3000; // �����ٶȲ����ֵ
int Dir[9] = {0, -1, -3, -5, -7, 7, 5, 3, 1};
int DirAvr[AVR_SIZE] = {0};
float DirAvrWeight[AVR_SIZE] = {0.1, 0.1, 0.2, 0.2, 0.4};
char PIDTxt[128];

/*************************************************************************
 *  �������ƣ�float constrainFloat(float num, float min, float max)
 *  ����˵��������float��һ����Χ
 *  ����˵����#1 float num �����Ƶ��� #2 float min ��Сֵ #3 float max ���ֵ
 *  �������أ�float ���ƺ����ֵ
 *  ��    ע��
 *************************************************************************/
float constrainFloat(float num, float min, float max) {
    return ((num) < (min) ? (min) : ((num) > (max) ? (max) : (num)));
}

/*************************************************************************
 *  �������ƣ�float movingAverage(int)
 *  ����˵����ȡ�ƶ�ƽ��ֵ
 *  ����˵����#1 dir ���·���ֵ
 *  �������أ��ƶ�ƽ��ֵ
 *  ��    ע��
 *************************************************************************/
float movingAverage(int dir) {
    int i;
    float avr = 0;
    for (i = 0; i < AVR_SIZE - 1; i++) {
        DirAvr[i] = DirAvr[i + 1];
        avr += DirAvr[i] * DirAvrWeight[i];
    }
    avr += dir * DirAvrWeight[i];
    DirAvr[i] = dir;
    return avr;
}

float movingAverage(int dir) {
    int i;
    float avr = 0;
    for (i = 0; i < AVR_SIZE - 1; i++) {
        DirAvr[i] = DirAvr[i + 1];
        avr += DirAvr[i] * DirAvrWeight[i];
    }
    avr += dir * DirAvrWeight[i];
    DirAvr[i] = dir;
    return avr;
}

/*************************************************************************
 *  �������ƣ�void getPIDIncResult(sint32* speedL, sint32* speedR, float errorL, float errorR)
 *  ����˵����PID��ֱ��
 *  ����˵����
 *  �������أ���
 *  ��    ע��
 *************************************************************************/
void getPIDIncResult(sint32* speedL, sint32* speedR, float errorL, float errorR) {
#ifdef DEBUG
    sprintf(PIDTxt, "errorL = %f\n", errorL);
    UART_PutStr(UART0, PIDTxt);
#endif

    float outL = *speedL + KpInc * (errorL - ErrorInc.lLast) + KiInc * errorL
                 + KdInc * ((errorL - ErrorInc.lLast) - ErrorInc.lLastD);
    *speedL = (sint32)constrainFloat(outL, PID_MIN_STRAIGHT, PID_MAX);

    float outR = *speedR + KpInc * (errorR - ErrorInc.rLast) + KiInc * errorR
                 + KdInc * ((errorR - ErrorInc.rLast) - ErrorInc.rLastD);
    *speedR = (sint32)constrainFloat(outR, PID_MIN_STRAIGHT, PID_MAX);

    if (*speedL > *speedR) {
        *speedL = (sint32)constrainFloat(*speedL, *speedR, *speedR + DiffMax);
    }
    else {
        *speedR = (sint32)constrainFloat(*speedR, *speedL, *speedL + DiffMax);
    }

    ErrorInc.lLastD = errorL - ErrorInc.lLast;
    ErrorInc.rLastD = errorR - ErrorInc.rLast;
    ErrorInc.lLast = errorL;
    ErrorInc.rLast = errorR;
}

/*************************************************************************
 *  �������ƣ�void getPIDLocResult (sint32*, sint32*, double, double)
 *  ����˵����PIDת����ȷ����
 *  ����˵����#1 sint32* speedL ����ת�� #2 sint32* speedR ����ת��
 *           #3 double gzDeg ��ǰ�Ƕ� #4 double gzDegTarget Ŀ��Ƕ�
 *  �������أ���
 *  ��    ע��
 *************************************************************************/
void getPIDLocResult(sint32* speedL, sint32* speedR, double gzDeg,
                     double gzDegTarget) {
    float errorL = gzDeg - gzDegTarget;
    float errorR = gzDegTarget - gzDeg;

#ifdef DEBUG
    sprintf(PIDTxt, "AngleError = %f\n", errorR);
    UART_PutStr(UART0, PIDTxt);
#endif

    ErrorLoc.lTotal += errorL;
    ErrorLoc.rTotal += errorR;

    float pOutL = KpLoc * errorL;
    float iOutL = KiLoc * ErrorLoc.lTotal;
    float dOutL = KdLoc * ErrorLoc.lLast;
    float pOutR = KpLoc * errorR;
    float iOutR = KiLoc * ErrorLoc.rTotal;
    float dOutR = KdLoc * ErrorLoc.rLast;

    iOutL = constrainFloat(iOutL, I_MIN, I_MAX);
    iOutR = constrainFloat(iOutR, I_MIN, I_MAX);

    float outL = pOutL + iOutL + dOutL;
    *speedL = (sint32)constrainFloat(outL, PID_MIN_TURN, PID_MAX);

    float outR = pOutR + iOutR + dOutR;
    *speedR = (sint32)constrainFloat(outR, PID_MIN_TURN, PID_MAX);

    /*if (*speedL > *speedR) {
     *speedL = constrainFloat(*speedL, *speedR, *speedR + DiffMax);
     }
     else {
     *speedR = constrainFloat(*speedR, *speedL, *speedL + DiffMax);
     }*/

    ErrorLoc.lLast = errorL;
    ErrorLoc.rLast = errorR;
}
