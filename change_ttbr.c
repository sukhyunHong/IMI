#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#define _4096 __attribute__ ((aligned(0x1000)))

#define __NR_iso_create_domain 436
#define __NR_iso_assign_memory 437
#define __NR_iso_init 438
#define __NR_iso_flush_tlb_all 439

// macro for flush_tlb_all
#define pgd_t unsigned long
#define atomic64_t unsigned long

typedef struct {
	unsigned long *pgd;
	unsigned long sp_val;
	unsigned long asid;
} dom_data;

/* 분명 inline인데 왜 jump를 뛰지?...
static inline void flush_tlb_all(void)
{
	asm volatile("dsb ishst\r\n"
			"tlbi vmalle1is\r\n"
			"nop\r\n"
			"nop\r\n"
			"dsb ish\r\n"
			"isb\r\n");
}
*/

asm ( ".align 12\n\t");
int _4096 glob_a; 

asm ( ".align 12\n\t");
void tramp_to_main(int cur_dom_num, int new_dom_num, unsigned long meta_addr,
		void (*fn_addr)(void))
{
	pgd_t *ttbr0_el1 = ((dom_data*)meta_addr)[new_dom_num].pgd;
	unsigned long new_sp = ((dom_data*)meta_addr)[new_dom_num].sp_val;
	//atomic64_t new_asid = ASID(((mm_struct*)meta_addr));
	atomic64_t new_asid;
	unsigned long *old_sp_addr;

	/*
	asm volatile("mov x1, sp\r\n"
			"str x1, [%0]\r\n"
			:
			:"r" (old_sp_addr));
	*/

	//new_sp -= 0x70;
	//*(unsigned long*)new_sp = 0;
	asm volatile ("msr ttbr0_el1, %0\r\n"
                    "mov sp, %1\r\n"
		    //"str %3, [sp, #88]\r\n" // 새로운 stack에 cxg_dom에서의 fn_addr 저장.
		    //"str %4, [sp, #96]\r\n" // 새로운 stack에 cxg_dom에서의 meta_addr 저장.
		    //"str %5, [sp, #108]\r\n" // 새로운 stack에 cxg_dom에서의 cur_dom_num위치에 바뀐 dom_num 저장.
		    //"mrs x4, ttbr1_el1\r\n"
		    //"bfi x4, %2, #48, #16\r\n"
		    //"msr ttbr1_el1, x4\r\n"
		    //"isb"
                    :
                    :"r" (ttbr0_el1), "r" (new_sp), "r" (new_asid), "r" (fn_addr), "r" (meta_addr), "r" (cur_dom_num));

	asm volatile("dsb ishst\r\n"
			"tlbi vmalle1is\r\n"
			"nop\r\n"
			"nop\r\n"
			"dsb ish\r\n"
			"isb\r\n");

	//flush_tlb_all();

	return;
}


asm ( ".align 12\n\t");
void tramp(int cur_dom_num, int new_dom_num, unsigned long meta_addr,
		void (*fn_addr)(void))
{
	pgd_t *ttbr0_el1 = ((dom_data*)meta_addr)[new_dom_num].pgd;
	unsigned long new_sp = ((dom_data*)meta_addr)[new_dom_num].sp_val;
	//atomic64_t new_asid = ASID(((mm_struct*)meta_addr));
	atomic64_t new_asid = ((dom_data*)meta_addr)[new_dom_num].asid;
	unsigned long *old_sp_addr = &(((dom_data*)meta_addr)[cur_dom_num].sp_val);

	asm volatile("mov x1, sp\r\n"
			"str x1, [%0]\r\n"
			:
			:"r" (old_sp_addr));

	new_sp -= 0x70;
	*(unsigned long*)new_sp = 0;
	asm volatile ("msr ttbr0_el1, %0\r\n"
                    "mov sp, %1\r\n"
		    "str %3, [sp, #88]\r\n" // 새로운 stack에 cxg_dom에서의 fn_addr 저장.
		    "str %4, [sp, #96]\r\n" // 새로운 stack에 cxg_dom에서의 meta_addr 저장.
		    "str %w5, [sp, #108]\r\n" // 새로운 stack에 cxg_dom에서의 cur_dom_num위치에 바뀐 dom_num 저장.
		    //"mrs x4, ttbr1_el1\r\n"
		    //"bfi x4, %2, #48, #16\r\n"
		    //"msr ttbr1_el1, x4\r\n"
		    //"isb"
                    :
                    :"r" (ttbr0_el1), "r" (new_sp), "r" (new_asid), "r" (fn_addr), "r" (meta_addr), "r" (cur_dom_num));

	asm volatile("dsb ishst\r\n"
			"tlbi vmalle1is\r\n"
			"nop\r\n"
			"nop\r\n"
			"dsb ish\r\n"
			"isb\r\n");

	//flush_tlb_all();

	return;
}

asm ( ".align 12\n\t");
void increase_glob_a(void)
{
	glob_a++;
}
asm ( ".align 12\n\t");
void cxg_dom(int dom_num, unsigned long meta_addr,
		void (*fn_addr)(void))
{
	tramp(0, dom_num, meta_addr, fn_addr);
	//(fn_addr)();
	//syscall(__NR_iso_flush_tlb_all);
	increase_glob_a();
	tramp_to_main(dom_num, 0, meta_addr, fn_addr);
	//syscall(__NR_iso_flush_tlb_all);
	return;
}




asm ( ".align 12\n\t");
int main(void)
{
	int dom_num = 1;
	unsigned long meta_addr;
	//unsigned long fn_addr = (unsigned long)increase_glob_a;
	void (*fn_addr)(void) = increase_glob_a;
	unsigned long *new_ttbr0;
	unsigned long stack_addr;
	volatile unsigned int tmp;

	glob_a = 0;
	increase_glob_a();
	printf("glob_a: %d\n", glob_a);
	printf("glob_a addr: %p\n", &glob_a);


	printf("tramp_1st_addr: %p\n", tramp);
	printf("cxg_dom_1st_addr: %p\n", cxg_dom);
	printf("inc_glob_a_1st_addr: %p\n", increase_glob_a);

	meta_addr = syscall(__NR_iso_init);

	asm volatile("nop\r\n"
			"nop\r\n"
			"nop\r\n");

	tmp = *(int*)tramp;
	tmp = *(int*)cxg_dom;
	tmp = *(int*)increase_glob_a;
	
	//return 0;

	syscall(__NR_iso_create_domain, &new_ttbr0, &stack_addr);
	syscall(__NR_iso_assign_memory, 1, (char*)&glob_a - 4096*3, 4096); // got 위치 강제 매핑.
	syscall(__NR_iso_assign_memory, 1, &glob_a, 4096);

	//syscall(__NR_iso_assign_memory, 1, tramp, 4096*3);
	syscall(__NR_iso_assign_memory, 1, meta_addr, 4096);
	syscall(__NR_iso_assign_memory, 1, tramp_to_main, 4096);
	syscall(__NR_iso_assign_memory, 1, tramp, 4096);
	syscall(__NR_iso_assign_memory, 1, cxg_dom, 4096);
	syscall(__NR_iso_assign_memory, 1, increase_glob_a, 4096);
	//syscall(__NR_iso_flush_tlb_all);
	

	asm volatile("dsb ishst\r\n"
			"tlbi vmalle1is\r\n"
			"nop\r\n"
			"nop\r\n"
			"dsb ish\r\n"
			"isb\r\n");


	cxg_dom(dom_num, meta_addr, fn_addr);

	printf("glob_a: %d\n", glob_a);


	return 0;
}
asm ( ".align 12\n\t");
