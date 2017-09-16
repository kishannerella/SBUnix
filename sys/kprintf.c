#include <sys/kprintf.h>
#include <stdio.h>
#include <stdarg.h>

typedef int boolean;

#define true 1
#define false 0
#define MAX_BUFFER 4096
#define width 160
#define height 15

int stringlen(const char *a) 
{
    int length = 0;
    while(*a)
    {   
        length++;
        a++;
    }   
    return length;
}
void strrev(char *a) 
{
   char temp;
   for(int i = stringlen(a)-1, j=0; i>=j;i--, j++)
   {   
       temp = a[i];
       a[i] = a[j];
       a[j] = temp;
   }   
}
void intTOstring(int number, char *p, int base)
{
    int i=0;
    boolean isNeg = false;
    if(number == 0)
    {   
        p[i++]='0';
        p[i]='\0';
    }   
    if(number < 0)
    {   
        number = -1*number;
        isNeg = true;
    }
    while(number !=0)
    {
        int rem = number % base;
        p[i++] = rem + '0';
        number = number/base;
    }
    if(isNeg)
    {
        p[i++] = '-';
    }
    p[i] = '\0';
    strrev(p);
}
void intTOhexa( unsigned int number, char *p, int base)
{   
    int i=0;
    if(number == 0)
    {   
        p[i++]='0';
        p[i]='\0';
    }
    while(number !=0)
    {   
        int rem = number % base;
        if(rem >= 0 && rem <= 9)
        {   
            p[i++] = rem + '0';
        }else
        {   
            if(rem == 10) p[i++] = 'a';
            if(rem == 11) p[i++] = 'b';
            if(rem == 12) p[i++] = 'c';
            if(rem == 13) p[i++] = 'd';
            if(rem == 14) p[i++] = 'e';
            if(rem == 15) p[i++] = 'f';
        }
        number = number/base;
    }
    p[i] = '\0';
    strrev(p);
}
void longTOhexa( unsigned long number, char *p, int base){
    int i=0;
    if(number == 0){
        p[i++]='0';
        p[i]='\0';
    }
    while(number !=0){
        int rem = number % base;
        if(rem >= 0 && rem <= 9)
        {
            p[i++] = rem + '0';
        }else
        {
            if(rem == 10) p[i++] = 'a';
            if(rem == 11) p[i++] = 'b';
            if(rem == 12) p[i++] = 'c';
            if(rem == 13) p[i++] = 'd';
            if(rem == 14) p[i++] = 'e';
            if(rem == 15) p[i++] = 'f';
        }
        number = number/base;
    }
    p[i++] = 'x';
    p[i++] = '0';
    p[i] = '\0';
    strrev(p);
}
int pos =0;
void memcopy()
{
     register char *temp3, *temp2;
     pos =0;
     temp2 = (char*)0xb8000+pos;
     for(temp3 = (char*)0xb8000+width+pos; temp3 < (char*)0xb8000+width*height; temp3 += 2, temp2 += 2, pos += 2)
     {
        *temp2 = *temp3;
        *temp3 = ' ';
     }
}
void kprintf(const char *fmt, ...)
{

    va_list arg;
    va_start(arg, fmt);

    // as of now hard coded, but size shall depend on size of fmt 
    // considering substitution of %s
    char buffer[stringlen(fmt) + MAX_BUFFER];

    int k=0;
    for(int i=0; fmt[i]!='\0';i++)
    {
        if(fmt[i] == '%')
        {
            if(fmt[i+1] == 'd')
            {
                unsigned int num = va_arg(arg, int);
                char snum[12];
                intTOstring(num, snum, 10);
                for(int j=0; j<stringlen(snum); j++)
                {
                    buffer[k++] = snum[j];
                }
                i++;
            }

            if(fmt[i+1] == 'c')
            {
                char ch = va_arg(arg, int);
                buffer[k++] = ch;
                i++;
            }

            if(fmt[i+1] == 's')
            {
                char *sub = va_arg(arg, char *);
                for(int j=0; j<stringlen(sub); j++)
                {
                    buffer[k++] = sub[j];
                }
                i++;
            }

            if(fmt[i+1] == 'x')
            {
                unsigned int num = va_arg(arg, unsigned int);
                char hnum[20];
                intTOhexa(num, hnum, 16);
                for(int j=0; j<stringlen(hnum); j++)
                {
                    buffer[k++] = hnum[j];
                }
                i++;
            }
            if(fmt[i+1] == 'p')
            {
                unsigned long num = va_arg(arg, unsigned long);
                char hnum[20];
                longTOhexa(num, hnum, 16);
                for(int j=0; j<stringlen(hnum); j++)
                {
                    buffer[k++] = hnum[j];
                }
                i++;
            }
        }else
        {
            buffer[k++]= fmt[i];
        }
    }
    va_end(arg);
    buffer[k]='\0';
  
  register char *temp1, *temp2;  
  for(temp2 = (char*)0xb8001; temp2 < (char*)0xb8000+160*25; temp2 += 2)
     *temp2 = 7;

  if(pos >= width*height)
  {   
     /* Scroll: push all the lines on video address by one line upwards, discarding the first line */
     memcopy();

     pos = width*height - width;
     temp2 = (char*)0xb8000 + pos;
  }else
     temp2 = (char*)0xb8000+pos;  
  
  for(temp1 = buffer; *temp1; temp1 += 1,temp2 += 2,pos += 2)
  {
     if(*temp1 == '\n')
     {
        pos = pos + width - (pos % width)-2;
        temp2 = (char*)0xb8000 + pos;
     }else
     if(*temp1 == '\r')
     {
        pos = pos - (pos % width)-2;
        temp2 = (char*)0xb8000 + pos;
     }else
     {
        *temp2 = *temp1;
     }
  }

}
    
