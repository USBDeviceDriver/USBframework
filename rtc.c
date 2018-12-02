#include "rtc.h"

volatile int rtc_flag = 0;

void rtc_init(rtc_time_t *t)
{
	// clock disable & reset
	LPC_RTC->CCR = BV(CCR_RST);
	// init time & day regrs
	LPC_RTC->SEC = t->sec;
	LPC_RTC->MIN = t->min;
	LPC_RTC->HOUR = t->hour;
	LPC_RTC->DOM = t->dom;
	LPC_RTC->DOW = t->dow;
	LPC_RTC->DOY = t->doy;
	LPC_RTC->MONTH = t->mon;
	LPC_RTC->YEAR = t->year;
	// enable secs incr intr.
	LPC_RTC->CIIR |= BV(CIIR_IMSEC);
	// enable rtc intr in NVIC.
	NVIC_EnableIRQ(RTC_IRQn);
	// clock enable
	LPC_RTC->CCR = BV(CCR_EN);
}

void rtc_get(rtc_time_t *t)
{
	// get time & day regrs
	t->sec = LPC_RTC->SEC;
	t->min = LPC_RTC->MIN;
	t->hour = LPC_RTC->HOUR;
	t->dom = LPC_RTC->DOM;
	t->dow = LPC_RTC->DOW;
	t->doy = LPC_RTC->DOY;
	t->mon = LPC_RTC->MONTH;
	t->year = LPC_RTC->YEAR;
}

void RTC_IRQHandler(void)
{
	rtc_flag = 1;
	// clear timer intr
	if(LPC_RTC->ILR & BV(ILR_CIF))
		LPC_RTC->ILR |= BV(ILR_CIF);
}

