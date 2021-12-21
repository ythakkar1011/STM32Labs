
//===========================================================================
// ECE 362 lab experiment 8 -- SPI and DMA
//===========================================================================

#include "stm32f0xx.h"
#include "lcd.h"
#include <stdio.h> // for sprintf()

// Be sure to change this to your login...
const char login[] = "wolf103";

// Prototypes for miscellaneous things in lcd.c
void nano_wait(unsigned int);

// Write your subroutines below

void setup_bb()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &=  ~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER15);
    GPIOB->MODER |= (GPIO_MODER_MODER12_0 | GPIO_MODER_MODER13_0 | GPIO_MODER_MODER15_0);
    GPIOB->ODR = GPIO_ODR_12;
}

void small_delay()
{
    nano_wait(1000000);
}

void bb_write_bit(int boolean)
{

    GPIOB->ODR &= ~(1<<15);
    GPIOB->ODR |= (boolean<<15);
    small_delay();
    GPIOB->ODR |= GPIO_ODR_13;
    small_delay();
    GPIOB->ODR &= ~GPIO_ODR_13;
}

void bb_write_byte(int incoming)
{
    bb_write_bit( (incoming & 0x80)>>7 );
    bb_write_bit( (incoming & 0x40)>>6 );
    bb_write_bit( (incoming & 0x20)>>5 );
    bb_write_bit( (incoming & 0x10)>>4 );
    bb_write_bit( (incoming & 0x8)>>3 );
    bb_write_bit( (incoming & 0x4)>>2 );
    bb_write_bit( (incoming & 0x2)>>1 );
    bb_write_bit( (incoming & 0x1)>>0 );
}

void bb_cmd(int incoming)
{
    GPIOB->ODR &= ~GPIO_ODR_12;
    small_delay();
    bb_write_bit(0);
    bb_write_bit(0);
    bb_write_byte(incoming);
    small_delay();
    GPIOB->ODR |= GPIO_ODR_12;
    small_delay();
}

void bb_data(int incoming)
{
    GPIOB->ODR &= ~GPIO_ODR_12;
    small_delay();
    bb_write_bit(1);
    bb_write_bit(0);
    bb_write_byte(incoming);
    small_delay();
    GPIOB->ODR |= GPIO_ODR_12;
    small_delay();
}

void bb_init_oled()
{
    nano_wait(1000000); //wait 1ms for powerup and stabilize
    bb_cmd(0x38); // set for 8-bit operation
    bb_cmd(0x08); // turn display off
    bb_cmd(0x01); //clear display
    nano_wait(2000000); // wait 2 ms for display to clear
    bb_cmd(0x06); // set display to scroll
    bb_cmd(0x02); // move the cursor to the home position
    bb_cmd(0x0c); // turn the display on
}

void bb_display1(const char *sting)
{
    bb_cmd(0x02); // move the cursor to the home postion
    while (*sting != '\0')
    {
        bb_data(*sting);
        sting++;
    }

}

void bb_display2(const char *sting)
{
    bb_cmd(0xc0); // move the cursor to the home position
    while (*sting != '\0')
    {
        bb_data(*sting);
        sting++;
    }
}

void setup_spi2()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    GPIOB->MODER &= ~ (GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER15);
    GPIOB->MODER |= (GPIO_MODER_MODER12_1 | GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1);
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFR12 | GPIO_AFRH_AFR13 | GPIO_AFRH_AFR15);
    SPI2->CR1 |= (SPI_CR1_BR | SPI_CR1_MSTR);
    SPI2->CR2 = (SPI_CR2_DS_3 | SPI_CR2_DS_0);
    SPI2->CR2 |= (SPI_CR2_SSOE | SPI_CR2_NSSP);
    SPI2->CR1 |= SPI_CR1_SPE;
}

void spi_cmd(int copy)
{
    while((SPI2->SR & SPI_SR_TXE) == 0)
        ; // wait for the transmit buffer to be empty
    SPI2->DR = copy;
}

void spi_data(int copy)
{
    while((SPI2->SR & SPI_SR_TXE) == 0)
        ; // wait for the transmit buffer to be empty
    SPI2->DR = (copy | 0x200);

}

