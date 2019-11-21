#include<init.h>
#include<lib.h>
#include<memory.h>
#include<context.h>
void map(u64 va,u32 cr3,u32 sd_f);
void free_page(u64 va, u32 cr3,u32 num_page,u32 region);
int check_validity(u64 start_add , u64 p4_base){
	u64 start_add_12 = ((start_add & 0x0fff)<<52)>>52;               // 12 LSB
 	u64 start_add_21 = ((start_add & 0x1FFFFF)<<43)>>55;				    // next 9 bits 
 	u64 start_add_30 = ((start_add & 0x3FFFFFFF)<<34)>>55;				 // next 9 bits
 	u64 start_add_39 = ((start_add & 0x7FFFFFFFFF)<<25)>>55;			// next 9 bits
 	u64 start_add_48 = ((start_add & 0xFFFFFFFFFFFF)<<16)>>55;	 // next 9 bits 

 	u64 p4_add = p4_base + 8*start_add_48;
 	u32 p4_entry = *((unsigned long*)p4_add);
 	if((p4_entry & 0x1)){
 		u64 p3_base = (unsigned long)osmap((p4_entry)>>12);
 		u64 p3_add = p3_base + 8*start_add_39;
    u32 p3_entry = *((unsigned long*)p3_add);
 		if((p3_entry & 0x1)){
 			u64 p2_base = (unsigned long)osmap((p3_entry)>>12);
 			u64 p2_add = p2_base + 8*start_add_30;
 			u32 p2_entry = *((unsigned long*)p2_add);
 			if((p2_entry & 0x1)){
 				u64 p1_base = (unsigned long)osmap((p2_entry)>>12);
 				u64 p1_add = p1_base + 8*start_add_21;
 				u32 p1_entry = *((unsigned long*)p1_add);
 				if((p1_entry & 0x1)){
 					return 1;
 				}
 				else{
 					return 0;
 				}
 			}
 			else{
 				return 0;
 			}
 		}
 		else{
 			return 0;
 		}
 	}
 	else {
 		return 0;
 	}

}
/*System Call handler*/
long do_syscall(int syscall, u64 param1, u64 param2, u64 param3, u64 param4)
{
    struct exec_context *current = get_current_ctx(); 
    printf("[GemOS] System call invoked. syscall no  = %d\n", syscall);
    switch(syscall)
    {
          case SYSCALL_EXIT:
                              printf("[GemOS] exit code = %d\n", (int) param1);
                              do_exit();
                              break;
          case SYSCALL_GETPID:
                              printf("[GemOS] getpid called for process %s, with pid = %d\n", current->name, current->id);
                              return current->id;      
          case SYSCALL_WRITE:
                             {
                             	// printf("hello write syscall is called \n");
                             	if(param2 > 1024)return -1;
                             	u64 start_add = param1;
                             	u32 length = param2;
                             	u64 end_add = start_add + length;
                             	u64 p4_base = (unsigned long)osmap(current->pgd);
                             	if(check_validity(start_add,p4_base) && check_validity(end_add,p4_base)){
                             		// printf("COOL:valid address\n");
                             		u32 count =0;
                             		char *ptr = (char *)param1;
                             		while(count<length){
                             			printf("%c",*ptr);
                             			ptr++;
                             			count++;
                             		}
                             		return length;
                             	}
                             	else{
                             		printf("ERROR:Invalid address\n");
                             		return -1;
                             	}
	                            break;
                                     
                             }

          case SYSCALL_EXPAND:
                             {  
                              // printf("Expand is called\n");
                             	u64 num_page = param1;
                             	u64 flag=param2;
                              if(param2 == MAP_RD)flag = MM_SEG_RODATA;
                              else flag = MM_SEG_DATA;
                             	u64 before_next_free = current->mms[flag].next_free;
                             	if(num_page > 512){	
                             		printf("ERROR:size must be < 512 pages \n");
                             		return 0;
                             	}
                             	if(current ->mms[flag].next_free + num_page*4096 > current ->mms[flag].end){
                             		printf("expand exceed segment end \n");
                             		return 0;
                             	}
                             	// u64 p4_base = (unsigned long)osmap(current->pgd);
                             	// if(!check_validity(current->mms[param2].next_free,p4_base)){
                             	// 	printf("not mapped yet\n");
                             	// }
                              // printf("next free pages before expand is %x\n",current ->mms[flag].next_free);
                             	current ->mms[flag].next_free += num_page *4096;
	                            // printf("start address = %x\n",current ->mms[flag].start);
	                            // printf("privious next free = %x\n",before_next_free);
	                            // u32 k = 1/0;
	                            // printf("next free pages after expand is %x\n",current ->mms[flag].next_free);
                              // printf("End address = %x\n",current ->mms[flag].end);
	                            // printf("num of pages = %d\n",param1);

	                            return before_next_free;	
	                            break;
                             }
          case SYSCALL_SHRINK:
                             {  
	                            // printf("hello shrink sys call is called\n");
	                            u32 num_page = param1;             // size
	                     		    u64 flag;
                              if(param2 == MAP_RD)flag = MM_SEG_RODATA;
                              else flag = MM_SEG_DATA;                // segment
	                            u64 max_shrink = current ->mms[flag].next_free - current ->mms[flag].start -1;
	                            if(num_page*4096 > max_shrink){
	                            	printf("ERROR:maximum shrink limit exceed\n");
	                            	return 0;
	                            }
                              // printf("next free before shrink %x\n",current ->mms[flag].next_free);
                              // printf("start address = %x\n",current ->mms[flag].start);
                              free_page(current ->mms[flag].next_free-0x1 ,current->pgd,num_page,flag);
	                            current ->mms[flag].next_free -= num_page *4096;
                              // printf("next free after shrink %x\n",current ->mms[flag].next_free);
	                            return current ->mms[flag].end;
	                            break;
                             }
                             
          default:
                              return -1;
                                
    }
    return 0;   /*GCC shut up!*/
}

