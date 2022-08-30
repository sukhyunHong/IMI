#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#define _4096 __attribute__ ((aligned(0x1000)))

#define __NR_iso_create_domain 436
#define __NR_iso_assign_memory 437
#define __NR_iso_init 438
#define __NR_iso_flush_tlb_all 439

asm ( ".align 12\n\t");
int _4096 glob_a; 

asm ( ".align 12\n\t");
void increase_glob_a(void)
{
	glob_a++;
}

asm (".align 12\r\n");
void void_fn(void)
{
	return;
}

asm (".align 12\r\n");
void cxg_dom(int dom_num, void (*fn_addr)(void), unsigned long tmp)
{
	// for save link register, frame pointer register, need call function.
	// So call empty function.
	void_fn();
	asm volatile ("svc #1\r\n" // svc1 : change domain to dom_num
			"blr x1\r\n"
			"svc #2\r\n"); // svc2 : change domain dom_num to main domain

	// At address 0xFFFF FDFF FE5F A880, real change domain function is there...
	/*
	asm volatile (	"mov	x3, #0xA880\r\n"
			"movk	x3, #0xFE5F, lsl #16\r\n"
			"movk	x3, #0xFDFF, lsl #32\r\n"
			"movk	x3, #0xFFFF, lsl #48\r\n"
			"blr	x3\r\n");
	*/
}



asm ( ".align 12\n\t");
int main(void)
{
	int ret;
	int dom_num = 1;
	void (*fn_addr)(void) = increase_glob_a;
	uint64_t *new_ttbr0;
	unsigned long *stack_addr;
	
	ret = syscall(__NR_iso_init);
	if(ret == 0) {
		printf("iso initialize failed! exit process\n");
		return 0;
	}

	ret = syscall(__NR_iso_create_domain, &new_ttbr0, &stack_addr);
	if(ret == -1) {
		printf("iso create domain failed! exit process\n");
		return 0;
	}

	ret = syscall(__NR_iso_assign_memory, 1, (char*)&glob_a - 4096*3, 4096); // got 위치 강제 매핑.
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}
	syscall(__NR_iso_assign_memory, 1, &glob_a, 4096);	// global variable
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}
	syscall(__NR_iso_assign_memory, 1, increase_glob_a, 4096*3);	// functions...
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}

	printf("assign memory finished\n");

	// temporary, this function works print all vmas of domain 1
	syscall(__NR_iso_flush_tlb_all);
	
	cxg_dom(dom_num, fn_addr, 0);

	printf("glob_a: %d\n", glob_a);


	return 0;
}
asm ( ".align 12\n\t");
