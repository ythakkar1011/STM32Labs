
#include "stm32f0xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h> // for memset()
#include <stdio.h>
#include <math.h>
#include "lcd.h"
#include "fifo.h"
#include "tty.h"

int main(void);

void setup_spi2()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    GPIOB->MODER &= ~(0xcf000000);
    GPIOB->MODER |= 0x8a000000;
    SPI2->CR1 |= (SPI_CR1_BR);
    SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
    SPI2->CR2 = SPI_CR2_DS_3 | SPI_CR2_DS_0 | SPI_CR2_SSOE | SPI_CR2_NSSP;
    //SPI2->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP;
    SPI2->CR1 |= SPI_CR1_SPE;
}

void spi_cmd(int x)
{
    for(;;)
    {
      if ((SPI2->SR & SPI_SR_TXE) == SPI_SR_TXE)
      {
          SPI2->DR = x;
          break;
      }
    }
}

void spi_data(int x)
{
    for(;;)
    {
      if ((SPI2->SR & SPI_SR_TXE) == SPI_SR_TXE)
      {
          x |= 0x200;
          SPI2->DR = x;
          break;
      }
    }
}

void spi_init_oled()
{
    nano_wait(1000000);
    spi_cmd(0x38);
    spi_cmd(0x08);
    spi_cmd(0x01);
    nano_wait(2000000);
    spi_cmd(0x06);
    spi_cmd(0x02);
    spi_cmd(0x0c);
}

void spi_display1(const char *a)
{
    spi_cmd(0x02);

    while (*a != '\0')
    {
        spi_data(*a);
        ++a;
    }
}

void spi_display2(const char *a)
{
    spi_cmd(0xc0);

    while (*a != '\0')
    {
        spi_data(*a);
        ++a;
    }
}


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

int __io_putchar(int ch)
{
    return better_putchar(ch);
}

int __io_getchar(void)
{
    return line_buffer_getchar();
}

void setup_tim17()
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
    TIM17->PSC = 4800-1;
    TIM17->ARR = 100-1;
    TIM17->DIER |= TIM_DIER_UIE;
    TIM17->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = (1<<TIM17_IRQn);
    // Set this to invoke the ISR 100 times per second.
}

void setup_portb()
{
    // Enable Port B.
    // Set PB0-PB3 for output.
    // Set PB4-PB7 for input and enable pull-down resistors.
    // Turn on the output for the lower row.
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~0xffff;
    GPIOB->MODER |= 0x55;
    GPIOB->PUPDR &= ~0xff00;
    GPIOB->PUPDR |= 0xaa00;
    GPIOB->BSRR = GPIO_ODR_3;

}

char getdifficulty()
{
    char line[100];
    printf("\nChoose Difficulty 1, 2, or 3 with 1 being the Easiest: ");
    fgets(line, 99, stdin);
    line[99] = '\0'; // just in case
    for(;;)
    {
        if (line[0] == '1')
        {
            printf("\nYou entered: %c->Easy Difficulty", line[0]);
            break;
        }
        else if (line[0] == '2')
        {
            printf("\nYou entered: %c->Medium Difficulty", line[0]);
            break;
        }
        else if (line[0] == '3')
        {
            printf("\nYou entered: %c->Hard Difficulty", line[0]);
            break;
        }
        else
        {
            printf("\nPlease type a number that is 1, 2, or 3 with no spaces: ");
            memset(line, 0, sizeof(line));
            fgets(line, 99, stdin);
            line[99] = '\0'; // just in case
        }
    }
    return (line[0]);
}

char check_key()
{
    //int x = (GPIOB -> IDR) >> 4;
    if (GPIOB->IDR & GPIO_ODR_4)
        return '*';
    if (GPIOB->IDR & GPIO_ODR_5)
        return '0';
    if (GPIOB->IDR & GPIO_ODR_7)
        return 'D';
    return 0;
    // If the '*' key is pressed, return '*'
    // If the 'D' key is pressed, return 'D'
    // Otherwise, return 0
}

void setup_spi1()
{
    // Use setup_spi1() from lab 8.
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(0xcff0);
    GPIOA->MODER |= 0x8a00;
    GPIOA->MODER |= 0x0050;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    SPI1->CR1 &= ~(SPI_CR1_BR);
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
    SPI1->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP;
    SPI1->CR1 |= SPI_CR1_SPE;
}

// Copy a subset of a large source picture into a smaller destination.
// sx,sy are the offset into the source picture.
void pic_subset(Picture *dst, const Picture *src, int sx, int sy)
{
    int dw = dst->width;
    int dh = dst->height;
    if (dw + sx > src->width)
        for(;;)
            ;
    if (dh + sy > src->height)
        for(;;)
            ;
    for(int y=0; y<dh; y++)
        for(int x=0; x<dw; x++)
            dst->pix2[dw * y + x] = src->pix2[src->width * (y+sy) + x + sx];
}

