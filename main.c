//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "src/scicomm.h"

//
// DEFINES
//
#define TAM_BUFFER_DAC     200
#define TAM_BUFFER_ADC     200

//
// Buffers
//
volatile uint16_t dac_buffer[TAM_BUFFER_DAC];
volatile uint16_t adc_buffer[TAM_BUFFER_ADC];

//
// Protocolo SCI
//
volatile Protocol_Header_t g_prot_header = {CMD_NONE,0};

//
// Flags
//
volatile uint16_t adc_done = 0;

//
// MAIN
//
void main(void)
{
    uint16_t i;

    //
    // Inicialização
    //
    Device_init();

    Interrupt_initModule();

    Interrupt_initVectorTable();

    Board_init();

    //
    // Habilita SCI RX
    //
    //Interrupt_enable(INT_mySCI0_RX);


    Interrupt_enable(INT_mySCI0_RX);

    SCI_enableInterrupt(
    mySCI0_BASE,
    SCI_INT_RXFF
    );

Interrupt_enableMaster();

    //
    // Habilita interrupções globais
    //
    EINT;
    ERTM;

    //
    // Inicializa buffers
    //
    for(i = 0; i < TAM_BUFFER_DAC; i++)
    {
        dac_buffer[i] = 2048;
    }

    for(i = 0; i < TAM_BUFFER_ADC; i++)
    {
        adc_buffer[i] = 0;
    }

    //
    // Loop principal
    //
    while(1)
    {
        //
        // Recebe vetor enviado pelo Python
        //
        if(g_prot_header.cmd == CMD_RECEIVE_INT)
        {
            for(i = 0; i < TAM_BUFFER_DAC; i++)
            {
                dac_buffer[i] =
                    (uint16_t)protocolReceiveInt(mySCI0_BASE);
            }

            g_prot_header.cmd = CMD_NONE;

            SCI_clearInterruptStatus(
                mySCI0_BASE,
                SCI_INT_RXFF
            );
        }

        //
        // Envia vetor ADC para o Python
        //
        if(adc_done)
        {
            for(i = 0; i < TAM_BUFFER_ADC; i++)
            {
                protocolSendInt(
                    mySCI0_BASE,
                    adc_buffer[i]
                );
            }

            adc_done = 0;
        }
    }
}

//
// ISR ADC
//
__interrupt void INT_ADC0_1_ISR(void)
{
    static uint16_t cnt_adc = 0;

    adc_buffer[cnt_adc] =
        ADC_readResult(
            ADC0_RESULT_BASE,
            ADC0_SOC0
        );

    cnt_adc++;

    if(cnt_adc >= TAM_BUFFER_ADC)
    {
        cnt_adc = 0;
        adc_done = 1;
    }

    ADC_clearInterruptStatus(
        ADC0_BASE,
        ADC_INT_NUMBER1
    );

    Interrupt_clearACKGroup(
        INT_ADC0_1_INTERRUPT_ACK_GROUP
    );
}

//
// ISR Timer -> DAC
//
__interrupt void INT_myCPUTIMER1_ISR(void)
{
    static uint16_t cnt_dac = 0;

    DAC_setShadowValue(
        DAC0_BASE,
        dac_buffer[cnt_dac]
    );

    cnt_dac++;

    if(cnt_dac >= TAM_BUFFER_DAC)
    {
        cnt_dac = 0;
    }
}

//
// ISR SCI RX
//
__interrupt void INT_mySCI0_RX_ISR(void)
{
    uint16_t header[PROTOCOL_HEADER_SIZE];
    uint16_t cmd;

    SCI_readCharArray(
        mySCI0_BASE,
        header,
        PROTOCOL_HEADER_SIZE
    );

    cmd = header[0];

    g_prot_header.data_len =
        header[1] |
        (header[2] << 8);

    if(cmd < CMD_COUNT)
    {
        g_prot_header.cmd =
            (SCI_Command_e)cmd;
    }
    else
    {
        g_prot_header.cmd =
            CMD_NONE;
    }

    Interrupt_clearACKGroup(
        INT_mySCI0_RX_INTERRUPT_ACK_GROUP
    );
}