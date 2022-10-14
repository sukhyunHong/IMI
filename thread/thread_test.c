#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>


#define __NR_iso_create_domain 436
#define __NR_iso_assign_memory 437
#define __NR_iso_init 438
#define __NR_iso_flush_tlb_all 439
//thread 패턴인듯
#define PATTERN1 0xAFFEAFFE
#define PATTERN2 0xDEADBEEF
#define PATTERN3 0xC0FFFFEE


extern long _dom1_d_start;
extern long _dom1_d_end;

extern long _dom2_d_start;
extern long _dom2_d_end;

extern long _dom1_start;
extern long _dom1_end;

extern long _dom2_start;
extern long _dom2_end;

extern long _got_start;
extern long _got_end;

extern long __glob_start;
extern long __glob_end;

extern long __text_start;
extern long __text_end;

#define NUMBER_OF_DIGITS 32


int glob_c = 0;

__attribute__ ((section(".dom1.data"))) int glob_a = 0;


__attribute__ ((section(".dom1.text"))) void increase_glob_a(void)
{
	glob_a++;
	// glob_c++;  // global_c++ 을 추가할 경우 테이블을 찾을수없어서 오류가 남
}

__attribute__ ((section(".dom2.data"))) int glob_b = 0;


__attribute__ ((section(".dom2.text"))) void increase_glob_b(void)
{
	glob_b++;
}