// Overlay a picture onto a destination picture.
// xoffset,yoffset are the offset into the destination picture that the
// source picture is placed.
// Any pixel in the source that is the 'transparent' color will not be
// copied.  This defines a color in the source that can be used as a
// transparent color.
void pic_overlay(Picture *dst, int xoffset, int yoffset, const Picture *src, int transparent)
{
    for(int y=0; y<src->height; y++) {
        int dy = y+yoffset;
        if (dy < 0 || dy >= dst->height)
            continue;
        for(int x=0; x<src->width; x++) {
            int dx = x+xoffset;
            if (dx < 0 || dx >= dst->width)
                continue;
            unsigned short int p = src->pix2[y*src->width + x];
            if (p != transparent)
                dst->pix2[dy*dst->width + dx] = p;
        }
    }
}

// Called after a bounce, update the x,y velocity components in a
// pseudo random way.  (+/- 1)
void perturbHard(int *vx, int *vy)
{
    if (*vx > 0) {
        *vx += (random()%3) - 1;
        if (*vx >= 3)
            *vx = 4;
        if (*vx == 0)
            *vx = 3;
    } else {
        *vx += (random()%3) - 1;
        if (*vx <= -3)
            *vx = -4;
        if (*vx == 0)
            *vx = -3;
    }
    if (*vy > 0) {
        *vy += (random()%3) - 1;
        if (*vy >= 3)
            *vy = 4;
        if (*vy == 0)
            *vy = 3;
    } else {
        *vy += (random()%3) - 1;
        if (*vy <= -3)
            *vy = -4;
        if (*vy == 0)
            *vy = -3;
    }
}

void perturbMed(int *vx, int *vy)
{
    if (*vx > 0) {
        *vx += (random()%3) - 1;
        if (*vx >= 3)
            *vx = 3;
        if (*vx == 0)
            *vx = 2;
    } else {
        *vx += (random()%3) - 1;
        if (*vx <= -3)
            *vx = -3;
        if (*vx == 0)
            *vx = -2;
    }
    if (*vy > 0) {
        *vy += (random()%3) - 1;
        if (*vy >= 3)
            *vy = 3;
        if (*vy == 0)
            *vy = 2;
    } else {
        *vy += (random()%3) - 1;
        if (*vy <= -3)
            *vy = -3;
        if (*vy == 0)
            *vy = -2;
    }
}

void perturbEasy(int *vx, int *vy)
{
    if (*vx > 0) {
        *vx += (random()%3) - 1;
        if (*vx >= 3)
            *vx = 2;
        if (*vx == 0)
            *vx = 1;
    } else {
        *vx += (random()%3) - 1;
        if (*vx <= -3)
            *vx = -2;
        if (*vx == 0)
            *vx = -1;
    }
    if (*vy > 0) {
        *vy += (random()%3) - 1;
        if (*vy >= 3)
            *vy = 2;
        if (*vy == 0)
            *vy = 1;
    } else {
        *vy += (random()%3) - 1;
        if (*vy <= -3)
            *vy = -2;
        if (*vy == 0)
            *vy = -1;
    }
}
extern const Picture background; // A 240x320 background image
extern const Picture ball; // A 19x19 purple ball with white boundaries
extern const Picture paddle; // A 59x5 paddle

const int border = 20;
int xmin; // Farthest to the left the center of the ball can go
int xmax; // Farthest to the right the center of the ball can go
int ymin; // Farthest to the top the center of the ball can go
int ymax; // Farthest to the bottom the center of the ball can go
int x,y; // Center of ball
int vx,vy; // Velocity components of ball

int px; // Center of paddle offset
int newpx; // New center of paddle

// This C macro will create an array of Picture elements.
// Really, you'll just use it as a pointer to a single Picture
// element with an internal pix2[] array large enough to hold
// an image of the specified size.
// BE CAREFUL HOW LARGE OF A PICTURE YOU TRY TO CREATE:
// A 100x100 picture uses 20000 bytes.  You have 32768 bytes of SRAM.
#define TempPicturePtr(name,width,height) Picture name[(width)*(height)/6+2] = { {width,height,2} }

