#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>


#define __NR_iso_create_domain 436
#define __NR_iso_assign_memory 437
#define __NR_iso_init 438
#define __NR_iso_flush_tlb_all 439
#define NUMBER_OF_DIGITS 32
extern long _dom1_d_start;
extern long _dom1_d_end;

extern long _dom1_start;
extern long _dom1_end;

extern long _got_start;
extern long _got_end;

extern long __glob_start;
extern long __glob_end;

extern long __text_start;
extern long __text_end;

int glob_c = 0;


void void_fn(void)
{
	return;
}

// void ultoa(unsigned long value, char* string, int radix)
// {
// unsigned char index;
// char buffer[NUMBER_OF_DIGITS];  /* space for NUMBER_OF_DIGITS + '\0' */

//   index = NUMBER_OF_DIGITS;

//   do {
//     buffer[--index] = '0' + (value % radix);
//     if ( buffer[index] > '9') buffer[index] += 'A' - ':'; /* continue with A, B,... */
//     value /= radix;
//   } while (value != 0);

//   do {
//     *string++ = buffer[index++];
//   } while ( index < NUMBER_OF_DIGITS );

//   *string = 0;  /* string terminator */
// }

// void ltoa(long value, char* string, int radix)
// {
//   if (value < 0 && radix == 10) {
//     *string++ = '-';
//     ultoa(-value, string, radix);
//   }
//   else {
//     ultoa(value, string, radix);
//   }
// }

void cxg_dom(int dom_num, void (*fn_addr)(void), unsigned long tmp)
{
	// for save link register, frame pointer register, need call function.
	// So call empty function.
	asm volatile ("svc #1\r\n" // svc1 : change domain to dom_num
			"blr x1\r\n":::"x30"); // inline assembly formmat update 
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

__attribute__ ((section(".dom1.data"))) int glob_a = 0;


__attribute__ ((section(".dom1.text"))) void increase_glob_a(void)
{
	glob_a++;
}



int main(void)
{
	int ret;
	int dom_num = 1;
	void (*fn_addr)(void) = increase_glob_a;
	uint64_t *new_ttbr0;
	unsigned long *stack_addr;

	// long hex_size = (long)&__text_end - (long)&__text_start;
	// int base;
    // char buffer[33];
	// ltoa(hex_size,buffer, 10);
	// long func_size = atol(buffer);
	// printf("hex_size: %s, func_size: %d\n",buffer, func_size);


	printf("got_d_start %lx\n", &_got_start);
	printf("__glob_start %lx\n", &__glob_start);
	printf("__text_start %lx\n", &__text_start);
	printf("text size %lx\n", (long)&__text_end - (long)&__text_start);

	printf("glob size %lx\n", (long)&__glob_end - (long)&__glob_start);

	printf("got size %lx\n", (long)&_got_end - (long)&_got_start);

	printf("dom1_d size %lx\n", (long)&_dom1_d_end - (long)&_dom1_d_start);
	printf("dom1_d_start %lx\n", &_dom1_d_start);
	printf("dom1_d_end %lx\n\n\n", &_dom1_d_end);
	
	// printf("dom2_d size %lx\n", (long)&_dom2_d_end - (long)&_dom2_d_start);
	// printf("dom2_d_start %lx\n", &_dom2_d_start);
	// printf("dom2_d_end %lx\n\n\n", &_dom2_d_end);
	
	printf("dom1 size %lx\n", (long)&_dom1_end - (long)&_dom1_start);
	printf("dom1_start %lx\n", &_dom1_start);
	printf("dom1_end %lx\n\n\n", &_dom1_end);





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

	ret = syscall(__NR_iso_assign_memory, 1, &__text_start, 4096);	// global variable
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}
	// fucntions 14096으로 잡으면 오류가 생김, 4096 *3 으로해야지만 중간에 오류가 나지 않음
	// ret = syscall(__NR_iso_assign_memory, 1, &__text_start, 4096 * 2);	// global variable
	// if(ret != 0) {
		// printf("iso assign memory failed! exit process\n");
		// return 0;
	// }

	syscall(__NR_iso_assign_memory, 1, &_dom1_start, 4096);	// functions...
	if(ret != 0) {
		printf("iso assign memory failed! exit process\n");
		return 0;
	}


	printf("assign memory finished\n");

	// temporary, this function works print all vmas of domain 1
	syscall(__NR_iso_flush_tlb_all);
	
	cxg_dom(1, fn_addr, 0);

	printf("glob_a: %d\n", glob_a);

	printf("glob_c: %d\n", glob_c);


	return 0;
}

