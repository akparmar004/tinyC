	.text
.LC0:
	.string	"%d\n"
printint:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	-4(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	nop
	leave
	ret

	.comm	i,8,8
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$0, %r8
	movq	%r8, i(%rip)
L1:
	movq	i(%rip), %r8
	movq	$10, %r9
	cmpq	%r9, %r8
	jge	L2
	movq	i(%rip), %r8
	movq	%r8, %rdi
	call	printint
	movq	i(%rip), %r8
	movq	$1, %r9
	addq	%r8, %r9
	movq	%r9, i(%rip)
	jmp	L1
L2:
	movl	$0, %eax
	popq	%rbp
	ret
