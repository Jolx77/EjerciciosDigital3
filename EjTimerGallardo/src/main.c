/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */


#include "LPC17xx.h"

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"


volatile uint8_t periodo = 1;
uint8_t estaenT1 = 1;
uint8_t savePeriodo;
uint32_t secuencia = 0xA0A3;
uint8_t estoyEnAlto = 0; //Sirve para avisar que hay que esperar otros 5 ms x estar en alto
uint8_t counter = 1; //Empieza en 1 porque la primera recorrida la hacemos en el handler de la interrupcion
uint8_t polaridad = 0;



void configTimer0(){
	//Seteamos el pin 1.28 en match
	PINSEL_CFG_Type pinCfg;
	pinCfg.Pinnum = PINSEL_PIN_28;
	pinCfg.Pinmode = PINSEL_PINMODE_NORMAL;
	pinCfg.Portnum = PINSEL_PORT_1;
	pinCfg.Funcnum = PINSEL_FUNC_1;
	PINSEL_ConfigPin(&pinCfg);

	TIM_TIMERCFG_Type timCfg;
	timCfg.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timCfg.PrescaleValue = 5000;

	TIM_MATCHCFG_Type matCfg;
	matCfg.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE; //Como se en que valor empieza?
	matCfg.IntOnMatch = ENABLE;
	matCfg.MatchChannel = 0;
	matCfg.MatchValue = ((((50*10^-3)/periodo)/(5001))*(100*10^6))-1;
	matCfg.ResetOnMatch = ENABLE;
	matCfg.StopOnMatch = DISABLE;
	savePeriodo = periodo;
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,(void*) &timCfg);
	TIM_ConfigMatch(LPC_TIM0,&matCfg);
	TIM_Cmd(LPC_TIM0,ENABLE);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void configTimer1(){

	TIM_TIMERCFG_Type tim1Cfg;
	tim1Cfg.PrescaleOption = TIM_PRESCALE_TICKVAL;
	tim1Cfg.PrescaleValue = 5000;

	TIM_MATCHCFG_Type mat1Cfg;
	mat1Cfg.ExtMatchOutputType = 0;
	mat1Cfg.IntOnMatch = ENABLE;
	mat1Cfg.MatchChannel = 1;
	mat1Cfg.MatchValue = ((((5*10^-3))/(5001))*(100*10^6))-1;
	mat1Cfg.ResetOnMatch = ENABLE;
	mat1Cfg.StopOnMatch = DISABLE;
	TIM_ConfigStructInit(TIM_TIMER_MODE,(void*) &tim1Cfg);
	TIM_ConfigMatch(LPC_TIM1,&mat1Cfg);
	TIM_Cmd(LPC_TIM1,DISABLE);
}




void TIMER0_IRQHandler(){
	if(estaenT1 == 1){
		estaenT1 = 0;
		LPC_TIM0->MR0 = (50*10^-3)-((50*10^-3)/savePeriodo);
		TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
		return;
	}
	else{
		savePeriodo = periodo;
		estaenT1 = 1;
		LPC_TIM0->MR0 = ((50*10^-3)/savePeriodo);
		TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
		return;
	}
}

void TIMER1_IRQHandler(){
	if(estoyEnAlto == 1){
		estoyEnAlto = 0;
		counter++;
		if(counter == 16){counter = 0;}
		TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
		return;
	}

	if((secuencia & (1<<counter)) == 1){
		estoyEnAlto = 1;
		LPC_GPIO1->FIOPIN |= (1<<28);
		TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
		return;
	}
	else{
		LPC_GPIO1->FIOPIN &= ~(1<<28);
		counter++;
		if(counter == 16){counter = 0;}
		TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
		return;
	}
}



void configEINT(void){
	PINSEL_CFG_Type pinCfg;
	pinCfg.Portnum = PINSEL_PORT_2;
	pinCfg.Pinnum = PINSEL_PIN_10;
	pinCfg.Funcnum = PINSEL_FUNC_1;
	pinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	PINSEL_ConfigPin(&pinCfg);


	EXTI_InitTypeDef extiCfg;
	extiCfg.EXTI_Line = EXTI_EINT0;
	extiCfg.EXTI_Mode = EXTI_MODE_LEVEL_SENSITIVE;
	extiCfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
	EXTI_Config(&extiCfg);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
	NVIC_EnableIRQ(EINT0_IRQn);
}


