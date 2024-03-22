/*********************************************************************************************************************
* CYT4BB Opensourec Library ���� CYT4BB ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
*
* ���ļ��� CYT4BB ��Դ���һ����
*
* CYT4BB ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
*
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
*
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
*
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ���֤Э�� �����������Ϊ���İ汾
* �������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ���֤������ libraries �ļ����� �����ļ����µ� LICENSE �ļ�
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
*
* �ļ�����          main_cm7_0
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          IAR 9.40.1
* ����ƽ̨          CYT4BB
* ��������          https://seekfree.taobao.com/
*
* �޸ļ�¼
* ����              ����                ��ע
* 2024-1-4       pudding            first version
********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "SoundLocation_v1.h"


// �궨��-----------------------------------------------------------------------
#define ADC_TO_V_F 0.000805664 // ADCֵ����ѹ(V)ת��ϵ��(3.3 / 4096)
#define DEBOUNCE_TIME 300      // ��������ʱ��(ms)
#define V_TEST_TO_REAL_F 4     // ��ѹ���˿ڵ�ѹ��ʵ�ʵ�ѹת��ϵ��
#define CHARGE_THRESHOLD 12    // ���������ѹֵ(V)
#define INIT_SPEED 3000        // �����ʼ�ٶ�(ռ�ձ�20%)
#define TURNING_SPEED 2000     // ת��ʱ�����ٶ�(ռ�ձ�20%)
#define TURN_TIME_MAX 6000     // ת�����ʱ��(ms)
#define BRAKE_TIME 500         // ת���ɲ��ʱ��(ms)
#define SPEED_TIMEOUT 1000     // ����ʱ������(ms)(�������ʱ�仹û��ת��1/6Ȧ��Ϊ�ٶ�Ϊ0)
#define CTRL_INTERVAL 30       // ���ƴ�������ʱ����(ms)
#define TURN_TARGET_F 0.8      // ת��Ŀ��Ƕ�ϵ��
#define TURN_SPEED 0.2         // ת�䶯�����ٶ�(m/s)
#define TURN_STEP 20           // ת�䶯���ֲ���ֵ(ռ�ձ�)
#define TURN_INIT_SPEED 1500   // ת�䶯���ֳ�ʼֵ(ռ�ձ�)

// �������Ͷ���------------------------------------------------------------------
typedef enum mode_t{
    Idle,Charging,RunningStraight,Turning
} mode_t;

typedef enum debugMode_t{
    Normal,NormalDebug,StraightDeubug,TurnDebug
} debugMode_t;



// ȫ�ֱ���---------------------------------------------------------------------    
mode_t Mode = Idle;     //��ʾ��ǰ����������ģʽ
debugMode_t DebugMode = Normal;     //��ʾ��ǰ�ĵ���ģʽ
direction_t Direction = Forward;    //���ڱ�ʾ�������н����򣬿����� Forward��ǰ�������������Ƶķ���
sint32 SpeedL = 0;  //�ֱ��ʾ�����ֵ��ٶ�
sint32 SpeedR = 0;

signed short mpu6050_gyro_x , mpu6050_gyro_y , mpu6050_gyro_z = 0;    //�ֱ��ʾ�������������������ݡ�
signed short mpu6050_acc_x , mpu6050_acc_y , mpu6050_acc_z = 0;    //�ֱ��ʾ������������ٶȼ�����
double GzDegPerSec = 0;     //��ʾ�����ǵĽ��ٶȺͽǶ�
double GzDeg = 0;
double AxMPerSqrSec = 0;    //��ʾ���ٶȼƵļ��ٶ�
double AxMPerSec = 0;

signed short Gx , Gy , Gz = 0;
signed short Ax , Ay , Az = 0;
double GzDegPerSec = 0;
double GzDeg = 0;
double AxMPerSec = 0;
double AxMPerSec = 0;

int DebugDeg = 8;   //���ڵ��ԵĽǶȺͷ���
int DebugDir = BackRight;

float StraightSpeed = 1; // ֱ���ٶ�(m/s)
float StraightGzTolerance = 100; // ֱ�н��ٶ��ݲ����ֵ

sint32 TurnSpeedL[9] = {0, 0, 2500, 2300, 2000, 0, 0, 0, 0};    //�ֱ��ʾ���ֺ������ڲ�ͬת������µ��ٶ�
sint32 TurnSpeedR[9] = {0, 0, 0, 0, 0, 2000, 2300, 2500, 0};

long LastTurnTick = 0;  //��¼��һ��ת������������ݸ��µ�ʱ��
long LastMPUTick = 0;

volatile long TickMillis = 0;

volatile long LastSpeedTickL = 0;   //��¼�������ٶȵ���һ�θ���ʱ��
volatile long LastSpeedTickR = 0;

volatile float SpeedLReal = 0;  //��¼ʵ�ʵ��������ٶ�
volatile float SpeedRReal = 0;

long LastCtrlTick = 0;  //��¼��һ�ο��Ʋ�����ʱ��Ϳ��Ʋ�����ʱ����
long CtrlInterval = 0;

char txt[128] = {0};

// **************************** �������� ****************************

int core0_main(void)
{
    clock_init(SYSTEM_CLOCK_250M); 	// ʱ�����ü�ϵͳ��ʼ��<��ر���>
    debug_info_init();                  // ���Դ�����Ϣ��ʼ��
    
    // �˴���д�û����� ���������ʼ�������
    
    //�ر�CPU���ж�
    
    //�رտ��Ź�����������ÿ��Ź�ι����Ҫ�ر�
    
    //��ȡ����Ƶ��
    
    gpio_init(P19_0, GPO, GPIO_LOW, GPO_PUSH_PULL);              // ��ʼ�� LED1 ��� Ĭ�ϸߵ�ƽ �������ģʽ

    gpio_init(P20_0, GPI, GPIO_HIGH, GPI_PULL_UP);               // ��ʼ�� KEY1 ���� Ĭ�ϸߵ�ƽ ��������
    gpio_init(P20_1, GPI, GPIO_HIGH, GPI_PULL_UP);               // ��ʼ�� KEY2 ���� Ĭ�ϸߵ�ƽ ��������
    gpio_init(P20_2, GPI, GPIO_HIGH, GPI_PULL_UP);               // ��ʼ�� KEY3 ���� Ĭ�ϸߵ�ƽ ��������
    gpio_init(P20_3, GPI, GPIO_HIGH, GPI_PULL_UP);               // ��ʼ�� KEY4 ���� Ĭ�ϸߵ�ƽ ��������

    gpio_init(P21_5, GPI, GPIO_HIGH, GPI_PULL_UP);            // ��ʼ�� SWITCH1 ���� Ĭ�ϸߵ�ƽ ��������
    gpio_init(P21_6, GPI, GPIO_HIGH, GPI_PULL_UP);            // ��ʼ�� SWITCH2 ���� Ĭ�ϸߵ�ƽ ��������
    
    uart_sbus_init(UART_4,115200,UART4_RX_P14_0,UART4_TX_P14_1);    //����P14.0�ܽ������P14.1���룬������115200
    //soft_iic_init()    ͨ���߳�ʼ��
    while(mpu6050_init());

    motor_information_out_init();
    motor_speed_curve_init();

    adc_init(ADC_CHANNEL1, ADC_12BIT);                                          
    adc_init(ADC_CHANNEL2, ADC_12BIT);                                          
    adc_init(ADC_CHANNEL3, ADC_10BIT);                                          
    adc_init(ADC_CHANNEL4, ADC_8BIT);                                           

    adc_init(ADC_CHANNEL5, ADC_12BIT);                                          
    adc_init(ADC_CHANNEL6, ADC_10BIT);                                         
    adc_init(ADC_CHANNEL7, ADC_8BIT);       S                                    
    adc_init(ADC_CHANNEL8, ADC_8BIT); 
    
    
    //�����ⲿ�жϣ����ڴ����ⲿ������ʱ��
    //����CPU���ж�
    //֪ͨCPU1,CPU0��ʼ�����

//��ѭ��-------------------------------------------------------------
    while(true)
    {
        getDirection();     //��ȡ������Ϣ
        switchMode();    //�л�ģʽ
        readMPU6050();    //��ȡMPU6050����������
        testStopped();    //�����Ƿ�ͣ��
        CtrlInterval = TickMillis - LastCtrlTick;       //������Ƽ��ʱ��
        if (CtrlInterval >= CTRL_INTERVAL) 
        {
            switch (Mode) {
                case Idle:
                    SpeedL = 0;
                    SpeedR = 0; // ͣ��
                    changeParam();
                    break;
                case Charging:
                    testCharge();
                    break;
                case RunningStraight:
                    runStraight();
                    break;
                case Turning:
                    turn();
                    break;
                default:
                    break;
            }
            LastCtrlTick = TickMillis;
        }

        MotorCtrl(SpeedL, SpeedR);
    }

}

// **************************** �������� ****************************

//LED����˸times��
void blink(unsigned char times) 
{
    int c;
    for(c = 0; c < 2*times;c++)
    {
        gpio_set_level(P19_0, 1)
    }
}


//�л�ģʽ
void switchMode()
{
    DebugMode = ((P21_5) << 1) + gpio_get_level(P21_6);     //ͨ����ȡ�����ź����õ���ģʽ(DebugMode)
    switch (DebugMode)
    {
        case NormalDebug:
        case StraightDeubug:
            if(key_get_state(KEY_1))
            {
                if(Mode == Idle)
                {
                    sprintf(txt, "Running\n");
                    uart_write_string(UART_0,txt);
                    Mode = RunningStraight;
                    SpeedL = INIT_SPEED;
                    SpeedR = INIT_SPEED;
                    GzDeg = 0;
                    AxMPerSec = 0;
                    ADCCount = 0;
                }
                else{
                    sprintf(txt, "Idle\n");
                    uart_write_string(UART_0,txt);
                    Mode = Idle;
                }
                system_delay_ms(DEBOUNCE_TIME);
            }
            break;
        case Normal:
            if(Mode == Idle)
            {
                Mode = Charging;
            }
            break;
        case TurnDebug:
            if(key_get_state(KEY_1))
            {
                if(Mode == Idle)
                {
                    sprintf(txt, "Turning\n");
                    uart_write_string(UART_0,txt);
                    Mode = Turning;
                    SpeedL = 0;
                    SpeedR = 0;
                    Motor();
                    GzDeg = 0;
                }
                else{
                    sprintf(txt, "Idle\n");
                    uart_write_string(UART_0,txt);
                    Mode = Idle;
                }
                system_delay_ms(DEBOUNCE_TIME);
            }
            break;
        default:
            break;
    }

    if(DebugMode == Normal || DebugMode == NormalDebug)
    {
        if (Direction == Forward || Direction == FrontRight
            || Direction == FrontLeft) {
            if (Mode == Turning) 
            {
                //Motor();
                SpeedL = 0;
                SpeedR = 0;
                AxMPerSec = 0;
                SpeedLReal = 0;
                SpeedRReal = 0;
                Mode = RunningStraight;
                Direction = Forward;
                ErrorInc.lLast = 0;
                ErrorInc.lLastD = 0;
                ErrorInc.rLast = 0;
                ErrorInc.rLastD = 0;
                GzDegPerSec = 0;
                GzDeg = 0;
                AxMPerSec = 0;
                if (DebugMode == TurnDebug) {
                    Mode = Idle;
                }
                system_delay_ms(BRAKE_TIME);
                SpeedL = INIT_SPEED;
                SpeedR = INIT_SPEED;
            }
        }
        else if (Mode == RunningStraight) { 
            
            Mode = Turning;

            LastTurnTick = TickMillis;

            ErrorLoc.lLast = 0;
            ErrorLoc.rLast = 0;
            ErrorLoc.lTotal = 0;
            ErrorLoc.rTotal = 0;

            if (Dir[Direction] > 1) {
                SpeedL = 0;
                SpeedR = TURN_INIT_SPEED;
            }
            else if (Dir[Direction] < -1) {
                SpeedR = 0;
                SpeedL = TURN_INIT_SPEED;
            }

            //Motor();
            GzDeg = 0;
            AxMPerSec = 0;
        }
    }
}

//�������Ƿ�����������򷢳�
void testCharge()
{
    //float chargeV = adc_convert(ADC);
    chargeV *= ADC_TO_V_F * V_TEST_TO_REAL_F;
    if (chargeV >= CHARGE_THRESHOLD) {
        Mode = RunningStraight;
        SpeedL = INIT_SPEED;
        SpeedR = INIT_SPEED;
        Motor(SpeedL, SpeedR);
        system_delay_ms(2000);
    }
#ifdef DEBUG
    sprintf(txt, "chargeV = %f\n", chargeV);
    uart_write_string(UART_0,txt);
    system_delay_ms(200);
#endif
}

//��ֱ��
void runStraight()
{
    getPIDIncResult(&SpeedL, &SpeedR, StraightSpeed - SpeedLReal,StraightSpeed - SpeedRReal);
    if (GzDegPerSec > StraightGzTolerance || GzDegPerSec < -StraightGzTolerance) {
        SpeedL = 0;
        SpeedR = 0;
    }

#ifdef DEBUG
    sprintf(txt, "Running straight:\n");
    uart_write_string(UART_0,txt);
    sprintf(txt, "SpeedLReal = %f    SpeedRReal = %f\n", SpeedLReal,
            SpeedRReal);
    uart_write_string(UART_0,txt);
    sprintf(txt, "SpeedL = %ld    SpeedR = %ld\n", SpeedL, SpeedR);
    uart_write_string(UART_0,txt);
    sprintf(txt, "GzDegPerSec = %f\n", GzDegPerSec);
    Uuart_write_string(UART_0,txt);
#endif  
}

//ת��
void turn()
{
    unsigned char stopTurning = 0;

    if (DebugMode == TurnDebug) {
        Direction = DebugDir;
    }
    if (Dir[Direction] > 1) {
    // �����ǰ��ͷ������δ����Ŀ�귽�򣬲���ת��ʱ��δ�����趨�����ʱ��
        if (GzDeg < Dir[Direction] * 22.5 && TickMillis - LastTurnTick < TURN_TIME_MAX) {
            SpeedL = 0;  // �����ٶ���Ϊ0
            // ����ʵ�������ٶȵ��������ٶ�
            if (SpeedRReal < TURN_SPEED) {
                SpeedR += TURN_STEP;
            } else if (SpeedRReal > TURN_SPEED) {
                SpeedR -= TURN_STEP;
            }
        } else {
            stopTurning = 1;  // ֹͣת��
        }
    }
    // ���Ŀ�귽��С��-1����ʾ��Ҫ��ת
    else if (Dir[Direction] < -1) {
        // �����ǰ��ͷ������δ����Ŀ�귽�򣬲���ת��ʱ��δ�����趨�����ʱ��
        if (GzDeg > Dir[Direction] * 22.5 && TickMillis - LastTurnTick < TURN_TIME_MAX) {
            SpeedR = 0;  // �����ٶ���Ϊ0
            // ����ʵ�������ٶȵ��������ٶ�
            if (SpeedLReal < TURN_SPEED) {
                SpeedL += TURN_STEP;
            } else if (SpeedLReal > TURN_SPEED) {
                SpeedL -= TURN_STEP;
            }
        } else {
            stopTurning = 1;  // ֹͣת��
        }
    }
    // ���Ŀ�귽����[-1, 1]֮�䣬ֹͣת��
    else {
        stopTurning = 1;
    }

    // �����Ҫֹͣת��
    if (stopTurning) {
        // ֹͣ������ƣ������ٶȺ���ز���
        //MotorCtrl(0, 0);
        SpeedL = 0;
        SpeedR = 0;
        AxMPerSec = 0;
        SpeedLReal = 0;
        SpeedRReal = 0;
        Mode = RunningStraight;
        Direction = Forward;
        //ADCCount = 0;
        ErrorInc.lLast = 0;
        ErrorInc.lLastD = 0;
        ErrorInc.rLast = 0;
        ErrorInc.rLastD = 0;
        GzDegPerSec = 0;
        GzDeg = 0;
        AxMPerSec = 0;
        if (DebugMode == TurnDebug) {
            Mode = Idle;
        }
        delayms(BRAKE_TIME);  // ͣ�������ʱ
        SpeedL = INIT_SPEED;  // ���������ٶ�
        SpeedR = INIT_SPEED;
    }
}

//���ű�λ��
void getDirection() 
{
    if (ADCCount >= XCORR_SIZE) {
        Direction = locateSound((sint16*)s1, (sint16*)s2, (sint16*)s3,(sint16*)s4);
        //ADCCount = 0;

#ifdef DEBUG
        sprintf(txt, "direction = %d\n", Direction);
        UART_PutStr(UART0, txt);
#endif

    }

#ifdef DEBUG
    sprintf(txt, "ADCCount = %d\n", ADCCount);
    UART_PutStr(UART0, txt);
#endif
}

//��ȡMPU6050���ٶȼƺ�����������
void readMPU6050()
{
    if(LastMPUTick != TickMillis)
    {
        int dt;
        mpu6050_get_acc();
        mpu6050_get_gyro();
        dt = TickMillis - LastMPUTick;
        LastMPUTick = TickMillis;
        AxMPerSqrSec = -mpu6050_acc_x * 9.8 / 16384;
        AxMPerSec += AxMPerSqrSec / 1000 * dt;
        GzDegPerSec = mpu6050_gyro_z / 65.5;
        GzDeg += GzDegPerSec / 1000 * dt;
    }
}

//����ٶ��Ƿ�Ϊ0
void testStopped()
{
    if (TickMillis - LastSpeedTickL > SPEED_TIMEOUT) {
        SpeedLReal = 0;
    }
    if (TickMillis - LastSpeedTickR > SPEED_TIMEOUT) {
        SpeedRReal = 0;
    }
}