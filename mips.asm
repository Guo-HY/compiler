.data

lgv: .word 0
l.const_str3: .asciiz "now a = "
l.const_str2: .asciiz "hello, this is foo2, depth = "
l.const_str1: .asciiz "\n"
l.const_str0: .asciiz "hello, this is foo1, depth = "
.text

jal lmain
 j gsc
lfoo1:
lfoo1.1:
	addiu  $sp, $sp, -84
	sw  $ra, 16($sp)
	nop  
	lw  $t0, 84($sp)
	sw  $t0, 24($sp)
	lw  $t0, 24($sp)
	sw  $t0, 20($sp)
	lw  $t0, 20($sp)
	sw  $t0, 28($sp)
	la  $t0, l.const_str0
	sw  $t0, 32($sp)
	addiu  $t0, $zero, 120
	sw  $t0, 36($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 40($sp)
	lw  $t0, 36($sp)
	lw  $t1, 40($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 36($sp)
	lw  $t1, 32($sp)
	lw  $t2, 36($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 32($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 36($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 44($sp)
	lw  $t0, 36($sp)
	lw  $t1, 44($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 36($sp)
	lw  $t1, 32($sp)
	lw  $t2, 36($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 32($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 32($sp)
	addu  $a0, $zero, $t0
	syscall  
	addiu  $v0, $zero, 1
	lw  $t0, 28($sp)
	addu  $a0, $zero, $t0
	syscall  
	la  $t0, l.const_str1
	sw  $t0, 48($sp)
	addiu  $t0, $zero, 8
	sw  $t0, 52($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 56($sp)
	lw  $t0, 52($sp)
	lw  $t1, 56($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 52($sp)
	lw  $t1, 48($sp)
	lw  $t2, 52($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 48($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 52($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 60($sp)
	lw  $t0, 52($sp)
	lw  $t1, 60($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 52($sp)
	lw  $t1, 48($sp)
	lw  $t2, 52($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 48($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 48($sp)
	addu  $a0, $zero, $t0
	syscall  
	lw  $t0, 20($sp)
	sw  $t0, 64($sp)
	addiu  $t0, $zero, 5
	sw  $t0, 68($sp)
	lw  $t1, 64($sp)
	lw  $t2, 68($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 72($sp)
	lw  $t0, lgv
	sw  $t0, 76($sp)
	lw  $t1, 72($sp)
	lw  $t2, 76($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 80($sp)
	lw  $ra, 16($sp)
	lw  $t0, 80($sp)
	addu  $v0, $zero, $t0
	addiu  $sp, $sp, 84
	jr  $ra


lfoo2:
lfoo2.1:
	addiu  $sp, $sp, -84
	sw  $ra, 16($sp)
	nop  
	lw  $t0, 84($sp)
	sw  $t0, 24($sp)
	lw  $t0, 24($sp)
	sw  $t0, 20($sp)
	lw  $t0, 20($sp)
	sw  $t0, 28($sp)
	la  $t0, l.const_str2
	sw  $t0, 32($sp)
	addiu  $t0, $zero, 120
	sw  $t0, 36($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 40($sp)
	lw  $t0, 36($sp)
	lw  $t1, 40($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 36($sp)
	lw  $t1, 32($sp)
	lw  $t2, 36($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 32($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 36($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 44($sp)
	lw  $t0, 36($sp)
	lw  $t1, 44($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 36($sp)
	lw  $t1, 32($sp)
	lw  $t2, 36($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 32($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 32($sp)
	addu  $a0, $zero, $t0
	syscall  
	addiu  $v0, $zero, 1
	lw  $t0, 28($sp)
	addu  $a0, $zero, $t0
	syscall  
	la  $t0, l.const_str1
	sw  $t0, 48($sp)
	addiu  $t0, $zero, 8
	sw  $t0, 52($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 56($sp)
	lw  $t0, 52($sp)
	lw  $t1, 56($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 52($sp)
	lw  $t1, 48($sp)
	lw  $t2, 52($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 48($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 52($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 60($sp)
	lw  $t0, 52($sp)
	lw  $t1, 60($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 52($sp)
	lw  $t1, 48($sp)
	lw  $t2, 52($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 48($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 48($sp)
	addu  $a0, $zero, $t0
	syscall  
	lw  $t0, 20($sp)
	sw  $t0, 64($sp)
	addiu  $t0, $zero, 5
	sw  $t0, 68($sp)
	lw  $t1, 64($sp)
	lw  $t2, 68($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 72($sp)
	lw  $t0, lgv
	sw  $t0, 76($sp)
	lw  $t1, 72($sp)
	lw  $t2, 76($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 80($sp)
	lw  $ra, 16($sp)
	lw  $t0, 80($sp)
	addu  $v0, $zero, $t0
	addiu  $sp, $sp, 84
	jr  $ra


lmain:
lmain.1:
	addiu  $sp, $sp, -124
	sw  $ra, 16($sp)
	nop  
	addiu  $t0, $zero, 5
	sw  $t0, 24($sp)
	lw  $t0, 24($sp)
	sw  $t0, 20($sp)
	lw  $t0, 20($sp)
	sw  $t0, 28($sp)
	lw  $t0, 28($sp)
	sw  $t0, 0($sp)
	jal  lfoo1
	add  $t0, $zero, $v0
	sw  $t0, 32($sp)
	lw  $t0, 32($sp)
	sw  $t0, 20($sp)
	lw  $t0, 20($sp)
	sw  $t0, 36($sp)
	la  $t0, l.const_str3
	sw  $t0, 40($sp)
	addiu  $t0, $zero, 36
	sw  $t0, 44($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 48($sp)
	lw  $t0, 44($sp)
	lw  $t1, 48($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 44($sp)
	lw  $t1, 40($sp)
	lw  $t2, 44($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 40($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 44($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 52($sp)
	lw  $t0, 44($sp)
	lw  $t1, 52($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 44($sp)
	lw  $t1, 40($sp)
	lw  $t2, 44($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 40($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 40($sp)
	addu  $a0, $zero, $t0
	syscall  
	addiu  $v0, $zero, 1
	lw  $t0, 36($sp)
	addu  $a0, $zero, $t0
	syscall  
	la  $t0, l.const_str1
	sw  $t0, 56($sp)
	addiu  $t0, $zero, 8
	sw  $t0, 60($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 64($sp)
	lw  $t0, 60($sp)
	lw  $t1, 64($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 60($sp)
	lw  $t1, 56($sp)
	lw  $t2, 60($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 56($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 60($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 68($sp)
	lw  $t0, 60($sp)
	lw  $t1, 68($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 60($sp)
	lw  $t1, 56($sp)
	lw  $t2, 60($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 56($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 56($sp)
	addu  $a0, $zero, $t0
	syscall  
	addiu  $t0, $zero, 5
	sw  $t0, 72($sp)
	lw  $t0, 72($sp)
	sw  $t0, lgv
	lw  $t0, 20($sp)
	sw  $t0, 76($sp)
	lw  $t0, 76($sp)
	sw  $t0, 0($sp)
	jal  lfoo2
	add  $t0, $zero, $v0
	sw  $t0, 80($sp)
	lw  $t0, 80($sp)
	sw  $t0, 20($sp)
	lw  $t0, 20($sp)
	sw  $t0, 84($sp)
	la  $t0, l.const_str3
	sw  $t0, 88($sp)
	addiu  $t0, $zero, 36
	sw  $t0, 92($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 96($sp)
	lw  $t0, 92($sp)
	lw  $t1, 96($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 92($sp)
	lw  $t1, 88($sp)
	lw  $t2, 92($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 88($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 92($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 100($sp)
	lw  $t0, 92($sp)
	lw  $t1, 100($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 92($sp)
	lw  $t1, 88($sp)
	lw  $t2, 92($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 88($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 88($sp)
	addu  $a0, $zero, $t0
	syscall  
	addiu  $v0, $zero, 1
	lw  $t0, 84($sp)
	addu  $a0, $zero, $t0
	syscall  
	la  $t0, l.const_str1
	sw  $t0, 104($sp)
	addiu  $t0, $zero, 8
	sw  $t0, 108($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 112($sp)
	lw  $t0, 108($sp)
	lw  $t1, 112($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 108($sp)
	lw  $t1, 104($sp)
	lw  $t2, 108($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 104($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 108($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 116($sp)
	lw  $t0, 108($sp)
	lw  $t1, 116($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 108($sp)
	lw  $t1, 104($sp)
	lw  $t2, 108($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 104($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 104($sp)
	addu  $a0, $zero, $t0
	syscall  
	lw  $ra, 16($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 120($sp)
	lw  $t0, 120($sp)
	addu  $v0, $zero, $t0
	addiu  $sp, $sp, 124
	jr  $ra


gsc:
