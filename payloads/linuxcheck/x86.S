.code32
.globl flags
flags:
	pushfl
	pop %eax
	ret
