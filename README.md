# tinyC : C Compiler implementation

A C Compiler implementation as shown in (https://github.com/DoctorWkt/acwj)

## Building

make command : 
```
make tinyC
```
## Usage
### Options :
```
-v give verbose output of the compilation stages
-c generate object files but don't link them
-S generate assembly files but don't link them
-T dump the AST trees for each input file
-M dump the symbol table for each input file
-o outfile, produce the outfile executable file
```
### Example : 
Simple C program :
```c
//file.c
#include<stdio.h>
void main()
{
        int a;
        a = 10;
        a += 10;
        printf("%d\n",a);
}
```
To execute above code :
```
tinyC file.c
```
Output : 
```
: ./a.out
20
```
Now let's execute using option : 
```
tinyC -S file.c
```
Generated assembly file using -S option which is file.s below :
```assembly
//file.s
# freeing all registers
	.text
# internal switch(expr) routine
# %rsi = switch table, %rax = expr
# from SubC: http://www.t3x.org/subc/

__switch:
        pushq   %rsi
        movq    %rdx, %rsi
        movq    %rax, %rbx
        cld
        lodsq
        movq    %rax, %rcx
__next:
        lodsq
        movq    %rax, %rdx
        lodsq
        cmpq    %rdx, %rbx
        jnz     __no
        popq    %rsi
        jmp     *%rax
__no:
        loop    __next
        lodsq
        popq    %rsi
        jmp     *%rax

L18:
	.byte	37
	.byte	100
	.byte	10
	.byte	0
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	addq	$-16,%rsp
# allocated register %r10
	movq	$10, %r10
	movl	%r10d, -4(%rbp)
# freeing all registers
# allocated register %r10
	movslq	-4(%rbp), %r10
# allocated register %r11
	movq	$10, %r11
	addq	%r11, %r10
# freeing reg %r11
	movl	%r10d, -4(%rbp)
# freeing all registers
# freeing all registers
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
# allocated register %r10
	movslq	-4(%rbp), %r10
	movq	%r10, %rsi
# freeing reg %r10
# allocated register %r10
	leaq	L18(%rip), %r10
	movq	%r10, %rdi
# freeing reg %r10
	call	printf@PLT
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
# allocated register %r10
	movq	%rax, %r10
# freeing all registers
L17:
	addq	$16,%rsp
	popq	%rbp
	ret
# freeing all registers
```