// Create a 29x29 object to hold the ball plus padding
TempPicturePtr(object,29,29);
int getNum = 0;
char diff;
int i=0;
int history=0;
void TIM17_IRQHandler(void)
{

    TIM17->SR &= ~TIM_SR_UIF;
    char key = check_key();

    if (getNum == 0)
    {
        diff = getdifficulty();
        getNum = 1;
    }
    if (key == '*')
        newpx -= 1;
    else if (key == 'D')
        newpx += 1;
    if (newpx - paddle.width/2 <= border || newpx + paddle.width/2 >= 240-border)
        newpx = px;
    if (newpx != px) {
        px = newpx;
        // Create a temporary object two pixels wider than the paddle.
        // Copy the background into it.
        // Overlay the paddle image into the center.
        // Draw the result.
        //
        // As long as the paddle moves no more than one pixel to the left or right
        // it will clear the previous parts of the paddle from the screen.
        TempPicturePtr(tmp,61,5);
        pic_subset(tmp, &background, px-tmp->width/2, background.height-border-tmp->height); // Copy the background
        pic_overlay(tmp, 1, 0, &paddle, -1);
        LCD_DrawPicture(px-tmp->width/2, background.height-border-tmp->height, tmp);
    }

    x += vx;
    y += vy;
    if (x <= xmin) {
        // Ball hit the left wall.
        TIM1->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E);
        vx = - vx;
        if (x < xmin)
            x += vx;
        if(diff == '1')
            perturbEasy(&vx,&vy);
        else if(diff == '2')
            perturbMed(&vx,&vy);
        else if(diff == '3')
            perturbHard(&vx,&vy);
    }
    if (x >= xmax) {
        // Ball hit the right wall.
        TIM1->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E);
        vx = -vx;
        if (x > xmax)
            x += vx;
        if(diff == '1')
            perturbEasy(&vx,&vy);
        else if(diff == '2')
            perturbMed(&vx,&vy);
        else if(diff == '3')
            perturbHard(&vx,&vy);
    }
    if (y <= ymin) {
        // Ball hit the top wall.
        TIM1->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E);
        vy = - vy;
        if (y < ymin)
            y += vy;
        if(diff == '1')
            perturbEasy(&vx,&vy);
        else if(diff == '2')
            perturbMed(&vx,&vy);
        else if(diff == '3')
            perturbHard(&vx,&vy);
    }

    if (y >= ymax - paddle.height &&
        x >= (px - paddle.width/2) &&
        x <= (px + paddle.width/2)) {
        // The ball has hit the paddle.  Bounce.
        spi_init_oled();
        ++i;
        char buffer[100];
        itoa(i,buffer,10);
        spi_display1(buffer);
        TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;
        int pmax = ymax - paddle.height;
        vy = -vy;
        if (y > pmax)
            y += vy;
    }
    else if (y >= ymax) {
        // The ball has hit the bottom wall.  Set velocity of ball to 0,0.
        vx = 0;
        vy = 0;
        char ask[100];
        printf("\nGAME OVER!! Play again with different settings? Enter 'y' for yes or 'n' for no.If no, press '0' on the keypad to use same settings again: ");
        char str[100];
        if(i > history)
        {
            history = i;
            char buffer[100];
            itoa(i,buffer,10);

            strcpy(str, "High Score: ");
            strcat(str, buffer);
        }
        spi_display2(str);
        fgets(ask, 99, stdin);
        ask[99] = '\0'; // just in case
        if(ask[0] == 'y')
        {
            getNum = 0;
            i = 0;
            main();
        }
        else if(ask[0] == 'n')
        {
            while(check_key() != '0');

            i = 0;
            main();
        }
    }

    TempPicturePtr(tmp,29,29); // Create a temporary 29x29 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    pic_overlay(tmp, 0,0, object, 0xffff); // Overlay the object
    pic_overlay(tmp, (px-paddle.width/2) - (x-tmp->width/2),
            (background.height-border-paddle.height) - (y-tmp->height/2),
            &paddle, 0xffff); // Draw the paddle into the image
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Re-draw it to the screen
    // The object has a 5-pixel border around it that holds the background
    // image.  As long as the object does not move more than 5 pixels (in any
    // direction) from it's previous location, it will clear the old object.
}

struct {
            float frequency;
            uint16_t duration;
} song[] = {
            { 523.25, 1000 }, // C5
            { 587.33, 1000 }, // D5
            { 659.25, 1000 }, // E5
            { 698.46, 1000 }, // F5
            { 783.99, 1000 }, // G5
            { 880.00, 1000 }, // A6
            { 987.77, 1000 }, // B6
            { 1046.50, 1000 }, // C6
};
struct {
            float frequency;
            uint16_t duration;
} song2[] = {
            { 554.37, 1000 }, //
            { 783.99, 1000 }, //
            { 659.25, 1000 }, //
            { 440, 1000 }, //
            { 493.88, 1000 }, // B4
            { 466.16, 1000 }, // Bb
            { 440, 1000 }, //
            { 783.99, 1000 }, // G5
            { 1396.1, 1000 },
            { 830.61, 1000 },
            { 932.33, 1000 },//A^
            { 739.99, 1000 },
            { 830.61, 1000 },
};
struct {
            float frequency;
            uint16_t duration;
} song3[] = {
            { 510, 1000 }, //
            { 380, 1000 }, //
            { 320, 1000 }, //
            { 440, 1000 }, //
            { 480, 1000 }, // B4
            { 450, 1000 }, // Bb
            { 430, 1000 }, //
            { 380, 1000 }, // G5
            { 660, 1000 },
            { 760, 1000 },
            { 860, 1000 },//A^
            { 700, 1000 },
            { 760, 1000 },
};
int note = 0;

