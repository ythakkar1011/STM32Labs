.syntax unified
.cpu cortex-m0
.fpu softvfp
.thumb

//===================================================================
// ECE 362 Lab Experiment 3
// General Purpose I/O
//===================================================================

.equ  RCC,      0x40021000
.equ  AHBENR,   0x14
.equ  GPIOCEN,  0x00080000
.equ  GPIOBEN,  0x00040000
.equ  GPIOAEN,  0x00020000
.equ  GPIOC,    0x48000800
.equ  GPIOB,    0x48000400
.equ  GPIOA,    0x48000000
.equ  MODER,    0x00
.equ  PUPDR,    0x0c
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28

//===========================================================
// micro_wait: Wait for the number of microseconds specified
// in argument 1.  Maximum delay is (1<<31)-1 microseconds,
// or 2147 seconds.
.global micro_wait
micro_wait:
            movs r1, #10    // 1 cycle
loop:       subs r1, #1     // 1 cycle
            bne loop        // 3 cycles
            nop             // 1 cycle
            nop             // 1 cycle
            nop             // 1 cycle
            subs r0, #1     // 1 cycle
            bne  micro_wait // 3 cycles
            bx  lr          // 1 cycle
            // Total delay = r0 * (1 + 10*(1+3) + 1 + 1 + 1 + 1 + 3) + 1

