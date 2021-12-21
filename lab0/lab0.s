.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.equ RCC, 0x40021000
.equ AHBENR, 0x14
.equ IOPCEN, 0x80000

.equ GPIOC, 0x48000800
.equ MODER, 0x00
.equ ODR, 0x14
.equ OUT7, 0x4000
.equ PC7, 0x80

.global main
main:
    //bl autotest
    bl yellowLED

yellowLED:
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =IOPCEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]
	ldr r2, =OUT7
	orrs r1, r2
	str r1, [r0, #MODER]

	ldr r0, =GPIOC
	ldr r1, =PC7
	ldr r2, [r0, #ODR]
	orrs r2, r1
	str r2, [r0, #ODR]

	bx lr
    bkpt

.global login
login:
    .asciz "ythakkar"
.align 2
