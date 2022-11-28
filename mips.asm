.data

lN: .word 10
la: .word 0,1,2,3,4,5,6,7,8,9
l.const_str1: .asciiz "\n"
l.const_str0: .asciiz ", "
.text

jal lmain
 j gsc
lfib:
lfib.1:
	addiu  $sp, $sp, -96
	sw  $ra, 16($sp)
	nop  
	lw  $t0, 96($sp)
	sw  $t0, 24($sp)
	lw  $t0, 24($sp)
	sw  $t0, 20($sp)
	lw  $t0, 20($sp)
	sw  $t0, 28($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 32($sp)
	lw  $t1, 32($sp)
	lw  $t2, 28($sp)
	xor  $t0, $t1, $t2
	sw  $t0, 36($sp)
	lw  $t1, 36($sp)
	sltiu  $t0, $t1, 1
	sw  $t0, 36($sp)
	lw  $t0, 36($sp)
	bne  $t0, $zero, lfib.4
	j  lfib.5

lfib.4:
	lw  $ra, 16($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 40($sp)
	lw  $t0, 40($sp)
	addu  $v0, $zero, $t0
	addiu  $sp, $sp, 96
	jr  $ra

lfib.8:
	j  lfib.5

lfib.5:
	lw  $t0, 20($sp)
	sw  $t0, 44($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 48($sp)
	lw  $t1, 48($sp)
	lw  $t2, 44($sp)
	xor  $t0, $t1, $t2
	sw  $t0, 52($sp)
	lw  $t1, 52($sp)
	sltiu  $t0, $t1, 1
	sw  $t0, 52($sp)
	lw  $t0, 52($sp)
	bne  $t0, $zero, lfib.9
	j  lfib.10

lfib.9:
	lw  $ra, 16($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 56($sp)
	lw  $t0, 56($sp)
	addu  $v0, $zero, $t0
	addiu  $sp, $sp, 96
	jr  $ra

lfib.13:
	j  lfib.10

lfib.10:
	lw  $t0, 20($sp)
	sw  $t0, 60($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 64($sp)
	lw  $t1, 60($sp)
	lw  $t2, 64($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 68($sp)
	lw  $t0, 68($sp)
	sw  $t0, 0($sp)
	jal  lfib
	add  $t0, $zero, $v0
	sw  $t0, 72($sp)
	lw  $t0, 20($sp)
	sw  $t0, 76($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 80($sp)
	lw  $t1, 76($sp)
	lw  $t2, 80($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 84($sp)
	lw  $t0, 84($sp)
	sw  $t0, 0($sp)
	jal  lfib
	add  $t0, $zero, $v0
	sw  $t0, 88($sp)
	lw  $t1, 72($sp)
	lw  $t2, 88($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 92($sp)
	lw  $ra, 16($sp)
	lw  $t0, 92($sp)
	addu  $v0, $zero, $t0
	addiu  $sp, $sp, 96
	jr  $ra


lmain:
lmain.1:
	addiu  $sp, $sp, -1048
	sw  $ra, 16($sp)
	nop  
	addiu  $t0, $zero, 2
	sw  $t0, 40($sp)
	lw  $t0, 40($sp)
	sw  $t0, 20($sp)
	nop  
	addiu  $t0, $zero, 5
	sw  $t0, 44($sp)
	lw  $t0, 44($sp)
	sw  $t0, 24($sp)
	nop  
	addiu  $t0, $zero, 1
	sw  $t0, 48($sp)
	lw  $t0, 48($sp)
	sw  $t0, 28($sp)
	nop  
	addiu  $t0, $zero, 2
	sw  $t0, 52($sp)
	lw  $t0, 52($sp)
	sw  $t0, 32($sp)
	addiu  $v0, $zero, 5
	syscall  
	addu  $t0, $zero, $v0
	sw  $t0, 56($sp)
	lw  $t0, 56($sp)
	sw  $t0, 20($sp)
	addiu  $v0, $zero, 5
	syscall  
	addu  $t0, $zero, $v0
	sw  $t0, 60($sp)
	lw  $t0, 60($sp)
	sw  $t0, 24($sp)
	lw  $t0, 20($sp)
	sw  $t0, 64($sp)
	lw  $t0, 24($sp)
	sw  $t0, 68($sp)
	lw  $t0, 64($sp)
	lw  $t1, 68($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 72($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 76($sp)
	lw  $t1, 76($sp)
	lw  $t2, 72($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 80($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 84($sp)
	lw  $t0, 84($sp)
	sw  $t0, 0($sp)
	jal  lfib
	add  $t0, $zero, $v0
	sw  $t0, 88($sp)
	lw  $t0, 80($sp)
	lw  $t1, 88($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 92($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 96($sp)
	lw  $t1, 92($sp)
	lw  $t2, 96($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 100($sp)
	la  $t0, la
	sw  $t0, 104($sp)
	addiu  $t0, $zero, 40
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
	addiu  $t0, $zero, 1
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
	lw  $t1, 104($sp)
	lw  $t0, 0($t1)
	sw  $t0, 120($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 124($sp)
	lw  $t0, 120($sp)
	lw  $t1, 124($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 128($sp)
	lw  $t1, 100($sp)
	lw  $t2, 128($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 132($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 136($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 140($sp)
	lw  $t0, 136($sp)
	lw  $t1, 140($sp)
	div  $t0, $t1
	mflo  $t0
	sw  $t0, 144($sp)
	lw  $t1, 132($sp)
	lw  $t2, 144($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 148($sp)
	addiu  $t0, $zero, 5
	sw  $t0, 152($sp)
	lw  $t0, 148($sp)
	lw  $t1, 152($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 156($sp)
	lw  $t0, 156($sp)
	sw  $t0, 20($sp)
	addiu  $t0, $zero, 7
	sw  $t0, 160($sp)
	addiu  $t0, $zero, 5923
	sw  $t0, 164($sp)
	lw  $t0, 160($sp)
	lw  $t1, 164($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 168($sp)
	addiu  $t0, $zero, 56
	sw  $t0, 172($sp)
	lw  $t0, 168($sp)
	lw  $t1, 172($sp)
	div  $t0, $t1
	mfhi  $t0
	sw  $t0, 176($sp)
	addiu  $t0, $zero, 57
	sw  $t0, 180($sp)
	lw  $t0, 176($sp)
	lw  $t1, 180($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 184($sp)
	addiu  $t0, $zero, 5
	sw  $t0, 188($sp)
	lw  $t0, 188($sp)
	sw  $t0, 0($sp)
	jal  lfib
	add  $t0, $zero, $v0
	sw  $t0, 192($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 196($sp)
	lw  $t1, 192($sp)
	lw  $t2, 196($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 200($sp)
	lw  $t0, 200($sp)
	sw  $t0, 0($sp)
	jal  lfib
	add  $t0, $zero, $v0
	sw  $t0, 204($sp)
	lw  $t1, 184($sp)
	lw  $t2, 204($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 208($sp)
	lw  $t0, 28($sp)
	sw  $t0, 212($sp)
	lw  $t0, 32($sp)
	sw  $t0, 216($sp)
	lw  $t1, 212($sp)
	lw  $t2, 216($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 220($sp)
	addiu  $t0, $zero, 89
	sw  $t0, 224($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 228($sp)
	lw  $t0, 224($sp)
	lw  $t1, 228($sp)
	div  $t0, $t1
	mflo  $t0
	sw  $t0, 232($sp)
	addiu  $t0, $zero, 36
	sw  $t0, 236($sp)
	lw  $t0, 232($sp)
	lw  $t1, 236($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 240($sp)
	addiu  $t0, $zero, 53
	sw  $t0, 244($sp)
	lw  $t1, 240($sp)
	lw  $t2, 244($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 248($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 252($sp)
	lw  $t0, 248($sp)
	lw  $t1, 252($sp)
	div  $t0, $t1
	mflo  $t0
	sw  $t0, 256($sp)
	addiu  $t0, $zero, 6
	sw  $t0, 260($sp)
	lw  $t0, 256($sp)
	lw  $t1, 260($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 264($sp)
	lw  $t1, 220($sp)
	lw  $t2, 264($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 268($sp)
	addiu  $t0, $zero, 45
	sw  $t0, 272($sp)
	addiu  $t0, $zero, 56
	sw  $t0, 276($sp)
	lw  $t0, 272($sp)
	lw  $t1, 276($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 280($sp)
	addiu  $t0, $zero, 85
	sw  $t0, 284($sp)
	lw  $t0, 280($sp)
	lw  $t1, 284($sp)
	div  $t0, $t1
	mflo  $t0
	sw  $t0, 288($sp)
	addiu  $t0, $zero, 56
	sw  $t0, 292($sp)
	lw  $t1, 288($sp)
	lw  $t2, 292($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 296($sp)
	addiu  $t0, $zero, 35
	sw  $t0, 300($sp)
	addiu  $t0, $zero, 56
	sw  $t0, 304($sp)
	lw  $t0, 300($sp)
	lw  $t1, 304($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 308($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 312($sp)
	lw  $t0, 308($sp)
	lw  $t1, 312($sp)
	div  $t0, $t1
	mflo  $t0
	sw  $t0, 316($sp)
	lw  $t1, 296($sp)
	lw  $t2, 316($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 320($sp)
	addiu  $t0, $zero, 9
	sw  $t0, 324($sp)
	lw  $t1, 320($sp)
	lw  $t2, 324($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 328($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 332($sp)
	lw  $t0, 332($sp)
	lw  $t1, 328($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 336($sp)
	lw  $t1, 268($sp)
	lw  $t2, 336($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 340($sp)
	lw  $t1, 208($sp)
	lw  $t2, 340($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 344($sp)
	lw  $t0, 344($sp)
	sw  $t0, 24($sp)
	nop  
	addiu  $t0, $zero, 0
	sw  $t0, 348($sp)
	addiu  $t0, $zero, 6
	sw  $t0, 352($sp)
	lw  $t1, 348($sp)
	lw  $t2, 352($sp)
	subu  $t0, $t1, $t2
	sw  $t0, 356($sp)
	lw  $t0, 356($sp)
	sw  $t0, 36($sp)
	j  lmain.50

lmain.50:
	lw  $t0, 20($sp)
	sw  $t0, 360($sp)
	addiu  $t0, $zero, 100
	sw  $t0, 364($sp)
	lw  $t1, 364($sp)
	lw  $t2, 360($sp)
	slt  $t0, $t1, $t2
	sw  $t0, 368($sp)
	lw  $t1, 368($sp)
	sltiu  $t0, $t1, 1
	sw  $t0, 368($sp)
	lw  $t0, 368($sp)
	bne  $t0, $zero, lmain.52
	j  lmain.51

lmain.52:
	la  $t0, la
	sw  $t0, 372($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 376($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 380($sp)
	lw  $t0, 376($sp)
	lw  $t1, 380($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 376($sp)
	lw  $t1, 372($sp)
	lw  $t2, 376($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 372($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 376($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 384($sp)
	lw  $t0, 376($sp)
	lw  $t1, 384($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 376($sp)
	lw  $t1, 372($sp)
	lw  $t2, 376($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 372($sp)
	lw  $t1, 372($sp)
	lw  $t0, 0($t1)
	sw  $t0, 388($sp)
	lw  $t0, 36($sp)
	sw  $t0, 392($sp)
	lw  $t0, 36($sp)
	sw  $t0, 396($sp)
	lw  $t0, 392($sp)
	lw  $t1, 396($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 400($sp)
	lw  $t1, 388($sp)
	lw  $t2, 400($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 404($sp)
	la  $t0, la
	sw  $t0, 408($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 412($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 416($sp)
	lw  $t0, 412($sp)
	lw  $t1, 416($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 412($sp)
	lw  $t1, 408($sp)
	lw  $t2, 412($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 408($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 412($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 420($sp)
	lw  $t0, 412($sp)
	lw  $t1, 420($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 412($sp)
	lw  $t1, 408($sp)
	lw  $t2, 412($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 408($sp)
	lw  $t0, 404($sp)
	lw  $t1, 408($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 424($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 428($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 432($sp)
	lw  $t0, 428($sp)
	lw  $t1, 432($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 428($sp)
	lw  $t1, 424($sp)
	lw  $t2, 428($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 424($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 428($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 436($sp)
	lw  $t0, 428($sp)
	lw  $t1, 436($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 428($sp)
	lw  $t1, 424($sp)
	lw  $t2, 428($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 424($sp)
	lw  $t1, 424($sp)
	lw  $t0, 0($t1)
	sw  $t0, 440($sp)
	lw  $t0, 36($sp)
	sw  $t0, 444($sp)
	lw  $t0, 36($sp)
	sw  $t0, 448($sp)
	lw  $t0, 444($sp)
	lw  $t1, 448($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 452($sp)
	lw  $t1, 440($sp)
	lw  $t2, 452($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 456($sp)
	la  $t0, la
	sw  $t0, 460($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 464($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 468($sp)
	lw  $t0, 464($sp)
	lw  $t1, 468($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 464($sp)
	lw  $t1, 460($sp)
	lw  $t2, 464($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 460($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 464($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 472($sp)
	lw  $t0, 464($sp)
	lw  $t1, 472($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 464($sp)
	lw  $t1, 460($sp)
	lw  $t2, 464($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 460($sp)
	lw  $t0, 456($sp)
	lw  $t1, 460($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 476($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 480($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 484($sp)
	lw  $t0, 480($sp)
	lw  $t1, 484($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 480($sp)
	lw  $t1, 476($sp)
	lw  $t2, 480($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 476($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 480($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 488($sp)
	lw  $t0, 480($sp)
	lw  $t1, 488($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 480($sp)
	lw  $t1, 476($sp)
	lw  $t2, 480($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 476($sp)
	lw  $t1, 476($sp)
	lw  $t0, 0($t1)
	sw  $t0, 492($sp)
	lw  $t0, 36($sp)
	sw  $t0, 496($sp)
	lw  $t0, 36($sp)
	sw  $t0, 500($sp)
	lw  $t0, 496($sp)
	lw  $t1, 500($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 504($sp)
	lw  $t1, 492($sp)
	lw  $t2, 504($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 508($sp)
	la  $t0, la
	sw  $t0, 512($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 516($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 520($sp)
	lw  $t0, 516($sp)
	lw  $t1, 520($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 516($sp)
	lw  $t1, 512($sp)
	lw  $t2, 516($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 512($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 516($sp)
	addiu  $t0, $zero, 2
	sw  $t0, 524($sp)
	lw  $t0, 516($sp)
	lw  $t1, 524($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 516($sp)
	lw  $t1, 512($sp)
	lw  $t2, 516($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 512($sp)
	lw  $t0, 508($sp)
	lw  $t1, 512($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 528($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 532($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 536($sp)
	lw  $t0, 532($sp)
	lw  $t1, 536($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 532($sp)
	lw  $t1, 528($sp)
	lw  $t2, 532($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 528($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 532($sp)
	addiu  $t0, $zero, 3
	sw  $t0, 540($sp)
	lw  $t0, 532($sp)
	lw  $t1, 540($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 532($sp)
	lw  $t1, 528($sp)
	lw  $t2, 532($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 528($sp)
	lw  $t1, 528($sp)
	lw  $t0, 0($t1)
	sw  $t0, 544($sp)
	lw  $t0, 36($sp)
	sw  $t0, 548($sp)
	lw  $t0, 36($sp)
	sw  $t0, 552($sp)
	lw  $t0, 548($sp)
	lw  $t1, 552($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 556($sp)
	lw  $t1, 544($sp)
	lw  $t2, 556($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 560($sp)
	la  $t0, la
	sw  $t0, 564($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 568($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 572($sp)
	lw  $t0, 568($sp)
	lw  $t1, 572($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 568($sp)
	lw  $t1, 564($sp)
	lw  $t2, 568($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 564($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 568($sp)
	addiu  $t0, $zero, 3
	sw  $t0, 576($sp)
	lw  $t0, 568($sp)
	lw  $t1, 576($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 568($sp)
	lw  $t1, 564($sp)
	lw  $t2, 568($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 564($sp)
	lw  $t0, 560($sp)
	lw  $t1, 564($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 580($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 584($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 588($sp)
	lw  $t0, 584($sp)
	lw  $t1, 588($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 584($sp)
	lw  $t1, 580($sp)
	lw  $t2, 584($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 580($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 584($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 592($sp)
	lw  $t0, 584($sp)
	lw  $t1, 592($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 584($sp)
	lw  $t1, 580($sp)
	lw  $t2, 584($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 580($sp)
	lw  $t1, 580($sp)
	lw  $t0, 0($t1)
	sw  $t0, 596($sp)
	lw  $t0, 36($sp)
	sw  $t0, 600($sp)
	lw  $t0, 36($sp)
	sw  $t0, 604($sp)
	lw  $t0, 600($sp)
	lw  $t1, 604($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 608($sp)
	lw  $t1, 596($sp)
	lw  $t2, 608($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 612($sp)
	la  $t0, la
	sw  $t0, 616($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 620($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 624($sp)
	lw  $t0, 620($sp)
	lw  $t1, 624($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 620($sp)
	lw  $t1, 616($sp)
	lw  $t2, 620($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 616($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 620($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 628($sp)
	lw  $t0, 620($sp)
	lw  $t1, 628($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 620($sp)
	lw  $t1, 616($sp)
	lw  $t2, 620($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 616($sp)
	lw  $t0, 612($sp)
	lw  $t1, 616($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 632($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 636($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 640($sp)
	lw  $t0, 636($sp)
	lw  $t1, 640($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 636($sp)
	lw  $t1, 632($sp)
	lw  $t2, 636($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 632($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 636($sp)
	addiu  $t0, $zero, 5
	sw  $t0, 644($sp)
	lw  $t0, 636($sp)
	lw  $t1, 644($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 636($sp)
	lw  $t1, 632($sp)
	lw  $t2, 636($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 632($sp)
	lw  $t1, 632($sp)
	lw  $t0, 0($t1)
	sw  $t0, 648($sp)
	lw  $t0, 36($sp)
	sw  $t0, 652($sp)
	lw  $t0, 36($sp)
	sw  $t0, 656($sp)
	lw  $t0, 652($sp)
	lw  $t1, 656($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 660($sp)
	lw  $t1, 648($sp)
	lw  $t2, 660($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 664($sp)
	la  $t0, la
	sw  $t0, 668($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 672($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 676($sp)
	lw  $t0, 672($sp)
	lw  $t1, 676($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 672($sp)
	lw  $t1, 668($sp)
	lw  $t2, 672($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 668($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 672($sp)
	addiu  $t0, $zero, 5
	sw  $t0, 680($sp)
	lw  $t0, 672($sp)
	lw  $t1, 680($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 672($sp)
	lw  $t1, 668($sp)
	lw  $t2, 672($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 668($sp)
	lw  $t0, 664($sp)
	lw  $t1, 668($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 684($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 688($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 692($sp)
	lw  $t0, 688($sp)
	lw  $t1, 692($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 688($sp)
	lw  $t1, 684($sp)
	lw  $t2, 688($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 684($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 688($sp)
	addiu  $t0, $zero, 6
	sw  $t0, 696($sp)
	lw  $t0, 688($sp)
	lw  $t1, 696($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 688($sp)
	lw  $t1, 684($sp)
	lw  $t2, 688($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 684($sp)
	lw  $t1, 684($sp)
	lw  $t0, 0($t1)
	sw  $t0, 700($sp)
	lw  $t0, 36($sp)
	sw  $t0, 704($sp)
	lw  $t0, 36($sp)
	sw  $t0, 708($sp)
	lw  $t0, 704($sp)
	lw  $t1, 708($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 712($sp)
	lw  $t1, 700($sp)
	lw  $t2, 712($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 716($sp)
	la  $t0, la
	sw  $t0, 720($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 724($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 728($sp)
	lw  $t0, 724($sp)
	lw  $t1, 728($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 724($sp)
	lw  $t1, 720($sp)
	lw  $t2, 724($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 720($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 724($sp)
	addiu  $t0, $zero, 6
	sw  $t0, 732($sp)
	lw  $t0, 724($sp)
	lw  $t1, 732($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 724($sp)
	lw  $t1, 720($sp)
	lw  $t2, 724($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 720($sp)
	lw  $t0, 716($sp)
	lw  $t1, 720($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 736($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 740($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 744($sp)
	lw  $t0, 740($sp)
	lw  $t1, 744($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 740($sp)
	lw  $t1, 736($sp)
	lw  $t2, 740($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 736($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 740($sp)
	addiu  $t0, $zero, 7
	sw  $t0, 748($sp)
	lw  $t0, 740($sp)
	lw  $t1, 748($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 740($sp)
	lw  $t1, 736($sp)
	lw  $t2, 740($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 736($sp)
	lw  $t1, 736($sp)
	lw  $t0, 0($t1)
	sw  $t0, 752($sp)
	lw  $t0, 36($sp)
	sw  $t0, 756($sp)
	lw  $t0, 36($sp)
	sw  $t0, 760($sp)
	lw  $t0, 756($sp)
	lw  $t1, 760($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 764($sp)
	lw  $t1, 752($sp)
	lw  $t2, 764($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 768($sp)
	la  $t0, la
	sw  $t0, 772($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 776($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 780($sp)
	lw  $t0, 776($sp)
	lw  $t1, 780($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 776($sp)
	lw  $t1, 772($sp)
	lw  $t2, 776($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 772($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 776($sp)
	addiu  $t0, $zero, 7
	sw  $t0, 784($sp)
	lw  $t0, 776($sp)
	lw  $t1, 784($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 776($sp)
	lw  $t1, 772($sp)
	lw  $t2, 776($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 772($sp)
	lw  $t0, 768($sp)
	lw  $t1, 772($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 788($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 792($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 796($sp)
	lw  $t0, 792($sp)
	lw  $t1, 796($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 792($sp)
	lw  $t1, 788($sp)
	lw  $t2, 792($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 788($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 792($sp)
	addiu  $t0, $zero, 8
	sw  $t0, 800($sp)
	lw  $t0, 792($sp)
	lw  $t1, 800($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 792($sp)
	lw  $t1, 788($sp)
	lw  $t2, 792($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 788($sp)
	lw  $t1, 788($sp)
	lw  $t0, 0($t1)
	sw  $t0, 804($sp)
	lw  $t0, 36($sp)
	sw  $t0, 808($sp)
	lw  $t0, 36($sp)
	sw  $t0, 812($sp)
	lw  $t0, 808($sp)
	lw  $t1, 812($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 816($sp)
	lw  $t1, 804($sp)
	lw  $t2, 816($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 820($sp)
	la  $t0, la
	sw  $t0, 824($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 828($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 832($sp)
	lw  $t0, 828($sp)
	lw  $t1, 832($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 828($sp)
	lw  $t1, 824($sp)
	lw  $t2, 828($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 824($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 828($sp)
	addiu  $t0, $zero, 8
	sw  $t0, 836($sp)
	lw  $t0, 828($sp)
	lw  $t1, 836($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 828($sp)
	lw  $t1, 824($sp)
	lw  $t2, 828($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 824($sp)
	lw  $t0, 820($sp)
	lw  $t1, 824($sp)
	sw  $t0, 0($t1)
	la  $t0, la
	sw  $t0, 840($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 844($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 848($sp)
	lw  $t0, 844($sp)
	lw  $t1, 848($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 844($sp)
	lw  $t1, 840($sp)
	lw  $t2, 844($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 840($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 844($sp)
	addiu  $t0, $zero, 9
	sw  $t0, 852($sp)
	lw  $t0, 844($sp)
	lw  $t1, 852($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 844($sp)
	lw  $t1, 840($sp)
	lw  $t2, 844($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 840($sp)
	lw  $t1, 840($sp)
	lw  $t0, 0($t1)
	sw  $t0, 856($sp)
	lw  $t0, 36($sp)
	sw  $t0, 860($sp)
	lw  $t0, 36($sp)
	sw  $t0, 864($sp)
	lw  $t0, 860($sp)
	lw  $t1, 864($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 868($sp)
	lw  $t1, 856($sp)
	lw  $t2, 868($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 872($sp)
	la  $t0, la
	sw  $t0, 876($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 880($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 884($sp)
	lw  $t0, 880($sp)
	lw  $t1, 884($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 880($sp)
	lw  $t1, 876($sp)
	lw  $t2, 880($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 876($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 880($sp)
	addiu  $t0, $zero, 9
	sw  $t0, 888($sp)
	lw  $t0, 880($sp)
	lw  $t1, 888($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 880($sp)
	lw  $t1, 876($sp)
	lw  $t2, 880($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 876($sp)
	lw  $t0, 872($sp)
	lw  $t1, 876($sp)
	sw  $t0, 0($t1)
	lw  $t0, 20($sp)
	sw  $t0, 892($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 896($sp)
	lw  $t1, 892($sp)
	lw  $t2, 896($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 900($sp)
	lw  $t0, 900($sp)
	sw  $t0, 20($sp)
	j  lmain.50

lmain.51:
	addiu  $t0, $zero, 0
	sw  $t0, 904($sp)
	lw  $t0, 904($sp)
	sw  $t0, 20($sp)
	j  lmain.127

lmain.127:
	lw  $t0, 20($sp)
	sw  $t0, 908($sp)
	addiu  $t0, $zero, 10
	sw  $t0, 912($sp)
	lw  $t1, 908($sp)
	lw  $t2, 912($sp)
	slt  $t0, $t1, $t2
	sw  $t0, 916($sp)
	lw  $t0, 916($sp)
	bne  $t0, $zero, lmain.129
	j  lmain.128

lmain.129:
	lw  $t0, 20($sp)
	sw  $t0, 920($sp)
	la  $t0, la
	sw  $t0, 924($sp)
	addiu  $t0, $zero, 40
	sw  $t0, 928($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 932($sp)
	lw  $t0, 928($sp)
	lw  $t1, 932($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 928($sp)
	lw  $t1, 924($sp)
	lw  $t2, 928($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 924($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 928($sp)
	lw  $t0, 928($sp)
	lw  $t1, 920($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 928($sp)
	lw  $t1, 924($sp)
	lw  $t2, 928($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 924($sp)
	lw  $t1, 924($sp)
	lw  $t0, 0($t1)
	sw  $t0, 936($sp)
	addiu  $v0, $zero, 1
	lw  $t0, 936($sp)
	addu  $a0, $zero, $t0
	syscall  
	la  $t0, l.const_str0
	sw  $t0, 940($sp)
	addiu  $t0, $zero, 12
	sw  $t0, 944($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 948($sp)
	lw  $t0, 944($sp)
	lw  $t1, 948($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 944($sp)
	lw  $t1, 940($sp)
	lw  $t2, 944($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 940($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 944($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 952($sp)
	lw  $t0, 944($sp)
	lw  $t1, 952($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 944($sp)
	lw  $t1, 940($sp)
	lw  $t2, 944($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 940($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 940($sp)
	addu  $a0, $zero, $t0
	syscall  
	lw  $t0, 20($sp)
	sw  $t0, 956($sp)
	addiu  $t0, $zero, 1
	sw  $t0, 960($sp)
	lw  $t1, 956($sp)
	lw  $t2, 960($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 964($sp)
	lw  $t0, 964($sp)
	sw  $t0, 20($sp)
	j  lmain.127

lmain.128:
	lw  $t0, 20($sp)
	sw  $t0, 968($sp)
	lw  $t0, 24($sp)
	sw  $t0, 972($sp)
	lw  $t0, 36($sp)
	sw  $t0, 976($sp)
	la  $t0, l.const_str1
	sw  $t0, 980($sp)
	addiu  $t0, $zero, 8
	sw  $t0, 984($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 988($sp)
	lw  $t0, 984($sp)
	lw  $t1, 988($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 984($sp)
	lw  $t1, 980($sp)
	lw  $t2, 984($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 980($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 984($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 992($sp)
	lw  $t0, 984($sp)
	lw  $t1, 992($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 984($sp)
	lw  $t1, 980($sp)
	lw  $t2, 984($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 980($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 980($sp)
	addu  $a0, $zero, $t0
	syscall  
	addiu  $v0, $zero, 1
	lw  $t0, 968($sp)
	addu  $a0, $zero, $t0
	syscall  
	la  $t0, l.const_str0
	sw  $t0, 996($sp)
	addiu  $t0, $zero, 12
	sw  $t0, 1000($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 1004($sp)
	lw  $t0, 1000($sp)
	lw  $t1, 1004($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 1000($sp)
	lw  $t1, 996($sp)
	lw  $t2, 1000($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 996($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 1000($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 1008($sp)
	lw  $t0, 1000($sp)
	lw  $t1, 1008($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 1000($sp)
	lw  $t1, 996($sp)
	lw  $t2, 1000($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 996($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 996($sp)
	addu  $a0, $zero, $t0
	syscall  
	addiu  $v0, $zero, 1
	lw  $t0, 972($sp)
	addu  $a0, $zero, $t0
	syscall  
	la  $t0, l.const_str0
	sw  $t0, 1012($sp)
	addiu  $t0, $zero, 12
	sw  $t0, 1016($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 1020($sp)
	lw  $t0, 1016($sp)
	lw  $t1, 1020($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 1016($sp)
	lw  $t1, 1012($sp)
	lw  $t2, 1016($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 1012($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 1016($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 1024($sp)
	lw  $t0, 1016($sp)
	lw  $t1, 1024($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 1016($sp)
	lw  $t1, 1012($sp)
	lw  $t2, 1016($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 1012($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 1012($sp)
	addu  $a0, $zero, $t0
	syscall  
	addiu  $v0, $zero, 1
	lw  $t0, 976($sp)
	addu  $a0, $zero, $t0
	syscall  
	la  $t0, l.const_str1
	sw  $t0, 1028($sp)
	addiu  $t0, $zero, 8
	sw  $t0, 1032($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 1036($sp)
	lw  $t0, 1032($sp)
	lw  $t1, 1036($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 1032($sp)
	lw  $t1, 1028($sp)
	lw  $t2, 1032($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 1028($sp)
	addiu  $t0, $zero, 4
	sw  $t0, 1032($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 1040($sp)
	lw  $t0, 1032($sp)
	lw  $t1, 1040($sp)
	mult  $t0, $t1
	mflo  $t0
	sw  $t0, 1032($sp)
	lw  $t1, 1028($sp)
	lw  $t2, 1032($sp)
	addu  $t0, $t1, $t2
	sw  $t0, 1028($sp)
	addiu  $v0, $zero, 4
	lw  $t0, 1028($sp)
	addu  $a0, $zero, $t0
	syscall  
	lw  $ra, 16($sp)
	addiu  $t0, $zero, 0
	sw  $t0, 1044($sp)
	lw  $t0, 1044($sp)
	addu  $v0, $zero, $t0
	addiu  $sp, $sp, 1048
	jr  $ra


gsc:
