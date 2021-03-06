#include <sys/defs.h>
#include <sys/util.h>

/*void sleep(uint32_t secs)
{
   uint32_t i,j;
   //while(1);
   while(secs--)
      for (i = 0;i < 10000;i++)
         for (j = 0;j < 10000;j++); 
}*/

uint64_t min(uint64_t a, uint64_t b)
{
   if (a < b)
      return a;
   return b;
}

void memcpy(char* dest, char* src, uint64_t bytes){
   while(bytes--)
   {
     *dest = *src;
     dest++;
     src++;
   }
}

uint32_t strlen(char *str)
{
   char *c = str;
   uint32_t   len = 0;

   if (!str)
      return -1;

   while (*c != '\0')
   {
      c++;
      len++;
   }

   return len;
}

void strncpy(char* dest, char* src, uint32_t len)
{
   int i;
  
   for (i = 0;i < len && src[i];i++)
   {
      dest[i] = src[i];
   }

   if (i == len)
   {
      dest[i-1] = '\0';
   }
   else
   {
      dest[i] = '\0';
   }
}

void strcpy(char* dest, char* src)
{
   int i;
  
   for (i = 0;src[i];i++)
   {
      dest[i] = src[i];
   }
   dest[i] = '\0';
}

int strcmp(char *str1, char *str2)
{
   int i;

   if (str1 == NULL && str2 == NULL)
      return 0;

   if (str1 == NULL)
      return 1;

   if (str2 == NULL)
      return -1;

   for (i = 0; str1[i] && str2[i];i++)
   {   
      if (str1[i] < str2[i]) 
         return -1;  
      if (str1[i] > str2[i]) 
         return 1;  
   }   

   if (str1[i] == '\0'&& 
       str2[i] == '\0')
      return 0;

   if (str1[i])
      return 1;
   
   return -1;
}