void cxg_dom(int dom_num, void (*fn_addr)(void), unsigned long tmp)
{
	// for save link register, frame pointer register, need call function.
	// So call empty function.
	// void_fn(); //void function이 없을 시 pte를 찾을 수 없어서 오류가남
	asm volatile ("svc #1\r\n" // svc1 : change domain to dom_num
			"blr x1\r\n":::"x30"); // inline assembly formmat update 함수 호출을 명시하는방법 
	asm volatile(		
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

void* pthread1(void* arg){

    uintptr_t var = (uintptr_t)arg;
    for (size_t i = 0; i < 1000; i++){
        var++;
        sched_yield();
    }
    return (void*) var;

}

void* pthread2(void* arg){
    uintptr_t var = (uintptr_t)arg;
    for (size_t i =0; i < 1000; i++){
        var +=7;
        sched_yield();
    }
    return (void*)var;
}

void* pthread3(void* arg) {
    printf("I am thread 3: %p\n", arg);
    uintptr_t var = (uintptr_t)arg;
    for (size_t i = 0; i < 1000; i++) {
      var+=5;
      sched_yield();//user level에서 프로세스가 다른 프로세스에 넘길때 
    }
    printf("thread3 returning %lx", var);
    //pthread_exit((void*)var);
    return (void*)var;
}

void test4_pthread(){
    int ret;
    pthread_t thread1, thread2, thread3;
    ret = pk_pthread_create(&thread1, NULL, pthread1, (void*)PATTERN1);
    // assert(ret == 0);
    ret = pk_pthread_create(&thread2, NULL, pthread2, (void*)PATTERN2);
    // assert(ret == 0);
    ret = pk_pthread_create(&thread3, NULL, pthread3, (void*)PATTERN3);
    // assert(ret == 0);
    printf("Main waiting for other thread");

    uintptr_t retval;
    ret = pthread_join(thread3, (void*)&retval);
    // assert(ret == 0);
    printf("retval = %lx", retval);
    printf(retval == PATTERN3+1000*5);

    ret = pthread_join(thread2, (void*)&retval);
    // assert(ret == 0);
    printf("retval = %lx", retval);
    printf(retval == PATTERN2+1000*7);

    ret = pthread_join(thread1, (void*)&retval);
    // assert(ret == 0);
    printf("retval = %lx", retval);
    printf(retval == PATTERN1+1000);

    printf("Main done waiting");
}




int main(void)
{
	int ret;
	int dom_num = 1;
	void (*fn_addr)(void) = increase_glob_a;
	uint64_t *new_ttbr0;
	unsigned long *stack_addr;

	void (*fn_addr1)(void) = increase_glob_b;
	uint64_t *new_ttbr1;
	unsigned long *stack_addr1;


	printf("got_d_start %lx\n", &_got_start);
	printf("__glob_start %lx\n", &__glob_start);
	printf("__text_start %lx\n", &__text_start);
	
	printf("text size %lx\n", (long)&__text_end - (long)&__text_start);

	printf("glob size %lx\n", (long)&__glob_end - (long)&__glob_start);

	printf("got size %lx\n", (long)&_got_end - (long)&_got_start);

	printf("cxg_dom address: %lx\n", &cxg_dom);

	// printf("function size %lx\n", (long)&__text_end - (long)&__text_start);
	// long hex_size = (long)&__text_end - (long)&__text_start;
	// int base;
    // char buffer[33];
	// ltoa(hex_size,buffer, 10);
	// long func_size = atol(buffer);
	// printf("hex_size: %s, func_size: %d\n",buffer, func_size);
	// printf( "%d %d\n", base, ltoa(func_size,buffer, base));
	// printf("size변경 %s\n", ch);
	// int a = changeHex(ch);

	printf("dom1_d size %lx\n", (long)&_dom1_d_end - (long)&_dom1_d_start);
	printf("dom1_d_start %lx\n", &_dom1_d_start);
	printf("dom1_d_end %lx\n\n\n", &_dom1_d_end);
	
	printf("dom2_d size %lx\n", (long)&_dom2_d_end - (long)&_dom2_d_start);
	printf("dom2_d_start %lx\n", &_dom2_d_start);
	printf("dom2_d_end %lx\n\n\n", &_dom2_d_end);
	
	printf("dom1 size %lx\n", (long)&_dom1_end - (long)&_dom1_start);
	printf("dom1_start %lx\n", &_dom1_start);
	printf("dom1_end %lx\n\n\n", &_dom1_end);

	printf("dom1 size %lx\n", (long)&_dom2_end - (long)&_dom2_start);
	printf("dom1_start %lx\n", &_dom2_start);
	printf("dom1_end %lx\n\n\n", &_dom2_end);





	ret = syscall(__NR_iso_init);
	if(ret == 0) {
		printf("iso initialize failed! exit process\n");
		return 0;
	}

	ret = syscall(__NR_iso_create_domain,1 , &new_ttbr0, &stack_addr);
	if(ret == -1) {
		printf("iso create domain failed! exit process\n");
		return 0;
	}

	ret = syscall(__NR_iso_create_domain,2 , &new_ttbr1, &stack_addr1);
	if(ret == -1) {
		printf("iso create domain failed! exit process\n");
		return 0;
	}



	ret = syscall(__NR_iso_assign_memory, 1, &_got_start, 4096); // got 위치 강제 매핑.
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}
	ret = syscall(__NR_iso_assign_memory, 1, &__glob_start, 4096);	// global variable
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}

	syscall(__NR_iso_assign_memory, 1, &_dom1_d_start, 4096);	// functions...
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}

	// ret = syscall(__NR_iso_assign_memory, 1, &__text_start, (int)func_size);	// global variable
	// if(ret != 0) {
	// 	printf("iso assign memory failed! exit process\n");
	// 	return 0;
	// }
	// fucntions 14096으로 잡으면 오류가 생김, 4096 *3 으로해야지만 중간에 오류가 나지 않음
	ret = syscall(__NR_iso_assign_memory, 1, &__text_start, 4096);	// global variable
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}

	syscall(__NR_iso_assign_memory, 1, &_dom1_start, 4096);	// functions...
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}


	ret = syscall(__NR_iso_assign_memory, 2, &_got_start, 4096); // got 위치 강제 매핑.
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}
	ret = syscall(__NR_iso_assign_memory, 2, &__glob_start, 4096);	// global variable
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}

	ret = syscall(__NR_iso_assign_memory, 2, &__text_start, 4096);	// global variable
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}



	syscall(__NR_iso_assign_memory, 2, &_dom2_d_start, 4096);	// dom2_data...
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}


	syscall(__NR_iso_assign_memory, 2, &_dom2_start, 4096);	// dom2_functions...
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}



	printf("assign memory finished\n");

	// temporary, this function works print all vmas of domain 1
	syscall(__NR_iso_flush_tlb_all);
	cxg_dom(1, fn_addr, 0);
	printf("glob_a: %d\n", glob_a);
	cxg_dom(2, fn_addr1, 0);
	printf("glob_b: %d\n", glob_b);

	cxg_dom(1, fn_addr, 0);
	printf("glob_a: %d\n", glob_a);




	printf("glob_c: %d\n", glob_c);


	return 0;
}

