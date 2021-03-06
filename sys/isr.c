#include <sys/os.h>

void clr_page(void* ptr)
{
   int i;
   uint64_t* p = (uint64_t*)ptr;
   for (i = 0;i < PAGE_SIZE/8;i++,p++)
       *p = 0;
}

void add_single_page(uint64_t addr)
{
   uint64_t page = (uint64_t) _get_page();
   create_page_tables(ADDR_FLOOR(addr), ADDR_FLOOR(addr) + PAGE_SIZE - 1,
                      PHYS_ADDR((uint64_t)page), 
                      (uint64_t*)VIRT_ADDR((uint64_t)get_cur_cr3()),
                      PG_P|PG_RW|PG_U);

   clr_page((uint64_t*)page);
}

void flush_tlb()
{
   
   __asm__ __volatile__(
                        "movq %%cr3, %%rax\n\t" // Flush TLB
                        "movq %%rax, %%cr3\n\t"
                         : 
                         :
                         : "rax");
}

/* ISR for page fault.
 *   eno - Page fault error type
 *   cr2 - Address on which the fault occured
 */
void isr_page_fault(uint64_t eno, uint64_t cr2)
{
   task *t = cur_task;
   vma  *p = &t->mm_struct;
   int   i;

   p = p->next;
   while (p != NULL)
   {
      if (cr2 >= p->start && cr2 < p->end)
      {
         uint64_t vaddr = cr2;
         if (eno == 7) // Present | USER |WRITE
         {
            uint64_t pte;
            if (trans_vaddr_pt(vaddr, &pte) &&
                bit(pte, PG_COW))              // COW
            {
                uint64_t old_page = VIRT_ADDR(CL12(pte));
                uint64_t ref_count = get_pd_ref(old_page);
                if (ref_count == 1)
                {
                   /* If there is only one page referencing this,
                      just update the flags */
                   create_page_tables(ADDR_FLOOR(vaddr), 
                           ADDR_FLOOR(vaddr) + PAGE_SIZE -1,
                           CL12(pte),
                           (uint64_t*)VIRT_ADDR((uint64_t)get_cur_cr3()),
                           PG_P|PG_RW|PG_U);
                }
                else
                {
                   uint64_t new_page = (uint64_t) _get_page();
                   memcpy((char*)new_page, (char*)old_page, PAGE_SIZE);

                   /* Update the address to the new page and remove COW
                      flags */ 
                   create_page_tables(ADDR_FLOOR(vaddr), 
                           ADDR_FLOOR(vaddr) + PAGE_SIZE -1,
                           PHYS_ADDR(new_page),
                           (uint64_t*)VIRT_ADDR((uint64_t)get_cur_cr3()),
                           PG_P|PG_U|PG_RW);
                   decr_pd_ref(old_page);
                }
            }
            else
            {
                ERROR("Segmentation fault\n");
            }
         }
         else
         {
            if (!p->anon)
            {
                /* All the following complicated code is because elf64 sections
                 * need not be page aligned and fsize is different from msize */

                uint64_t new_page = (uint64_t) _get_page();
                char* cur_start_vaddr = (char*) ADDR_FLOOR(vaddr);
                char* content = (char*)p->node.fstart;      // Content starts here
                int   j   =(int)(((uint64_t)cur_start_vaddr) - (p->start));

                /* Add the new page to page tables */
                create_page_tables(ADDR_FLOOR(vaddr), 
                        ADDR_FLOOR(vaddr) + PAGE_SIZE -1,
                        PHYS_ADDR(new_page),
                        (uint64_t*)VIRT_ADDR((uint64_t)get_cur_cr3()),
                        PG_P|PG_U|PG_RW);
                flush_tlb();
                
                for (i = 0;i < PAGE_SIZE;i++,j++)
                {
                   if (j >= p->node.fsize || j < 0)
                   {
                      cur_start_vaddr[i] = 0;
                   }
                   else
                   {
                      cur_start_vaddr[i] = content[j];
                   }
                }
            }
            else
            {
                /* malloc or stack */
                add_single_page(vaddr);
            }
         }
         flush_tlb();
         return;
      }
      p = p->next;
   }

   kprintf("Segmentation fault\n");
   exit(-1);
   //ERROR("Invalid Page fault erno - %p, cr2 - %p", eno, cr2);
}
