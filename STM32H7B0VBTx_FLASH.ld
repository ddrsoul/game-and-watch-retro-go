/*
******************************************************************************
**

**  File        : LinkerScript.ld
**
**  Author		: Auto-generated by System Workbench for STM32
**
**  Abstract    : Linker script for STM32H7B0VBTx series
**                128Kbytes FLASH and 1216Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed “as is,” without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** <h2><center>&copy; COPYRIGHT(c) 2019 STMicroelectronics</center></h2>
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**   1. Redistributions of source code must retain the above copyright notice,
**      this list of conditions and the following disclaimer.
**   2. Redistributions in binary form must reproduce the above copyright notice,
**      this list of conditions and the following disclaimer in the documentation
**      and/or other materials provided with the distribution.
**   3. Neither the name of STMicroelectronics nor the names of its contributors
**      may be used to endorse or promote products derived from this software
**      without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = _stack_top;    /* end of RAM */

/* Generate a link error if heap and stack don't fit into RAM */
_Heap_Size = 32 * 1024;     /* required amount of heap */

_Stack_Redzone_Size = 256;  /* size of stack overflow protection */
_Min_Stack_Size = 20 * 1024; /* required amount of stack */


/* Useful for scripts */
//__NULLPTR_LENGTH__  = 0x100;
__NULLPTR_LENGTH__  = 0x0;
__ITCMRAM_LENGTH__  = 64K;
__DTCMRAM_LENGTH__  = 128K;
__RAM_UC_LENGTH__   = 300K;
__RAM_CORE_LENGTH__ = 0K;
__RAM_EMU_LENGTH__  = 1024K - __RAM_UC_LENGTH__ - __RAM_CORE_LENGTH__;
__AHBRAM_LENGTH__   = 128K;
__FLASH_LENGTH__    = DEFINED(__FLASH_LENGTH__) ? __FLASH_LENGTH__ : 128k;
__FLASH2_LENGTH__   = 256K; /* bank2 */

__EXTFLASH_TOTAL_LENGTH__ = DEFINED(__EXTFLASH_TOTAL_LENGTH__) ? __EXTFLASH_TOTAL_LENGTH__ : 1024K;
__EXTFLASH_OFFSET__ = DEFINED(__EXTFLASH_OFFSET__) ? __EXTFLASH_OFFSET__ : 0;
__EXTFLASH_BASE__ = 0x90000000;  /* Beginning of addressable extflash addresses */
__EXTFLASH_START__ = __EXTFLASH_BASE__ + __EXTFLASH_OFFSET__;  /* Beginning of where we're going to store emulator and rom data */

__RAM_START__       = 0x24000000;
__RAM_UC_START__    = __RAM_START__;
__RAM_CORE_START__  = __RAM_START__ + __RAM_UC_LENGTH__;
__RAM_EMU_START__   = __RAM_CORE_START__ + __RAM_CORE_LENGTH__;

__RAM_EMU_END__     = __RAM_EMU_START__ + __RAM_EMU_LENGTH__;

/* saveflash.ld sets __SAVEFLASH_LENGTH__ */
/* offsaveflash.ld sets __OFFSAVEFLASH_LENGTH__ */
INCLUDE build/saveflash.ld
INCLUDE build/cacheflash.ld
INCLUDE build/offsaveflash.ld
__CONFIGFLASH_LENGTH__ = 4096;
__FBFLASH_LENGTH__ = ENABLE_SCREENSHOT ? ((320 * 240 * 2 + 4095) / 4096) * 4096 : 0;