void configGPIOINT(){
	PINSEL_CFG_Type pin00Cfg;
	pin00Cfg.Portnum = PINSEL_PORT_0;
	pin00Cfg.Pinnum = PINSEL_PIN_0;
	pin00Cfg.Funcnum = PINSEL_FUNC_0;
	pin00Cfg.Pinmode = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pin00Cfg);
	PINSEL_CFG_Type pin01Cfg;
	pin01Cfg.Portnum = PINSEL_PORT_0;
	pin01Cfg.Pinnum = PINSEL_PIN_1;
	pin01Cfg.Funcnum = PINSEL_FUNC_0;
	pin01Cfg.Pinmode = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pin01Cfg);
	PINSEL_CFG_Type pin02Cfg;
	pin02Cfg.Portnum = PINSEL_PORT_0;
	pin02Cfg.Pinnum = PINSEL_PIN_2;
	pin02Cfg.Funcnum = PINSEL_FUNC_0;
	pin02Cfg.Pinmode = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pin02Cfg);
	PINSEL_CFG_Type pin03Cfg;
	pin03Cfg.Portnum = PINSEL_PORT_0;
	pin03Cfg.Pinnum = PINSEL_PIN_3;
	pin03Cfg.Funcnum = PINSEL_FUNC_0;
	pin03Cfg.Pinmode = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pin03Cfg);

	LPC_GPIOINT->IO0IntEnF |= 0x0F; //Me interesa que interrumpa tanto por subida como por bajada
	LPC_GPIOINT->IO0IntEnR |= 0x0F;
	NVIC_EnableIRQ(EINT3_IRQn);

}

void EINT3_IRQHandler(){
	periodo = (LPC_GPIO0->FIOPIN & (0b1111)); //Lsb:pin derecha (Conectado a P0.0), Msb: Pin izquierda (conectado a P0.3)
	if(periodo == 0){periodo = 1;}
	LPC_GPIOINT->IO0IntClr |= 0x0F;
}

void EINT0_IRQHandler(){
	if(polaridad == 0){
	polaridad = 1;
	estoyEnAlto = 1;
	counter = 1;
	PINSEL_CFG_Type pin1Cfg;
	pin1Cfg.Pinnum = PINSEL_PIN_28;
	pin1Cfg.Pinmode = PINSEL_PINMODE_NORMAL;
	pin1Cfg.Portnum = PINSEL_PORT_1;
	pin1Cfg.Funcnum = PINSEL_FUNC_0;
	PINSEL_ConfigPin(&pin1Cfg);
	if((secuencia & 1) == 1){
		estoyEnAlto = 1;
		LPC_GPIO1->FIOPIN |= (1<<28);
	}
	else{
		LPC_GPIO1->FIOPIN |= ~(1<<28);
	}
	TIM_Cmd(LPC_TIM0,DISABLE);
	TIM_Cmd(LPC_TIM1,ENABLE);
	NVIC_EnableIRQ(TIMER1_IRQn);
	EXTI_ClearEXTIFlag(EXTI_EINT0);
	EXTI_SetPolarity(EINT3_IRQn,EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE); //Cambiamos la polaridad para leer cuando deje de estar pulsado
	}
	else{
		polaridad = 0;
		EXTI_SetPolarity(EINT3_IRQn,EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE);
		TIM_Cmd(LPC_TIM1,DISABLE);
		NVIC_DisableIRQ(TIMER1_IRQn);
		TIM_Cmd(LPC_TIM0,ENABLE);
		EXTI_ClearEXTIFlag(EXTI_EINT0);
	}

}

int main(void) {

	configTimer0();
	configEINT();
	configTimer1();
	configGPIOINT();
	while(1){};
    return 0 ;
}
