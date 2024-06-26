#ifndef _CONTEXT_H
#define _CONTEXT_H

#define save_ctx()                                                        \
    asm volatile("push r0                   \n\t"                              \
                 "push r1                   \n\t"                              \
                 "clr r1                    \n\t"                              \
                 "push r2                   \n\t"                              \
                 "push r3                   \n\t"                              \
                 "push r4                   \n\t"                              \
                 "push r5                   \n\t"                              \
                 "push r6                   \n\t"                              \
                 "push r7                   \n\t"                              \
                 "push r8                   \n\t"                              \
                 "push r9                   \n\t"                              \
                 "push r10                  \n\t"                              \
                 "push r11                  \n\t"                              \
                 "push r12                  \n\t"                              \
                 "push r13                  \n\t"                              \
                 "push r14                  \n\t"                              \
                 "push r15                  \n\t"                              \
                 "push r16                  \n\t"                              \
                 "push r17                  \n\t"                              \
                 "in r17, __SREG__          \n\t"                              \
                 "cli                       \n\t"                              \
                 "push r17                  \n\t"                              \
                 "push r18                  \n\t"                              \
                 "push r19                  \n\t"                              \
                 "push r20                  \n\t"                              \
                 "push r21                  \n\t"                              \
                 "push r22                  \n\t"                              \
                 "push r23                  \n\t"                              \
                 "push r24                  \n\t"                              \
                 "push r25                  \n\t"                              \
                 "push r26                  \n\t"                              \
                 "push r27                  \n\t"                              \
                 "push r28                  \n\t"                              \
                 "push r29                  \n\t"                              \
                 "push r30                  \n\t"                              \
                 "push r31                  \n\t"                              \
                 "ldi r26, lo8(stack_exe)   \n\t"                              \
                 "ldi r27, hi8(stack_exe)   \n\t"                              \
                 "in r0, __SP_L__           \n\t"                              \
                 "st x+, r0                 \n\t"                              \
                 "in r0, __SP_H__           \n\t"                              \
                 "st x+, r0                 \n\t"                              \
                 );


#define restore_ctx()                                                          \
    asm volatile("ldi r26, lo8(stack_exe)   \n\t"                              \
                 "ldi r27, hi8(stack_exe)   \n\t"                              \
                 "ld r28, x+                \n\t"                              \
                 "out __SP_L__, r28         \n\t"                              \
                 "ld r29, x+                \n\t"                              \
                 "out __SP_H__, r29         \n\t"                              \
                 "pop r31                   \n\t"                              \
                 "pop r30                   \n\t"                              \
                 "pop r29                   \n\t"                              \
                 "pop r28                   \n\t"                              \
                 "pop r27                   \n\t"                              \
                 "pop r26                   \n\t"                              \
                 "pop r25                   \n\t"                              \
                 "pop r24                   \n\t"                              \
                 "pop r23                   \n\t"                              \
                 "pop r22                   \n\t"                              \
                 "pop r21                   \n\t"                              \
                 "pop r20                   \n\t"                              \
                 "pop r19                   \n\t"                              \
                 "pop r18                   \n\t"                              \
                 "pop r17                   \n\t"                              \
                 "in r16, __SREG__           \n\t"                             \
                 "ori r17, 0x80              \n\t"                             \
                 "ori r16, 0x7F              \n\t"                             \
                 "and r17, r16                \n\t"                            \
                 "out __SREG__, r17          \n\t"                             \
                 "pop r17                    \n\n"                             \
                 "pop r16                   \n\t"                              \
                 "pop r15                   \n\t"                              \
                 "pop r14                   \n\t"                              \
                 "pop r13                   \n\t"                              \
                 "pop r12                   \n\t"                              \
                 "pop r11                   \n\t"                              \
                 "pop r10                   \n\t"                              \
                 "pop r9                    \n\t"                              \
                 "pop r8                    \n\t"                              \
                 "pop r7                    \n\t"                              \
                 "pop r6                    \n\t"                              \
                 "pop r5                    \n\t"                              \
                 "pop r4                    \n\t"                              \
                 "pop r3                    \n\t"                              \
                 "pop r2                    \n\t"                              \
                 "pop r1                    \n\t"                              \
                 "pop r0                    \n\t");


#endif // _CONTEXT_H
