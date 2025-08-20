/*
 * written by dhjung
 */
	.global		checkmagic

checkmagic :
	bal     endofmagic
	nop
	.byte	'B','O','O','T'
	.long	uboot_end_data - TEXT_BASE
endofmagic :
