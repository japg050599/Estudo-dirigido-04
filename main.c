//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "math.h"

//
// Main
//
#define TAM_BUFFER_DAC 200
#define TAM_BUFFER_ADC 100
extern uint16_t dac_buffer[];
volatile uint16_t adc_buffer[TAM_BUFFER_ADC];
volatile float gain = 1.0f;

void main(void)
{
    // Device Initialization
    Device_init();


    //
    // Initializes PIE and clears PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();
    //
    // Initializes the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    Board_init();

    //
    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //
    EINT;
    ERTM;

    while(1)
    {
    }

}

__interrupt void INT_ADC0_1_ISR(void)
{
    static uint16_t cnt_adc =0;
    cnt_adc = (cnt_adc+1)%TAM_BUFFER_ADC;
    adc_buffer[cnt_adc] = ADC_readResult(ADC0_RESULT_BASE, ADC0_SOC0);
    ADC_clearInterruptStatus(ADC0_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INT_ADC0_1_INTERRUPT_ACK_GROUP);

}

__interrupt void INT_myCPUTIMER1_ISR(void)
{
    static uint16_t cnt_dac = 0;
    DAC_setShadowValue(DAC0_BASE, (uint16_t) (gain*dac_buffer[cnt_dac]));
    cnt_dac = (cnt_dac+1)%TAM_BUFFER_DAC;
}