//===========================================================
// enable_ports: Autotest check 1
// Enable Ports B and C in the RCC AHBENR
// No parameters.
// No expected return value.
.global enable_ports
enable_ports:
    push    {lr}
    // Student code goes here
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOBEN
	orrs r1, r2
	str r1, [r0, #AHBENR]

	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOCEN
	orrs r1, r2
	str r1, [r0, #AHBENR]
    // End of student code
    pop     {pc}

//===========================================================
// port_c_output: Autotest check 2
// Set bits 0-3 of Port C to be outputs.
// No parameters.
// No expected return value.
.global port_c_output
port_c_output:
    push    {lr}
    // Student code goes here
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]
	ldr r2, =0x55
	orrs r1, r2
	str r1, [r0, #MODER]
    // End of student code
    pop     {pc}

//===========================================================
// port_b_input: Autotest check 3
// Set bits 3-4 of Port B to be inputs.
// No parameters.
// No expected return value.
.global port_b_input
port_b_input:
    push    {lr}
    // Student code goes here
	ldr r0, =GPIOB
	ldr r1, [r0, #MODER]
	ldr r2, =0x3c0
	bics r1, r2
	str r1, [r0, #MODER]

    // End of student code
    pop     {pc}

//===========================================================
// setpin: Autotest check 4
// Set the state of a single output pin to be high.
// Do not affect the other bits of the port.
// Parameter 1 is the GPIOx base address.
// Praameter 2 is the bit number of the pin.
// No expected return value.
.global setpin
setpin:
    push    {lr}
    // Student code goes here
    ldr r2, =0x1
    lsls r2, r1
	str r2, [r0, #BSRR]
    // End of student code
    pop     {pc}

//===========================================================
// clrpin: Autotest check 5
// Set the state of a single output pin to be low.
// Do not affect the other bits of the port.
// Parameter 1 is the GPIOx base address.
// Parameter 2 is the bit number of the pin.
// No expected return value.
.global clrpin
clrpin:
    push    {lr}
    // Student code goes here
    ldr r2, =0x1
    lsls r2, r1
	str r2, [r0, #BRR]
    // End of student code
    pop     {pc}

//===========================================================
// getpin: Autotest check 6
// Get the state of the input data register of
// the specified GPIO.
// Parameter 1 is GPIOx base address.
// Parameter 2 is the bit number of pin.
// The subroutine should return 0x1 if the pin is high
// or 0x0 if the pin is low.
.global getpin
getpin:
    push    {lr}
    // Student code goes here
	ldr r2, [r0, #IDR]
	lsrs r2, r1
	movs r3, #1
	ands r2, r3
	movs r0, r2

    // End of student code
    pop     {pc}

//===========================================================
// seq_leds: Autotest check 7
// Update the selected illuminated LED by turning off the currently
// selected LED, incrementing or decrementing 'state' and turning
// on the newly selected LED.
// Parameter 1 is the direction of the sequence
//
// Performs the following logic
// 1) clrpin(GPIOC, state)
// 2) If R0 == 0
//      (a) Increment state by 1
//      (b) Check if state > 3
//      (c) If so set it to 0
// 3) If R1 != 0
//      (a) Decrement state by 1
//      (b) Check if state < 0
//      (c) If so set it to 3
// 4) setpin(GPIOC, state)
// No return value
.data
.align 4
.global state
state: .word 0

.text
.global seq_leds
seq_leds:
    push    {r4,lr}
    // Student code goes here
	movs r4, r0
	ldr r3, =state
	ldr r0, =GPIOC
	ldr r1, [r3]
	bl clrpin

	cmp r4, #0
	beq if1
	b else1

if1:
	adds r1, #1
	cmp r1, #3
	ble setpininst
	movs r1, #0
	b setpininst

else1:
	subs r1, #1
	cmp r1, #0
	bge setpininst
	movs r1, #3
	b setpininst

setpininst:
	bl setpin
	b break

break:
	str r1, [r3]
	// End of student code
    pop     {r4,pc}

//===========================================================
// detect_buttons: Autotest check 8
// Invoke seq_leds(0) when a high signal is detected on
// PB3 and wait for it to go low again.
// Invoke seq_leds(1) when a high signal is detected on
// PB4 and wait for it to go low again.
// No parameters.
// No expected return value.
.global detect_buttons
detect_buttons:
    push    {lr}
    // Student code goes here
ifdetect1:
	ldr r0, =GPIOB
	movs r1, #3
	bl getpin
	cmp r0, #1
	bne ifdetect2

thendetect1:
	movs r0, #0
	bl seq_leds

whiledetect1:
	ldr r0, =GPIOB
	movs r1, #3
	bl getpin
	cmp r0, #1
	bne	ifdetect2
	b whiledetect1

ifdetect2:
	ldr r0, =GPIOB
	movs r1, #4
	bl getpin
	cmp r0, #1
	bne breakdetect1

thendetect2:
	movs r0, #1
	bl seq_leds

whiledetect2:
	ldr r0, =GPIOB
	movs r1, #4
	bl getpin
	cmp r0, #1
	bne breakdetect1
	b whiledetect2

breakdetect1:
    // End of student code
    pop     {pc}

//===========================================================
// enable_port_a: Autotest check A
// Enable Port A in the RCC AHBENR
// No parameters.
// No expected return value.
.global enable_port_a
enable_port_a:
    push    {lr}
    // Student code goes here
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =GPIOAEN
	orrs r1, r2
	str r1, [r0, #AHBENR]
    // End of student code
    pop     {pc}

//===========================================================
// port_a_input: Autotest check B
// Set bit 0 of Port A to be an input and enable its pull-down resistor.
// No parameters.
// No expected return value.
.global port_a_input
port_a_input:
    push    {lr}
    // Student code goes here
	ldr r0, =GPIOA
	ldr r1, [r0, #MODER]
	ldr r2, =0x03
	bics r1, r2
	str r1, [r0, #MODER]

	ldr r0, =GPIOA
	ldr r1, [r0, #PUPDR]
	ldr r2, =0x02
	orrs r1, r2
	str r1, [r0, #PUPDR]
    // End of student code
    pop     {pc}

//===========================================================
// port_b_input2: Autotest check C
// Set bit 2 of Port B to be an input and enable its pull-down resistor.
// No parameters.
// No expected return value.
.global port_b_input2
port_b_input2:
    push    {lr}
    // Student code goes here
	ldr r0, =GPIOB
	ldr r1, [r0, #MODER]
	ldr r2, =0x030
	bics r1, r2
	str r1, [r0, #MODER]

	ldr r0, =GPIOB
	ldr r1, [r0, #PUPDR]
	ldr r2, =0x020
	orrs r1, r2
	str r1, [r0, #PUPDR]
    // End of student code
    pop     {pc}

//===========================================================
// port_c_output: Autotest check D
// Set bits 6-9 of Port C to be outputs.
// No parameters.
// No expected return value.
.global port_c_output2
port_c_output2:
    push    {lr}
    // Student code goes here
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]
	ldr r2, =0x55000
	orrs r1, r2
	str r1, [r0, #MODER]
    // End of student code
    pop     {pc}

//===========================================================
// seq_leds2: Autotest check E
// Update the selected illuminated LED by turning off the currently
// selected LED, incrementing or decrementing 'state2' and turning
// on the newly selected LED.
// Parameter 1 is the direction of the sequence
//
// Performs the following logic
// 1) clrpin(PORTC, state2)
// 2) If R0 == 0
//      (a) Increment state2 by 1
//      (b) Check if state2 > 9
//      (c) If so set it to 6
// 3) If R1 != 0
//      (a) Decrement state2 by 1
//      (b) Check if state2 < 6
//      (c) If so set it to 9
// 4) setpin(PORTC, state2)
// No return value
.data
.align 4
.global state2
state2: .word 6

.text
.global seq_leds2
seq_leds2:
    push    {r4,lr}
    // Student code goes here
	movs r4, r0
	ldr	r0, =GPIOC
	ldr	r3, =state2
	ldr r1, [r3]
	bl	clrpin

	cmp r4, #0
	beq if2
	b else2

if2:
	adds r1, #1
	cmp r1, #9
	ble setpininst2
	movs r1, #6
	b setpininst2

else2:
	subs r1, #1
	cmp r1, #6
	bge setpininst2
	movs r1, #9
	b setpininst2

setpininst2:
	bl setpin
	b break2

break2:
	str r1, [r3]
    // End of student code
    pop     {r4,pc}

//===========================================================
// detect_buttons2: Autotest check F
// Invoke seq_leds2(0) when a high signal is detected on
// PA0 and wait for it to go low again.
// Invoke seq_leds2(1) when a high signal is detected on
// PB2 and wait for it to go low again.
// No parameters.
// No expected return value.
.global detect_buttons2
detect_buttons2:
    push    {lr}
    // Student code goes here
ifdetect21:
	ldr r0, =GPIOA
	movs r1, #0
	bl getpin
	cmp r0, #1
	bne ifdetect22

thendetect21:
	movs r0, #0
	bl seq_leds2

whiledetect21:
	ldr r0, =GPIOA
	movs r1, #0
	bl getpin
	cmp r0, #1
	bne	ifdetect2
	b whiledetect21

ifdetect22:
	ldr r0, =GPIOB
	movs r1, #2
	bl getpin
	cmp r0, #1
	bne breakdetect21

thendetect22:
	movs r0, #1
	bl seq_leds2

whiledetect22:
	ldr r0, =GPIOB
	movs r1, #2
	bl getpin
	cmp r0, #1
	bne breakdetect21
	b whiledetect22

breakdetect21:
    // End of student code
    pop     {pc}

//===========================================================
// The main subroutine calls everything else.
// It never returns.
.global login
login: .string "ythakkar" // Change to your login
.align 2
.global main
main:
	//bl   autotest // Uncomment when most things are working
	bl   enable_ports
	bl   port_c_output
	// Turn on LED for PC0
	ldr  r0,=GPIOC
	movs r1,#0
	bl   setpin
	bl   port_b_input
	bl   enable_port_a
	bl   port_a_input
	bl   port_b_input2
	bl   port_c_output2
	// Turn on the LED for PC6
	ldr  r0,=GPIOC
	movs r1,#6
	bl   setpin
endless_loop:
	bl   detect_buttons
	bl   detect_buttons2
	b    endless_loop