/****
 * External Flash Layout
 *
 * 0x90000000 +----------------------------------+  __EXTFLASH_BASE__
 *            |                                  |
 *            |        Untouched data if         |
 *            | __EXTFLASH_OFFSET__ is specified |
 *            |                                  |
 *            +----------------------------------+  __EXTFLASH_START__
 *            |                                  |
 *            |   Executable emulator data and   |
 *            |         constant ROM data        |
 *            |                                  |
 *            +----------------------------------+  __EXTFLASH_END__ __CACHEFLASH_START__
 *            |                                  |
 *            |          Reserved space          |
 *            |    to decompress Roms not        |
 *            |         fitting in RAM           |
 *            +----------------------------------+  __CACHEFLASH_END__  __OFFSAVEFLASH_START__
 *            |                                  |
 *            |        ON/OFF state save         |
 *            |                                  |
 *            +----------------------------------+  __OFFSAVEFLASH_END__  __SAVEFLASH_START__
 *            |                                  |
 *            |          Save States             |
 *            |                                  |
 *            +----------------------------------+  __SAVEFLASH_END__  __CONFIGFLASH_START__
 *            |                                  |
 *            |    Store changable persistent    |
 *            |      retro-go configurations     |
 *            |                                  |
 *            +----------------------------------+  __CONFIGFLASH_END__
 *            |                                  |
 *            | Framebuffer / Screenshot storage |
 *            |                                  |
 *            +----------------------------------+  __FBFLASH_END__
 */
__EXTFLASH_LENGTH__ = __EXTFLASH_TOTAL_LENGTH__ - (__CACHEFLASH_LENGTH__ + __OFFSAVEFLASH_LENGTH__ + __SAVEFLASH_LENGTH__ + __CONFIGFLASH_LENGTH__ + __FBFLASH_LENGTH__);
__EXTFLASH_END__ = __EXTFLASH_START__ + __EXTFLASH_LENGTH__;
__CACHEFLASH_START__ = __EXTFLASH_END__;
__CACHEFLASH_END__   = __CACHEFLASH_START__ + __CACHEFLASH_LENGTH__;
__OFFSAVEFLASH_START__ = __CACHEFLASH_END__;
__OFFSAVEFLASH_END__   = __OFFSAVEFLASH_START__ + __OFFSAVEFLASH_LENGTH__;
__SAVEFLASH_START__ = __OFFSAVEFLASH_END__;
__SAVEFLASH_END__   = __SAVEFLASH_START__ + __SAVEFLASH_LENGTH__;
__CONFIGFLASH_START__ = __SAVEFLASH_END__;
__CONFIGFLASH_END__   = __CONFIGFLASH_START__ + __CONFIGFLASH_LENGTH__;
__FBFLASH_START__ = __CONFIGFLASH_END__;
__FBFLASH_END__   = __FBFLASH_START__ + __FBFLASH_LENGTH__;


__INTFLASH__ = DEFINED(__INTFLASH__) ? __INTFLASH__ : 0x08000000;

/* Specify the memory areas */
MEMORY
{
  /* RAM */
  NULLPTR  (xrw) : ORIGIN = 0x00000000, LENGTH =   __NULLPTR_LENGTH__
  ITCMRAM  (xrw) : ORIGIN = 0x00000000 + __NULLPTR_LENGTH__, LENGTH = __ITCMRAM_LENGTH__ - __NULLPTR_LENGTH__
  DTCMRAM  (xrw) : ORIGIN = 0x20000000, LENGTH =  __DTCMRAM_LENGTH__
  RAM_UC   (xrw) : ORIGIN = __RAM_UC_START__, LENGTH = __RAM_UC_LENGTH__
  RAM      (xrw) : ORIGIN = __RAM_CORE_START__, LENGTH = __RAM_CORE_LENGTH__
  RAM_EMU  (xrw) : ORIGIN = __RAM_EMU_START__, LENGTH = __RAM_EMU_LENGTH__
  AHBRAM   (xrw) : ORIGIN = 0x30000000, LENGTH =  __AHBRAM_LENGTH__

  /* FLASH */
  FLASH      (xr ) : ORIGIN = __INTFLASH__,          LENGTH =  __FLASH_LENGTH__
  FLASH2   (xr ) : ORIGIN = 0x08100000,          LENGTH = __FLASH2_LENGTH__
  EXTFLASH   (xr ) : ORIGIN = __EXTFLASH_START__,  LENGTH = __EXTFLASH_LENGTH__
  CACHEFLASH (xr ) : ORIGIN = __CACHEFLASH_START__,  LENGTH = __CACHEFLASH_LENGTH__
  OFFSAVEFLASH (xr ) : ORIGIN = __OFFSAVEFLASH_START__, LENGTH = __OFFSAVEFLASH_LENGTH__
  SAVEFLASH  (xr ) : ORIGIN = __SAVEFLASH_START__, LENGTH = __SAVEFLASH_LENGTH__
  CONFIGFLASH(xr ) : ORIGIN = __CONFIGFLASH_START__, LENGTH = __CONFIGFLASH_LENGTH__
  FBFLASH (xr ) : ORIGIN = __FBFLASH_START__, LENGTH = __FBFLASH_LENGTH__
}


