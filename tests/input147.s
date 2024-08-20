.data
.balign 8
L19:
	.byte 37
	.byte 100
	.byte 10
	.byte 0
/* end data */

.data
.balign 8
L20:
	.byte 37
	.byte 100
	.byte 10
	.byte 0
/* end data */

.text
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $0, %esi
	leaq L19(%rip), %rdi
	callq printf
	movl $98, %esi
	leaq L20(%rip), %rdi
	callq printf
	leave
	ret
.type main, @function
.size main, .-main
/* end function main */

.section .note.GNU-stack,"",@progbits
