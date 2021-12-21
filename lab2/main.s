.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.data
.align 4
// Your global variables go here
.global result
result: .word 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

.global source
source: .word 1, 2, 2, 4, 5, 9, 12, 8, 9, 10, 11

.global str
str: .string "hello, 01234 world! 56789-+"

.text
.global intsub
intsub:
    // Your code for intsub goes here
	//push {r1,r2,r3,r4,r6,lr}

for1:
	movs r0, #0

check1:
	cmp r0, #10
	bge break1

if1:
	movs r1, #2
	ands r1, r0
	cmp r1, #2
	bne else1

then1:
	ldr r1, =source
	movs r2, #4
	muls r2, r0
	ldr r2, [r1, r2]
	movs r3, #1
	lsls r2, r3

	ldr r1, =result
	movs r3, #4
	muls r3, r0
	str r2, [r1, r3]
	b incre

else1:
	ldr r1, =source
	movs r3, #4
	adds r2, r0, #1
	muls r2, r3
	ldr r2, [r1, r2]
	muls r3, r0
	ldr r3, [r1, r3]
	subs r2, r2, r3
	ldr r1, =result
	movs r3, #4
	muls r3, r0
	str r2, [r1, r3]
	b incre

incre:
	adds r0, r0, #1
	b check1


break1:
	//pop {r1,r2,r3,r4,r6,pc}
    bx lr



.global charsub
charsub:
    // Your code for charsub goes here
	//push {r1,r2,r3,r4,r6,lr}

for2:
	movs r0, #0

check2:
	ldr r1, =str
    ldrb r2, [r1, r0]
    cmp r2, #0x00
    beq break2

ifcheck1:
	ldr r1, =str
	ldrb r2, [r1, r0]
	cmp r2, #0x30
	blt incre2

ifcheck2:
	ldr r1, =str
	ldrb r2, [r1, r0]
	cmp r2, #0x39
	bgt incre2

then2:
	ldr r1, =str
	ldrb r2, [r1, r0]
	adds r2, #0x31
	strb r2, [r1, r0]
	b incre2

incre2:
    adds r0, #1
    b check2

break2:
	//pop {r1,r2,r3,r4,r6,pc}
    bx lr


.global login
login: .string "ythakkar" // Make sure you put your login here.
.align 2
.global main
main:
    bl autotest // uncomment AFTER you debug your subroutines
    bl intsub
    bl charsub
    bkpt
