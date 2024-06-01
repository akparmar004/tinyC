	.text
	.data
	.globl	i
i:	.long	0
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$1, %r8
	movl	%r8d, i(%rip)
L2:
	movzbl	i(%rip), %r8
	movq	$10, %r9
	cmpq	%r9, %r8
	jg	L3
	movzbl	i(%rip), %r8
	movq	%r8, %rdi
	call	printint
	movq	%rax, %r9
	movzbl	i(%rip), %r8
	movq	$1, %r9
	addq	%r8, %r9
	movl	%r9d, i(%rip)
	jmp	L2
L3:
L1:
	popq	%rbp
	ret
	.data
	.globl	a
a:	.long	0
	.data
	.globl	b
b:	.long	0
	.text
	.globl	fred
	.type	fred, @function
fred:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$12, %r8
	movl	%r8d, a(%rip)
	movq	$3, %r8
	movzbl	a(%rip), %r9
	imulq	%r8, %r9
	movl	%r9d, b(%rip)
	movzbl	a(%rip), %r8
	movzbl	b(%rip), %r9
	cmpq	%r9, %r8
	jl	L5
	movq	$2, %r8
	movzbl	b(%rip), %r9
	imulq	%r8, %r9
	movzbl	a(%rip), %r8
	subq	%r8, %r9
	movq	%r9, %rdi
	call	printint
	movq	%rax, %r8
L5:
L4:
	popq	%rbp
	ret
