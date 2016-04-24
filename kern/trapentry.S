/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps


/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
.text
TRAPHANDLER_NOEC(thndlr0, 0)
TRAPHANDLER_NOEC(thndlr1, 1)
TRAPHANDLER_NOEC(thndlr2, 2)
TRAPHANDLER_NOEC(thndlr3, 3)
TRAPHANDLER_NOEC(thndlr4, 4)
TRAPHANDLER_NOEC(thndlr5, 5)
TRAPHANDLER_NOEC(thndlr6, 6)
TRAPHANDLER_NOEC(thndlr7, 7)
TRAPHANDLER	(thndlr8, 8)
TRAPHANDLER_NOEC(thndlr9, 9)
TRAPHANDLER	(thndlr10, 10)
TRAPHANDLER	(thndlr11, 11)
TRAPHANDLER	(thndlr12, 12)
TRAPHANDLER	(thndlr13, 13)
TRAPHANDLER	(thndlr14, 14)
TRAPHANDLER_NOEC(thndlr15, 15)
TRAPHANDLER_NOEC(thndlr16, 16)
TRAPHANDLER	(thndlr17, 17)
TRAPHANDLER_NOEC(thndlr18, 18)
TRAPHANDLER_NOEC(thndlr19, 19)
TRAPHANDLER_NOEC(thndlr20, 20)
TRAPHANDLER_NOEC(thndlr21, 21)
TRAPHANDLER_NOEC(thndlr22, 22)
TRAPHANDLER_NOEC(thndlr23, 23)
TRAPHANDLER_NOEC(thndlr24, 24)
TRAPHANDLER_NOEC(thndlr25, 25)
TRAPHANDLER_NOEC(thndlr26, 26)
TRAPHANDLER_NOEC(thndlr27, 27)
TRAPHANDLER_NOEC(thndlr28, 28)
TRAPHANDLER_NOEC(thndlr29, 29)
TRAPHANDLER_NOEC(thndlr30, 30)
TRAPHANDLER_NOEC(thndlr31, 31)
TRAPHANDLER_NOEC(thndlr32, 32)
TRAPHANDLER_NOEC(thndlr33, 33)
TRAPHANDLER_NOEC(thndlr34, 34)
TRAPHANDLER_NOEC(thndlr35, 35)
TRAPHANDLER_NOEC(thndlr36, 36)
TRAPHANDLER_NOEC(thndlr37, 37)
TRAPHANDLER_NOEC(thndlr38, 38)
TRAPHANDLER_NOEC(thndlr39, 39)
TRAPHANDLER_NOEC(thndlr40, 40)
TRAPHANDLER_NOEC(thndlr41, 41)
TRAPHANDLER_NOEC(thndlr42, 42)
TRAPHANDLER_NOEC(thndlr43, 43)
TRAPHANDLER_NOEC(thndlr44, 44)
TRAPHANDLER_NOEC(thndlr45, 45)
TRAPHANDLER_NOEC(thndlr46, 46)
TRAPHANDLER_NOEC(thndlr47, 47)
TRAPHANDLER_NOEC(thndlr48, 48)
TRAPHANDLER_NOEC(thndlr49, 49)
TRAPHANDLER_NOEC(thndlr50, 50)
TRAPHANDLER_NOEC(thndlr51, 51)
TRAPHANDLER_NOEC(thndlr52, 52)
TRAPHANDLER_NOEC(thndlr53, 53)
TRAPHANDLER_NOEC(thndlr54, 54)
TRAPHANDLER_NOEC(thndlr55, 55)
TRAPHANDLER_NOEC(thndlr56, 56)
TRAPHANDLER_NOEC(thndlr57, 57)
TRAPHANDLER_NOEC(thndlr58, 58)
TRAPHANDLER_NOEC(thndlr59, 59)
TRAPHANDLER_NOEC(thndlr60, 60)
TRAPHANDLER_NOEC(thndlr61, 61)
TRAPHANDLER_NOEC(thndlr62, 62)
TRAPHANDLER_NOEC(thndlr63, 63)
TRAPHANDLER_NOEC(thndlr64, 64)
TRAPHANDLER_NOEC(thndlr65, 65)
TRAPHANDLER_NOEC(thndlr66, 66)
TRAPHANDLER_NOEC(thndlr67, 67)
TRAPHANDLER_NOEC(thndlr68, 68)
TRAPHANDLER_NOEC(thndlr69, 69)
TRAPHANDLER_NOEC(thndlr70, 70)
TRAPHANDLER_NOEC(thndlr71, 71)
TRAPHANDLER_NOEC(thndlr72, 72)
TRAPHANDLER_NOEC(thndlr73, 73)
TRAPHANDLER_NOEC(thndlr74, 74)
TRAPHANDLER_NOEC(thndlr75, 75)
TRAPHANDLER_NOEC(thndlr76, 76)
TRAPHANDLER_NOEC(thndlr77, 77)
TRAPHANDLER_NOEC(thndlr78, 78)
TRAPHANDLER_NOEC(thndlr79, 79)
TRAPHANDLER_NOEC(thndlr80, 80)
TRAPHANDLER_NOEC(thndlr81, 81)
TRAPHANDLER_NOEC(thndlr82, 82)
TRAPHANDLER_NOEC(thndlr83, 83)
TRAPHANDLER_NOEC(thndlr84, 84)
TRAPHANDLER_NOEC(thndlr85, 85)
TRAPHANDLER_NOEC(thndlr86, 86)
TRAPHANDLER_NOEC(thndlr87, 87)
TRAPHANDLER_NOEC(thndlr88, 88)
TRAPHANDLER_NOEC(thndlr89, 89)
TRAPHANDLER_NOEC(thndlr90, 90)
TRAPHANDLER_NOEC(thndlr91, 91)
TRAPHANDLER_NOEC(thndlr92, 92)
TRAPHANDLER_NOEC(thndlr93, 93)
TRAPHANDLER_NOEC(thndlr94, 94)
TRAPHANDLER_NOEC(thndlr95, 95)
TRAPHANDLER_NOEC(thndlr96, 96)
TRAPHANDLER_NOEC(thndlr97, 97)
TRAPHANDLER_NOEC(thndlr98, 98)
TRAPHANDLER_NOEC(thndlr99, 99)
TRAPHANDLER_NOEC(thndlr100, 100)
TRAPHANDLER_NOEC(thndlr101, 101)
TRAPHANDLER_NOEC(thndlr102, 102)
TRAPHANDLER_NOEC(thndlr103, 103)
TRAPHANDLER_NOEC(thndlr104, 104)
TRAPHANDLER_NOEC(thndlr105, 105)
TRAPHANDLER_NOEC(thndlr106, 106)
TRAPHANDLER_NOEC(thndlr107, 107)
TRAPHANDLER_NOEC(thndlr108, 108)
TRAPHANDLER_NOEC(thndlr109, 109)
TRAPHANDLER_NOEC(thndlr110, 110)
TRAPHANDLER_NOEC(thndlr111, 111)
TRAPHANDLER_NOEC(thndlr112, 112)
TRAPHANDLER_NOEC(thndlr113, 113)
TRAPHANDLER_NOEC(thndlr114, 114)
TRAPHANDLER_NOEC(thndlr115, 115)
TRAPHANDLER_NOEC(thndlr116, 116)
TRAPHANDLER_NOEC(thndlr117, 117)
TRAPHANDLER_NOEC(thndlr118, 118)
TRAPHANDLER_NOEC(thndlr119, 119)
TRAPHANDLER_NOEC(thndlr120, 120)
TRAPHANDLER_NOEC(thndlr121, 121)
TRAPHANDLER_NOEC(thndlr122, 122)
TRAPHANDLER_NOEC(thndlr123, 123)
TRAPHANDLER_NOEC(thndlr124, 124)
TRAPHANDLER_NOEC(thndlr125, 125)
TRAPHANDLER_NOEC(thndlr126, 126)
TRAPHANDLER_NOEC(thndlr127, 127)
TRAPHANDLER_NOEC(thndlr128, 128)
TRAPHANDLER_NOEC(thndlr129, 129)
TRAPHANDLER_NOEC(thndlr130, 130)
TRAPHANDLER_NOEC(thndlr131, 131)
TRAPHANDLER_NOEC(thndlr132, 132)
TRAPHANDLER_NOEC(thndlr133, 133)
TRAPHANDLER_NOEC(thndlr134, 134)
TRAPHANDLER_NOEC(thndlr135, 135)
TRAPHANDLER_NOEC(thndlr136, 136)
TRAPHANDLER_NOEC(thndlr137, 137)
TRAPHANDLER_NOEC(thndlr138, 138)
TRAPHANDLER_NOEC(thndlr139, 139)
TRAPHANDLER_NOEC(thndlr140, 140)
TRAPHANDLER_NOEC(thndlr141, 141)
TRAPHANDLER_NOEC(thndlr142, 142)
TRAPHANDLER_NOEC(thndlr143, 143)
TRAPHANDLER_NOEC(thndlr144, 144)
TRAPHANDLER_NOEC(thndlr145, 145)
TRAPHANDLER_NOEC(thndlr146, 146)
TRAPHANDLER_NOEC(thndlr147, 147)
TRAPHANDLER_NOEC(thndlr148, 148)
TRAPHANDLER_NOEC(thndlr149, 149)
TRAPHANDLER_NOEC(thndlr150, 150)
TRAPHANDLER_NOEC(thndlr151, 151)
TRAPHANDLER_NOEC(thndlr152, 152)
TRAPHANDLER_NOEC(thndlr153, 153)
TRAPHANDLER_NOEC(thndlr154, 154)
TRAPHANDLER_NOEC(thndlr155, 155)
TRAPHANDLER_NOEC(thndlr156, 156)
TRAPHANDLER_NOEC(thndlr157, 157)
TRAPHANDLER_NOEC(thndlr158, 158)
TRAPHANDLER_NOEC(thndlr159, 159)
TRAPHANDLER_NOEC(thndlr160, 160)
TRAPHANDLER_NOEC(thndlr161, 161)
TRAPHANDLER_NOEC(thndlr162, 162)
TRAPHANDLER_NOEC(thndlr163, 163)
TRAPHANDLER_NOEC(thndlr164, 164)
TRAPHANDLER_NOEC(thndlr165, 165)
TRAPHANDLER_NOEC(thndlr166, 166)
TRAPHANDLER_NOEC(thndlr167, 167)
TRAPHANDLER_NOEC(thndlr168, 168)
TRAPHANDLER_NOEC(thndlr169, 169)
TRAPHANDLER_NOEC(thndlr170, 170)
TRAPHANDLER_NOEC(thndlr171, 171)
TRAPHANDLER_NOEC(thndlr172, 172)
TRAPHANDLER_NOEC(thndlr173, 173)
TRAPHANDLER_NOEC(thndlr174, 174)
TRAPHANDLER_NOEC(thndlr175, 175)
TRAPHANDLER_NOEC(thndlr176, 176)
TRAPHANDLER_NOEC(thndlr177, 177)
TRAPHANDLER_NOEC(thndlr178, 178)
TRAPHANDLER_NOEC(thndlr179, 179)
TRAPHANDLER_NOEC(thndlr180, 180)
TRAPHANDLER_NOEC(thndlr181, 181)
TRAPHANDLER_NOEC(thndlr182, 182)
TRAPHANDLER_NOEC(thndlr183, 183)
TRAPHANDLER_NOEC(thndlr184, 184)
TRAPHANDLER_NOEC(thndlr185, 185)
TRAPHANDLER_NOEC(thndlr186, 186)
TRAPHANDLER_NOEC(thndlr187, 187)
TRAPHANDLER_NOEC(thndlr188, 188)
TRAPHANDLER_NOEC(thndlr189, 189)
TRAPHANDLER_NOEC(thndlr190, 190)
TRAPHANDLER_NOEC(thndlr191, 191)
TRAPHANDLER_NOEC(thndlr192, 192)
TRAPHANDLER_NOEC(thndlr193, 193)
TRAPHANDLER_NOEC(thndlr194, 194)
TRAPHANDLER_NOEC(thndlr195, 195)
TRAPHANDLER_NOEC(thndlr196, 196)
TRAPHANDLER_NOEC(thndlr197, 197)
TRAPHANDLER_NOEC(thndlr198, 198)
TRAPHANDLER_NOEC(thndlr199, 199)
TRAPHANDLER_NOEC(thndlr200, 200)
TRAPHANDLER_NOEC(thndlr201, 201)
TRAPHANDLER_NOEC(thndlr202, 202)
TRAPHANDLER_NOEC(thndlr203, 203)
TRAPHANDLER_NOEC(thndlr204, 204)
TRAPHANDLER_NOEC(thndlr205, 205)
TRAPHANDLER_NOEC(thndlr206, 206)
TRAPHANDLER_NOEC(thndlr207, 207)
TRAPHANDLER_NOEC(thndlr208, 208)
TRAPHANDLER_NOEC(thndlr209, 209)
TRAPHANDLER_NOEC(thndlr210, 210)
TRAPHANDLER_NOEC(thndlr211, 211)
TRAPHANDLER_NOEC(thndlr212, 212)
TRAPHANDLER_NOEC(thndlr213, 213)
TRAPHANDLER_NOEC(thndlr214, 214)
TRAPHANDLER_NOEC(thndlr215, 215)
TRAPHANDLER_NOEC(thndlr216, 216)
TRAPHANDLER_NOEC(thndlr217, 217)
TRAPHANDLER_NOEC(thndlr218, 218)
TRAPHANDLER_NOEC(thndlr219, 219)
TRAPHANDLER_NOEC(thndlr220, 220)
TRAPHANDLER_NOEC(thndlr221, 221)
TRAPHANDLER_NOEC(thndlr222, 222)
TRAPHANDLER_NOEC(thndlr223, 223)
TRAPHANDLER_NOEC(thndlr224, 224)
TRAPHANDLER_NOEC(thndlr225, 225)
TRAPHANDLER_NOEC(thndlr226, 226)
TRAPHANDLER_NOEC(thndlr227, 227)
TRAPHANDLER_NOEC(thndlr228, 228)
TRAPHANDLER_NOEC(thndlr229, 229)
TRAPHANDLER_NOEC(thndlr230, 230)
TRAPHANDLER_NOEC(thndlr231, 231)
TRAPHANDLER_NOEC(thndlr232, 232)
TRAPHANDLER_NOEC(thndlr233, 233)
TRAPHANDLER_NOEC(thndlr234, 234)
TRAPHANDLER_NOEC(thndlr235, 235)
TRAPHANDLER_NOEC(thndlr236, 236)
TRAPHANDLER_NOEC(thndlr237, 237)
TRAPHANDLER_NOEC(thndlr238, 238)
TRAPHANDLER_NOEC(thndlr239, 239)
TRAPHANDLER_NOEC(thndlr240, 240)
TRAPHANDLER_NOEC(thndlr241, 241)
TRAPHANDLER_NOEC(thndlr242, 242)
TRAPHANDLER_NOEC(thndlr243, 243)
TRAPHANDLER_NOEC(thndlr244, 244)
TRAPHANDLER_NOEC(thndlr245, 245)
TRAPHANDLER_NOEC(thndlr246, 246)
TRAPHANDLER_NOEC(thndlr247, 247)
TRAPHANDLER_NOEC(thndlr248, 248)
TRAPHANDLER_NOEC(thndlr249, 249)
TRAPHANDLER_NOEC(thndlr250, 250)
TRAPHANDLER_NOEC(thndlr251, 251)
TRAPHANDLER_NOEC(thndlr252, 252)
TRAPHANDLER_NOEC(thndlr253, 253)
TRAPHANDLER_NOEC(thndlr254, 254)
TRAPHANDLER_NOEC(thndlr255, 255)


