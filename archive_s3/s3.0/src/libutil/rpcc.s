	.ugen	
	.verstamp	3 11
	.text	
	.align	4
	.file	2 "rpcc.c"
	.globl	rpcc
	.loc	2 2
 #
 #	works only on the alphas
 # 
 #    1	unsigned int rpcc ( void )
 #    2	{
	.ent	rpcc 2
rpcc:
	.option	O1
	ldgp	$gp, 0($27)
	lda	$sp, -32($sp)
	.frame	$sp, 32, $26, 0
	.prologue	1
	.loc	2 2

	.loc	2 6
 #    3	    unsigned long x, y;
 #    4	    unsigned int z;
 #    5	    
 #    6	    y = x;
	rpcc	$0
	sll	$0, 32, $1
	addq	$0, $1, $0
	srl	$0, 32, $0
	.livereg	0xFC7F0002,0x3FC00000
	lda	$sp, 32($sp)
	ret	$31, ($26), 1
	.end	rpcc
