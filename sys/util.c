#include <sys/defs.h>
#include <sys/util.h>

void sleep(uint32_t secs)
{
   uint32_t i,j;
   //while(1);
   while(secs--)
      for (i = 0;i < 10000;i++)
         for (j = 0;j < 100000;j++); 
}
uint64_t min(uint64_t a, uint64_t b)
{
   if (a < b)
      return a;
   return b;
}