void spi_init_oled()
{
    nano_wait(1000000); //wait 1ms for powerup and stabilize
    spi_cmd(0x38); // set for 8-bit operation
    spi_cmd(0x08); // turn display off
    spi_cmd(0x01); //clear display
    nano_wait(2000000); // wait 2 ms for display to clear
    spi_cmd(0x06); // set display to scroll
    spi_cmd(0x02); // move the cursor to the home position
    spi_cmd(0x0c); // turn the display on
}


void spi_display1(const char *sting)
{
    spi_cmd(0x02); // move the cursor to the home postion
    while (*sting != '\0')
    {
        spi_data(*sting);
        sting++;
    }
}

void spi_display2(const char *sting)
{
    spi_cmd(0xc0); // move the cursor to the home postion
    int n = 0;
    while (*sting != '\0')
    {
        spi_data(*sting);
        sting++;
    }
}

void spi_setup_dma(const short * pirate)
{
    RCC->AHBENR |= RCC_AHBENR_DMAEN;
    DMA1_Channel5->CPAR = (uint32_t) (&(SPI2->DR));
    DMA1_Channel5->CMAR = (uint32_t) pirate;
    DMA1_Channel5->CNDTR = 34;
    DMA1_Channel5->CCR |= (DMA_CCR_DIR | DMA_CCR_MINC);
    DMA1_Channel5->CCR &= ~ (DMA_CCR_MSIZE | DMA_CCR_PSIZE);
    DMA1_Channel5->CCR |= (DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0);
    DMA1_Channel5->CCR |= DMA_CCR_CIRC;
    SPI2->CR2 |= SPI_CR2_TXDMAEN;
}

void enable_dma()
{
    RCC->AHBENR |= RCC_AHBENR_DMAEN;
    DMA1_Channel5->CCR |= DMA_CCR_EN;

}

void setup_spi1()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER7 | GPIO_MODER_MODER3);
    GPIOA->MODER |= (GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1 | GPIO_MODER_MODER3_0);
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFR5 | GPIO_AFRL_AFR7);
    GPIOB->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
    GPIOB->MODER |= (GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_0);
    GPIOB->ODR |= (GPIO_ODR_10 | GPIO_ODR_11);
    GPIOA->ODR |= GPIO_ODR_3;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR2 = (SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0);
    SPI1->CR1 |= (SPI_CR1_MSTR);
    SPI1->CR1 &= ~(SPI_CR1_BR);
    SPI1->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI);
    SPI1->CR1 |= SPI_CR1_SPE;
}



// Write your subroutines above

void show_counter(short buffer[])
{
    for(int i=0; i<10000; i++) {
        char line[17];
        sprintf(line,"% 16d", i);
        for(int b=0; b<16; b++)
            buffer[1+b] = line[b] | 0x200;
    }
}



void internal_clock();
void demo();
void autotest();

extern const Picture *image;

int main(void)
{
    //internal_clock();
    //demo();
    //autotest();
/*
    setup_bb();
    bb_init_oled();
    bb_display1("Hello,");
    bb_display2(login);

    setup_spi2();
    spi_init_oled();
    spi_display1("Hello again,");
    spi_display2(login);
*/
    short buffer[34] = {
            0x02, // This word sets the cursor to beginning of line 1.
            // Line 1 consists of spaces (0x20)
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0xc0, // This word sets the cursor to beginning of line 2.
            // Line 2 consists of spaces (0x20)
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
    };

    spi_setup_dma(buffer);
    enable_dma();
    show_counter(buffer);
    setup_spi1();

    //LCD_Init(0,0,0);
    LCD_Clear(BLACK);
    LCD_DrawLine(10,20,100,200, WHITE);
    LCD_DrawRectangle(10,20,100,200, GREEN);
    LCD_DrawFillRectangle(120,20,220,200, RED);
    LCD_Circle(50, 260, 50, 1, BLUE);
    LCD_DrawFillTriangle(130,130, 130,200, 190,160, YELLOW);
    LCD_DrawChar(150,155, BLACK, WHITE, 'X', 16, 1);
    LCD_DrawString(140,60,  WHITE, BLACK, "ECE 362", 16, 0);
    LCD_DrawString(140,80,  WHITE, BLACK, "has the", 16, 1);
    LCD_DrawString(130,100, BLACK, GREEN, "best toys", 16, 0);
    LCD_DrawPicture(110,220,(const Picture *)&image);

}
