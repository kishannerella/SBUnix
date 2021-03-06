#include <sys/pci.h>
#include <sys/io.h>
#include <sys/kprintf.h>

#define CONFIG_ADDRESS  0xCF8
#define CONFIG_DATA     0xCFC

#define CONFIG_ENABLE_BIT 0x80000000
#define REG_UPPER_MASK    0xFFFF0000
#define REG_INNER_MASK    0x0000FFFF

//#define AHCI_BASE         0x3FFF0000
//#define AHCI_BASE         0x100000
//#define AHCI_BASE         0xbd810000
#define AHCI_BASE         0xA6000
//#define AHCI_BASE 0x100000000UL

#define PCI_VENDOR(b,s,f)    ((pci_config_read_register(b, s, f, 0)) & 0xFFFF)
#define PCI_CLASS(b,s,f)     ((uint8_t)(((pci_config_read_register(b, s, f, 8)) & 0xFF000000) >> 24))
#define PCI_SUBCLASS(b,s,f)  ((uint8_t)(((pci_config_read_register(b, s, f, 8)) & 0x00FF0000) >> 16))
#define PCI_PROGIF(b,s,f)    ((uint8_t)(((pci_config_read_register(b, s, f, 8)) & 0x0000FF00) >> 8))
#define PCI_BAR4(b,s,f)      (pci_config_read_register(b, s, f, 0x20))
#define PCI_BAR5(b,s,f)      (pci_config_read_register(b, s, f, 0x24))

void* ahci_abar;

uint32_t pci_config_read_register(uint32_t bus, uint32_t slot,
                               uint32_t func, uint32_t offset)
{
   uint32_t address = (bus << 16) | (slot << 11) | (func << 8) |
                      (offset & 0xfc) | CONFIG_ENABLE_BIT;

   outl(CONFIG_ADDRESS, address);
   return inl(CONFIG_DATA);
}

void pci_config_write_register(uint32_t bus, uint32_t slot,
                                   uint32_t func, uint32_t offset,
                                   uint32_t data)
{
   uint32_t address = (bus << 16) | (slot << 11) | (func << 8) |
                      (offset & 0xfc) | CONFIG_ENABLE_BIT;

   outl(CONFIG_ADDRESS, address);
   outl(CONFIG_DATA, data);
}

void* pci_abar(uint8_t bus, uint8_t slot, uint8_t fn)
{
   uint64_t bar5 = PCI_BAR5(bus, slot, fn);

   kprintf("bar5 - 0x%x\n", bar5);

   /* HOMEWORK - Using the loader built for the homework does not need
                 the following line. If the original loader is used and
                 the following line is commented, we get error */
   pci_config_write_register(bus, slot, fn, 0x24, AHCI_BASE);
   bar5 = PCI_BAR5(bus, slot, fn);

   return (void*) bar5;
}

uint16_t pci_enum()
{
   uint8_t bus;
   uint8_t device;
   uint8_t fn;

   kprintf("Walking through PCI config space...\n");
   for (bus = 0; bus < 255;bus++)
      for (device = 0; device < 32; device++)
         for (fn = 0; fn < 8;fn++)
         if (PCI_VENDOR(bus, device, fn) != 0xFFFF &&
             PCI_CLASS(bus, device, fn) == 0x01 &&
             PCI_SUBCLASS(bus, device, fn) == 0x06 /*&&
             PCI_PROGIF(bus, device) == 0x01*/)
         {
            ahci_abar = pci_abar(bus, device, fn);
            kprintf("line1 - 0x%x\n", pci_config_read_register(bus, device, fn, 0x0));
            kprintf("line2 - 0x%x\n", pci_config_read_register(bus, device, fn, 0x4));
            kprintf("line3 - 0x%x\n", pci_config_read_register(bus, device, fn, 0x8));
            kprintf("line4 - 0x%x\n", pci_config_read_register(bus, device, fn, 0xC));
            kprintf("AHCI controller fn - %d, bar5 - %x\n", fn, PCI_BAR5(bus, device, fn));
            kprintf("AHCI controller fn - %d, bar4 - %x\n", fn, PCI_BAR4(bus, device, fn));
            /*for (fn = 1; fn < 8;fn++)
              if (PCI_VENDOR(bus, device, fn) != 0xFFFF)
                 kprintf("Multi-function AHCI.. Please re-configure\n");
            */
         }

   return 0;
}

void* pci_ahci_abr()
{
   return ahci_abar;
} 
