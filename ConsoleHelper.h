/** CConsoleHelper simplifies DPP spectrum acquisition.
 *  Coordinates DPP communications and spectrum acquisition.
 *
 */

#pragma once

#include <string>
#include <vector>
#include "DppLibUsb.h"			// LibUsb Support
#include "DP5Protocol.h"		// DPP Protocol Support
#include "ParsePacket.h"		// Packet Parser
#include "SendCommand.h"		// Command Generator
#include "DP5Status.h"			// Status Decoder
#include <time.h>				// time library for rand seed

typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef unsigned char BYTE;

typedef struct _SpectrumFileType {
    string strTag;
    string strDescription;
    short m_iNumChan;
	unsigned long SerialNumber;
	string strSpectrumConfig;
	string strSpectrumStatus;
} SpectrumFileType;

class CConsoleHelper
{
public:
	CConsoleHelper(void);
	~CConsoleHelper(void);

	/// LibUsb communications class.
	CDppLibUsb DppLibUsb;
	CDP5Status DP5Status;
	/// LibUsb is connected if true.
	bool LibUsb_isConnected;
	/// Specify if device connection is closed
	bool bConnectionClosed;
	/// LibUsb number of devices found.
	int  LibUsb_NumDevices;
	void KeepMX2_Alive();
	//Turn Volume on/off
	void SendMX2_Volume(string strVol);
	/// Turn HV on / off
	void SendMX2_HVandI(string stringHV, string stringI);
	//
	void SendMX2_HV(string stringHV);
	//
	void DailyWarmup();
	/// Find HV and I configuration
	void ReadbackMX2_HVandI();
	/// Send Command
	void SendCommandDataMX2(TRANSMIT_PACKET_TYPE XmtCmd, string strDataIn);
	/// Send Command
	void SendCommandData(TRANSMIT_PACKET_TYPE XmtCmd, BYTE DataOut[]);
	// 
	void RemCallParsePacket(BYTE PacketIn[]);
	//
	void ParsePacketEx(Packet_In PIN, DppStateType DppState);
	//
	void ProcessNetFinderM2Ex(Packet_In PIN, DppStateType DppState);
	//
	void ProcessTimestampRecordMX2Ex(Packet_In PIN, DppStateType DppState);
	//
	void ProcessWarmupTableMX2Ex(Packet_In PIN, DppStateType DppState);
	//
	string Process_MNX_Warmup_Table();
	//
	void ProcessFaultRecordMX2Ex(Packet_In PIN, DppStateType DppState);
	//
	string Process_MNX_Fault_Record(Packet_In PIN);
	//
	void ListDevices();
	/// LibUsb connect to the default DPP.
	bool LibUsb_Connect_Default_DPP();
	///  LibUsb count number of Amptek devices
	int LibUsb_CountDP5Devices();
	/// LibUsb connect to a specific DPP device.
	bool LibUsb_Connect_Specific_DPP(int Num_Device);
	/// LibUsb close the current connection.
	bool LibUsb_Close_Connection();
	/// LibUsb send a command that does not require additional processing.
	bool LibUsb_SendCommand(TRANSMIT_PACKET_TYPE XmtCmd);
	/// LibUsb send a command that requires configuration options processing.
	bool LibUsb_SendCommand_Config(TRANSMIT_PACKET_TYPE XmtCmd, CONFIG_OPTIONS CfgOptions);
	///  LibUsb receive data.
	bool LibUsb_ReceiveData();

	// communications helper functions

	/// Defines and implements DPP protocol.
	CDP5Protocol DP5Proto;
	/// Generates command packet to be sent.
	CSendCommand SndCmd;
	/// DPP packet parsing.
	CParsePacket ParsePkt;
	/// DPP status processing.
	CDP5Status DP5Stat;			
	
	// DPP packet processing functions.

	/// Processes DPP data from all communication interfaces (USB,RS232,INET)
	bool ReceiveData();
	
