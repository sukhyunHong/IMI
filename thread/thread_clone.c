#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>


#define __NR_iso_create_domain 436
#define __NR_iso_assign_memory 437
#define __NR_iso_init 438
#define __NR_iso_flush_tlb_all 439
#define __NR_clone 120

#define NUMBER_OF_DIGITS 32


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


pthread_t thread1, thread2, thread3;
pthread_mutex_t mutex;
int cnt = 0;
int cnt_1 = 0;
// void (*fn_addr)(void);//

void *count(void *arg){
    int i;
    char* name = (char*)arg;

	pid_t pid;            // process id
    pthread_t tid;        // thread id
	
    pid = getpid();
    tid = pthread_self();

    pthread_mutex_lock(&mutex);

    //======== critical section =============
    cnt=0;
    for (i = 0; i <10; i++)
    {
		printf("pid:%u, tid:%x --- %d\n",
			(unsigned int)pid, (unsigned int)tid);		
        printf("%s cnt: %d\n", name,cnt);
		// cxg_dom(1, fn_addr, 0);
        cnt++;
        usleep(1);
		sched_yield();
    }
    //========= critical section ============
    pthread_mutex_unlock(&mutex);
}

void *count1(void *arg){
    int i;
    char* name = (char*)arg;

	pid_t pid;            // process id
    pthread_t tid;        // thread id
 
    pid = getpid();
    tid = pthread_self();

    pthread_mutex_lock(&mutex);

    //======== critical section =============
    cnt_1=0;
    for (i = 0; i <10; i++)
    {
		printf("pid:%u, tid:%x --- %d\n",
			(unsigned int)pid, (unsigned int)tid);			
        printf("%s cnt: %d\n", name,cnt_1);
        cnt_1++;
        usleep(1);
		sched_yield();
    }
    //========= critical section ============
    pthread_mutex_unlock(&mutex);
}



__attribute__ ((section(".dom1.data"))) int cnt1  = 0;

__attribute__ ((section(".dom1.text"))) void pthread1(void* arg)
{
	// pthread_create(&thread1, NULL, count, (void *)"thread1");
	cnt1++;
}



__attribute__ ((section(".dom2.data"))) int cnt2  = 0;

__attribute__ ((section(".dom2.text"))) void pthread2(void* arg)
{
	cnt2++;
}


void cxg_dom(int dom_num, void (*fn_addr)(void), unsigned long tmp)
{
	// for save link register, frame pointer register, need call function.

	asm volatile ("svc #1\r\n" // svc1 : change domain to dom_num
			"blr x1\r\n":::"x30"); // inline assembly formmat update 함수 호출을 명시하는방법 
	asm volatile("svc #2\r\n"); // svc2 : change domain dom_num to main domain

	// At address 0xFFFF FDFF FE5F A880, real change domain function is there...
	/*
	asm volatile (	"mov	x3, #0xA880\r\n"
			"movk	x3, #0xFE5F, lsl #16\r\n"
			"movk	x3, #0xFDFF, lsl #32\r\n"
			"movk	x3, #0xFFFF, lsl #48\r\n"
			"blr	x3\r\n");
	*/
}



// void _pthread_init_function_asm(void * arg){

// 	__asm__ volatile(
//     "mov %0,    %%rdi\n"               // save start_routine as first argument
//     "mov %%rsp, %%rsi\n"               // save current_user_stack as second argument
//     "mov %1,    %%rsp\n"               // load exception stack
//     "mov %2,    %%rbx\n"               // load *arg for start_routine in callee-saved register (rbx)
//     "call _pthread_init_function_c\n"
//     "mov %%rbx, %%rdi\n"               // load *arg as first argument for start_routine
//     // "jmp " S_PIC(_pk_exception_handler_end) "\n"
//     : // no output operands
//     // : "m"(pk_data.pthread_arg.start_routine),
//     //   "m"(pk_data.pthread_arg.exception_stack_top),
//     //   "m"(pk_data.pthread_arg.arg) // Ensure we're using callee-saved register since GCC resolves %2 before the call.
// 	);
// }


