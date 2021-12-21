
//============================================================================
// ECE 362 lab experiment 9 -- I2C
//============================================================================

#include "stm32f0xx.h"
#include <stdint.h> // for uint8_t
#include <string.h> // for strlen() and strcmp()

// Be sure to change this to your login...
const char login[] = "ythakkar";

//============================================================================
// Wait for n nanoseconds. (Maximum: 4.294 seconds)
//============================================================================
void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

// Write your subroutines below...
void setup_i2c()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    GPIOB->MODER &= ~(0xf0000);
    GPIOB->MODER |= 0xa0000;
    GPIOB->AFR[1] |= 0x11;

    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;
    I2C1->CR1 &= ~I2C_CR1_ERRIE;
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;

    I2C1->TIMINGR &= ~I2C_TIMINGR_PRESC;
    I2C1->TIMINGR |= 0 << 28;
    I2C1->TIMINGR |= 3 << 20;
    I2C1->TIMINGR |= 1 << 16;
    I2C1->TIMINGR |= 3 << 8;
    I2C1->TIMINGR |= 9 << 0;

    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR2 &= ~I2C_OAR2_OA2EN;

    I2C1->CR2 &= ~I2C_CR2_ADD10;
    I2C1->CR2 |= I2C_CR2_AUTOEND;
    I2C1->CR1 |= I2C_CR1_PE;
}

void i2c_waitidle(void)
{
    while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
}

void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir)
{
    uint32_t tmpreg = I2C1->CR2;
    tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);
    if (dir == 1)
        tmpreg |= I2C_CR2_RD_WRN;
    else
        tmpreg &= ~I2C_CR2_RD_WRN;
    tmpreg |= ((devaddr<<1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);
    tmpreg |= I2C_CR2_START;
    I2C1->CR2 = tmpreg;
}

void i2c_stop()
{
    if (I2C1->ISR & I2C_ISR_STOPF)
        return;
    I2C1->CR2 |= I2C_CR2_STOP;
    while((I2C1->ISR & I2C_ISR_STOPF) == 0);
    I2C1->ICR |= I2C_ICR_STOPCF;
}

int8_t i2c_senddata(uint8_t devaddr, void *pdata, uint8_t size)
{
    int i;
    if (size <= 0 || pdata == 0) return -1;
    uint8_t *udata = (uint8_t*)pdata;
    i2c_waitidle();
    i2c_start(devaddr, size, 0);

    for(i=0; i<size; i++)
    {
        int count = 0;
        while((I2C1->ISR & I2C_ISR_TXIS) == 0)
        {
            count += 1;
            if (count > 1000000) return -1;
            //if (i2c_checknack()) {i2c_clearnack(); i2c_stop(); return -1;}
        }
        I2C1->TXDR = udata[i] & I2C_TXDR_TXDATA;
    }

    while ((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    if((I2C1->ISR & I2C_ISR_NACKF) != 0)
        return -1;
    i2c_stop();
    return 0;
}

int8_t i2c_recvdata(uint8_t devaddr, void *pdata, uint8_t size)
{
    int i;
    if (size <= 0 || pdata == 0) return -1;

    uint8_t *udata = (uint8_t*)pdata;
    i2c_waitidle();
    i2c_start(devaddr, size, 1);
    for (i=0; i < size; i++)
    {
        while((I2C1->ISR & I2C_ISR_RXNE) == 0);
        udata[i] = I2C1->RXDR & I2C_RXDR_RXDATA;
    }
    while((I2C1->ISR & I2C_ISR_TC) == 0);

    i2c_stop();
    return 0;
}


void i2c_set_iodir(int x)
{
    uint8_t add[] = {0x0, x};
    i2c_senddata(0x27, add, sizeof(add));
}

void i2c_set_gpio(int x)
{
    uint8_t add[] = {0x9, x};
    i2c_senddata(0x27, add, sizeof(add));
}

int i2c_get_gpio()
{
    char get[32];
    i2c_recvdata(0x27, get, sizeof(get));
    return get[0];

}

void i2c_write_flash(uint16_t loc, const char *data, uint8_t len)
{

    uint8_t buffer[34];
    buffer[0] = (loc & 0xFF00)>>8;
    buffer[1] = (loc & 0xFF);
    for(int x=2; x<=len+2; x++){
        buffer[x] = *data;
        data++;
    }
    //while (*data != '\0'){
    //    buffer[x] = *data;
    //    data++;
    //    x++;
    //}
    i2c_senddata(0x57, buffer, len+2);

}

int i2c_write_flash_complete()
{
    i2c_waitidle();
    i2c_start(0x57,0,0);
    while ((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);
    if ((I2C1->ISR & I2C_ISR_NACKF))
    {
        I2C1->ICR |= I2C_ICR_NACKCF;
        i2c_stop();
        return 0;
    }
    else
    {
        i2c_stop();
        return 1;
    }
}

void i2c_read_flash(uint16_t loc, char data[], uint8_t len)
{
    uint8_t buffer[2] = {(loc & 0xFF00)>>8, loc & 0xFF};
    i2c_senddata(0x57, buffer, sizeof(buffer));
    i2c_recvdata(0x57, data, len);
}

void internal_clock();
void demo();
void autotest();

int main(void)
{
    //internal_clock();
    //demo();
    //autotest();

    setup_i2c();
    //i2c_test();

    i2c_set_iodir(0xf0); //  upper 4 bits input / lower 4 bits output

    // Show the happy LEDs for 4 seconds.
    for(int i=0; i<10; i++) {
        for(int n=1; n <= 8; n <<= 1) {
            i2c_set_gpio(n);
            int value = i2c_get_gpio();
            if ((value & 0xf) != n)
                break;
            nano_wait(100000000); // 0.1 s
        }
    }

    const char string[] = "This is a test.";
    int len = strlen(string) + 1;
    i2c_write_flash(0x200, string, len);

    int count = 0;
    while(1) {
        if (i2c_write_flash_complete())
            break;
        count++;
    }

    if (count == 0) {
        // It could not have completed immediately.
        // i2c_write_flash_complete() does not work.  Show slow angry LEDs.
        int all = 0xf;
        for(;;) {
            i2c_set_gpio(all);
            all ^= 0xf;
            nano_wait(500000000);
        }
    }

    char readback[100];
    i2c_read_flash(0x200, readback, len);
    if (strcmp(string,readback) == 0) {
        // String comparison matched.  Show the happy LEDs.
        for(;;) {
            for(int n=1; n <= 8; n <<= 1) {
                i2c_set_gpio(n);
                int value = i2c_get_gpio();
                if ((value & 0xf) != n)
                    break;
                nano_wait(100000000); // 0.1 s
            }
        }
    } else {
        // String comparison failed.  Show the angry LEDs.
        int all = 0xf;
        for(;;) {
            i2c_set_gpio(all);
            all ^= 0xf;
            nano_wait(100000000);
        }
    }
}