/* Define output sections */
SECTIONS
{
  ._itcram :
  {
    __itcram_start__ = .;
    . = ALIGN(4);
    *(.itcram_data)
    . = ALIGN(4);
    *(.itcram_text)
    . = ALIGN(4);
  } > ITCMRAM

  /* Copied from flash to ram at boot */
  _sitcram_hot = LOADADDR(._itcram_hot);
  ._itcram_hot :
  {
    __itcram_hot_start__ = .;
    . = ALIGN(4);
    *(.itcram_hot_data)
    . = ALIGN(4);
    *(.itcram_hot_text)
    . = ALIGN(4);
    __itcram_hot_end__ = .;
    __itcram_end__ = .;
  } >ITCMRAM AT> EXTFLASH

  /* Uncached and unbuffered memory for the LCD framebuffers */
  ._ram_uc (NOLOAD) :
  {
    __ram_uc_start__ = .;
    . = ALIGN(4);
    *(.lcd1)
    *(.lcd2)
    __ram_uc_end__ = .;
  } > RAM_UC

  /* Data that should be copied from extflash to axiram */
  /* used by the startup to initialize data */
  _siramdata = LOADADDR(._ram_exec);
  __extflash_start__ = LOADADDR(._ram_exec);
  ._ram_exec :
  {
    __ram_start__ = .;
    __ram_exec_start__ = .;
    . = ALIGN(4);
    _sram_text = .;
    *(.ram_text)
    _eram_text = .;
    _sram_data = .;
    *(.ram_data)
    _eram_data = .;
    __ram_exec_end__ = .;
  } >RAM AT> EXTFLASH

  ._extflash :
  {
    . = ALIGN(4);
    __extflash_font_start__ = .;
    *(.extflash_font)
    . = ALIGN(4);
    __extflash_font_end__ = .;
    __extflash_emu_data_start__ = .;
    *(.extflash_emu_data)
    . = ALIGN(4);
    __extflash_emu_data_end__ = .;
    __extflash_logo_data_start__ = .;
    *(.extflash_logo)
    . = ALIGN(4);
    __extflash_logo_data_start__ = .;
    __extflash_nes_start__ = .;
    build/core/logo*.o (.text .text* .rodata .rodata*)
    . = ALIGN(4);
    build/core/header*.o (.text .text* .rodata .rodata*)
    . = ALIGN(4);
    __extflash_end_start__ = .;
    . = ALIGN(4);
    __extflash_text_start__ = .;
    *(.extflash_text)
    . = ALIGN(4);
    __extflash_text_end__ = .;
    __extflash_data_start__ = .;
    . = ALIGN(4);
    *(.extflash_data)
    . = ALIGN(4);
    __extflash_data_end__ = .;
    . = ALIGN(4K);
    __extflash_game_rom_start__ = .;
    *(.extflash_game_rom)
    __extflash_game_rom_end__ = .;
    INCLUDE build/zelda3_extflash.ld
    INCLUDE build/smw_extflash.ld
  } > EXTFLASH

  .overlay_nes __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_nes_start__ = .;
    build/nes/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_NES_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_NES_LOAD_START = LOADADDR(.overlay_nes);
  _OVERLAY_NES_SIZE = SIZEOF(.overlay_nes);

  .overlay_nes_bss _OVERLAY_NES_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_NES_BSS_START = .;
    build/nes/*.o (.bss .bss*)
    . = ALIGN(4);
    build/nes/*.o (COMMON)
    . = ALIGN(4);
    _NES_ROM_UNPACK_BUFFER = .;
    _OVERLAY_NES_BSS_END = .;
    __ram_emu_nes_end__ = .;
    ASSERT(ABSOLUTE(_OVERLAY_NES_BSS_END) < __RAM_EMU_END__, "Error: NES BSS overflow");
  }
  _OVERLAY_NES_BSS_SIZE = SIZEOF(.overlay_nes_bss);
  _NES_ROM_UNPACK_BUFFER_SIZE = __RAM_EMU_START__ + __RAM_EMU_LENGTH__ - _NES_ROM_UNPACK_BUFFER;

  .overlay_nes_fceu __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_nes_fceu_start__ = .;
    build/nes_fceu/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_NES_FCEU_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_NES_FCEU_LOAD_START = LOADADDR(.overlay_nes_fceu);
  _OVERLAY_NES_FCEU_SIZE = SIZEOF(.overlay_nes_fceu);

  .overlay_nes_fceu_bss _OVERLAY_NES_FCEU_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_NES_FCEU_BSS_START = .;
    build/nes_fceu/*.o (.bss .bss*)
    . = ALIGN(4);
    build/nes_fceu/*.o (COMMON)
    . = ALIGN(4);
    _NES_FCEU_ROM_UNPACK_BUFFER = .;
    _OVERLAY_NES_FCEU_BSS_END = .;
    __ram_emu_nes_fceu_end__ = .;
    ASSERT(ABSOLUTE(_OVERLAY_NES_FCEU_BSS_END) < __RAM_EMU_END__, "Error: NES_FCEU BSS overflow");
  }
  _OVERLAY_NES_FCEU_BSS_SIZE = SIZEOF(.overlay_nes_fceu_bss);
  _NES_FCEU_ROM_UNPACK_BUFFER_SIZE = __RAM_EMU_START__ + __RAM_EMU_LENGTH__ - _NES_FCEU_ROM_UNPACK_BUFFER;

  .overlay_gb __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_gb_start__ = .;
    build/gnuboy/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_GB_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_GB_LOAD_START = LOADADDR(.overlay_gb);
  _OVERLAY_GB_SIZE = SIZEOF(.overlay_gb);

  .overlay_gb_bss _OVERLAY_GB_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_GB_BSS_START = .;
    build/gnuboy/*.o (COMMON)
    . = ALIGN(4);
    build/gnuboy/*.o (.bss .bss*)
    _GB_ROM_UNPACK_BUFFER = .;
    _OVERLAY_GB_BSS_END = .;
    __ram_emu_gb_end__ = .;
    ASSERT(ABSOLUTE(_OVERLAY_GB_BSS_END) < __RAM_EMU_END__, "Error: GB BSS overflow");
  }
  _OVERLAY_GB_BSS_SIZE = SIZEOF(.overlay_gb_bss);
   _GB_ROM_UNPACK_BUFFER_SIZE = __RAM_EMU_START__ + __RAM_EMU_LENGTH__ - _GB_ROM_UNPACK_BUFFER;

  .overlay_sms __RAM_EMU_START__ : {
      . = ALIGN(4);
    __ram_emu_sms_start__ = .;
      build/smsplusgx/*.o (.data .data* .text .text* .rodata .rodata*)
      . = ALIGN(4);
      _OVERLAY_SMS_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_SMS_LOAD_START = LOADADDR(.overlay_sms);
  _OVERLAY_SMS_SIZE = SIZEOF(.overlay_sms);

  .overlay_sms_bss _OVERLAY_SMS_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_SMS_BSS_START = .;
    build/smsplusgx/*.o (.bss .bss*)
    . = ALIGN(4);
    build/smsplusgx/*.o (COMMON)
    __ram_emu_sms_end__ = .;
    _OVERLAY_SMS_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_SMS_BSS_END) < __RAM_EMU_END__, "Error: SMS BSS overflow");
  }
  _OVERLAY_SMS_BSS_SIZE = SIZEOF(.overlay_sms_bss);

   .overlay_pce __RAM_EMU_START__ : {
      . = ALIGN(4);
    __ram_emu_pce_start__ = .;
      build/pce/*.o (.data .data* .text .text* .rodata .rodata*)
      . = ALIGN(4);
      _OVERLAY_PCE_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_PCE_LOAD_START = LOADADDR(.overlay_pce);
  _OVERLAY_PCE_SIZE = SIZEOF(.overlay_pce);

  .overlay_pce_bss _OVERLAY_PCE_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_PCE_BSS_START = .;
    build/pce/*.o (.bss .bss*)
    . = ALIGN(4);
    build/pce/*.o (COMMON)
    . = ALIGN(4);
    _PCE_ROM_UNPACK_BUFFER = .;
    _OVERLAY_PCE_BSS_END = .;
    __ram_emu_pce_end__ = .;
    _OVERLAY_PCE_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_PCE_BSS_END) < __RAM_EMU_END__, "Error: PCE BSS overflow");
  }
  _OVERLAY_PCE_BSS_SIZE = SIZEOF(.overlay_pce_bss);
  _PCE_ROM_UNPACK_BUFFER_SIZE = __RAM_EMU_START__ + __RAM_EMU_LENGTH__ - _PCE_ROM_UNPACK_BUFFER;

  .overlay_msx __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_msx_start__ = .;
    build/msx/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_MSX_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_MSX_LOAD_START = LOADADDR(.overlay_msx);
  _OVERLAY_MSX_SIZE = SIZEOF(.overlay_msx);
  .overlay_msx_bss _OVERLAY_MSX_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_MSX_BSS_START = .;
    build/msx/*.o (.bss .bss*)
    . = ALIGN(4);
    build/msx/*.o (COMMON)
    __ram_emu_msx_end__ = .;
    _OVERLAY_MSX_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_MSX_BSS_END) < __RAM_EMU_END__, "Error: MSX BSS overflow");
  }
  _OVERLAY_MSX_BSS_SIZE = SIZEOF(.overlay_msx_bss);

  .overlay_wsv __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_wsv_start__ = .;
    build/wsv/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_WSV_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_WSV_LOAD_START = LOADADDR(.overlay_wsv);
  _OVERLAY_WSV_SIZE = SIZEOF(.overlay_wsv);
  .overlay_wsv_bss _OVERLAY_WSV_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_WSV_BSS_START = .;
    build/wsv/*.o (.bss .bss*)
    . = ALIGN(4);
    build/wsv/*.o (COMMON)
    __ram_emu_wsv_end__ = .;
    _OVERLAY_WSV_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_WSV_BSS_END) < __RAM_EMU_END__, "Error: WSV BSS overflow");
  }
  _OVERLAY_WSV_BSS_SIZE = SIZEOF(.overlay_wsv_bss);

  .overlay_gw __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_gw_start__ = .;
    build/gw/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_GW_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_GW_LOAD_START = LOADADDR(.overlay_gw);
  _OVERLAY_GW_SIZE = SIZEOF(.overlay_gw);

  .overlay_gw_bss _OVERLAY_GW_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_GW_BSS_START = .;
    build/gw/*.o (.bss .bss*)
    . = ALIGN(4);
    build/gw/*.o (COMMON)
    __ram_emu_gw_end__ = .;
    _OVERLAY_GW_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_GW_BSS_END) < __RAM_EMU_END__, "Error: GW BSS overflow");
  }
  _OVERLAY_GW_BSS_SIZE = SIZEOF(.overlay_gw_bss);

  .overlay_md __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_md_start__ = .;
    build/md/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_MD_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_MD_LOAD_START = LOADADDR(.overlay_md);
  _OVERLAY_MD_SIZE = SIZEOF(.overlay_md);

  .overlay_md_bss _OVERLAY_MD_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_MD_BSS_START = .;
    build/md/*.o (.bss .bss*)
    . = ALIGN(4);
    build/md/*.o (COMMON)
    __ram_emu_md_end__ = .;
    _OVERLAY_MD_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_MD_BSS_END) < __RAM_EMU_END__, "Error: MD BSS overflow");
  }
  _OVERLAY_MD_BSS_SIZE = SIZEOF(.overlay_md_bss);

  .overlay_a7800 __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_a7800_start__ = .;
    build/a7800/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_A7800_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_A7800_LOAD_START = LOADADDR(.overlay_a7800);
  _OVERLAY_A7800_SIZE = SIZEOF(.overlay_a7800);

  .overlay_a7800_bss _OVERLAY_A7800_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_A7800_BSS_START = .;
    build/a7800/*.o (.bss .bss*)
    . = ALIGN(4);
    build/a7800/*.o (COMMON)
    __ram_emu_a7800_end__ = .;
    _OVERLAY_A7800_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_A7800_BSS_END) < __RAM_EMU_END__, "Error: A7800 BSS overflow");
  }
  _OVERLAY_A7800_BSS_SIZE = SIZEOF(.overlay_a7800_bss);

  .overlay_amstrad __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_amstrad_start__ = .;
    build/amstrad/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_AMSTRAD_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_AMSTRAD_LOAD_START = LOADADDR(.overlay_amstrad);
  _OVERLAY_AMSTRAD_SIZE = SIZEOF(.overlay_amstrad);

  .overlay_amstrad_bss _OVERLAY_AMSTRAD_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_AMSTRAD_BSS_START = .;
    build/amstrad/*.o (.bss .bss*)
    . = ALIGN(4);
    build/amstrad/*.o (COMMON)
    __ram_emu_amstrad_end__ = .;
    _OVERLAY_AMSTRAD_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_AMSTRAD_BSS_END) < __RAM_EMU_END__, "Error: AMSTRAD BSS overflow");
  }
  _OVERLAY_AMSTRAD_BSS_SIZE = SIZEOF(.overlay_amstrad_bss);

  .overlay_zelda3 __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_zelda3_start__ = .;
    build/zelda3/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_ZELDA3_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_ZELDA3_LOAD_START = LOADADDR(.overlay_zelda3);
  _OVERLAY_ZELDA3_SIZE = SIZEOF(.overlay_zelda3);

  .overlay_zelda3_bss _OVERLAY_ZELDA3_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_ZELDA3_BSS_START = .;
    build/zelda3/*.o (.bss .bss*)
    . = ALIGN(4);
    build/zelda3/*.o (COMMON)
    __ram_emu_zelda3_end__ = .;
    _OVERLAY_ZELDA3_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_ZELDA3_BSS_END) < __RAM_EMU_END__, "Error: ZELDA3 BSS overflow");
  }
  _OVERLAY_ZELDA3_BSS_SIZE = SIZEOF(.overlay_zelda3_bss);

  .overlay_smw __RAM_EMU_START__ : {
    . = ALIGN(4);
    __ram_emu_smw_start__ = .;
    build/smw/*.o (.data .data* .text .text* .rodata .rodata*)
    . = ALIGN(4);
    _OVERLAY_SMW_LOAD_END = .;
  } AT> EXTFLASH
  _OVERLAY_SMW_LOAD_START = LOADADDR(.overlay_smw);
  _OVERLAY_SMW_SIZE = SIZEOF(.overlay_smw);

  .overlay_smw_bss _OVERLAY_SMW_LOAD_END : {
    . = ALIGN(4);
    _OVERLAY_SMW_BSS_START = .;
    build/smw/*.o (.bss .bss*)
    . = ALIGN(4);
    build/smw/*.o (COMMON)
    __ram_emu_smw_end__ = .;
    _OVERLAY_SMW_BSS_END = .;
    ASSERT(ABSOLUTE(_OVERLAY_SMW_BSS_END) < __RAM_EMU_END__, "Error: SMW BSS overflow");
  }
  _OVERLAY_SMW_BSS_SIZE = SIZEOF(.overlay_smw_bss);


  /* Place this symbol after the last overlay definition */
  .overlay_end : {
    __extflash_end__ = .;
  } AT> EXTFLASH

  ._ram_space_check_md : {
    . = . + SIZEOF(.overlay_md);
    . = . + SIZEOF(.overlay_md_bss);
  } >RAM_EMU

  ._cacheflash (NOLOAD) :
  {
    __cacheflash_start__ = .;
    *(.cacheflash)
    __cacheflash_end__ = .;
  } > CACHEFLASH

  ._offsaveflash (NOLOAD) :
  {
    __offsaveflash_start__ = .;
    *(.offsaveflash)
    __offsaveflash_end__ = .;
  } > OFFSAVEFLASH

  ._saveflash (NOLOAD) :
  {
    __saveflash_start__ = .;
    *(.saveflash)
    __saveflash_end__ = .;
  } > SAVEFLASH

  ._configflash (NOLOAD):
  {
    . = ALIGN(4K);
    __configflash_start__ = .;
    *(.configflash)
    __configflash_end__ = .;
  } > CONFIGFLASH

  ._fbflash (NOLOAD):
  {
    . = ALIGN(4K);
    __fbflash_start__ = .;
    *(.fbflash)
    __fbflash_end__ = .;
  } > FBFLASH

  ._ram (NOLOAD) :
  {
    . = ALIGN(4);
    *(.emulator_data)
    __ram_end__ = .;
  } > RAM

  ._ahbram (NOLOAD) :
  {
    __ahbram_start__ = .;
    . = ALIGN(4);
    *(.audio)
    *(.ahb)
    . = ALIGN(4);
    __ahbram_end__ = .;
  } > AHBRAM

  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    __flash_start__ = .;
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    __dtcram_start__ = .;

    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM AT> FLASH

  ._flash_end :
  {
    . = ALIGN(4);
    __flash_end__ = .;
  }

  /* put *fast access* MSX read only stuff on the second bank */
  .flash2 :
  {
    __flash2_start__ = .;
    . = ALIGN(4);
    *(.flash2)
    __flash2_end__ = .;
  } >FLASH2

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >DTCMRAM

  ._persistent (NOLOAD) :
  {
    . = ALIGN(8);
    *(.persistent)
    . = ALIGN(8);
  } >DTCMRAM

  ._user_heap :
  {
    . = ALIGN(16);
    _heap_start = . ;
    . = . + _Heap_Size;
    . = ALIGN(16);
    _heap_end = . ;
  } >DTCMRAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_stack :
  {
    /* Pad the unused area between the last symbol and the start of the redzone */
    __dtc_padding_start__ = .;
    . = . + __DTCMRAM_LENGTH__ - (_Min_Stack_Size + _Stack_Redzone_Size) - (__dtc_padding_start__ - __dtcram_start__);
    __dtc_padding_end__ = .;

    _stack_redzone = .;
    . = . + _Stack_Redzone_Size;
    _stack_bottom = .;
    . = . + _Min_Stack_Size;
    _stack_top = .;
    __dtcram_end__ = .;
  } >DTCMRAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