void setup_tim1()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0xaa0000;
    GPIOA->AFR[1] |= 0x2222;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->PSC = 1 - 1;
    TIM1->ARR = 2400 - 1;
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
    TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
    TIM1->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;
    TIM1->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;
    TIM1->CCMR2 |= TIM_CCMR2_OC4PE;
    TIM1->CCER  |= TIM_CCER_CC4E;
    TIM1->CR1 |= TIM_CR1_CEN;
}

//============================================================================
// Parameters for the wavetable size and expected synthesis rate.
//============================================================================
#define N 1000
#define RATE 20000
short int wavetable[N];

//============================================================================
// init_wavetable()    (Autotest #2)
// Write the pattern for one complete cycle of a sine wave into the
// wavetable[] array.
// Parameters: none
//============================================================================
void init_wavetable(void)
{
    for(int i=0; i < N; i++)
    {
        wavetable[i] = 32767 * sin(2 * M_PI * i / N);
    }
}

//============================================================================
// Global variables used for four-channel synthesis.
//============================================================================
int volume = 2048;
int stepa = 0;
int offseta = 0;


//============================================================================
// set_freq_n()    (Autotest #2)
// Set the four step and four offset variables based on the frequency.
// Parameters: f: The floating-point frequency desired.
//============================================================================
void set_freq_a(float f)
{
    stepa = f * N / RATE * (1<<16);

    if (f == 0){
        stepa = 0;
        offseta = 0;
    }
}



//============================================================================
// Timer 6 ISR    (Autotest #2)
// The ISR for Timer 6 which computes the DAC samples.
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================
void TIM6_DAC_IRQHandler(void)
{
    TIM6->SR &= ~TIM_SR_UIF;
    //DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;

    offseta += stepa;

    if ((offseta>>16) >= N)
    {
        offseta -= N<<16;
    }


    int sample = 0;
    sample += wavetable[offseta>>16];

    //sample = sample / 32 + 2048;
    sample = ((sample * volume)>>17) + 1200;
    if (sample > 4095)
    {
        sample = 4095;
    }

    if (sample < 0)
    {
        sample = 0;
    }

    TIM1->CCR4 = sample;

    //start_adc_channel(0);
    //volume = read_adc();
}

//============================================================================
// setup_tim6()    (Autotest #2)
// Set the four step and four offset variables based on the frequency.
// Parameters: f: The floating-point frequency desired.
//============================================================================
void setup_tim6()
{
    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 80-1;
    TIM6->ARR = 30-1;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = (1<<TIM6_DAC_IRQn);
}

char offset;
//============================================================================
// Timer 7 ISR()    (Autotest #9)
// The Timer 7 ISR
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================
void TIM7_IRQHandler()
{
    TIM7->SR &= ~TIM_SR_UIF;

    set_freq_a(song3[note].frequency);
    note++;
    if(note >= 7)
    {
        note = 0;
    }
}

//============================================================================
// setup_tim7()    (Autotest #10)
// Configure timer 7.
// Parameters: none
//============================================================================
void setup_tim7()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    //RCC->APB1ENR &= ~(RCC_APB1ENR_TIM6EN);
    TIM7->PSC = 48000 - 1;
    TIM7->ARR = 500 - 1;
    TIM7->DIER |= TIM_DIER_UIE;
    TIM7->CR1 |= TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM7_IRQn);
}

int main(void)
{
    setup_usart5();

    setup_spi2();

    init_wavetable();

    // Uncomment these when you're asked to...
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);


    setup_portb();
    setup_spi1();
    LCD_Init();

    // Draw the background.
    LCD_DrawPicture(0,0,&background);

    // Set all pixels in the object to white.
    for(int i=0; i<29*29; i++)
        object->pix2[i] = 0xffff;

    // Center the 19x19 ball into center of the 29x29 object.
    // Now, the 19x19 ball has 5-pixel white borders in all directions.
    pic_overlay(object,5,5,&ball,0xffff);

    // Initialize the game state.
    xmin = border + ball.width/2;
    xmax = background.width - border - ball.width/2;
    ymin = border + ball.width/2;
    ymax = background.height - border - ball.height/2;
    x = (xmin+xmax)/2; // Center of ball
    y = ymin;
    vx = 0; // Velocity components of ball
    vy = 1;

    px = -1; // Center of paddle offset (invalid initial value to force update)
    newpx = (xmax+xmin)/2; // New center of paddle

    setup_tim17();
    setup_tim1();
    setup_tim6();
    setup_tim7();
}
