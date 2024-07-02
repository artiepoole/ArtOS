.extern exception_handler



.macro isr_err_stub_ i
    isr_stub_\i:
    call exception_handler
    iret
.endm

.macro isr_no_err_stub_ i
    isr_stub_\i:
    call exception_handler
    iret
.endm



isr_no_err_stub_ 0
isr_no_err_stub_ 1
isr_no_err_stub_ 2
isr_no_err_stub_ 3
isr_no_err_stub_ 4
isr_no_err_stub_ 5
isr_no_err_stub_ 6
isr_no_err_stub_ 7
isr_err_stub_    8
isr_no_err_stub_ 9
isr_err_stub_    10
isr_err_stub_    11
isr_err_stub_    12
isr_err_stub_    13
isr_err_stub_    14
isr_no_err_stub_ 15
isr_no_err_stub_ 16
isr_err_stub_    17
isr_no_err_stub_ 18
isr_no_err_stub_ 19
isr_no_err_stub_ 20
isr_no_err_stub_ 21
isr_no_err_stub_ 22
isr_no_err_stub_ 23
isr_no_err_stub_ 24
isr_no_err_stub_ 25
isr_no_err_stub_ 26
isr_no_err_stub_ 27
isr_no_err_stub_ 28
isr_no_err_stub_ 29
isr_err_stub_    30
isr_no_err_stub_ 31


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

