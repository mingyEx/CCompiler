/*******************************************************************************
Vczh Library++ 2.0
JIT::X86汇编二进制格式表
开发者：陈梓瀚

接口：
类：
函数：
*******************************************************************************/
#ifndef VL_JIT_X86
#define VL_JIT_X86

namespace vl
{
	namespace jit
	{
		namespace x86
		{
			enum VLE_InsName
			{
				vinADC,
				vinADD,
				vinAND,
				vinBOUND,
				vinBSF,
				vinBSR,
				vinBSWAP,
				vinBT,
				vinBTC,
				vinBTR,
				vinBTS,
				vinCALL,
				vinCBW,
				vinCDQ,
				vinCLC,
				vinCLD,
				vinCLI,
				vinCMC,
				vinCMOVA,
				vinCMOVAE,
				vinCMOVB,
				vinCMOVBE,
				vinCMOVC,
				vinCMOVE,
				vinCMOVG,
				vinCMOVGE,
				vinCMOVL,
				vinCMOVLE,
				vinCMOVNA,
				vinCMOVNAE,
				vinCMOVNB,
				vinCMOVNBE,
				vinCMOVNC,
				vinCMOVNE,
				vinCMOVNG,
				vinCMOVNGE,
				vinCMOVNL,
				vinCMOVNLE,
				vinCMOVNO,
				vinCMOVNP,
				vinCMOVNS,
				vinCMOVNZ,
				vinCMOVO,
				vinCMOVP,
				vinCMOVPE,
				vinCMOVPO,
				vinCMOVS,
				vinCMOVZ,
				vinCMP,
				vinCMPSB,
				vinCMPSD,
				vinCMPSW,
				vinCWD,
				vinCWDE,
				vinDEC,
				vinDIV,
				vinF2XM1,
				vinFABS,
				vinFADD,
				vinFADDP,
				vinFCHS,
				vinFCMOVB,
				vinFCMOVBE,
				vinFCMOVE,
				vinFCMOVNB,
				vinFCMOVNBE,
				vinFCMOVNE,
				vinFCMOVNU,
				vinFCMOVU,
				vinFCOM,
				vinFCOMI,
				vinFCOMIP,
				vinFCOMP,
				vinFCOMPP,
				vinFCOS,
				vinFDECSTP,
				vinFDIV,
				vinFDIVP,
				vinFDIVR,
				vinFDIVRP,
				vinFIADD,
				vinFICOM,
				vinFICOMP,
				vinFIDIV,
				vinFIDIVR,
				vinFILD,
				vinFIMUL,
				vinFINCSTP,
				vinFINIT,
				vinFIST,
				vinFISTP,
				vinFISTTP,
				vinFISUB,
				vinFISUBR,
				vinFLD,
				vinFLD1,
				vinFLDL2E,
				vinFLDL2T,
				vinFLDLG2,
				vinFLDLN2,
				vinFLDPI,
				vinFLDZ,
				vinFMUL,
				vinFMULP,
				vinFNINIT,
				vinFNSTSW,
				vinFPATAN,
				vinFRNDINT,
				vinFSCALE,
				vinFSIN,
				vinFSINCOS,
				vinFSQRT,
				vinFST,
				vinFSTP,
				vinFSTSW,
				vinFSUB,
				vinFSUBP,
				vinFSUBR,
				vinFSUBRP,
				vinFTST,
				vinFUCOM,
				vinFUCOMI,
				vinFUCOMIP,
				vinFUCOMP,
				vinFUCOMPP,
				vinFXAM,
				vinFXCH,
				vinFXTRACT,
				vinFYL2X,
				vinFYL2XP1,
				vinIDIV,
				vinIMUL,
				vinINC,
				vinINT,
				vinINTO,
				vinJA,
				vinJAE,
				vinJB,
				vinJBE,
				vinJC,
				vinJCXZ,
				vinJE,
				vinJECXZ,
				vinJG,
				vinJGE,
				vinJL,
				vinJLE,
				vinJMP,
				vinJNA,
				vinJNAE,
				vinJNB,
				vinJNBE,
				vinJNC,
				vinJNE,
				vinJNG,
				vinJNGE,
				vinJNL,
				vinJNLE,
				vinJNO,
				vinJNP,
				vinJNS,
				vinJNZ,
				vinJO,
				vinJP,
				vinJPE,
				vinJPO,
				vinJS,
				vinJZ,
				vinLEA,
				vinLODSB,
				vinLODSD,
				vinLODSW,
				vinLOOP,
				vinLOOPE,
				vinLOOPNE,
				vinLOOPNZ,
				vinLOOPZ,
				vinMOV,
				vinMOVSB,
				vinMOVSD,
				vinMOVSW,
				vinMOVZX,
				vinMUL,
				vinNEG,
				vinNOT,
				vinOR,
				vinPOP,
				vinPOPA,
				vinPOPAD,
				vinPUSH,
				vinPUSHA,
				vinPUSHAD,
				vinRCL,
				vinRCR,
				vinRET,
				vinROL,
				vinROR,
				vinSAHF,
				vinSAL,
				vinSAR,
				vinSBB,
				vinSCASB,
				vinSCASD,
				vinSCASW,
				vinSETA,
				vinSETAE,
				vinSETB,
				vinSETBE,
				vinSETC,
				vinSETE,
				vinSETG,
				vinSETGE,
				vinSETL,
				vinSETLE,
				vinSETNA,
				vinSETNAE,
				vinSETNB,
				vinSETNBE,
				vinSETNC,
				vinSETNE,
				vinSETNG,
				vinSETNGE,
				vinSETNL,
				vinSETNLE,
				vinSETNO,
				vinSETNP,
				vinSETNS,
				vinSETNZ,
				vinSETO,
				vinSETP,
				vinSETPE,
				vinSETPO,
				vinSETS,
				vinSETZ,
				vinSHL,
				vinSHR,
				vinSTC,
				vinSTD,
				vinSTI,
				vinSTOSB,
				vinSTOSD,
				vinSTOSW,
				vinSUB,
				vinTEST,
				vinXCHG,
				vinXOR,
				vinUNKNOWN
			};

			enum VLE_InsExt
			{
				vie0,
				vie1,
				vie2,
				vie3,
				vie4,
				vie5,
				vie6,
				vie7,
				vieRegister,
				vieNoExt
			};

			enum VLE_InsPlus
			{
				vipRegister,
				vipFloat,
				vipNoPlus
			};

			enum VLE_InsImm
			{
				viiImmediate,
				viiConstant,
				viiNoImm
			};

			enum VLE_InsParam
			{
				vipREL_8,
				vipREL_16,
				vipREL_32,
				vipPTR_16_16,
				vipPTR_16_32,
				vipREG_8,
				vipREG_16,
				vipREG_32,
				vipIMM_8,
				vipIMM_16,
				vipIMM_32,
				vipRM_8,
				vipRM_16,
				vipRM_32,
				vipM_16_16,
				vipM_16_32,
				vipM_16_AND_16,
				vipM_16_AND_32,
				vipM_32_AND_32,
				vipMI_16,
				vipMI_32,
				vipMI_64,
				vipMF_32,
				vipMF_64,
				vipMF_80,
				vipSREG,
				vipST_0,
				vipST_I,
				vipAL,
				vipAX,
				vipEAX,
				vipCL,
				vipCX,
				vipECX,
				vipCS,
				vipDS,
				vipES,
				vipFS,
				vipGS,
				vipSS,
				vipCONST_1,
				vipCONST_3,
				vipNoParam
			};

			struct VLS_InsFormat
			{
				unsigned char		OpcodeLength;
				const char*			Opcode;
				bool				Prefix16;
				VLE_InsPlus			Plus;
				VLE_InsExt			Ext;
				VLE_InsImm			Imm;
				VLE_InsParam		Params[4];
				int Rank()
				{
					int rs = OpcodeLength;
					if (Prefix16)
						rs++;
					int paramLength = 0;
					for (int i = 0; i<4; i++)
						switch (Params[i])
						{
						case vipREL_8:
							paramLength++;
							break;
						case vipREL_16:
							paramLength+=2;
							break;
						case vipREL_32:
							paramLength+=4;
							break;
						case vipPTR_16_16:
						case vipPTR_16_32:
							paramLength+=4;
							break;
						case vipREG_8:
						case vipREG_16:
						case vipREG_32:
							paramLength++;
							break;
						case vipIMM_8:
							paramLength++;
							break;
						case vipIMM_16:
							paramLength+=2;
							break;
						case vipIMM_32:
							paramLength+=4;
							break;
						case vipRM_8:
						case vipRM_16:
						case vipRM_32:
							paramLength+=6;
							break;
						case vipM_16_16:
						case vipM_16_32:
						case vipM_16_AND_16:
						case vipM_16_AND_32:
						case vipM_32_AND_32:
						case vipMI_16:
						case vipMI_32:
						case vipMI_64:
						case vipMF_32:
						case vipMF_64:
						case vipMF_80:
							paramLength+=6;
							break;
						case vipSREG:
						case vipST_0:
						case vipST_I:
						case vipAL:
						case vipAX:
						case vipEAX:
						case vipCL:
						case vipCX:
						case vipECX:
						case vipCS:
						case vipDS:
						case vipES:
						case vipFS:
						case vipGS:
						case vipSS:
						case vipCONST_1:
						case vipCONST_3:
						case vipNoParam:
						default:
							break;
						}
					if (Plus != vipNoPlus)
						paramLength--;
					return rs+paramLength;
				}
			};

			extern int				InsOffset[246];
			extern int				InsCount[246];
			extern const wchar_t*	InsName[246];
			extern VLS_InsFormat	InsFormat[677];
			extern VLE_InsName		NameToIns(const wchar_t* Name);
		}
	}
}

#endif