extern int handle_div_by_zero(void)
{
    /*Your code goes in here*/
    u64 rip = 1;
    
    asm volatile ("mov 8(%rbp),%rax;"
                  "mov %rax ,-8(%rbp);"

                  );
    printf("ERROR:Div-by-zero detected at [RIP:%x]\n",rip);
    do_exit();
    printf("Div-by-zero handler:is Not working\n");
    return 0;
}

extern int handle_page_fault(void)
{
    /*Your code goes in here*/
    u64 error_code=5 ,rip=3;
    u64 cr2 = 9;
    u64 tmp=7;
  	asm volatile (
          "push %rax;"
          "push %rbx;"
          "push %rcx;"
          "push %rdx;"
          "push %rsi;"
          "push %rdi;"
          "push %r8;"
          "push %r9;"
          "push %r10;"
          "push %r11;"
          "push %r12;"
          "push %r13;"
          "push %r14;"
          "push %r15;"
  				"mov 8(%rbp),%rax;" //error code
  				"mov %rax,-8(%rbp);"
  				"mov 16(%rbp),%rax;" //rip
  				"mov %rax,-16(%rbp);"
  				"mov %cr2,%rax;"    //VA
  				"mov %rax ,-24(%rbp);"
          "mov %rsp ,-32(%rbp);"// save rsp
  				);
    printf("Page fault handler is called with [VA:%x][RIP:%x][error_code:%x]\n",cr2,rip,error_code);

  	struct exec_context *ctx = get_current_ctx();
  	u32 b1 = error_code & 0x1;
  	u32 b2 = (error_code & 0x2)>>1;
  	u32 b3 = (error_code & 0x4)>>2;
  	u32 cr3 = ctx->pgd;
  	u64 stack_st = ctx->mms[MM_SEG_STACK].start;
  	u64 stack_nd = ctx->mms[MM_SEG_STACK].end;
  	u64 data_st = ctx->mms[MM_SEG_DATA].start;
  	u64 data_nd = ctx->mms[MM_SEG_DATA].end;
  	u64 data_nf = ctx->mms[MM_SEG_DATA].next_free;
  	u64 rd_st = ctx->mms[MM_SEG_RODATA].start;
  	u64 rd_nd = ctx->mms[MM_SEG_RODATA].end;
  	u64 rd_nf = ctx->mms[MM_SEG_RODATA].next_free;
  	// printf("\nstack->starts: %x\n",stack_st);
  	// printf("stack->ends: %x\n",stack_nd);
  	// printf("data->starts: %x\n",data_st);
  	// printf("data->next_free: %x\n",data_nf);
  	// printf("data ends: %x\n",data_nd);
  	// printf("RODATA->starts: %x\n",rd_st);
  	// printf("RODATA->next_free: %x\n",rd_nf);
  	// printf("rd ends: %x\n\n",rd_nd);
  	if(cr2> stack_nd && cr2 <= stack_st){
  		// printf("VR belongs to stack segment\n");
  			map(cr2,cr3,0x7);   //flag = 7[111]
        printf("SUCCESS:Page fault  occured in stack region and handled successfully\n");
        asm volatile (  "mov -32(%rbp) , %rax;"   // restore rsp
                        "mov %rax , %rsp;"
                        "pop %r15;"
                        "pop %r14;"
                        "pop %r13;"
                        "pop %r12;"
                        "pop %r11;"
                        "pop %r10;"
                        "pop %r9;"
                        "pop %r8;"
                        "pop %rdi;"
                        "pop %rsi;"
                        "pop %rdx;"
                        "pop %rcx;"
                        "pop %rbx;"
                        "pop %rax;"
                        "mov %rbp,%rsp;"
                        "add $16 ,%rsp;"
                        "mov -16(%rsp),%rbp;"
                        "iretq"
                      );
        // do_exit();
        
  	}
  	if(cr2>= data_st && cr2 < data_nd){
  		// printf("VR belongs to data segment\n");
      if(cr2 >=data_nf){
        printf("ERROR:you are trying to access unallocated memory[VA:%x][RIP:%x][error_code:%x]\n",cr2,rip,error_code);
        do_exit();
      }
      else{
        map(cr2,cr3,0x7); //flag = 7 [111]
        printf("SUCCESS:Page fault  occured in data region and handled successfully\n");
             asm volatile (  "mov -32(%rbp) , %rax;"    // restore rsp
                        "mov %rax , %rsp;"
                        "pop %r15;"
                        "pop %r14;"
                        "pop %r13;"
                        "pop %r12;"
                        "pop %r11;"
                        "pop %r10;"
                        "pop %r9;"
                        "pop %r8;"
                        "pop %rdi;"
                        "pop %rsi;"
                        "pop %rdx;"
                        "pop %rcx;"
                        "pop %rbx;"
                        "pop %rax;"
                        "mov %rbp,%rsp;"
                        "add $16 ,%rsp;"
                        "mov -16(%rsp),%rbp;"
                        "iretq"
                      );
             printf("restore failed !!!\n");
      }

  	}
  	else if(cr2>= rd_st && cr2 < rd_nd){
  		// printf("VR belongs to RD segment\n");
      if(cr2 >=rd_nf){
          printf("ERROR:your are trying to access unallocated memory[VA:%x][RIP:%x][error_code:%x]\n",cr2,rip,error_code);
          do_exit();
        }
      else{
        if(b2==1){
          printf("ERROR:you are not allowed to write here.Please go away[VA:%x][RIP:%x][error_code:%x]\n",cr2,rip,error_code);
          do_exit();
        }
        else{
          map(cr2,cr3,0x5); //flag=0x05[101]
          printf("SUCCESS:Page fault  occured in RODATA region and handled successfully\n");
          asm volatile (  "mov -32(%rbp) , %rax;"   // restore rsp
                    "mov %rax , %rsp;"
                    "pop %r15;"
                    "pop %r14;"
                    "pop %r13;"
                    "pop %r12;"
                    "pop %r11;"
                    "pop %r10;"
                    "pop %r9;"
                    "pop %r8;"
                    "pop %rdi;"
                    "pop %rsi;"
                    "pop %rdx;"
                    "pop %rcx;"
                    "pop %rbx;"
                    "pop %rax;"
                    "mov %rbp,%rsp;"
                    "add $16 ,%rsp;"
                    "mov -16(%rsp),%rbp;"
                    "iretq"
                  );
        }
        
      }

      
  	}
    else{
      printf("ERROR: address do not belongs any of segment [error code = %x][rip= %x][VA = %x]\n",error_code,rip,cr2);
      do_exit();
    }
  	
    printf("page fault handler: is Not working \n");
    return 0;
}

