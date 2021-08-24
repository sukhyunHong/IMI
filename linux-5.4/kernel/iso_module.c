#include <linux/syscalls.h>

#include <asm/pgalloc.h>
#include <asm/memory.h>
#include <asm/pgalloc.h>
#include <asm/tlbflush.h>

/*
 *
 */
/*
SYSCALL_DEFINE1(iso_create_domain, uint64_t **, ttbr0)
{
    
    int ret = make_domain_test();

    *ttbr0 = (uint64_t*)current->domain_mm->pgd;

    return 0;
}
*/


/*
.pushsection ".entry.cxgdom.text", "ax"
void _change_domain(int dom_num, unsigned long meta_addr, 
                    unsigned long fn_addr)
{
    // current로 접근해선 안됨. 이건 EL1의 SP를 가져오게 됨. trampoline에는 이 부분이 없음.
    // 따라서 arguments로 받은 meta_addr을 사용해야 함.
    pgd_t *ttbr0_el1 = ((mm_struct*)meta_addr)->pgd;
    unsigned long new_sp = ((mm_struct*)meta_addr)->stack_vm;
    atomic64_t new_asid = ASID(((mm_struct*)meta_addr));
    // backup registers

    asm volatile ("msr ttbr0_el1, %0\r\n"
                    ""
                    :
                    :"r" (ttbr0_el1), "r" (new_sp), "r" (new_asid), "r" (fn_addr));
}
.popsection //".entry.cxgdom.text", "ax"
*/