	/// EDITS
	void ProcessTextDataEx(Packet_In PIN, DppStateType DppState);
	void ProcessTubeInterlockTableMX2Ex(Packet_In PIN, DppStateType DppState);
	
	/// Processes spectrum packets.
	void ProcessSpectrumEx(Packet_In PIN, DppStateType DppState);
	/// Clears configuration readback format flags. 
	void ClearConfigReadFormatFlags();
	string strHV;
	string strI;
	int iDeviceType;
	/// Processes configuration packets.
	void ProcessCfgReadM2Ex(Packet_In PIN, DppStateType DppState);
	/// Processes configuration packets.
	void ProcessCfgReadEx(Packet_In PIN, DppStateType DppState);
	/// Populates the configuration command options data structure.
	void CreateConfigOptions(CONFIG_OPTIONS *CfgOptions, string strCfg, CDP5Status DP5Stat, bool bUseCoarseFineGain);

	//display support

	/// Provides a low resolution text console graph.
	void ConsoleGraph(long lData[], long chan, bool bLog, std::string strStatus);
	/// DPP status display string.
	string DppStatusString;
	string strTubeInterlockTable;
	// DPP configuration information variables 

	/// FPGA 80MHz clock when true, 20MHz clock otherwise.
	bool b80MHzMode;
	/// Holds MCA MODE display string. (NORM=MCA, MCS, FAST, etc.)
	string strMcaMode;
	/// DPP configuration command array.
	vector<string> Dp5CmdList;

	// configuration readback format control flags
	// these flags control how the configuration readback is formatted and processed

	/// format configuration for display
	bool DisplayCfg;
	/// format sca for display (sca config sent separately)
	bool DisplaySca;
	/// format configuration for general readback
	bool CfgReadBack;
	/// format configuration for file save
	bool SaveCfg;
	/// format configuration for print
	bool PrintCfg;
	/// configuration ready flag
	bool HwCfgReady;
	/// sca readback ready flag
	bool ScaReadBack;

	// Tuning and display variables.

	/// Holds the hardware configuration readback.
	string HwCfgDP5;
	/// Number of data channels.
	int mcaCH;
	/// Slow threshold in percent.
	double SlowThresholdPct;
	/// Fast channel threshold.
	int FastChThreshold;
	/// Peaking time value.
	double RiseUS;
	/// Total gain display string.
	string strGainDisplayValue;
	/// Acquisition mode. (0=MCA, 1=MCS)
	int AcqMode;				

	// configuration presets for display (status updates preset progress)

	/// preset count setting
	int PresetCount;
	/// preset acquisition time setting
	double PresetAcq;
	/// preset real time setting
	double PresetRt;
	/// presets mode summary (counts,accum. time,real time)
	string strPresetCmd;
	/// presets settings summary (preset values, counts,times)
	string strPresetVal;

	// configuration support functions

	/// Returns the configuration command data from a configuration command string.
	string GetCmdData(string strCmd, string strCfgData);
	/// Replaces (or inserts a command description (comment) in a configuration command string.
	string ReplaceCmdDesc(string strCmd, string strCfgData);
	/// Appends a command description (comment) in a configuration command string.
	string AppendCmdDesc(string strCmd, string strCfgData);
	/// Returns the command decription (comment) in a configuration command string.
	string GetCmdDesc(string strCmd);

	// oscilloscope support


	bool UpdateScopeCfg;
	string strInputOffset;
	string strAnalogOut;
	string strOutputOffset;
	string strTriggerSource;
	string strTriggerSlope;
	string strTriggerPosition;
	string strScopeGain;

    string CreateMCAData(long m_larDataBuffer[], SpectrumFileType sfInfo, DP4_FORMAT_STATUS cfgStatusLst);
	/// Saves a spectrum data string to a default file (SpectrumData.mca).
	void SaveSpectrumStringToFile(string strData);
    string CreateSpectrumConfig(string strRawCfgIn) ;
	vector<string> MakeDp5CmdList();
	SpectrumFileType sfInfo;

};






















