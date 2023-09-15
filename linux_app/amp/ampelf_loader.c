#include "elf_loader.h"
#include "public.h"

int elf_is_64(const void *elf_info)
{
	const unsigned char *tmp = elf_info;

	if (tmp[EI_CLASS] == ELFCLASS64)
		return 1;
	else
		return 0;
}

const char * get_elf_class (unsigned int elf_class)
{
    static char buff[32];
    switch (elf_class)
    {
        case ELFCLASSNONE: return ("none");
        case ELFCLASS32:   return "ELF32";
        case ELFCLASS64:   return "ELF64";
        default:
          snprintf (buff, sizeof (buff), ("<unknown: %x>"), elf_class);
          return buff;
    }
}

const char * get_data_encoding (unsigned int encoding)
{
    static char buff[32];
    switch (encoding)
    {
        case ELFDATANONE: return ("none");
        case ELFDATA2LSB: return ("2's complement, little endian");
        case ELFDATA2MSB: return ("2's complement, big endian");
        default:
          snprintf (buff, sizeof (buff), ("<unknown: %x>"), encoding);
          return buff;
    }
}

const char * get_osabi_name (unsigned e_machine, unsigned int osabi)
{
  static char buff[32];

  switch (osabi)
    {
    case ELFOSABI_NONE:		return "UNIX - System V";
    case ELFOSABI_HPUX:		return "UNIX - HP-UX";
    case ELFOSABI_NETBSD:	return "UNIX - NetBSD";
    case ELFOSABI_GNU:		return "UNIX - GNU";
    case ELFOSABI_SOLARIS:	return "UNIX - Solaris";
    case ELFOSABI_AIX:		return "UNIX - AIX";
    case ELFOSABI_IRIX:		return "UNIX - IRIX";
    case ELFOSABI_FREEBSD:	return "UNIX - FreeBSD";
    case ELFOSABI_TRU64:	return "UNIX - TRU64";
    case ELFOSABI_MODESTO:	return "Novell - Modesto";
    case ELFOSABI_OPENBSD:	return "UNIX - OpenBSD";
    case ELFOSABI_OPENVMS:	return "VMS - OpenVMS";
    case ELFOSABI_NSK:		return "HP - Non-Stop Kernel";
    case ELFOSABI_AROS:		return "AROS";
    case ELFOSABI_FENIXOS:	return "FenixOS";
    case ELFOSABI_CLOUDABI:	return "Nuxi CloudABI";
    case ELFOSABI_OPENVOS:	return "Stratus Technologies OpenVOS";
    default:
      if (osabi >= 64)
	switch (e_machine)
	  {
	  case EM_AMDGPU:
	    switch (osabi)
	      {
	      case ELFOSABI_AMDGPU_HSA:    return "AMD HSA";
	      case ELFOSABI_AMDGPU_PAL:    return "AMD PAL";
	      case ELFOSABI_AMDGPU_MESA3D: return "AMD Mesa3D";
	      default:
		break;
	      }
	    break;

	  case EM_ARM:
	    switch (osabi)
	      {
	      case ELFOSABI_ARM:	return "ARM";
	      case ELFOSABI_ARM_FDPIC:	return "ARM FDPIC";
	      default:
		break;
	      }
	    break;

	  case EM_MSP430:
	  case EM_MSP430_OLD:
	  case EM_VISIUM:
	    switch (osabi)
	      {
	      case ELFOSABI_STANDALONE:	return ("Standalone App");
	      default:
		break;
	      }
	    break;

	  case EM_TI_C6000:
	    switch (osabi)
	      {
	      case ELFOSABI_C6000_ELFABI:	return ("Bare-metal C6000");
	      case ELFOSABI_C6000_LINUX:	return "Linux C6000";
	      default:
		break;
	      }
	    break;

	  default:
	    break;
	  }
      snprintf (buff, sizeof (buff), ("<unknown: %x>"), osabi);
      return buff;
    }
}

char * get_file_type (unsigned type)
{
  unsigned e_type = type;
  static char buff[64];

  switch (e_type)
    {
    case ET_NONE: return ("NONE (None)");
    case ET_REL:  return ("REL (Relocatable file)");
    case ET_EXEC: return ("EXEC (Executable file)");
    case ET_DYN:
#if 0
      if (is_pie (filedata))
	return _("DYN (Position-Independent Executable file)");
      else
	return _("DYN (Shared object file)");
#endif
        return ("DYN (TBD)");
    case ET_CORE: return ("CORE (Core file)");

    default:
      if ((e_type >= ET_LOPROC) && (e_type <= ET_HIPROC))
	snprintf (buff, sizeof (buff), ("Processor Specific: (%x)"), e_type);
      else if ((e_type >= ET_LOOS) && (e_type <= ET_HIOS))
	snprintf (buff, sizeof (buff), ("OS Specific: (%x)"), e_type);
      else
	snprintf (buff, sizeof (buff), ("<unknown>: %x"), e_type);
      return buff;
    }
}

char *get_machine_name (unsigned e_machine)
{
  static char buff[64]; /* XXX */

  switch (e_machine)
    {
      /* Please keep this switch table sorted by increasing EM_ value.  */
      /* 0 */
    case EM_NONE:		return ("None");
    case EM_M32:		return "WE32100";
    case EM_SPARC:		return "Sparc";
    case EM_386:		return "Intel 80386";
    case EM_68K:		return "MC68000";
    case EM_88K:		return "MC88000";
    case EM_IAMCU:		return "Intel MCU";
    case EM_860:		return "Intel 80860";
    case EM_MIPS:		return "MIPS R3000";
    case EM_S370:		return "IBM System/370";
      /* 10 */
    case EM_MIPS_RS3_LE:	return "MIPS R4000 big-endian";
    case EM_OLD_SPARCV9:	return "Sparc v9 (old)";
    case EM_PARISC:		return "HPPA";
    case EM_VPP550:		return "Fujitsu VPP500";
    case EM_SPARC32PLUS:	return "Sparc v8+" ;
    case EM_960:		return "Intel 80960";
    case EM_PPC:		return "PowerPC";
      /* 20 */
    case EM_PPC64:		return "PowerPC64";
    case EM_S390_OLD:
    case EM_S390:		return "IBM S/390";
    case EM_SPU:		return "SPU";
      /* 30 */
    case EM_V800:		return "Renesas V850 (using RH850 ABI)";
    case EM_FR20:		return "Fujitsu FR20";
    case EM_RH32:		return "TRW RH32";
    case EM_MCORE:		return "MCORE";
      /* 40 */
    case EM_ARM:		return "ARM";
    case EM_OLD_ALPHA:		return "Digital Alpha (old)";
    case EM_SH:			return "Renesas / SuperH SH";
    case EM_SPARCV9:		return "Sparc v9";
    case EM_TRICORE:		return "Siemens Tricore";
    case EM_ARC:		return "ARC";
    case EM_H8_300:		return "Renesas H8/300";
    case EM_H8_300H:		return "Renesas H8/300H";
    case EM_H8S:		return "Renesas H8S";
    case EM_H8_500:		return "Renesas H8/500";
      /* 50 */
    case EM_IA_64:		return "Intel IA-64";
    case EM_MIPS_X:		return "Stanford MIPS-X";
    case EM_COLDFIRE:		return "Motorola Coldfire";
    case EM_68HC12:		return "Motorola MC68HC12 Microcontroller";
    case EM_MMA:		return "Fujitsu Multimedia Accelerator";
    case EM_PCP:		return "Siemens PCP";
    case EM_NCPU:		return "Sony nCPU embedded RISC processor";
    case EM_NDR1:		return "Denso NDR1 microprocesspr";
    case EM_STARCORE:		return "Motorola Star*Core processor";
    case EM_ME16:		return "Toyota ME16 processor";
      /* 60 */
    case EM_ST100:		return "STMicroelectronics ST100 processor";
    case EM_TINYJ:		return "Advanced Logic Corp. TinyJ embedded processor";
    case EM_X86_64:		return "Advanced Micro Devices X86-64";
    case EM_PDSP:		return "Sony DSP processor";
    case EM_PDP10:		return "Digital Equipment Corp. PDP-10";
    case EM_PDP11:		return "Digital Equipment Corp. PDP-11";
    case EM_FX66:		return "Siemens FX66 microcontroller";
    case EM_ST9PLUS:		return "STMicroelectronics ST9+ 8/16 bit microcontroller";
    case EM_ST7:		return "STMicroelectronics ST7 8-bit microcontroller";
    case EM_68HC16:		return "Motorola MC68HC16 Microcontroller";
      /* 70 */
    case EM_68HC11:		return "Motorola MC68HC11 Microcontroller";
    case EM_68HC08:		return "Motorola MC68HC08 Microcontroller";
    case EM_68HC05:		return "Motorola MC68HC05 Microcontroller";
    case EM_SVX:		return "Silicon Graphics SVx";
    case EM_ST19:		return "STMicroelectronics ST19 8-bit microcontroller";
    case EM_VAX:		return "Digital VAX";
    case EM_CRIS:		return "Axis Communications 32-bit embedded processor";
    case EM_JAVELIN:		return "Infineon Technologies 32-bit embedded cpu";
    case EM_FIREPATH:		return "Element 14 64-bit DSP processor";
    case EM_ZSP:		return "LSI Logic's 16-bit DSP processor";
      /* 80 */
    case EM_MMIX:		return "Donald Knuth's educational 64-bit processor";
    case EM_HUANY:		return "Harvard Universitys's machine-independent object format";
    case EM_PRISM:		return "Vitesse Prism";
    case EM_AVR_OLD:
    case EM_AVR:		return "Atmel AVR 8-bit microcontroller";
    case EM_CYGNUS_FR30:
    case EM_FR30:		return "Fujitsu FR30";
    case EM_CYGNUS_D10V:
    case EM_D10V:		return "d10v";
    case EM_CYGNUS_D30V:
    case EM_D30V:		return "d30v";
    case EM_CYGNUS_V850:
    case EM_V850:		return "Renesas V850";
    case EM_CYGNUS_M32R:
    case EM_M32R:		return "Renesas M32R (formerly Mitsubishi M32r)";
    case EM_CYGNUS_MN10300:
    case EM_MN10300:		return "mn10300";
      /* 90 */
    case EM_CYGNUS_MN10200:
    case EM_MN10200:		return "mn10200";
    case EM_PJ:			return "picoJava";
    case EM_OR1K:		return "OpenRISC 1000";
    case EM_ARC_COMPACT:	return "ARCompact";
    case EM_XTENSA_OLD:
    case EM_XTENSA:		return "Tensilica Xtensa Processor";
    case EM_VIDEOCORE:		return "Alphamosaic VideoCore processor";
    case EM_TMM_GPP:		return "Thompson Multimedia General Purpose Processor";
    case EM_NS32K:		return "National Semiconductor 32000 series";
    case EM_TPC:		return "Tenor Network TPC processor";
    case EM_SNP1K:	        return "Trebia SNP 1000 processor";
      /* 100 */
    case EM_ST200:		return "STMicroelectronics ST200 microcontroller";
    case EM_IP2K_OLD:
    case EM_IP2K:		return "Ubicom IP2xxx 8-bit microcontrollers";
    case EM_MAX:		return "MAX Processor";
    case EM_CR:			return "National Semiconductor CompactRISC";
    case EM_F2MC16:		return "Fujitsu F2MC16";
    case EM_MSP430:		return "Texas Instruments msp430 microcontroller";
    case EM_BLACKFIN:		return "Analog Devices Blackfin";
    case EM_SE_C33:		return "S1C33 Family of Seiko Epson processors";
    case EM_SEP:		return "Sharp embedded microprocessor";
    case EM_ARCA:		return "Arca RISC microprocessor";
      /* 110 */
    case EM_UNICORE:		return "Unicore";
    case EM_EXCESS:		return "eXcess 16/32/64-bit configurable embedded CPU";
    case EM_DXP:		return "Icera Semiconductor Inc. Deep Execution Processor";
    case EM_ALTERA_NIOS2:	return "Altera Nios II";
    case EM_CRX:		return "National Semiconductor CRX microprocessor";
    case EM_XGATE:		return "Motorola XGATE embedded processor";
    case EM_C166:
    case EM_XC16X:		return "Infineon Technologies xc16x";
    case EM_M16C:		return "Renesas M16C series microprocessors";
    case EM_DSPIC30F:		return "Microchip Technology dsPIC30F Digital Signal Controller";
    case EM_CE:			return "Freescale Communication Engine RISC core";
      /* 120 */
    case EM_M32C:	        return "Renesas M32c";
      /* 130 */
    case EM_TSK3000:		return "Altium TSK3000 core";
    case EM_RS08:		return "Freescale RS08 embedded processor";
    case EM_ECOG2:		return "Cyan Technology eCOG2 microprocessor";
    case EM_SCORE:		return "SUNPLUS S+Core";
    case EM_DSP24:		return "New Japan Radio (NJR) 24-bit DSP Processor";
    case EM_VIDEOCORE3:		return "Broadcom VideoCore III processor";
    case EM_LATTICEMICO32:	return "Lattice Mico32";
    case EM_SE_C17:		return "Seiko Epson C17 family";
      /* 140 */
    case EM_TI_C6000:		return "Texas Instruments TMS320C6000 DSP family";
    case EM_TI_C2000:		return "Texas Instruments TMS320C2000 DSP family";
    case EM_TI_C5500:		return "Texas Instruments TMS320C55x DSP family";
    case EM_TI_PRU:		return "TI PRU I/O processor";
      /* 160 */
    case EM_MMDSP_PLUS:		return "STMicroelectronics 64bit VLIW Data Signal Processor";
    case EM_CYPRESS_M8C:	return "Cypress M8C microprocessor";
    case EM_R32C:		return "Renesas R32C series microprocessors";
    case EM_TRIMEDIA:		return "NXP Semiconductors TriMedia architecture family";
    case EM_QDSP6:		return "QUALCOMM DSP6 Processor";
    case EM_8051:		return "Intel 8051 and variants";
    case EM_STXP7X:		return "STMicroelectronics STxP7x family";
    case EM_NDS32:		return "Andes Technology compact code size embedded RISC processor family";
    case EM_ECOG1X:		return "Cyan Technology eCOG1X family";
    case EM_MAXQ30:		return "Dallas Semiconductor MAXQ30 Core microcontrollers";
      /* 170 */
    case EM_XIMO16:		return "New Japan Radio (NJR) 16-bit DSP Processor";
    case EM_MANIK:		return "M2000 Reconfigurable RISC Microprocessor";
    case EM_CRAYNV2:		return "Cray Inc. NV2 vector architecture";
    case EM_RX:			return "Renesas RX";
    case EM_METAG:		return "Imagination Technologies Meta processor architecture";
    case EM_MCST_ELBRUS:	return "MCST Elbrus general purpose hardware architecture";
    case EM_ECOG16:		return "Cyan Technology eCOG16 family";
    case EM_CR16:
    case EM_MICROBLAZE:
    case EM_MICROBLAZE_OLD:	return "Xilinx MicroBlaze";
    case EM_ETPU:		return "Freescale Extended Time Processing Unit";
    case EM_SLE9X:		return "Infineon Technologies SLE9X core";
      /* 180 */
    case EM_L1OM:		return "Intel L1OM";
    case EM_K1OM:		return "Intel K1OM";
    case EM_INTEL182:		return "Intel (reserved)";
    case EM_AARCH64:		return "AArch64";
    case EM_ARM184:		return "ARM (reserved)";
    case EM_AVR32:		return "Atmel Corporation 32-bit microprocessor";
    case EM_STM8:		return "STMicroeletronics STM8 8-bit microcontroller";
    case EM_TILE64:		return "Tilera TILE64 multicore architecture family";
    case EM_TILEPRO:		return "Tilera TILEPro multicore architecture family";
      /* 190 */
    case EM_CUDA:		return "NVIDIA CUDA architecture";
    case EM_TILEGX:		return "Tilera TILE-Gx multicore architecture family";
    case EM_CLOUDSHIELD:	return "CloudShield architecture family";
    case EM_COREA_1ST:		return "KIPO-KAIST Core-A 1st generation processor family";
    case EM_COREA_2ND:		return "KIPO-KAIST Core-A 2nd generation processor family";
    case EM_ARC_COMPACT2:	return "ARCv2";
    case EM_OPEN8:		return "Open8 8-bit RISC soft processor core";
    case EM_RL78:		return "Renesas RL78";
    case EM_VIDEOCORE5:		return "Broadcom VideoCore V processor";
    case EM_78K0R:		return "Renesas 78K0R";
      /* 200 */
    case EM_56800EX:		return "Freescale 56800EX Digital Signal Controller (DSC)";
    case EM_BA1:		return "Beyond BA1 CPU architecture";
    case EM_BA2:		return "Beyond BA2 CPU architecture";
    case EM_XCORE:		return "XMOS xCORE processor family";
    case EM_MCHP_PIC:		return "Microchip 8-bit PIC(r) family";
    case EM_INTELGT:		return "Intel Graphics Technology";
      /* 210 */
    case EM_KM32:		return "KM211 KM32 32-bit processor";
    case EM_KMX32:		return "KM211 KMX32 32-bit processor";
    case EM_KMX16:		return "KM211 KMX16 16-bit processor";
    case EM_KMX8:		return "KM211 KMX8 8-bit processor";
    case EM_KVARC:		return "KM211 KVARC processor";
    case EM_CDP:		return "Paneve CDP architecture family";
    case EM_COGE:		return "Cognitive Smart Memory Processor";
    case EM_COOL:		return "Bluechip Systems CoolEngine";
    case EM_NORC:		return "Nanoradio Optimized RISC";
    case EM_CSR_KALIMBA:	return "CSR Kalimba architecture family";
      /* 220 */
    case EM_Z80:		return "Zilog Z80";
    case EM_VISIUM:		return "CDS VISIUMcore processor";
    case EM_FT32:               return "FTDI Chip FT32";
    case EM_MOXIE:              return "Moxie";
    case EM_AMDGPU: 	 	return "AMD GPU";
      /* 230 (all reserved) */
      /* 240 */
    case EM_RISCV: 	 	return "RISC-V";
    case EM_LANAI:		return "Lanai 32-bit processor";
    case EM_CEVA:		return "CEVA Processor Architecture Family";
    case EM_CEVA_X2:		return "CEVA X2 Processor Family";
    case EM_BPF:		return "Linux BPF";
    case EM_GRAPHCORE_IPU:	return "Graphcore Intelligent Processing Unit";
    case EM_IMG1:		return "Imagination Technologies";
      /* 250 */
    case EM_NFP:		return "Netronome Flow Processor";
    case EM_VE:			return "NEC Vector Engine";
    case EM_CSKY:		return "C-SKY";
    case EM_ARC_COMPACT3_64:	return "Synopsys ARCv2.3 64-bit";
    case EM_MCS6502:		return "MOS Technology MCS 6502 processor";
    case EM_ARC_COMPACT3:	return "Synopsys ARCv2.3 32-bit";
    case EM_KVX:		return "Kalray VLIW core of the MPPA processor family";
    case EM_65816:		return "WDC 65816/65C816";
    case EM_LOONGARCH:		return "LoongArch";
    case EM_KF32:		return "ChipON KungFu32";

      /* Large numbers...  */
    case EM_MT:                 return "Morpho Techologies MT processor";
    case EM_ALPHA:		return "Alpha";
    case EM_WEBASSEMBLY:	return "Web Assembly";
    case EM_DLX:		return "OpenDLX";
    case EM_XSTORMY16:		return "Sanyo XStormy16 CPU core";
    case EM_IQ2000:       	return "Vitesse IQ2000";
    case EM_M32C_OLD:
    case EM_NIOS32:		return "Altera Nios";
    case EM_CYGNUS_MEP:         return "Toshiba MeP Media Engine";
    case EM_ADAPTEVA_EPIPHANY:	return "Adapteva EPIPHANY";
    case EM_CYGNUS_FRV:		return "Fujitsu FR-V";
    case EM_S12Z:               return "Freescale S12Z";

    default:
      snprintf (buff, sizeof (buff), ("<unknown>: 0x%x"), e_machine);
      return buff;
    }
}



static void decode_ARM_machine_flags (unsigned e_flags, char buf[])
{
  unsigned eabi;
  bool unknown = false;

  eabi = EF_ARM_EABI_VERSION (e_flags);
  e_flags &= ~ EF_ARM_EABIMASK;

  /* Handle "generic" ARM flags.  */
  if (e_flags & EF_ARM_RELEXEC)
    {
      strcat (buf, ", relocatable executable");
      e_flags &= ~ EF_ARM_RELEXEC;
    }

  if (e_flags & EF_ARM_PIC)
    {
      strcat (buf, ", position independent");
      e_flags &= ~ EF_ARM_PIC;
    }

  /* Now handle EABI specific flags.  */
  switch (eabi)
    {
    default:
      strcat (buf, ", <unrecognized EABI>");
      if (e_flags)
	unknown = true;
      break;

    case EF_ARM_EABI_VER1:
      strcat (buf, ", Version1 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_SYMSARESORTED: /* Conflicts with EF_ARM_INTERWORK.  */
	      strcat (buf, ", sorted symbol tables");
	      break;

	    default:
	      unknown = true;
	      break;
	    }
	}
      break;

    case EF_ARM_EABI_VER2:
      strcat (buf, ", Version2 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_SYMSARESORTED: /* Conflicts with EF_ARM_INTERWORK.  */
	      strcat (buf, ", sorted symbol tables");
	      break;

	    case EF_ARM_DYNSYMSUSESEGIDX:
	      strcat (buf, ", dynamic symbols use segment index");
	      break;

	    case EF_ARM_MAPSYMSFIRST:
	      strcat (buf, ", mapping symbols precede others");
	      break;

	    default:
	      unknown = true;
	      break;
	    }
	}
      break;

    case EF_ARM_EABI_VER3:
      strcat (buf, ", Version3 EABI");
      break;

    case EF_ARM_EABI_VER4:
      strcat (buf, ", Version4 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_BE8:
	      strcat (buf, ", BE8");
	      break;

	    case EF_ARM_LE8:
	      strcat (buf, ", LE8");
	      break;

	    default:
	      unknown = true;
	      break;
	    }
	}
      break;

    case EF_ARM_EABI_VER5:
      strcat (buf, ", Version5 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_BE8:
	      strcat (buf, ", BE8");
	      break;

	    case EF_ARM_LE8:
	      strcat (buf, ", LE8");
	      break;

	    case EF_ARM_ABI_FLOAT_SOFT: /* Conflicts with EF_ARM_SOFT_FLOAT.  */
	      strcat (buf, ", soft-float ABI");
	      break;

	    case EF_ARM_ABI_FLOAT_HARD: /* Conflicts with EF_ARM_VFP_FLOAT.  */
	      strcat (buf, ", hard-float ABI");
	      break;

	    default:
	      unknown = true;
	      break;
	    }
	}
      break;

    case EF_ARM_EABI_UNKNOWN:
      strcat (buf, ", GNU EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_INTERWORK:
	      strcat (buf, ", interworking enabled");
	      break;

	    case EF_ARM_APCS_26:
	      strcat (buf, ", uses APCS/26");
	      break;

	    case EF_ARM_APCS_FLOAT:
	      strcat (buf, ", uses APCS/float");
	      break;

	    case EF_ARM_PIC:
	      strcat (buf, ", position independent");
	      break;

	    case EF_ARM_ALIGN8:
	      strcat (buf, ", 8 bit structure alignment");
	      break;

	    case EF_ARM_NEW_ABI:
	      strcat (buf, ", uses new ABI");
	      break;

	    case EF_ARM_OLD_ABI:
	      strcat (buf, ", uses old ABI");
	      break;

	    case EF_ARM_SOFT_FLOAT:
	      strcat (buf, ", software FP");
	      break;

	    case EF_ARM_VFP_FLOAT:
	      strcat (buf, ", VFP");
	      break;

	    case EF_ARM_MAVERICK_FLOAT:
	      strcat (buf, ", Maverick FP");
	      break;

	    default:
	      unknown = true;
	      break;
	    }
	}
    }

  if (unknown)
    strcat (buf,(", <unknown>"));
}

char *get_machine_flags (unsigned e_flags, unsigned e_machine)
{
  static char buf[1024];
  buf[0] = '\0';
    if (e_flags)
    {
        switch (e_machine)
	{
	    default:
	    break;
	    case EM_ARM:
	    decode_ARM_machine_flags (e_flags, buf);
	    break;
        }
    }
  return buf;
}

const char *get_section_type_name (unsigned int sh_type)
{
    static char buff[32];
    switch (sh_type)
    {
        case SHT_NULL:		return "NULL";
        case SHT_PROGBITS:		return "PROGBITS";
        case SHT_SYMTAB:		return "SYMTAB";
        case SHT_STRTAB:		return "STRTAB";
        case SHT_RELA:		return "RELA";
        case SHT_RELR:		return "RELR";
        case SHT_HASH:		return "HASH";
        case SHT_DYNAMIC:		return "DYNAMIC";
        case SHT_NOTE:		return "NOTE";
        case SHT_NOBITS:		return "NOBITS";
        case SHT_REL:		return "REL";
        case SHT_SHLIB:		return "SHLIB";
        case SHT_DYNSYM:		return "DYNSYM";
        case SHT_INIT_ARRAY:	return "INIT_ARRAY";
        case SHT_FINI_ARRAY:	return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY:	return "PREINIT_ARRAY";
        case SHT_GNU_HASH:		return "GNU_HASH";
        case SHT_GROUP:		return "GROUP";
        case SHT_SYMTAB_SHNDX:	return "SYMTAB SECTION INDICES";
        case SHT_GNU_verdef:	return "VERDEF";
        case SHT_GNU_verneed:	return "VERNEED";
        case SHT_GNU_versym:	return "VERSYM";
        case 0x6ffffff0:		return "VERSYM";
        case 0x6ffffffc:		return "VERDEF";
        case 0x7ffffffd:		return "AUXILIARY";
        case 0x7fffffff:		return "FILTER";
        case SHT_GNU_LIBLIST:	return "GNU_LIBLIST";
        case SHT_ARM_EXIDX:           return "ARM_EXIDX";
        case SHT_ARM_PREEMPTMAP:      return "ARM_PREEMPTMAP";
        case SHT_ARM_ATTRIBUTES:      return "ARM_ATTRIBUTES";
        case SHT_ARM_DEBUGOVERLAY:    return "ARM_DEBUGOVERLAY";
        case SHT_ARM_OVERLAYSECTION:  return "ARM_OVERLAYSECTION";
        default:
          	snprintf (buff, sizeof (buff), ("%08x: <unknown>"), sh_type);
          return buff;
    }

}

const char *get_segment_type (unsigned e_machine, unsigned long p_type)
{
    switch (p_type)
    {
        case PT_NULL:	return "NULL";
        case PT_LOAD:	return "LOAD";
        case PT_DYNAMIC:	return "DYNAMIC";
        case PT_INTERP:	return "INTERP";
        case PT_NOTE:	return "NOTE";
        case PT_SHLIB:	return "SHLIB";
        case PT_PHDR:	return "PHDR";
        case PT_TLS:	return "TLS";
        case PT_GNU_EH_FRAME: return "GNU_EH_FRAME";
        case PT_GNU_STACK:	return "GNU_STACK";
        case PT_GNU_RELRO:  return "GNU_RELRO";
        case PT_GNU_PROPERTY: return "GNU_PROPERTY";
        case PT_GNU_SFRAME: return "GNU_SFRAME";

        case PT_OPENBSD_RANDOMIZE: return "OPENBSD_RANDOMIZE";
        case PT_OPENBSD_WXNEEDED: return "OPENBSD_WXNEEDED";
        case PT_OPENBSD_BOOTDATA: return "OPENBSD_BOOTDATA";

        default:
          if ((p_type >= PT_LOPROC) && (p_type <= PT_HIPROC))
    	    {
    	        switch (e_machine)
    	        {
        	    case EM_AARCH64:
                    switch (p_type)
                    {
                        case PT_AARCH64_ARCHEXT:  return "AARCH64_ARCHEXT";
                        case PT_AARCH64_MEMTAG_MTE:	return "AARCH64_MEMTAG_MTE";
                        default:                  return NULL;
                    }
        	      break;
        	    case EM_ARM:
        	        switch (p_type)
                       {
                            case PT_ARM_EXIDX: return "EXIDX";
                            default:           return NULL;
                       }
        	      break;
    	        }
            }
        }
    return NULL;
}

static size_t elf_ehdr_size(const void *elf_info)
{
	if (!elf_info)
		return sizeof(Elf_64_Ehdr);
	else if (elf_is_64(elf_info) != 0)
		return sizeof(Elf_64_Ehdr);
	else
		return sizeof(Elf_32_Ehdr);
}

static size_t elf_phoff(const void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		const Elf_32_Ehdr *ehdr = elf_info;

		return ehdr->e_phoff;
	} else {
		const Elf_64_Ehdr *ehdr = elf_info;

		return ehdr->e_phoff;
	}
}

static size_t elf_phentsize(const void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		const Elf_32_Ehdr *ehdr = elf_info;

		return ehdr->e_phentsize;
	} else {
		const Elf_64_Ehdr *ehdr = elf_info;

		return ehdr->e_phentsize;
	}
}

static int elf_phnum(const void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		const Elf_32_Ehdr *ehdr = elf_info;

		return ehdr->e_phnum;
	} else {
		const Elf_64_Ehdr *ehdr = elf_info;

		return ehdr->e_phnum;
	}
}

static size_t elf_shoff(const void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		const Elf_32_Ehdr *ehdr = elf_info;

		return ehdr->e_shoff;
	} else {
		const Elf_64_Ehdr *ehdr = elf_info;

		return ehdr->e_shoff;
	}
}

static size_t elf_shentsize(const void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		const Elf_32_Ehdr *ehdr = elf_info;

		return ehdr->e_shentsize;
	} else {
		const Elf_64_Ehdr *ehdr = elf_info;

		return ehdr->e_shentsize;
	}
}

static int elf_shnum(const void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		const Elf_32_Ehdr *ehdr = elf_info;

		return ehdr->e_shnum;
	} else {
		const Elf_64_Ehdr *ehdr = elf_info;

		return ehdr->e_shnum;
	}
}

static int elf_shstrndx(const void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		const Elf_32_Ehdr *ehdr = elf_info;

		return ehdr->e_shstrndx;
	} else {
		const Elf_64_Ehdr *ehdr = elf_info;

		return ehdr->e_shstrndx;
	}
}

static void **elf_phtable_ptr(void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		struct elf32_info *einfo = elf_info;

		return (void **)&einfo->phdrs;
	} else {
		struct elf64_info *einfo = elf_info;

		return (void **)&einfo->phdrs;
	}
}

static void **elf_shtable_ptr(void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		struct elf32_info *einfo = elf_info;

		return (void **)&einfo->shdrs;
	} else {
		struct elf64_info *einfo = elf_info;

		return (void **)&einfo->shdrs;
	}
}

static void **elf_shstrtab_ptr(void *elf_info)
{
	if (elf_is_64(elf_info) == 0) {
		struct elf32_info *einfo = elf_info;

		return &einfo->shstrtab;
	} else {
		struct elf64_info *einfo = elf_info;

		return &einfo->shstrtab;
	}
}

__attribute__ ((used))  static void elf_parse_segment(void *elf_info, const void *elf_phdr,
			      unsigned int *p_type, size_t *p_offset,
			      UADDR *p_vaddr,
			      UADDR *p_paddr,
			      size_t *p_filesz, size_t *p_memsz)
{
	if (elf_is_64(elf_info) == 0) {
		const Elf_32_Phdr *phdr = elf_phdr;

		if (p_type)
			*p_type = (unsigned int)phdr->p_type;
		if (p_offset)
			*p_offset = (size_t)phdr->p_offset;
		if (p_vaddr)
			*p_vaddr = (UADDR)phdr->p_vaddr;
		if (p_paddr)
			*p_paddr = (UADDR)phdr->p_paddr;
		if (p_filesz)
			*p_filesz = (size_t)phdr->p_filesz;
		if (p_memsz)
			*p_memsz = (size_t)phdr->p_memsz;
	} else {
		const Elf_64_Phdr *phdr = elf_phdr;

		if (p_type)
			*p_type = (unsigned int)phdr->p_type;
		if (p_offset)
			*p_offset = (size_t)phdr->p_offset;
		if (p_vaddr)
			*p_vaddr = (UADDR)phdr->p_vaddr;
		if (p_paddr)
			*p_paddr = (UADDR)phdr->p_paddr;
		if (p_filesz)
			*p_filesz = (size_t)phdr->p_filesz;
		if (p_memsz)
			*p_memsz = (size_t)phdr->p_memsz;
	}
}

__attribute__ ((used)) static const void *elf_get_segment_from_index(void *elf_info, int index)
{
	if (elf_is_64(elf_info) == 0) {
		const struct elf32_info *einfo = elf_info;
		const Elf_32_Ehdr *ehdr = &einfo->ehdr;
		const Elf_32_Phdr *phdrs = einfo->phdrs;

		if (!phdrs)
			return NULL;
		if (index < 0 || index >= ehdr->e_phnum)
			return NULL;
		return &phdrs[index];
	} else {
		const struct elf64_info *einfo = elf_info;
		const Elf_64_Ehdr *ehdr = &einfo->ehdr;
		const Elf_64_Phdr *phdrs = einfo->phdrs;

		if (!phdrs)
			return NULL;
		if (index < 0 || index >= ehdr->e_phnum)
			return NULL;
		return &phdrs[index];
	}
}


void *elf_get_section_from_name(void *elf_info, const char *name)
{
	unsigned int i;
	const char *name_table;
	if (elf_is_64(elf_info) == 0) 
        {
		struct elf32_info *einfo = elf_info;
		Elf_32_Ehdr *ehdr = &einfo->ehdr;
		Elf_32_Shdr *shdr = einfo->shdrs;
		name_table = einfo->shstrtab;
		if (!shdr || !name_table)
			return NULL;
            for (i = 0; i < ehdr->e_shnum; i++, shdr++) 
            {
                if (strcmp(name, name_table + shdr->sh_name))
                    continue;
                else
                    return shdr;
            }
	} 
        else 
        {
		struct elf64_info *einfo = elf_info;
		Elf_64_Ehdr *ehdr = &einfo->ehdr;
		Elf_64_Shdr *shdr = einfo->shdrs;
		name_table = einfo->shstrtab;
		if (!shdr || !name_table)
                return NULL;
            for (i = 0; i < ehdr->e_shnum; i++, shdr++) 
            {
                if (strcmp(name, name_table + shdr->sh_name))
                    continue;
                else
                    return shdr;
            }
	}
	return NULL;
}

static void *elf_get_section_from_index(void *elf_info, int index)
{
	if (elf_is_64(elf_info) == 0) {
		struct elf32_info *einfo = elf_info;
		Elf_32_Ehdr *ehdr = &einfo->ehdr;
		Elf_32_Shdr *shdr = einfo->shdrs;

		if (!shdr)
			return NULL;
		if (index < 0 || index >= ehdr->e_shnum)
			return NULL;
		return &einfo->shdrs[index];
	} else {
		struct elf64_info *einfo = elf_info;
		Elf_64_Ehdr *ehdr = &einfo->ehdr;
		Elf_64_Shdr *shdr = einfo->shdrs;

		if (!shdr)
			return NULL;
		if (index < 0 || index >= ehdr->e_shnum)
			return NULL;
		return &einfo->shdrs[index];
	}
}

static void elf_parse_section(void *elf_info, void *elf_shdr,
			      unsigned int *sh_type, unsigned int *sh_flags,
			      UADDR *sh_addr,
			      size_t *sh_offset, size_t *sh_size,
			      unsigned int *sh_link, unsigned int *sh_info,
			      unsigned int *sh_addralign,
			      size_t *sh_entsize)
{
	if (elf_is_64(elf_info) == 0) {
		Elf_32_Shdr *shdr = elf_shdr;

		if (sh_type)
			*sh_type = shdr->sh_type;
		if (sh_flags)
			*sh_flags = shdr->sh_flags;
		if (sh_addr)
			*sh_addr = (UADDR)shdr->sh_addr;
		if (sh_offset)
			*sh_offset = shdr->sh_offset;
		if (sh_size)
			*sh_size = shdr->sh_size;
		if (sh_link)
			*sh_link = shdr->sh_link;
		if (sh_info)
			*sh_info = shdr->sh_info;
		if (sh_addralign)
			*sh_addralign = shdr->sh_addralign;
		if (sh_entsize)
			*sh_entsize = shdr->sh_entsize;
	} else {
		Elf_64_Shdr *shdr = elf_shdr;

		if (sh_type)
			*sh_type = shdr->sh_type;
		if (sh_flags)
			*sh_flags = shdr->sh_flags;
		if (sh_addr)
			*sh_addr = (UADDR)shdr->sh_addr;
		if (sh_offset)
			*sh_offset = shdr->sh_offset;
		if (sh_size)
			*sh_size = shdr->sh_size;
		if (sh_link)
			*sh_link = shdr->sh_link;
		if (sh_info)
			*sh_info = shdr->sh_info;
		if (sh_addralign)
			*sh_addralign = shdr->sh_addralign;
		if (sh_entsize)
			*sh_entsize = shdr->sh_entsize;
	}
}

static U32 elf_info_size(const void *img_data)
{
	if (elf_is_64(img_data) == 0)
		return sizeof(struct elf32_info);
	else
		return sizeof(struct elf64_info);
}

int elf_identify(const void *img_data, size_t len)
{
	if (len < 4 || !img_data)
		return -RET_NOK;
	if (memcmp(img_data, ELFMAG, SELFMAG) != 0)
		return -RET_NOK;
	else
		return RET_OK;
}

int elf_load_header(const void *img_data,void **img_info)
{
    size_t tmpsize= 0;
    size_t infosize  = 0;
    size_t phdrs_size = 0;
    size_t phdrs_offset = 0;
    size_t shdrs_size = 0;
    size_t shdrs_offset = 0;
    size_t shstrtab_size =0;
    size_t shstrtab_offset =0;
    int shstrndx =0;
    void *shdr;
    void **shstrtab;
    void **shdrs;
    const void *img_shdrs;
    void **phdrs;
    const void *img_phdrs;
    /* Get ELF header */
    printf("Loading ELF headering\r\n");
    tmpsize = elf_ehdr_size(img_data);
    infosize= elf_info_size(img_data);
    *img_info = malloc(infosize);
    if(*img_info ==NULL)
    {
        printf("infosize malloc failed\r\n");
        return -RET_NOK;
    }
    memset(*img_info, 0, infosize);
    memcpy(*img_info, img_data, tmpsize);
    /* Get ELF program headers */
    printf("Loading ELF program headering\r\n");
    phdrs_offset = elf_phoff(*img_info);
    phdrs_size = elf_phnum(*img_info) * elf_phentsize(*img_info);
    /* calculate the programs headers offset to the image_data */
    img_phdrs = (const char *)img_data + phdrs_offset;
    phdrs = elf_phtable_ptr(*img_info);
    *phdrs = malloc(phdrs_size);
    if (!*phdrs)
    {
        printf("phdrs_size malloc failed\r\n");
        return -RET_NOK;
    }
    memcpy(*phdrs, img_phdrs, phdrs_size);
    /* Get ELF Section Headers */
    printf("Loading ELF section header.\r\n");
    shdrs_offset = elf_shoff(*img_info);
    if (elf_shnum(*img_info) == 0) 
    {
        printf("ELF section num is 0 \r\n");
        return -RET_NOK;
    }
    shdrs_size = elf_shnum(*img_info) * elf_shentsize(*img_info);
    /* calculate the sections headers offset to the image_data */
    img_shdrs = (const char *)img_data + shdrs_offset;
    shdrs = elf_shtable_ptr(*img_info);
    *shdrs = malloc(shdrs_size);
    if (!*shdrs)
    {
        printf("phdrs_size malloc failed\r\n");
        return -RET_NOK;
    }
    memcpy(*shdrs, img_shdrs, shdrs_size);
    /* Get ELF SHSTRTAB section */
    printf("Loading ELF shstrtab.\r\n");
    shstrndx = elf_shstrndx(*img_info);
    shdr = elf_get_section_from_index(*img_info, shstrndx);
    if (!shdr)
    {
        printf("shdr get failed\r\n");
        return -RET_NOK;
    }
    elf_parse_section(*img_info, shdr, NULL, NULL,
    		  NULL, &shstrtab_offset,
    		  &shstrtab_size, NULL, NULL,
    		  NULL, NULL);
    /* Calculate shstrtab section offset to the input image data */
    shstrtab = elf_shstrtab_ptr(*img_info);
    *shstrtab = malloc(shstrtab_size);
    if (!*shstrtab)
    {
        printf("shstrtab malloc failed\r\n");
        return -RET_NOK;
    }
    memcpy(*shstrtab,(const char *)img_data + shstrtab_offset,
           shstrtab_size);
    return RET_OK;
}

int elf_load(const void *img_data,void **img_info)
{
    int rc  =0;
    rc = elf_load_header(img_data,img_info);
    return rc;
}


int elf_locate_rsc_table(void *elf_info, UADDR *da,
			 size_t *offset, size_t *size)
{
	char *sect_name = ".resource_table";
	void *shdr;

	if (!elf_info)
		return -RET_NOK;
	shdr = elf_get_section_from_name(elf_info, sect_name);
	if (!shdr) 
       {
            *size = 0;
            return 0;
	}
	elf_parse_section(elf_info, shdr, NULL, NULL,
			  da, offset, size,
			  NULL, NULL, NULL, NULL);
	return 0;
}

UADDR  elf_get_entry(void *elf_info)
{
	if (!elf_info)
		return RET_NOK;

	if (elf_is_64(elf_info) == 0) {
		Elf_32_Ehdr *elf_ehdr = elf_info;
		Elf32_Addr e_entry;

		e_entry = elf_ehdr->e_entry;
		return (UADDR)e_entry;
	} else {
		Elf_64_Ehdr *elf_ehdr = elf_info;
		Elf64_Addr e_entry;

		e_entry = elf_ehdr->e_entry;
		return (UADDR)e_entry;
	}
}



