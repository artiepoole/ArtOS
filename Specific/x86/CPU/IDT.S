/*
 * IDT_STUB_COUNT in IDT.h must be updated after adding entries to this file.
 */
.section    .trampoline.text
.extern exception_handler
.extern irq_handler

#define KERNEL_DS 0x10

.macro isr_err_stub_ i
    isr_stub_\i:
    // errorcode already pushed and is non-zero
    push $\i
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $KERNEL_DS, %ax    // Load the Kernel Data Segment descriptor!
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
    mov $KERNEL_DS, %ax    // Load the Kernel Data Segment descriptor!
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
    // This either already contains the old user_SS, useresp, eflags, cs, eip or only eflags, cs, eip.
    push $0 // no error
    push $\i
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $KERNEL_DS, %ax    // Load the Kernel Data Segment descriptor!
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
irq_stub 48 // LAPIC
irq_stub 49 // LAPIC_Calibrate
irq_stub 50
irq_stub 51
irq_stub 52
irq_stub 53
irq_stub 54
irq_stub 55
irq_stub 56
irq_stub 57
irq_stub 58
irq_stub 59
irq_stub 60
irq_stub 61
irq_stub 62
irq_stub 63
irq_stub 64
irq_stub 65
irq_stub 66
irq_stub 67
irq_stub 68
irq_stub 69
irq_stub 70
irq_stub 71
irq_stub 72
irq_stub 73
irq_stub 74
irq_stub 75
irq_stub 76
irq_stub 77
irq_stub 78
irq_stub 79
irq_stub 80
irq_stub 81
irq_stub 82
irq_stub 83
irq_stub 84
irq_stub 85
irq_stub 86
irq_stub 87
irq_stub 88
irq_stub 89
irq_stub 90
irq_stub 91
irq_stub 92
irq_stub 93
irq_stub 94
irq_stub 95
irq_stub 96
irq_stub 97
irq_stub 98
irq_stub 99
irq_stub 100
irq_stub 101
irq_stub 102
irq_stub 103
irq_stub 104
irq_stub 105
irq_stub 106
irq_stub 107
irq_stub 108
irq_stub 109
irq_stub 110
irq_stub 111
irq_stub 112
irq_stub 113
irq_stub 114
irq_stub 115
irq_stub 116
irq_stub 117
irq_stub 118
irq_stub 119
irq_stub 120
irq_stub 121
irq_stub 122
irq_stub 123
irq_stub 124
irq_stub 125
irq_stub 126
irq_stub 127
irq_stub 128 // syscall interrupts
irq_stub 129
irq_stub 130
irq_stub 131
irq_stub 132
irq_stub 133
irq_stub 134
irq_stub 135
irq_stub 136
irq_stub 137
irq_stub 138
irq_stub 139
irq_stub 140
irq_stub 141
irq_stub 142
irq_stub 143
irq_stub 144
irq_stub 145
irq_stub 146
irq_stub 147
irq_stub 148
irq_stub 149
irq_stub 150
irq_stub 151
irq_stub 152
irq_stub 153
irq_stub 154
irq_stub 155
irq_stub 156
irq_stub 157
irq_stub 158
irq_stub 159
irq_stub 160
irq_stub 161
irq_stub 162
irq_stub 163
irq_stub 164
irq_stub 165
irq_stub 166
irq_stub 167
irq_stub 168
irq_stub 169
irq_stub 170
irq_stub 171
irq_stub 172
irq_stub 173
irq_stub 174
irq_stub 175
irq_stub 176
irq_stub 177
irq_stub 178
irq_stub 179
irq_stub 180
irq_stub 181
irq_stub 182
irq_stub 183
irq_stub 184
irq_stub 185
irq_stub 186
irq_stub 187
irq_stub 188
irq_stub 189
irq_stub 190
irq_stub 191
irq_stub 192
irq_stub 193
irq_stub 194
irq_stub 195
irq_stub 196
irq_stub 197
irq_stub 198
irq_stub 199
irq_stub 200
irq_stub 201
irq_stub 202
irq_stub 203
irq_stub 204
irq_stub 205
irq_stub 206
irq_stub 207
irq_stub 208
irq_stub 209
irq_stub 210
irq_stub 211
irq_stub 212
irq_stub 213
irq_stub 214
irq_stub 215
irq_stub 216
irq_stub 217
irq_stub 218
irq_stub 219
irq_stub 220
irq_stub 221
irq_stub 222
irq_stub 223
irq_stub 224
irq_stub 225
irq_stub 226
irq_stub 227
irq_stub 228
irq_stub 229
irq_stub 230
irq_stub 231
irq_stub 232
irq_stub 233
irq_stub 234
irq_stub 235
irq_stub 236
irq_stub 237
irq_stub 238
irq_stub 239
irq_stub 240 // spurious
irq_stub 241
irq_stub 242
irq_stub 243
irq_stub 244
irq_stub 245
irq_stub 246
irq_stub 247
irq_stub 248
irq_stub 249
irq_stub 250
irq_stub 251
irq_stub 252
irq_stub 253
irq_stub 254
irq_stub 255



