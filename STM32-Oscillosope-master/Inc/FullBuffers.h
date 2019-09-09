#ifndef _FullBuffers_H
#define _FullBuffers_H

#include "stm32f4xx_hal.h"
#include "string.h"

#define UDA1380_Buffor_Size 1024*6 / 2//289 //nie moze byc zbyt duze, bo VS1003B nie przyjmie tak duzej paczki//w sumie to nie jest prawda
#define Sound_Buffer_Size 1024*6 / 2


typedef struct{
	int32_t buffer_M[Sound_Buffer_Size];
}MonoBuffer;

typedef struct{
	int32_t Mid[Sound_Buffer_Size];
	int32_t Side[Sound_Buffer_Size];
}StereoBuffer;

typedef struct{
	MonoBuffer Buffer_0, Buffer_1;
}DoubleMonoBuffer;

typedef struct{
	StereoBuffer Buffer_0, Buffer_1;
}DoubleStereoBuffer;

typedef struct{
	DoubleMonoBuffer Double_Buffer_Mic;
	volatile uint16_t head_0, head_1;
	volatile uint16_t select;
}FullDoubleBuffer;


void FullDoubleBuffer_Init(FullDoubleBuffer *fullDoubleStereo_buff);
uint8_t FullDoubleBuffer_FillPop(FullDoubleBuffer *full_double_buff, uint16_t *data_mic, uint16_t *data_out);
StereoBuffer *FullDoubleBuffer_TakeAudioBuff(FullDoubleBuffer *full_double_buff);
MonoBuffer *FullDoubleBuffer_TakeMicBuff(FullDoubleBuffer *full_double_buff);
void FullDoubleBuffer_GiveOutBuff(int16_t *Mid, int16_t *Side, FullDoubleBuffer *full_double_buff);


#endif
