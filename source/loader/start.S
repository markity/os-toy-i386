.code16
.text
.global _start
/* 表示loader16_entry为外部符号, 链接的时候寻找 */
.extern loader16_entry
_start:
	/* 0x8000处的代码, 进入后直接jmp到loader16.c中loader16_entry */

	/* 疑难杂症
	16 位 jmp
	1. jmp 16位立即数									: 即为普通jmp, 相对位置浮动的跳跃(16位立即数)
	2. jmp 16位立即数, 16位立即数						 : 等价于ljmp的形式
	3. ljmp 16位立即数a, 16位立即数b			   		 : 与上一条一致, 使用段+偏移的跳转方法
	4. ljmpw *(%16/32位寄存器或$16位立即数)				 : 与上一条一个功效, 但是要求使用数据来跳转
	5. ljmpl *(%16/32位寄存器或$16位立即数)				 : 这是从16位模式下直接跳转到32位代码的一个方法
	32 位 jmp
	1. jmp即为普通jmp, 相对位置浮动的跳跃(32位立即数)
	2. ljmp 16位立即数a, 32位立即数b
	3. ljmpw
	4. ljmpl
	*/

	/* 此处是浮动跳转 */
	jmp loader16_entry

.code32
.text
.global protect_entry
.extern load_kernel
protect_entry:
	/* 体会一下看了A20总线后, 虽然操作数为16位, 但是可以通过32位寄存器jmp进1M以上内存的操作 */
	/*
	mov $0xFFFFF, %eax
	inc %eax
	mov (%eax), %ebx
	mov $0x0, %eax
	mov (%eax), %ebx
	*/

	/* 初始化数据段, 见后面定义的数据段, 此处定义了代码段和数据段 */
	mov $16, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	/* 此处是远跳转 */
	ljmp $8, $load_kernel
