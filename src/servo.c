#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_gpio.h"

#include "servo.h"
#include "Timer.h"

#define _SERVO_MOTOR_MAX_ANGLE 60.0
#define _SERVO_ARR 5624
#define _SERVO_PSC 255

static TIM_TypeDef *ServoTimer;

static void servo_init(void);

void servo_start(TIM_TypeDef *timer)
{
	ServoTimer = timer;
	
	servo_init();
	servo_setAngle(0);
	Timer_Start(ServoTimer);
}

/* Servo Motor can only work with TIM1 or TIM4 */
static void servo_init(void)
{
	/* Servo Motor on PIN8 */
	LL_GPIO_InitTypeDef LLGPIO_struct;
	
	/* Timer configuration in PWM mode */
	Timer_PWM_output_conf(ServoTimer, _SERVO_ARR, _SERVO_PSC);
	
	/* TIM1 and TIM4 will both use the eighth pin of respectively GPIOA and GPIOB */
	LLGPIO_struct.Pin 					= LL_GPIO_PIN_8;
	LLGPIO_struct.Mode       		= LL_GPIO_MODE_ALTERNATE;
	LLGPIO_struct.Speed     		= LL_GPIO_SPEED_FREQ_HIGH;
	LLGPIO_struct.OutputType		= LL_GPIO_OUTPUT_PUSHPULL;
	LLGPIO_struct.Pull					= LL_GPIO_PULL_DOWN;
	
	if (ServoTimer == TIM4) {
		/* Enable clock */
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
		
		LL_GPIO_Init(GPIOB, &LLGPIO_struct);
	} else if (ServoTimer == TIM1) {
		/* Enable clock */
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
		
		LL_GPIO_Init(GPIOA, &LLGPIO_struct);
	}
}

/* weatherVineAngle: angle from [0 ; 360] */
void servo_setAngle(int weatherVineAngle)
{
	/* Servo moteur contained in [0�;60�] */
	/* CRR = (degree * ARR) / 60  */
	
	double tau;
	int crr;
	
	
	if (weatherVineAngle < 45 || weatherVineAngle > 315) {
		tau = _SERVO_MOTOR_MAX_ANGLE;
	}
	
	else if (weatherVineAngle > 45 && weatherVineAngle <= 180) {
		tau = _SERVO_MOTOR_MAX_ANGLE - (_SERVO_MOTOR_MAX_ANGLE / 90.0) * ((2.0 / 3.0) * (double) weatherVineAngle - 30);
	}
	else {
		tau = _SERVO_MOTOR_MAX_ANGLE - (_SERVO_MOTOR_MAX_ANGLE / 90.0) * ((2.0 / 3.0) * (360.0 - (double) weatherVineAngle) - 30);
	}
	
	/*
	if (weatherVineAngle < 45 || weatherVineAngle > 315) {
		tau = 0;
	}
	
	else if (weatherVineAngle > 45 && weatherVineAngle <= 180) {
		tau = (_SERVO_MOTOR_MAX_ANGLE / 90.0) * ((2.0 / 3.0) * (double) weatherVineAngle - 30);
	}
	weatherVineAngle > 180 && weatherVineAngle < 315
	else {
		tau = (_SERVO_MOTOR_MAX_ANGLE / 90.0) * ((2.0 / 3.0) * (360.0 - (double) weatherVineAngle) - 30);
	}
	*/
	
	/*crr = (tau * (double) _SERVO_ARR) / _SERVO_MOTOR_MAX_ANGLE;*/
	
	crr = (((double)_SERVO_ARR) / 20.0) * ((tau / (double)_SERVO_MOTOR_MAX_ANGLE) + 1);
	
	Timer_PWM_set_high_level_counter(ServoTimer, crr);
}