.section	.trampoline.rodata
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
    .long isr_stub_48
    .long isr_stub_49
    .long isr_stub_50
    .long isr_stub_51
    .long isr_stub_52
    .long isr_stub_53
    .long isr_stub_54
    .long isr_stub_55
    .long isr_stub_56
    .long isr_stub_57
    .long isr_stub_58
    .long isr_stub_59
    .long isr_stub_60
    .long isr_stub_61
    .long isr_stub_62
    .long isr_stub_63
    .long isr_stub_64
    .long isr_stub_65
    .long isr_stub_66
    .long isr_stub_67
    .long isr_stub_68
    .long isr_stub_69
    .long isr_stub_70
    .long isr_stub_71
    .long isr_stub_72
    .long isr_stub_73
    .long isr_stub_74
    .long isr_stub_75
    .long isr_stub_76
    .long isr_stub_77
    .long isr_stub_78
    .long isr_stub_79
    .long isr_stub_80
    .long isr_stub_81
    .long isr_stub_82
    .long isr_stub_83
    .long isr_stub_84
    .long isr_stub_85
    .long isr_stub_86
    .long isr_stub_87
    .long isr_stub_88
    .long isr_stub_89
    .long isr_stub_90
    .long isr_stub_91
    .long isr_stub_92
    .long isr_stub_93
    .long isr_stub_94
    .long isr_stub_95
    .long isr_stub_96
    .long isr_stub_97
    .long isr_stub_98
    .long isr_stub_99
    .long isr_stub_100
    .long isr_stub_101
    .long isr_stub_102
    .long isr_stub_103
    .long isr_stub_104
    .long isr_stub_105
    .long isr_stub_106
    .long isr_stub_107
    .long isr_stub_108
    .long isr_stub_109
    .long isr_stub_110
    .long isr_stub_111
    .long isr_stub_112
    .long isr_stub_113
    .long isr_stub_114
    .long isr_stub_115
    .long isr_stub_116
    .long isr_stub_117
    .long isr_stub_118
    .long isr_stub_119
    .long isr_stub_120
    .long isr_stub_121
    .long isr_stub_122
    .long isr_stub_123
    .long isr_stub_124
    .long isr_stub_125
    .long isr_stub_126
    .long isr_stub_127
    .long isr_stub_128
    .long isr_stub_129
    .long isr_stub_130
    .long isr_stub_131
    .long isr_stub_132
    .long isr_stub_133
    .long isr_stub_134
    .long isr_stub_135
    .long isr_stub_136
    .long isr_stub_137
    .long isr_stub_138
    .long isr_stub_139
    .long isr_stub_140
    .long isr_stub_141
    .long isr_stub_142
    .long isr_stub_143
    .long isr_stub_144
    .long isr_stub_145
    .long isr_stub_146
    .long isr_stub_147
    .long isr_stub_148
    .long isr_stub_149
    .long isr_stub_150
    .long isr_stub_151
    .long isr_stub_152
    .long isr_stub_153
    .long isr_stub_154
    .long isr_stub_155
    .long isr_stub_156
    .long isr_stub_157
    .long isr_stub_158
    .long isr_stub_159
    .long isr_stub_160
    .long isr_stub_161
    .long isr_stub_162
    .long isr_stub_163
    .long isr_stub_164
    .long isr_stub_165
    .long isr_stub_166
    .long isr_stub_167
    .long isr_stub_168
    .long isr_stub_169
    .long isr_stub_170
    .long isr_stub_171
    .long isr_stub_172
    .long isr_stub_173
    .long isr_stub_174
    .long isr_stub_175
    .long isr_stub_176
    .long isr_stub_177
    .long isr_stub_178
    .long isr_stub_179
    .long isr_stub_180
    .long isr_stub_181
    .long isr_stub_182
    .long isr_stub_183
    .long isr_stub_184
    .long isr_stub_185
    .long isr_stub_186
    .long isr_stub_187
    .long isr_stub_188
    .long isr_stub_189
    .long isr_stub_190
    .long isr_stub_191
    .long isr_stub_192
    .long isr_stub_193
    .long isr_stub_194
    .long isr_stub_195
    .long isr_stub_196
    .long isr_stub_197
    .long isr_stub_198
    .long isr_stub_199
    .long isr_stub_200
    .long isr_stub_201
    .long isr_stub_202
    .long isr_stub_203
    .long isr_stub_204
    .long isr_stub_205
    .long isr_stub_206
    .long isr_stub_207
    .long isr_stub_208
    .long isr_stub_209
    .long isr_stub_210
    .long isr_stub_211
    .long isr_stub_212
    .long isr_stub_213
    .long isr_stub_214
    .long isr_stub_215
    .long isr_stub_216
    .long isr_stub_217
    .long isr_stub_218
    .long isr_stub_219
    .long isr_stub_220
    .long isr_stub_221
    .long isr_stub_222
    .long isr_stub_223
    .long isr_stub_224
    .long isr_stub_225
    .long isr_stub_226
    .long isr_stub_227
    .long isr_stub_228
    .long isr_stub_229
    .long isr_stub_230
    .long isr_stub_231
    .long isr_stub_232
    .long isr_stub_233
    .long isr_stub_234
    .long isr_stub_235
    .long isr_stub_236
    .long isr_stub_237
    .long isr_stub_238
    .long isr_stub_239
    .long isr_stub_240
    .long isr_stub_241
    .long isr_stub_242
    .long isr_stub_243
    .long isr_stub_244
    .long isr_stub_245
    .long isr_stub_246
    .long isr_stub_247
    .long isr_stub_248
    .long isr_stub_249
    .long isr_stub_250
    .long isr_stub_251
    .long isr_stub_252
    .long isr_stub_253
    .long isr_stub_254
    .long isr_stub_255

