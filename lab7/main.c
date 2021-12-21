
//============================================================================
// ECE 362 lab experiment 7 -- Pulse-Width Modulation
//============================================================================

#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for M_PI
#include <stdio.h>

// Be sure to change this to your login...
const char login[] = "ythakkar";

//============================================================================
// setup_tim1()    (Autotest #1)
// Configure Timer 1 and the PWM output pins.
// Parameters: none
//============================================================================
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
    TIM1->CCER  |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
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
int stepb = 0;
int stepc = 0;
int stepd = 0; // not used
int offseta = 0;
int offsetb = 0;
int offsetc = 0;
int offsetd = 0; // not used

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

void set_freq_b(float f)
{
    stepb = f * N / RATE * (1<<16);
    if (f == 0){
        stepb = 0;
        offsetb = 0;
    }
}

void set_freq_c(float f)
{
    stepc = f * N / RATE * (1<<16);
    if (f == 0){
        stepc = 0;
        offsetc = 0;
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
    offsetb += stepb;
    offsetc += stepc;
    offsetd += stepd;

    if ((offseta>>16) >= N)
    {
        offseta -= N<<16;
    }
    if ((offsetb>>16) >= N)
    {
        offsetb -= N<<16;
    }
    if ((offsetc>>16) >= N)
    {
        offsetc -= N<<16;
    }
    if ((offsetd>>16) >= N)
    {
        offsetd -= N<<16;
    }

    int sample = 0;
    sample += wavetable[offseta>>16];
    sample += wavetable[offsetb>>16];
    sample += wavetable[offsetc>>16];
    sample += wavetable[offsetd>>16];

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

//============================================================================
// enable_ports()    (Autotest #3)
// Configure GPIO Ports B and C.
// Parameters: none
//============================================================================
void enable_ports()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(0x3fffff);
    GPIOC->MODER |= 0x155555;
    GPIOB->MODER &= ~(0xffff);
    GPIOB->MODER |= 0x55;
    GPIOB->PUPDR &= ~(0xff00);
    GPIOB->PUPDR |= 0xaa00;
}

char offset;
char history[16];
char display[8];
char queue[2];
int  qin;
int  qout;

//============================================================================
// show_digit()    (Autotest #4)
// Output a single digit on the seven-segment LED array.
// Parameters: none
//============================================================================
void show_digit()
{
    int off = offset & 7;
    GPIOC->ODR = (off << 8) | display[off];

}

//============================================================================
// set_row()    (Autotest #5)
// Set the row active on the keypad matrix.
// Parameters: none
//============================================================================
void set_row()
{
    int row = offset & 3;
    GPIOB->BSRR = 0xf0000 | (1<<row);
}

//============================================================================
// get_cols()    (Autotest #6)
// Read the column pins of the keypad matrix.
// Parameters: none
// Return value: The 4-bit value read from PC[7:4].
//============================================================================
int get_cols()
{
    return (GPIOB->IDR >> 4) & 0xf;
}

//============================================================================
// insert_queue()    (Autotest #7)
// Insert the key index number into the two-entry queue.
// Parameters: n: the key index number
//============================================================================
void insert_queue(int n)
{
    queue[qin] = n | 0x80;
    if (qin == 0)
    {
        qin = 1;
    }
    else
    {
        qin = 0;
    }
}

//============================================================================
// update_hist()    (Autotest #8)
// Check the columns for a row of the keypad and update history values.
// If a history entry is updated to 0x01, insert it into the queue.
// Parameters: none
//============================================================================
void update_hist(int cols)
{
    int row = offset & 3;
    for(int i=0; i < 4; i++){
        history[4*row+i] = (history[4*row+i]<<1) + ((cols>>i)&1);
        if (history[4*row+i] == 0x1)
        {
            insert_queue(4*row+i);
        }
    }
}

//============================================================================
// Timer 7 ISR()    (Autotest #9)
// The Timer 7 ISR
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================
void TIM7_IRQHandler()
{
    TIM7->SR &= ~TIM_SR_UIF;
    show_digit();
    int cols = get_cols();
    update_hist(cols);
    offset = (offset + 1) & 0x7; // count 0 ... 7 and repeat
    set_row();
}

//============================================================================
// setup_tim7()    (Autotest #10)
// Configure timer 7.
// Parameters: none
//============================================================================
void setup_tim7()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    RCC->APB1ENR &= ~(RCC_APB1ENR_TIM6EN);
    TIM7->PSC = 24000 - 1;
    TIM7->ARR = 2 - 1;
    TIM7->DIER |= TIM_DIER_UIE;
    TIM7->CR1 |= TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM7_IRQn);
}

//============================================================================
// getkey()    (Autotest #11)
// Wait for an entry in the queue.  Translate it to ASCII.  Return it.
// Parameters: none
// Return value: The ASCII value of the button pressed.
//============================================================================
int getkey()
{
    while(queue[qout] == 0)
    {
        asm volatile("wfi");
    }

    char copy = queue[qout];
    queue[qout] = 0;
    qout = !qout;
    copy = copy & 0x7f;


    if (copy == 0)
        return '1';
    if (copy == 1)
        return '2';
    if (copy == 2)
        return '3';
    if (copy == 3)
        return 'A';
    if (copy == 4)
        return '4';
    if (copy == 5)
        return '5';
    if (copy == 6)
        return '6';
    if (copy == 7)
        return 'B';
    if (copy == 8)
        return '7';
    if (copy == 9)
        return '8';
    if (copy == 10)
        return '9';
    if (copy == 11)
        return 'C';
    if (copy == 12)
        return '*';
    if (copy == 13)
        return '0';
    if (copy == 14)
        return '#';
    if (copy == 15)
        return 'D';
}

