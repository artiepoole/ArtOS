.extern exception_handler
.extern irq_handler

.macro isr_err_stub_ i
    isr_stub_\i:
    // errorcode already pushed and is non-zero
    push $\i
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x18, %ax    // Load the Kernel Data Segment descriptor!
    mov  %ax, %ds
    mov  %ax, %es
    mov  %ax, %fs
    mov  %ax, %gs
    mov  %esp, %eax
    push %eax // Push us the stack
    mov $exception_handler, %eax
    call *%eax // A special call, preserves the 'eip' register
    //call exception_handler
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp     // Cleans up the pushed error code and pushed ISR number
    iretl
.endm

.macro dummy_isr_ i
    isr_stub_\i:
    iretl
.endm

.macro isr_no_err_stub_ i
    isr_stub_\i:
    push $0x00000000 // no error
    push $\i
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x18, %ax    // Load the Kernel Data Segment descriptor!
    mov  %ax, %ds
    mov  %ax, %es
    mov  %ax, %fs
    mov  %ax, %gs
    mov  %esp, %eax
    push %eax // Push us the stack
    mov $exception_handler, %eax
    call *%eax // A special call, preserves the 'eip' register
    //call exception_handler
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp     // Cleans up the pushed error code and pushed ISR number
    iretl
.endm

.macro irq_stub i
    isr_stub_\i:
    push $0 // no error
    push $\i
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x18, %ax    // Load the Kernel Data Segment descriptor!
    mov  %ax, %ds
    mov  %ax, %es
    mov  %ax, %fs
    mov  %ax, %gs
    mov  %esp, %eax
    push %eax // Push us the stack
    mov $irq_handler, %eax
    call *%eax // A special call, preserves the 'eip' register
    //call exception_handler
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp     // Cleans up the pushed error code and pushed ISR number
    iretl
.endm


isr_no_err_stub_ 0 // div by zero
isr_no_err_stub_ 1 // debug exception
isr_no_err_stub_ 2 // non-maskable interrupt exception
isr_no_err_stub_ 3 // breakpoint exception
isr_no_err_stub_ 4 // into detected overflow exception
isr_no_err_stub_ 5 // out of bounds exception
isr_no_err_stub_ 6 // invalid opcode exception
isr_no_err_stub_ 7 // no coprocessor exception
isr_err_stub_    8 // double fault exception
isr_no_err_stub_ 9 // coprocessor segment overrun exception
isr_err_stub_    10 // bad TSS exception
isr_err_stub_    11 // segment not present exception
isr_err_stub_    12 // stack fault exception
isr_err_stub_    13 // general protection fault exception
isr_err_stub_    14 // page fault exception
isr_no_err_stub_ 15 // unknown interrupt exception
isr_no_err_stub_ 16 // coprocessor fault exception
isr_err_stub_    17 // alignment check exception
isr_no_err_stub_ 18 // machine check exception
isr_no_err_stub_ 19 // reserved exceptions
isr_no_err_stub_ 20 // reserved exceptions
isr_err_stub_ 21 // reserved exceptions
isr_no_err_stub_ 22 // reserved exceptions
isr_no_err_stub_ 23 // reserved exceptions
isr_no_err_stub_ 24 // reserved exceptions
isr_no_err_stub_ 25 // reserved exceptions
isr_no_err_stub_ 26 // reserved exceptions
isr_no_err_stub_ 27 // reserved exceptions
isr_no_err_stub_ 28 // reserved exceptions
isr_no_err_stub_ 29 // reserved exceptions
isr_err_stub_    30 // reserved exceptions
isr_no_err_stub_ 31 // reserved exceptions

irq_stub 32 // IRQ0
irq_stub 33 // IRQ1
irq_stub 34 // IRQ2
irq_stub 35 // IRQ3
irq_stub 36 // IRQ4
irq_stub 37 // IRQ5
irq_stub 38 // IRQ6
irq_stub 39 // IRQ7
irq_stub 40 // IRQ8
irq_stub 41 // IRQ9
irq_stub 42 // IRQ10
irq_stub 43 // IRQ11
irq_stub 44 // IRQ12
irq_stub 45 // IRQ13
irq_stub 46 // IRQ14
irq_stub 47 // IRQ15
irq_stub 255 // IRQ16 - spurious



.section	.data
.global isr_stub_table
isr_stub_table:
    .long isr_stub_0
    .long isr_stub_1
    .long isr_stub_2
    .long isr_stub_3
    .long isr_stub_4
    .long isr_stub_5
    .long isr_stub_6
    .long isr_stub_7
    .long isr_stub_8
    .long isr_stub_9
    .long isr_stub_10
    .long isr_stub_11
    .long isr_stub_12
    .long isr_stub_13
    .long isr_stub_14
    .long isr_stub_15
    .long isr_stub_16
    .long isr_stub_17
    .long isr_stub_18
    .long isr_stub_19
    .long isr_stub_20
    .long isr_stub_21
    .long isr_stub_22
    .long isr_stub_23
    .long isr_stub_24
    .long isr_stub_25
    .long isr_stub_26
    .long isr_stub_27
    .long isr_stub_28
    .long isr_stub_29
    .long isr_stub_30
    .long isr_stub_31
    .long isr_stub_32
    .long isr_stub_33
    .long isr_stub_34
    .long isr_stub_35
    .long isr_stub_36
    .long isr_stub_37
    .long isr_stub_38
    .long isr_stub_39
    .long isr_stub_40
    .long isr_stub_41
    .long isr_stub_42
    .long isr_stub_43
    .long isr_stub_44
    .long isr_stub_45
    .long isr_stub_46
    .long isr_stub_47
    .long isr_stub_255
