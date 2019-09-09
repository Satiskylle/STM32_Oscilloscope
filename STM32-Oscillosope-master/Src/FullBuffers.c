#include "FullBuffers.h"

static void MonoBuffer_Init(MonoBuffer *mono_buff)
{
//	arm_fill_q15(0, mono_buff->buffer_M, Sound_Buffer_Size);
	memset(mono_buff->buffer_M,0,Sound_Buffer_Size*sizeof(float));
}

static void StereoBuffer_Init(StereoBuffer *stereo_buff)
{
	memset(stereo_buff->Mid,0,Sound_Buffer_Size*sizeof(float));
	memset(stereo_buff->Mid,0,Sound_Buffer_Size*sizeof(float));
//	arm_fill_q15(0, stereo_buff->Mid, Sound_Buffer_Size);
//	arm_fill_q15(0, stereo_buff->Side, Sound_Buffer_Size);
}

static void DoubleMonoBuffer_Init(DoubleMonoBuffer *doubleMono_buff)
{
	MonoBuffer_Init(&doubleMono_buff->Buffer_0);
	MonoBuffer_Init(&doubleMono_buff->Buffer_1);
}

static void DoubleStereoBuffer_Init(DoubleStereoBuffer *double_buff)
{
	StereoBuffer_Init(&double_buff->Buffer_0);
	StereoBuffer_Init(&double_buff->Buffer_1);
}

void FullDoubleBuffer_Init(FullDoubleBuffer *fullDouble_buff)
{
//	DoubleStereoBuffer_Init(&fullDouble_buff->Double_Buffer_Audio);
	DoubleMonoBuffer_Init(&fullDouble_buff->Double_Buffer_Mic);
//	DoubleStereoBuffer_Init(&fullDouble_buff->Double_Buffer_Out);

	fullDouble_buff->head_0 = 0;
	fullDouble_buff->head_1 = 0;
	fullDouble_buff->select = 0;
}

static void MonoBuffer_Fill(MonoBuffer *mono_buff, uint16_t head, uint16_t *data)
{
	mono_buff->buffer_M[head] = ((int16_t)data[0]);
}

static void StereoBuffer_Fill(StereoBuffer *stereo_buff, uint16_t head, uint16_t *data)
{
	stereo_buff->Mid[head] = (int16_t)(data[0] + data[1])/2;
	stereo_buff->Side[head] = (int16_t)(data[0] - data[1])/2;
}

static void StereoBuffer_Pop(StereoBuffer *stereo_buff, uint16_t head, uint16_t *data)
{
	data[0] = 0.9 * (int16_t)(stereo_buff->Mid[head]);
	data[1] = 0.9 * (int16_t)(stereo_buff->Mid[head]);
}

uint8_t FullDoubleBuffer_FillPop(FullDoubleBuffer *full_double_buff, uint16_t *data_mic, uint16_t *data_out)
{
	if(full_double_buff->select == 0)
	{
//		StereoBuffer_Fill(&full_double_buff->Double_Buffer_Audio.Buffer_0, full_double_buff->head_0, data_audio);
		MonoBuffer_Fill(&full_double_buff->Double_Buffer_Mic.Buffer_0, full_double_buff->head_0, data_mic);
//		StereoBuffer_Pop(&full_double_buff->Double_Buffer_Out.Buffer_0, full_double_buff->head_0, data_out);

//		data_out[0] = (int16_t)(full_double_buff->Double_Buffer_Mic.Buffer_0.buffer_M[full_double_buff->head_0]);
//		data_out[1] = (int16_t)(full_double_buff->Double_Buffer_Mic.Buffer_0.buffer_M[full_double_buff->head_0]);


		full_double_buff->head_0++;

		if(full_double_buff->head_0 >= Sound_Buffer_Size)
		{
			 full_double_buff->head_0 = 0;
			 full_double_buff->select = 1;
			 return 1;
		}
	}
	else
	{
//		StereoBuffer_Fill(&full_double_buff->Double_Buffer_Audio.Buffer_1, full_double_buff->head_1, data_audio);
		MonoBuffer_Fill(&full_double_buff->Double_Buffer_Mic.Buffer_1, full_double_buff->head_1, data_mic);
//		StereoBuffer_Pop(&full_double_buff->Double_Buffer_Out.Buffer_1, full_double_buff->head_1, data_out);

//		data_out[0] = (int16_t)(full_double_buff->Double_Buffer_Mic.Buffer_1.buffer_M[full_double_buff->head_1]);
//		data_out[1] = (int16_t)(full_double_buff->Double_Buffer_Mic.Buffer_1.buffer_M[full_double_buff->head_1]);

		full_double_buff->head_1++;

		if(full_double_buff->head_1 >= Sound_Buffer_Size)
		{
			 full_double_buff->head_1 = 0;
			 full_double_buff->select = 0;
			 return 1;
		}
	}
	return 0;
}

MonoBuffer* FullDoubleBuffer_TakeMicBuff(FullDoubleBuffer *full_double_buff)
{
	if(full_double_buff->select == 0) return &full_double_buff->Double_Buffer_Mic.Buffer_1;
	else return &full_double_buff->Double_Buffer_Mic.Buffer_0;
}

void FullDoubleBuffer_GiveOutBuff(int16_t *Mid, int16_t *Side, FullDoubleBuffer *full_double_buff)
{
	if(full_double_buff->select == 0)
	{
//		arm_copy_q15(Mid, full_double_buff->Double_Buffer_Out.Buffer_1.Mid, Sound_Buffer_Size);
//		arm_copy_q15(Side, full_double_buff->Double_Buffer_Out.Buffer_1.Side, Sound_Buffer_Size);
	}
	else
	{
//		arm_copy_q15(Mid, full_double_buff->Double_Buffer_Out.Buffer_0.Mid, Sound_Buffer_Size);
//		arm_copy_q15(Side, full_double_buff->Double_Buffer_Out.Buffer_0.Side, Sound_Buffer_Size);
	}
}
