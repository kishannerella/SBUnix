.text

.globl uswitch_task
.globl save_child_state
.align 8

#EXTERN user_main
/* rdi - old thread
   rsi - new thread
   rdx - last thread &last_task*/
uswitch_task:
   movq %rdi, %r8 
   movq (%rsp), %rax

//   pushq %rdx 
   movq %r15, 0(%r8) 
   movq %r14, 8(%r8) 
   movq %r13,16(%r8) 
   movq %r12,24(%r8) 
   movq %rbp,32(%r8) 
   movq %rsp,40(%r8) 
   movq %rbx,48(%r8)
   movq %cr3, %r15
   movq %r15,64(%r8)
   # move return address into rip 
   movq %rax,56(%r8) 

  /* Get the new thread contents*/ 
   movq  %rsi, %r8
   movq  0(%r8), %r15  
   movq  8(%r8), %r14
   movq 16(%r8), %r13
   movq 24(%r8), %r12
   movq 32(%r8), %rbp
   movq 80(%r8), %rsp          # Since we're moving into userspace, change to user register
   movq 48(%r8), %rbx
   movq 56(%r8), %rax
   pushq $0x23        #SS
   pushq %rsp         #stack pointer
   pushf              #push rflags
   pushq $0x2B        #CS
   pushq %rax         #rip
   #movq %rax, (%rsp) 
//   popq %rdx
//   movq %rdi, (%rdx)
   #movq 64(%r8), %rax
   #movq %rax, %cr3
   iretq

 
/* rdi - parent 
   rsi - child
*/
save_child_state: 
   movq 64(%rsp), %rax
   movq %rax, 56(%rsi) //Move this to child rip
   #movq %rsp, %rax
   #add  $64, %rax
   #movq %rax, 40(%rsi) // Stack point for the child
#   movq %rsp, %rax
#   movq %rax, 40(%rsi) // Hack to make child and parent stack consistent
#   movq %rbx, 48(%rsi)
   retq

flush_tlb: 
   movq %cr3, %rax
   movq %rax, %cr3
   retq
