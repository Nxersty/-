#ifndef USER_USER_PID_H_
#define USER_USER_PID_H_

#include "stdint.h"
#include "stdio.h"

#define PID_MIN_TURN 0        // ת��ʱPID�����Сֵ
#define PID_MIN_STRAIGHT 0    // ֱ��ʱPID�����Сֵ
#define PID_MAX 3000          // PID������ֵ
#define I_MIN -1000           // ������Сֵ
#define I_MAX 1000            // �������ֵ
#define AVR_SIZE 5            // �ƶ�ƽ������

typedef struct PIDError {
        float lLast;
        float lLastD;
        float lTotal;
        float rLast;
        float rLastD;
        float rTotal;
} PIDError;

extern PIDError ErrorInc;
extern PIDError ErrorLoc;

float constrainFloat(float num, float min, float max);
void getPIDIncResult(sint32* speedL, sint32* speedR, float errorL,
                     float errorR);
void getPIDLocResult(sint32* speedL, sint32* speedR, double gzDeg,
                     double gzDegTarget);

#endif /* USER_USER_PID_H_ */