void map(u64 va,u32 cr3,u32 sd_f){
  // printf("Hiii map is called with [va:%x][cr3:%x][flag:%x]\n",va,cr3,sd_f);
	unsigned long p4_base = (unsigned long)osmap(cr3);

	u64 va_12 = ((va & 0x0fff)<<52)>>52;               // 12 LSB      //
 	u64 va_21 = ((va & 0x1FFFFF)<<43)>>55;				    // next 9 bits //
 	u64 va_30 = ((va & 0x3FFFFFFF)<<34)>>55;		     // next 9 bits //
 	u64 va_39 = ((va & 0x7FFFFFFFFF)<<25)>>55;			// next 9 bits //
 	u64 va_48 = ((va & 0xFFFFFFFFFFFF)<<16)>>55;	 // next 9 bits //


	unsigned long p3_base;
	unsigned long p2_base;
	unsigned long p1_base;

	unsigned long p4d_entry = p4_base + 8*(va_48);
	u64 pre_entry = *((unsigned long*)p4d_entry);
	// sd_f = 0x7;
  if(!(pre_entry & 0x1)){
    // printf("level4 of data\n");
    u32 fnd3 = os_pfn_alloc(OS_PT_REG);
    u32 fnd2 = os_pfn_alloc(OS_PT_REG);
    u32 fnd1 = os_pfn_alloc(OS_PT_REG);
    u32 fnd0 = os_pfn_alloc(USER_REG);
    // printf("allocate ->> %x %x %x %x\n",fnd0,fnd1,fnd2,fnd3);
    unsigned long p3d_base = (unsigned long)osmap(fnd3);
    unsigned long p2d_base = (unsigned long)osmap(fnd2);
    unsigned long p1d_base = (unsigned long)osmap(fnd1);
    // int count =0x0;
    // while(count<4096){
    //   *((unsigned long*)p3d_base + count) = 0x0;
    //   *((unsigned long*)p2d_base + count) = 0x0;
    //   *((unsigned long*)p1d_base + count) = 0x0;
    //   count += 0x8;
    // }
    *((unsigned long*)p4d_entry) = fnd3 << 12 | sd_f;
  
    unsigned long p3d_entry = p3d_base + 8*(va_39);
    
    *((unsigned long*)p3d_entry) = fnd2 << 12 | sd_f;  
    
    unsigned long p2d_entry = p2d_base + 8*(va_30);
  
    *((unsigned long*)p2d_entry) = fnd1 << 12 | sd_f;
    
    unsigned long p1d_entry = p1d_base + 8*(va_21);

    *((unsigned long*)p1d_entry) = fnd0 << 12 | sd_f;
  }
  else{
    p3_base = (unsigned long)osmap(pre_entry >>12);
    unsigned long p3d_entry = p3_base + 8*(va_39);
    pre_entry = *((unsigned long*)p3d_entry);
    // printf("from data1 seg pre_entry = %x offset = %x\n",pre_entry,sd_s_39>>30);
    // printf("from data2 seg pre_entry = %x offset = %x\n",*((unsigned long*)p3_base+(ss_s_39>>30)),ss_s_39>>30);
    // printf("------------%x\n",pre_entry );
    if(!(pre_entry & 0x1)){
      u32 fnd2 = os_pfn_alloc(OS_PT_REG);
      u32 fnd1 = os_pfn_alloc(OS_PT_REG);
      u32 fnd0 = os_pfn_alloc(USER_REG);
      // printf("allocate ->>  %x %x %x\n",fnd0,fnd1,fnd2);
      unsigned long p2d_base = (unsigned long)osmap(fnd2);
      unsigned long p1d_base = (unsigned long)osmap(fnd1);
      // int count =0x0;
      // while(count<4096){
      //   *((unsigned long*)p2d_base + count) = 0x0;
      //   *((unsigned long*)p1d_base + count) = 0x0;
      //   count += 0x8;
      // }
      // printf("level3 of data \n");
      *((unsigned long*)p3d_entry) = (fnd2 << 12) | sd_f;  
		
			unsigned long p2d_entry = p2d_base + 8*(va_30);
		
			*((unsigned long*)p2d_entry) = fnd1 << 12 | sd_f;
			
			unsigned long p1d_entry = p1d_base + 8*(va_21);

			*((unsigned long*)p1d_entry) = fnd0 << 12 | sd_f;
		}
		else{
      p2_base = (unsigned long)osmap(pre_entry >>12);
			unsigned long p2d_entry = p2_base + 8*(va_30);
			pre_entry = *((unsigned long*)p2d_entry);

			if(!(pre_entry & 0x1)){
				u32 fnd1 = os_pfn_alloc(OS_PT_REG);
				u32 fnd0 = os_pfn_alloc(USER_REG);
				// printf("allocate ->>  %x %x\n",fnd0,fnd1);
				unsigned long p1d_base = (unsigned long)osmap(fnd1);
				// int count =0x0;
				// while(count<4096){
				// 	*((unsigned long*)p1d_base + count) = 0x0;
				// 	count += 0x8;
				// }
				// printf("level2 of data \n");
				*((unsigned long*)p2d_entry) = fnd1 << 12 | sd_f;
			
				unsigned long p1d_entry = p1d_base + 8*(va_21);

				*((unsigned long*)p1d_entry) = fnd0 << 12 | sd_f;
			}
			else{
        p1_base = (unsigned long)osmap(pre_entry >>12);
        unsigned long p1d_entry = p1_base + 8*(va_21);
        pre_entry = *((unsigned long*)p1d_entry);
        // printf("PRE ENtry::%x\n",pre_entry );
        if(!(pre_entry & 0x1)){
          u32 fnd0 = os_pfn_alloc(USER_REG);
          // printf("allocate ->> %x\n",fnd0);
          // printf("level1 of data \n");
          unsigned long p1d_entry = p1_base + 8*(va_21);
          *((unsigned long*)p1d_entry) = fnd0 << 12 | sd_f;
          // printf("P1D entry::%x \n",*((unsigned long*)p1d_entry));
        }
        else{
          // printf("address already mapped \n");
          
        }
				
			}
		}
	}

// do_exit();
	return;
}