int f(void *arg)
{
	// char buf1[64], buf2[64];
	pid_t pid, tid;
	// ssize_t rv;

	// pid = sys_getpid();
	// tid = sys_gettid();
	// snprintf(buf1, sizeof(buf1), "%u/task/%u", pid, tid);

	// rv = readlink("/proc/thread-self", buf2, sizeof(buf2));
	// assert(rv == strlen(buf1));
	// buf2[rv] = '\0';
	// assert(streq(buf1, buf2));

	if (arg)
		exit(0);
	return 0;
}

int main(void)
{
	int ret;
	int dom_num = 1;
	// void (*fn_addr)(void) = pthread1;
	uint64_t *new_ttbr0;
	unsigned long *stack_addr;
	void (*fn_addr)(void) = pthread1;
	void (*fn_addr1)(void) = pthread2;
	// void (*fn_addr1)(void) = pthread2;
	uint64_t *new_ttbr1;
	unsigned long *stack_addr1;

	pthread_mutex_init(&mutex,NULL);
	pthread_create(&thread1, NULL, count, (void *)"thread1");
	pthread_create(&thread2, NULL, count1, (void *)"thread2");
	pthread_create(&thread3, NULL, count, (void*)"thread3");
	// pthread_create(&thread1, NULL, count, (void *)"thread4");
	// pthread_create(&thread2, NULL, count1, (void *)"thread5");
	// pthread_create(&thread3, NULL, count, (void*)"thread6");	
	printf("pthred_ fnaddr %lx\n", fn_addr);

	printf("pthred_ fnaddr1 %lx\n", fn_addr1);

	printf("pthred_t thread1 %lx\n",&thread1);

	printf("pthred_t thread2 %lx\n",&thread2);
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

	const int PAGE_SIZE = 2*1024*1024;
	pid_t pid;
	void *stack;

	/* main thread */
	f((void *)0);

	// stack = mmap(NULL, 2 * PAGE_SIZE, stack_addr, stack_addr, -1, 0);
	// assert(stack != MAP_FAILED);
	/* side thread */
	pid = clone(f, stack + PAGE_SIZE, stack_addr, (void *)1);
	printf("pid===clone %u", pid);
	// assert(pid > 0);
	pause();
 
	return 0;

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


	// ret = syscall(__NR_iso_assign_memory, 2, &_got_start, 4096); // got 위치 강제 매핑.
	// if(ret != 0) {
	// 	printf("iso assign memory failed! exit process\n");
	// 	return 0;
	// }
	// ret = syscall(__NR_iso_assign_memory, 2, &__glob_start, 4096);	// global variable
	// if(ret != 0) {
	// 	printf("iso assign memory failed! exit process\n");
	// 	return 0;
	// }

	// ret = syscall(__NR_iso_assign_memory, 2, &__text_start, 4096);	// global variable
	// if(ret != 0) {
	// 	printf("iso assign memory failed! exit process\n");
	// 	return 0;
	// }



	// syscall(__NR_iso_assign_memory, 2, &_dom2_d_start, 4096);	// dom2_data...
	// if(ret != 0) {
	// 	printf("iso assign memory failed! exit process\n");
	// 	return 0;
	// }


	// syscall(__NR_iso_assign_memory, 2, &_dom2_start, 4096);	// dom2_functions...
	// if(ret != 0) {
	// 	printf("iso assign memory failed! exit process\n");
	// 	return 0;
	// }



	printf("assign memory finished\n");
    // int ret;

    printf("Main waiting for other thread\n");

	// // ret = pthread_join(thread3, (void*)&retval);
    // // assert(ret == 0);
    // // printf("retval = %lx", retval);
    // // printf(retval == PATTERN3+1000*5);
	// // temporary, this function works print all vmas of domain 1
	int status;
	syscall(__NR_iso_flush_tlb_all);
	// pthread_create(&thread1, NULL, pthread1, (void *)"thread");
	// printf("thread1: %d\n", cnt);
	// pthread1((void*)"thread");
	// cxg_dom(1, fn_addr, 0);
	// cxg_dom(2, fn_addr1, 1);
	// printf("thread2: %d\n", cnt2);


    pthread_join(thread1, (void **)&status);
    pthread_join(thread2, (void **)&status);

    pthread_mutex_destroy(&mutex);



	return 0;
}

