#ifndef _IO_H
#define _IO_H

#include <sys/defs.h>

static __inline void outb(uint16_t port, uint8_t val)
{
   __asm__ __volatile__ ("outb %0, %1" 
                         : 
                         : "a"(val), "Nd"(port));
}

static __inline uint8_t inb(uint16_t port)
{
   uint8_t sysret;
   __asm__ __volatile__ ("inb %1, %0" 
                          : "=a"(sysret)
                          : "Nd"(port));

   return sysret;
}

static __inline void outl(uint16_t port, uint32_t val)
{
   __asm__ __volatile__ ("outl %0, %1" 
                         : 
                         : "a"(val), "Nd"(port));
}

static __inline uint32_t inl(uint16_t port)
{
   uint32_t sysret;
   __asm__ __volatile__ ("inl %1, %0" 
                          : "=a"(sysret)
                          : "Nd"(port));

   return sysret;
}

static __inline void io_wait()
{
   __asm__ __volatile__ ("jmp 1f\n\t"
                         "1:jmp 2f\n\t"
                         "2:");
}

#endif