//============================================================================
// This is a partial ASCII font for 7-segment displays.
// See how it is used below.
//============================================================================
const char font[] = {
        [' '] = 0x00,
        ['0'] = 0x3f,
        ['1'] = 0x06,
        ['2'] = 0x5b,
        ['3'] = 0x4f,
        ['4'] = 0x66,
        ['5'] = 0x6d,
        ['6'] = 0x7d,
        ['7'] = 0x07,
        ['8'] = 0x7f,
        ['9'] = 0x67,
        ['A'] = 0x77,
        ['B'] = 0x7c,
        ['C'] = 0x39,
        ['D'] = 0x5e,
        ['*'] = 0x49,
        ['#'] = 0x76,
        ['.'] = 0x80,
        ['?'] = 0x53,
        ['b'] = 0x7c,
        ['r'] = 0x50,
        ['g'] = 0x6f,
        ['i'] = 0x10,
        ['n'] = 0x54,
        ['u'] = 0x1c,
};

// Shift a new character into the display.
void shift(char c)
{
    memcpy(display, &display[1], 7);
    display[7] = font[c];
}

// Turn on the dot of the rightmost display element.
void dot()
{
    display[7] |= 0x80;
}

// Read an entire floating-point number.
float getfloat()
{
    int num = 0;
    int digits = 0;
    int decimal = 0;
    int enter = 0;
    memset(display,0,8);
    display[7] = font['0'];
    while(!enter) {
        int key = getkey();
        if (digits == 8) {
            if (key != '#')
                continue;
        }
        switch(key) {
        case '0':
            if (digits == 0)
                continue;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            num = num*10 + key-'0';
            decimal <<= 1;
            digits += 1;
            if (digits == 1)
                display[7] = font[key];
            else
                shift(key);
            break;
        case '*':
            if (decimal == 0) {
                decimal = 1;
                dot();
            }
            break;
        case '#':
            enter = 1;
            break;
        default: continue; // ABCD
        }
    }
    float f = num;
    while (decimal) {
        decimal >>= 1;
        if (decimal)
            f = f/10.0;
    }
    return f;
}

// Read a 6-digit BCD number for RGB components.
int getrgb()
{
    memset(display, 0, 8);
    display[0] = font['r'];
    display[1] = font['r'];
    display[2] = font['g'];
    display[3] = font['g'];
    display[4] = font['b'];
    display[5] = font['b'];
    int digits = 0;
    int rgb = 0;
    for(;;) {
        int key = getkey();
        if (key >= '0' || key <= '9') {
            display[digits] = font[key];
            digits += 1;
            rgb = (rgb << 4) + (key - '0');
        }
        if (digits == 6)
            break;
    }
    return rgb;
}

//============================================================================
// setrgb()    (Autotest #12)
// Accept a BCD-encoded value for the 3 color components.
// Update the CCR values appropriately.
// Parameters: rgb: the RGB color component values
//============================================================================
void setrgb(int rgb)
{
    int dec0 = (rgb & 0x0f);
    int dec1 = ((rgb & 0xf0) >> 4);
    int first = dec1 * 10 + dec0;
    int dec2 = ((rgb & 0x0f00) >> 8);
    int dec3 = ((rgb & 0xf000) >> 12);
    int second = dec3 * 10 + dec2;
    int dec4 = ((rgb & 0x0f0000) >> 16);
    int dec5 = ((rgb & 0xf00000) >> 20);
    int third = dec5 * 10 + dec4;

    int red = (1 - third / 100.00) * (TIM1->ARR + 1);
    if (red % 2 != 0) red++;
    TIM1->CCR1 = red;

    int green = (1 - second / 100.00) * (TIM1->ARR + 1);
    if (green % 2 != 0) green++;
    TIM1->CCR2 = green;

    int blue = (1 - first / 100.00) * (TIM1->ARR + 1);
    if (blue % 2 != 0) blue++;
    TIM1->CCR3 = blue;
}

void internal_clock();
void demo();
void autotest();

int main(void)
{
    //internal_clock();
    //demo();
    //autotest();
    printf("Hello");
    enable_ports();
    //init_wavetable();
    //set_freq_a(261.626); // Middle 'C'
    //set_freq_b(329.628); // The 'E' above middle 'C'
    //set_freq_c(391.996); // The 'G' above middle 'C'
    //setup_tim1();
    //setup_tim6();
    setup_tim7();

    /*display[0] = font['r'];
    display[1] = font['u'];
    display[2] = font['n'];
    display[3] = font['n'];
    display[4] = font['i'];
    display[5] = font['n'];
    display[6] = font['g'];*/
    for(;;) {
        char key = getkey();
        if (key == 'A')
            set_freq_a(getfloat());
        else if (key == 'B')
            set_freq_b(getfloat());
        else if (key == 'C')
            set_freq_c(getfloat());
        else if (key == 'D')
            setrgb(getrgb());
    }
}
