#include<init.h>
#include<memory.h>
static void exit(int);
static int main(void);


void init_start()
{
  int retval = main();
  exit(0);
}

/*Invoke system call with no additional arguments*/
static int _syscall0(int syscall_num)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}

/*Invoke system call with one argument*/

static int _syscall1(int syscall_num, int exit_code)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}
static unsigned long long _syscall2(int syscall_num, int num_pages,int flage)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}

static unsigned long long _syscall3(int syscall_num, char * buf,int length)
{
  asm volatile ( "int $0x80;"
                 "leaveq;"
                 "retq;"
                :::"memory");
  return 0;   /*gcc shutup!*/
}


static void exit(int code)
{
  _syscall1(SYSCALL_EXIT, code); 
}

static int getpid()
{
  return(_syscall0(SYSCALL_GETPID));
}

static unsigned long long expand(int size , int flage)
{
  // what is i1 and i2 ??
  // _syscall2(SYSCALL_EXPAND, size, flage);
  return (_syscall2(SYSCALL_EXPAND, size, flage));
  return   0x140000000;
}

static int write(char * buf , int length)
{
  return(_syscall3(SYSCALL_WRITE,buf , length));
  // printf("size from init.c = %d\n",strlen(buf));
}

static unsigned long long shrink(int size , int flage)
{
  // what is i1 and i2 ??
  return (_syscall2(SYSCALL_SHRINK, size, flage));
}

  static int main()
  {
    // void *ptr2;
    char *ptr2 = (char *) expand(9, MAP_WR);
    // char *buf[7];
    // char *ptr2 = (char *) expand(9, MAP_RD);
    // if(ptr2 == NULL)
    //             write("FAILED\n", 7);
    // write(ptr,16);
   *ptr2='k';
   write(ptr2,1);
   *(ptr2+1)='a';
   *(ptr2+2)='m';
   *(ptr2+3)='l';
   *(ptr2+4)='e';
   *(ptr2+5)='s';
   *(ptr2+6)='h';
   *(ptr2+7)='\n';
    write(ptr2,8);
    // *(ptr + 8192) = 'A';  // Page fault will occur and handled successfully
     // *(ptr2) = 'K';  // Page fault will occur and handled successfully
     // u32 var = 2;
    // char *ptr = (char *) shrink(9, MAP_WR);
    // *ptr2 = 'A';          /*Page fault will occur and handled successfully*/
    // write(ptr2,1);
    *(ptr2 + 4096) = 'A';   /*Page fault will occur and PF handler should termminate              the process (gemOS shell should be back) by printing an error message*/
    *(ptr2 + 4096) = 'B';
    write(ptr2+4096,1);
    exit(0);
  }

// static int main()
// {
//   unsigned long i;
// #if 0
//   unsigned long *ptr = (unsigned long *)0x100032;
//   i = *ptr;
// #endif
//   i = getpid();
//   exit(-5);
//   return 0;
// }

// static int main()
// {
//   void *ptr1;
//   char *ptr = (char *) expand(8, MAP_WR);
  
//   if(ptr == NULL)
//               {write("FAILED\n", 7);}
  
//   *(ptr + 8192) = 'A';   /*Page fault will occur and handled successfully*/
  
//   ptr1 = (char *) shrink(7, MAP_WR);
//   *ptr = 'A';          /*Page fault will occur and handled successfully*/

//   *(ptr + 4096) = 'A';   /*Page fault will occur and PF handler should termminate 
//                    the process (gemOS shell should be back) by printing an error message*/
//   exit(0);
// }