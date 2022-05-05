	.file	1 "fork.c"
gcc2_compiled.:
__gnu_compiled_c:
	.text
	.align	2
	.globl	func
	.ent	func
func:
	.frame	$fp,32,$31		# vars= 8, regs= 2/0, args= 16, extra= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sw	$31,28($sp)
	sw	$fp,24($sp)
	move	$fp,$sp
	li	$2,123			# 0x0000007b
	sw	$2,16($fp)
	li	$2,456			# 0x000001c8
	sw	$2,20($fp)
	move	$4,$0
	jal	Exit
$L1:
	move	$sp,$fp
	lw	$31,28($sp)
	lw	$fp,24($sp)
	addu	$sp,$sp,32
	j	$31
	.end	func
	.align	2
	.globl	main
	.ent	main
main:
	.frame	$fp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,20($sp)
	sw	$fp,16($sp)
	move	$fp,$sp
	jal	__main
	la	$4,func
	jal	Fork
	li	$4,100			# 0x00000064
	jal	Exit
$L2:
	move	$sp,$fp
	lw	$31,20($sp)
	lw	$fp,16($sp)
	addu	$sp,$sp,24
	j	$31
	.end	main
