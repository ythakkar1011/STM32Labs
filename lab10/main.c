
//============================================================================
// ECE 362 lab experiment 10 -- Asynchronous Serial Communication
//============================================================================

#include "stm32f0xx.h"
#include "ff.h"
#include "diskio.h"
#include "fifo.h"
#include "tty.h"
#include <string.h> // for memset()
#include <stdio.h> // for printf()

void advance_fattime(void);
void command_shell(void);

// Write your subroutines below.
void setup_usart5(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;
    GPIOC->MODER &= ~(0x3000000);
    GPIOC->MODER |= 0x2000000;
    GPIOC->AFR[1] |= 0x20000;
    GPIOD->MODER &= ~(0x30);
    GPIOD->MODER |= 0x20;
    GPIOD->AFR[0] |= 0x200;
    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
    USART5->CR1 &= ~(USART_CR1_UE);
    USART5->CR1 &= ~USART_CR1_M;
    USART5->CR2 &= ~USART_CR2_STOP;
    USART5->CR1 &= ~USART_CR1_PCE;
    USART5->CR1 &= ~USART_CR1_OVER8;
    USART5->BRR = 0x1a1;
    USART5->CR1 |= (USART_CR1_TE | USART_CR1_RE);
    USART5->CR1 |= USART_CR1_UE;
    while ((USART5->ISR & USART_ISR_TEACK) == 0);
    while ((USART5->ISR & USART_ISR_REACK) == 0);
}

int simple_putchar(int x)
{
    while ((USART5->ISR & USART_ISR_TXE) == 0);
    USART5->TDR = x;
    return x;
}

int simple_getchar()
{
    while ((USART5->ISR & USART_ISR_RXNE) == 0);
    int x = USART5->RDR;
    return (x);
}

int better_putchar(int x)
{
    if (x == 10)
    {
        while ((USART5->ISR & USART_ISR_TXE) == 0);
        USART5->TDR = 13;
        //USART5->TDR = 10;
    }

    while ((USART5->ISR & USART_ISR_TXE) == 0);
    USART5->TDR = x;
    return x;
}

int better_getchar()
{
    while ((USART5->ISR & USART_ISR_RXNE) == 0);
    int x = USART5->RDR;
    if (x == 13)
        return (10);
    else
        return (x);
}

int interrupt_getchar(void) {
    //USART_TypeDef *u = USART5;
    // If we missed reading some characters, clear the overrun flag.
    //if (u->ISR & USART_ISR_ORE)
    //    u->ICR |= USART_ICR_ORECF;
    // Wait for a newline to complete the buffer.
    //while(fifo_newline(&input_fifo) == 0) {
    //    while (!(u->ISR & USART_ISR_RXNE))
    //        ;
    //    insert_echo_char(u->RDR);
    //}

    for(;;)
    {
        if (fifo_newline(&input_fifo) == 1)
        {
            char cha = fifo_remove(&input_fifo);
            return cha;
        }
        else
        {
            asm volatile ("wfi");
        }
    }
    // Return a character from the line buffer.

}

void USART3_4_5_6_7_8_IRQHandler()
{
    USART_TypeDef *u = USART5;
    if(u->ISR & USART_ISR_ORE)
    {
        u->ICR |= USART_ICR_ORECF;
    }

    int x = USART5->RDR;
    if(fifo_full(&input_fifo) == 1)
    {
        return;
    }
    insert_echo_char(x);
}

void enable_tty_interrupt()
{
    USART5->CR1 |= USART_CR1_RXNEIE;
    NVIC->ISER[0] = (1<<29);
}

void setup_spi1()
{
    /*RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(0xfc0c);
    GPIOA->MODER |= 0xa804;
    GPIOA->AFR[0] |= 0x00000000;
    GPIOA->PUPDR &= ~(0x3000);
    GPIOA->PUPDR |= 0x1000;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 |= (SPI_CR1_BR);
    SPI1->CR1 &= ~(SPI_CR1_SPE);
    SPI1->CR1 &= ~(SPI_CR1_BIDIMODE);
    SPI1->CR1 &= ~(SPI_CR1_BIDIOE);
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR2 = SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0 | SPI_CR2_NSSP | SPI_CR2_FRXTH;
    //SPI1->CR2 |= SPI_CR2_FRXTH;
    SPI1->CR1 |= SPI_CR1_SPE;*/

    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODER12);
    GPIOA->MODER |= GPIO_MODER_MODER12_0;
    GPIOA->PUPDR &= ~(0x3000);
    GPIOA->PUPDR |= 0x1000;
    GPIOB->MODER &= ~ (GPIO_MODER_MODER13 | GPIO_MODER_MODER14 | GPIO_MODER_MODER15);
    GPIOB->MODER |= (GPIO_MODER_MODER15_1 | GPIO_MODER_MODER14_1 | GPIO_MODER_MODER13_1);
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFR15 |GPIO_AFRH_AFR14 | GPIO_AFRH_AFR13);
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2->CR1 |= (SPI_CR1_BR);
    SPI2->CR1 &= ~(SPI_CR1_SPE);
    SPI2->CR1 &= ~(SPI_CR1_BIDIMODE);
    SPI2->CR1 &= ~(SPI_CR1_BIDIOE);
    SPI2->CR1 |= SPI_CR1_MSTR;
    SPI2->CR2 = SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0 | SPI_CR2_NSSP | SPI_CR2_FRXTH;
    SPI2->CR1 |= SPI_CR1_SPE;
}

void spi_high_speed()
{
    /*SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 &= ~(SPI_CR1_BR);
    SPI1->CR1 |= (SPI_CR1_BR_1);
    SPI1->CR1 |= SPI_CR1_SPE;*/

    SPI2->CR1 &= ~SPI_CR1_SPE;
    SPI2->CR1 &= ~(SPI_CR1_BR);
    SPI2->CR1 |= (SPI_CR1_BR_1);
    SPI2->CR1 |= SPI_CR1_SPE;

}

void TIM14_IRQHandler()
{
    TIM14->SR &= ~TIM_SR_UIF;
    advance_fattime();
}

void setup_tim14()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    TIM14->PSC = 24000 - 1;
    TIM14->ARR = 4000 - 1;
    TIM14->DIER |= TIM_DIER_UIE;
    TIM14->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = (1<<19);
}
int __io_putchar(int ch)
{
    return better_putchar(ch);
}

int __io_getchar(void)
{
    return line_buffer_getchar();
}


// Write your subroutines above.


const char testline[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\r\n";

int main()
{
    setup_usart5();

    // Uncomment these when you're asked to...
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);

    // Test 2.2 simple_putchar()
    //
    //for(;;)
        //for(const char *t=testline; *t; t++)
            //simple_putchar(*t);

    // Test for 2.3 simple_getchar()
    //
    ///for(;;)
        //simple_putchar( simple_getchar() );

    // Test for 2.4 and 2.5 __io_putchar() and __io_getchar()
    //
    //printf("Hello!\n");
    //for(;;)
    //    putchar( getchar() );

    // Test for 2.6
    //
    //for(;;) {
    //    printf("Enter string: ");
    //    char line[100];
    //    fgets(line, 99, stdin);
    //    line[99] = '\0'; // just in case
    //    printf("You entered: %s", line);
    //}

    // Test for 2.7
    //
    //enable_tty_interrupt();
    //for(;;) {
    //    printf("Enter string: ");
    //    char line[100];
    //    fgets(line, 99, stdin);
    //    line[99] = '\0'; // just in case
    //    printf("You entered: %s", line);
    //}

    // Test for 2.8 Test the command shell and clock.
    //
    enable_tty_interrupt();
    //setup_tim14();

    //Work area
    FATFS fs_storage;
    FATFS *fs = &fs_storage;
    FIL rssiFile;
    BYTE buffer[4096];

    //Give work area to storage drive
    f_mount(fs, "", 1);
    //f_open(&rssiFile, "RSSI.txt", FA_WRITE | FA_OPEN_APPEND);
    //f_printf(&rssiFile, "RSSI Value = -52!!!\n");
    //f_close(&rssiFile);


    command_shell();


    return 0;
}