/*
 * Lab 3: Your code here for _alltraps
 */
.data
    .p2align 2
    .globl traphndlrs
traphndlrs:
.long thndlr0
.long thndlr1
.long thndlr2
.long thndlr3
.long thndlr4
.long thndlr5
.long thndlr6
.long thndlr7
.long thndlr8
.long thndlr9
.long thndlr10
.long thndlr11
.long thndlr12
.long thndlr13
.long thndlr14
.long thndlr15
.long thndlr16
.long thndlr17
.long thndlr18
.long thndlr19
.long thndlr20
.long thndlr21
.long thndlr22
.long thndlr23
.long thndlr24
.long thndlr25
.long thndlr26
.long thndlr27
.long thndlr28
.long thndlr29
.long thndlr30
.long thndlr31
.long thndlr32
.long thndlr33
.long thndlr34
.long thndlr35
.long thndlr36
.long thndlr37
.long thndlr38
.long thndlr39
.long thndlr40
.long thndlr41
.long thndlr42
.long thndlr43
.long thndlr44
.long thndlr45
.long thndlr46
.long thndlr47
.long thndlr48
.long thndlr49
.long thndlr50
.long thndlr51
.long thndlr52
.long thndlr53
.long thndlr54
.long thndlr55
.long thndlr56
.long thndlr57
.long thndlr58
.long thndlr59
.long thndlr60
.long thndlr61
.long thndlr62
.long thndlr63
.long thndlr64
.long thndlr65
.long thndlr66
.long thndlr67
.long thndlr68
.long thndlr69
.long thndlr70
.long thndlr71
.long thndlr72
.long thndlr73
.long thndlr74
.long thndlr75
.long thndlr76
.long thndlr77
.long thndlr78
.long thndlr79
.long thndlr80
.long thndlr81
.long thndlr82
.long thndlr83
.long thndlr84
.long thndlr85
.long thndlr86
.long thndlr87
.long thndlr88
.long thndlr89
.long thndlr90
.long thndlr91
.long thndlr92
.long thndlr93
.long thndlr94
.long thndlr95
.long thndlr96
.long thndlr97
.long thndlr98
.long thndlr99
.long thndlr100
.long thndlr101
.long thndlr102
.long thndlr103
.long thndlr104
.long thndlr105
.long thndlr106
.long thndlr107
.long thndlr108
.long thndlr109
.long thndlr110
.long thndlr111
.long thndlr112
.long thndlr113
.long thndlr114
.long thndlr115
.long thndlr116
.long thndlr117
.long thndlr118
.long thndlr119
.long thndlr120
.long thndlr121
.long thndlr122
.long thndlr123
.long thndlr124
.long thndlr125
.long thndlr126
.long thndlr127
.long thndlr128
.long thndlr129
.long thndlr130
.long thndlr131
.long thndlr132
.long thndlr133
.long thndlr134
.long thndlr135
.long thndlr136
.long thndlr137
.long thndlr138
.long thndlr139
.long thndlr140
.long thndlr141
.long thndlr142
.long thndlr143
.long thndlr144
.long thndlr145
.long thndlr146
.long thndlr147
.long thndlr148
.long thndlr149
.long thndlr150
.long thndlr151
.long thndlr152
.long thndlr153
.long thndlr154
.long thndlr155
.long thndlr156
.long thndlr157
.long thndlr158
.long thndlr159
.long thndlr160
.long thndlr161
.long thndlr162
.long thndlr163
.long thndlr164
.long thndlr165
.long thndlr166
.long thndlr167
.long thndlr168
.long thndlr169
.long thndlr170
.long thndlr171
.long thndlr172
.long thndlr173
.long thndlr174
.long thndlr175
.long thndlr176
.long thndlr177
.long thndlr178
.long thndlr179
.long thndlr180
.long thndlr181
.long thndlr182
.long thndlr183
.long thndlr184
.long thndlr185
.long thndlr186
.long thndlr187
.long thndlr188
.long thndlr189
.long thndlr190
.long thndlr191
.long thndlr192
.long thndlr193
.long thndlr194
.long thndlr195
.long thndlr196
.long thndlr197
.long thndlr198
.long thndlr199
.long thndlr200
.long thndlr201
.long thndlr202
.long thndlr203
.long thndlr204
.long thndlr205
.long thndlr206
.long thndlr207
.long thndlr208
.long thndlr209
.long thndlr210
.long thndlr211
.long thndlr212
.long thndlr213
.long thndlr214
.long thndlr215
.long thndlr216
.long thndlr217
.long thndlr218
.long thndlr219
.long thndlr220
.long thndlr221
.long thndlr222
.long thndlr223
.long thndlr224
.long thndlr225
.long thndlr226
.long thndlr227
.long thndlr228
.long thndlr229
.long thndlr230
.long thndlr231
.long thndlr232
.long thndlr233
.long thndlr234
.long thndlr235
.long thndlr236
.long thndlr237
.long thndlr238
.long thndlr239
.long thndlr240
.long thndlr241
.long thndlr242
.long thndlr243
.long thndlr244
.long thndlr245
.long thndlr246
.long thndlr247
.long thndlr248
.long thndlr249
.long thndlr250
.long thndlr251
.long thndlr252
.long thndlr253
.long thndlr254
.long thndlr255


.globl _alltraps
_alltraps:
	pushl %ds;
	pushl %es;
	pushal ;
	pushl $GD_KD; popl %ds;
	pushl $GD_KD; popl %es;
	pushl %esp;
	call trap;