void free_page(u64 va, u32 cr3,u32 num_page,u32 region){
  // printf("free_page is called with [VA:%x][pfn:%x][num of pages:%x][region:%d]\n",va,cr3,num_page,region);
  u64 va_12 = ((va & 0x0fff)<<52)>>52;                   // 12 LSB      //
  u64 va_21 = ((va & 0x1FFFFF)<<43)>>55;          // next 9 bits //
  u64 va_30 = ((va & 0x3FFFFFFF)<<34)>>55;         // next 9 bits //
  u64 va_39 = ((va & 0x7FFFFFFFFF)<<25)>>55;      // next 9 bits //
  u64 va_48 = ((va & 0xFFFFFFFFFFFF)<<16)>>55;     // next 9 bits //
  u32 flag =0;
  u32 count = 0;
  u64 p4_base = (unsigned long)osmap(cr3);
  u64 p4_add = p4_base + 8*va_48;
  while(p4_add>=p4_base && flag==0){
    u32 p4_entry = *((unsigned long*)p4_add);
    u64 p3_base = (unsigned long)osmap((p4_entry)>>12);
    u64 p3_add = p3_base + 8*va_39;
    while(p3_add >= p3_base && flag ==0){
      u32 p3_entry = *((unsigned long*)p3_add);
      u64 p2_base = (unsigned long)osmap((p3_entry)>>12);
      u64 p2_add = p2_base + 8*va_30;
      while(p2_add >= p2_base && flag == 0){
        u32 p2_entry = *((unsigned long*)p2_add);
        u64 p1_base = (unsigned long)osmap((p2_entry)>>12);
        u64 p1_add = p1_base + 8*va_21;
        while(p1_add >= p1_base && flag ==0){
          u32 p1_entry = *((unsigned long*)p1_add);
          u32 fn = p1_entry>>12;
          if(p1_entry & 0x1){
            // printf("let's call to free\n");
            os_pfn_free(USER_REG,fn);
            // printf("success!!\n");
          }
          p1_entry = 0x0;
          p1_add -=0x8;
          count ++;
          if(count>num_page){flag=1;break;}
        }
        // if(p1_add < p1_base)os_pfn_free(OS_PT_REG,p2_entry>>12);
        if(flag==1)break;
        // printf("hello from page table p1\n");
        p2_add -= 0x8;
      }
      // if(p2_add < p2_base)os_pfn_free(OS_PT_REG,p3_entry>>12);
      if(flag==1)break;
      // printf("hello from page table p2\n");
      p3_add -= 0x8;
    }
    // if(p3_add < p3_base)os_pfn_free(OS_PT_REG,p4_entry>>12);
    if(flag==1)break;
    // printf("hello from page table p3\n");
    p4_add -= 0x8;
  }

  return;
}









