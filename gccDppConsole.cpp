/** gccDppConsole.cpp */

// gccDppConsole.cpp : Defines the entry point for the console application.
#include <iostream>
using namespace std; 
#include "ConsoleHelper.h"
#include "stringex.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#define CLEAR_TERM "cls"
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#define _getch getchar
#define CLEAR_TERM "clear"
#endif


CConsoleHelper chdpp;					// DPP communications functions
bool bRunSpectrumTest = false;			// run spectrum test
bool bRunConfigurationTest = false;		// run configuration test
bool bHaveStatusResponse = false;		// have status response
bool bHaveConfigFromHW = false;			// have configuration from hardware
bool bTubeOn = false;


extern "C" {
	// Counts the number of DPP devices - Only run this if no devices are connected.
	int CountDP5Devices()
	{
		int NumDevices;
		NumDevices = chdpp.LibUsb_CountDP5Devices();
		return (NumDevices);
	}

	// Connect to a specific DPP device
	bool ConnectToSpecificDPP(int NumDevice)
	{
		if (chdpp.LibUsb_Connect_Specific_DPP(NumDevice)) {
			return true;
		} else {
			cout << "\t\tNo LibUsb DPP device present." << endl;
			return false;
		}
	}

	// Identify if DPP device is the DP5 device. Returns 1 for DP5 and 2 for MX2
	int GetDeviceType()
	{
		int iDeviceType = 0;
		if (chdpp.LibUsb_isConnected) {
			if (chdpp.LibUsb_SendCommand(XMTPT_SEND_STATUS)) {
				iDeviceType = chdpp.iDeviceType;
				cout << "Device: " << iDeviceType << endl;
				return iDeviceType;
			} 
		} 
		cout << "Can't find Device Type" << endl;
		return iDeviceType;
	}

	// Return the Status of the DP5 device
	bool GetDppStatus()
	{
		if (chdpp.LibUsb_isConnected) { // send and receive status
			if (chdpp.LibUsb_SendCommand(XMTPT_SEND_STATUS)) {	// request status
				return true;
			} else {
				cout << "Error sending status." << endl;
			}
		} else {
			cout << "Device Not Connected." << endl;
		}

		return false;
	}

	// Read the full configuration of the DP5 Device.
	void ReadDppConfigurationFromHardware()
	{	
		bool bDisplayCfg=true;
		cout << "ReadDppConfigFromHdwre" <<endl;
		CONFIG_OPTIONS CfgOptions;
		
		//test configuration functions
		// Set options for XMTPT_FULL_READ_CONFIG_PACKET
		chdpp.CreateConfigOptions(&CfgOptions, "", chdpp.DP5Stat, false);
		cout << endl;
		cout << "\tRequesting Full Configuration..." << endl;
		chdpp.ClearConfigReadFormatFlags();	// clear all flags, set flags only for specific readback properties
		//chdpp.DisplayCfg = false;	// DisplayCfg format overrides general readback format
		chdpp.CfgReadBack = true;	// requesting general readback format
		if (chdpp.LibUsb_SendCommand_Config(XMTPT_FULL_READ_CONFIG_PACKET, CfgOptions)) {	// request full configuration
			if (chdpp.LibUsb_ReceiveData()) {
				if (chdpp.HwCfgReady) {		// config is ready
					bHaveConfigFromHW = true;
					if (bDisplayCfg) {
						cout << "\t\t\tConfiguration Length: " << (unsigned int)chdpp.HwCfgDP5.length() << endl;
						cout << "\t================================================================" << endl;
						cout << chdpp.HwCfgDP5 << endl;
						cout << "\t================================================================" << endl;
						cout << "\t\t\tScroll up to see configuration settings." << endl;
						cout << "\t================================================================" << endl;
					} else {
						cout << "\t\tFull configuration received." << endl;
					}
				}
			}
		}
	}



	// Display Preset Settings
	//		CConsoleHelper::strPresetCmd	// preset mode
	//		CConsoleHelper::strPresetVal	// preset setting
	void DisplayPresets()
	{
		if (bHaveConfigFromHW) {
			cout << "\t\t\tPreset Mode: " << chdpp.strPresetCmd << endl;
			cout << "\t\t\tPreset Settings: " << chdpp.strPresetVal << endl;
		}
	}

	// Display Preset Settings
	//		CONFIG_OPTIONS								// holds configuration command options
	//		CConsoleHelper::CreateConfigOptions			// creates configuration options from last status read
	//		CConsoleHelper::HwCfgDP5Out					// preset setting
	//		CConsoleHelper::LibUsb_SendCommand_Config	// send command with options
	void SendPresetAcquisitionTime(string strPRET)
	{
		CONFIG_OPTIONS CfgOptions;
		cout << "\tSetting Preset Acquisition Time..." << strPRET << endl;
		chdpp.CreateConfigOptions(&CfgOptions, "", chdpp.DP5Stat, false);
		CfgOptions.HwCfgDP5Out = strPRET;
		// send PresetAcquisitionTime string, bypass any filters, read back the mode and settings
		if (chdpp.LibUsb_SendCommand_Config(XMTPT_SEND_CONFIG_PACKET_EX, CfgOptions)) {
			ReadDppConfigurationFromHardware();	// read setting back
			DisplayPresets();							// display new presets
		} else {
			cout << "\t\tPreset Acquisition Time NOT SET" << strPRET << endl;
		}
	}


	void SendPRET(const char* cstr)
	{
		string strPRET(cstr);
		SendPresetAcquisitionTime(strPRET);
	}


	void free_memory(long* ptr) {
		delete[] ptr;
	}

	bool ResetDevice()
	{
		cout << "\t\tDisabling MCA for spectrum data/status clear." << endl;
		chdpp.LibUsb_SendCommand(XMTPT_DISABLE_MCA_MCS);
		Sleep(1000);
		cout << "\t\tClearing spectrum data/status." << endl;
		chdpp.LibUsb_SendCommand(XMTPT_SEND_CLEAR_SPECTRUM_STATUS);
		Sleep(1000);
		cout << "\t\tEnabling MCA for spectrum data acquisition with status ." << endl;
		chdpp.LibUsb_SendCommand(XMTPT_ENABLE_MCA_MCS);
		Sleep(1000);
		return true;
	}


	bool DisableMCA()
	{
		cout << "\t\tSpectrum acquisition with status done. Disabling MCA." << endl;
		chdpp.LibUsb_SendCommand(XMTPT_DISABLE_MCA_MCS);
		return true;
	} 


	long* AcquireSpectrum()
	{
		long* TEMP_DATA = new long[2048];

		if (chdpp.LibUsb_SendCommand(XMTPT_SEND_SPECTRUM_STATUS)) {	// request spectrum+status
				if (chdpp.LibUsb_ReceiveData()) {
					memcpy(TEMP_DATA, chdpp.DP5Proto.SPECTRUM.DATA, sizeof(long) * chdpp.DP5Proto.SPECTRUM.CHANNELS);
				}
			} else {
				cout << "\t\tProblem acquiring spectrum." << endl;
			}

		return TEMP_DATA;
	}

	// Acquire Spectrum
	//		CConsoleHelper::LibUsb_SendCommand(XMTPT_DISABLE_MCA_MCS)		//disable for data/status clear
	//		CConsoleHelper::LibUsb_SendCommand(XMTPT_SEND_CLEAR_SPECTRUM_STATUS)  //clear spectrum/status
	//		CConsoleHelper::LibUsb_SendCommand(XMTPT_ENABLE_MCA_MCS);		// enabling MCA for spectrum acquisition
	//		CConsoleHelper::LibUsb_SendCommand(XMTPT_SEND_SPECTRUM_STATUS)) // request spectrum+status
	//		CConsoleHelper::LibUsb_ReceiveData()							// process spectrum and data
	//		CConsoleHelper::ConsoleGraph()	(low resolution display)		// graph data on console with status
	//		CConsoleHelper::LibUsb_SendCommand(XMTPT_DISABLE_MCA_MCS)		// disable mca after acquisition
	void AcquireSpectrumOld()
	{
		int MaxMCA = 2;
		bool bDisableMCA;
		cout << "\tRunning spectrum test..." << endl;
		cout << "\t\tDisabling MCA for spectrum data/status clear." << endl;
		chdpp.LibUsb_SendCommand(XMTPT_DISABLE_MCA_MCS);
		Sleep(1000);
		cout << "\t\tClearing spectrum data/status." << endl;
		chdpp.LibUsb_SendCommand(XMTPT_SEND_CLEAR_SPECTRUM_STATUS);
		Sleep(1000);
		cout << "\t\tEnabling MCA for spectrum data acquisition with status ." << endl;
		chdpp.LibUsb_SendCommand(XMTPT_ENABLE_MCA_MCS);
		Sleep(1000);
		for(int idxSpectrum=0;idxSpectrum<MaxMCA;idxSpectrum++) {
			//cout << "\t\tAcquiring spectrum data set " << (idxSpectrum+1) << " of " << MaxMCA << endl;
			if (chdpp.LibUsb_SendCommand(XMTPT_SEND_SPECTRUM_STATUS)) {	// request spectrum+status
				if (chdpp.LibUsb_ReceiveData()) {
					bDisableMCA = true;				// we are aquiring data, disable mca when done
					system(CLEAR_TERM);
															
					chdpp.ConsoleGraph(chdpp.DP5Proto.SPECTRUM.DATA, chdpp.DP5Proto.SPECTRUM.CHANNELS, true, chdpp.DppStatusString);
					Sleep(2000);
				}
			} else {
				cout << "\t\tProblem acquiring spectrum." << endl;
				break;
			}
		}
		if (bDisableMCA) {
			//system("Pause");
			//cout << "\t\tSpectrum acquisition with status done. Disabling MCA." << endl;
			chdpp.LibUsb_SendCommand(XMTPT_DISABLE_MCA_MCS);
			Sleep(1000);
			}
	}

	// Read Configuration File
	//		CConsoleHelper::SndCmd.GetDP5CfgStr("PX5_Console_Test.txt");
	void ReadConfigFile()
	{
		std::string strCfg;
		strCfg = chdpp.SndCmd.AsciiCmdUtil.GetDP5CfgStr("PX5_Console_Test.txt");
		cout << "\t\t\tConfiguration Length: " << (unsigned int)strCfg.length() << endl;
		cout << "\t================================================================" << endl;
		cout << strCfg << endl;
		cout << "\t================================================================" << endl;
	}

	//Following is an example of loading a configuration from file 
	//  then sending the configuration to the DPP device.
	//	SendConfigFileToDpp("NaI_detector_cfg.txt");    // calls SendCommandString
	//	AcquireSpectrum();
	//
	bool SendCommandString(string strCMD) {
		CONFIG_OPTIONS CfgOptions;
		chdpp.CreateConfigOptions(&CfgOptions, "", chdpp.DP5Stat, false);
		CfgOptions.HwCfgDP5Out = strCMD;
		// send ASCII command string, bypass any filters, read back the mode and settings
		if (chdpp.LibUsb_SendCommand_Config(XMTPT_SEND_CONFIG_PACKET_EX, CfgOptions)) {
			// command sent
		} else {
			cout << "\t\tASCII Command String NOT SENT" << strCMD << endl;
			return false;
		}
		return true;
	}

	std::string ShortenCfgCmds(std::string strCfgIn) {
		std::string strCfg("");
		strCfg = strCfgIn;
		long lCfgLen=0;						//ASCII Configuration Command String Length
		lCfgLen = (long)strCfg.length();
		if (lCfgLen > 0) {		
			strCfg = chdpp.SndCmd.AsciiCmdUtil.ReplaceCmdText(strCfg, "US;", ";");
			strCfg = chdpp.SndCmd.AsciiCmdUtil.ReplaceCmdText(strCfg, "OFF;", "OF;");
			strCfg = chdpp.SndCmd.AsciiCmdUtil.ReplaceCmdText(strCfg, "RISING;", "RI;");
			strCfg = chdpp.SndCmd.AsciiCmdUtil.ReplaceCmdText(strCfg, "FALLING;", "FA;");
		}
		return strCfg;
	}

	// run GetDppStatus(); first to get PC5_PRESENT, DppType
	// Includes Configuration Oversize Fix 20141224
	bool SendConfigFileToDpp(){
		string strFilename = "PX5_Console_Test.txt";
		std::string strCfg;
		long lCfgLen=0;						//ASCII Configuration Command String Length
		bool bCommandSent=false;
		bool isPC5Present=false;
		int DppType=0;
		int idxSplitCfg=0;					//Configuration split position, only if necessary
		bool bSplitCfg=false;				//Configuration split flag
		std::string strSplitCfg("");		//Configuration split string second buffer
		bool isDP5_RevDxGains;
		unsigned char DPP_ECO;

		isPC5Present = chdpp.DP5Stat.m_DP5_Status.PC5_PRESENT;
		cout << "isPC5Present" << isPC5Present <<endl;

		DppType = chdpp.DP5Stat.m_DP5_Status.DEVICE_ID;

		cout << DppType <<endl;
		isDP5_RevDxGains = chdpp.DP5Stat.m_DP5_Status.isDP5_RevDxGains;

		cout << isDP5_RevDxGains <<endl;
		DPP_ECO = chdpp.DP5Stat.m_DP5_Status.DPP_ECO;

		cout << DPP_ECO <<endl;


		strCfg = chdpp.SndCmd.AsciiCmdUtil.GetDP5CfgStr(strFilename);
		strCfg = chdpp.SndCmd.AsciiCmdUtil.RemoveCmdByDeviceType(strCfg,isPC5Present,DppType,isDP5_RevDxGains,DPP_ECO);
		lCfgLen = (long)strCfg.length();
		if ((lCfgLen > 0) && (lCfgLen <= 512)) {		// command length ok
			cout << "\t\t\tConfiguration Length: " << lCfgLen << endl;
		} else if (lCfgLen > 512) {	// configuration too large, needs fix
			cout << "\t\t\tConfiguration Length (Will Shorten): " << lCfgLen << endl;
			strCfg = ShortenCfgCmds(strCfg);
			lCfgLen = (long)strCfg.length();
			if (lCfgLen > 512) {	// configuration still too large, split config
				cout << "\t\t\tConfiguration Length (Will Split): " << lCfgLen << endl;
				bSplitCfg = true;
				idxSplitCfg = chdpp.SndCmd.AsciiCmdUtil.GetCmdChunk(strCfg);
				cout << "\t\t\tConfiguration Split at: " << idxSplitCfg << endl;
				strSplitCfg = strCfg.substr(idxSplitCfg);
				strCfg = strCfg.substr(0, idxSplitCfg);
			}
		} else {
			cout << "\t\t\tConfiguration Length Error: " << lCfgLen << endl;
			return false;
		}
		bCommandSent = SendCommandString(strCfg);
		if (bSplitCfg) {
			// Sleep(40);			// may need delay here
			bCommandSent = SendCommandString(strSplitCfg);
		}
		return bCommandSent;
	}

	// Close Connection
	//		CConsoleHelper::LibUsb_isConnected			// LibUsb DPP connection indicator
	//		CConsoleHelper::LibUsb_Close_Connection()	// close connection
	void CloseConnection()
	{
		if (chdpp.LibUsb_isConnected) { // send and receive status
			if (chdpp.LibUsb_Close_Connection()) {
				cout << "DP5 device connection closed." << endl;
			}
		}
	}

	// Helper functions for saving spectrum files
	void SaveSpectrumConfig()
	{
		string strSpectrumConfig;
		chdpp.Dp5CmdList = chdpp.MakeDp5CmdList();	// ascii text command list for adding comments
		strSpectrumConfig = chdpp.CreateSpectrumConfig(chdpp.HwCfgDP5);	// append configuration comments
		chdpp.sfInfo.strSpectrumConfig = strSpectrumConfig;
	}

	// Saving spectrum file
	void SaveSpectrumFile(string strFilename)
	{
		string strSpectrum;											// holds final spectrum file
		chdpp.sfInfo.strSpectrumStatus = chdpp.DppStatusString;		// save last status after acquisition
		chdpp.sfInfo.m_iNumChan = chdpp.mcaCH;						// number channels in spectrum
		chdpp.sfInfo.SerialNumber = chdpp.DP5Stat.m_DP5_Status.SerialNumber;	// dpp serial number
		chdpp.sfInfo.strDescription = "Amptek Spectrum File";					// description
		chdpp.sfInfo.strTag = "TestTag";										// tag
		// create spectrum file, save file to string
		strSpectrum = chdpp.CreateMCAData(chdpp.DP5Proto.SPECTRUM.DATA,chdpp.sfInfo,chdpp.DP5Stat.m_DP5_Status);
		chdpp.SaveSpectrumStringToFile(strSpectrum, strFilename);	// save spectrum file string to file
	}

		
	// 	chdpp.DP5Stat.m_DP5_Status.SerialNumber = 0;
	// 	if (chdpp.DP5Stat.STATUS_MNX.SN == 0) { return 1; }
	// 	//SaveSpectrumFile();
	// 	//cout << "Press the Enter key to continue . . .";
	// 	//_getch(); 

	// 	//system(CLEAR_TERM);
	// 	//SendPresetAcquisitionTime("PRET=OFF;");
	// 	//cout << "Press the Enter key to continue . . .";
	// 	//_getch(); 

	// 	//system(CLEAR_TERM);
	// 	//ReadConfigFile();
	// 	//cout << "Press the Enter key to continue . . .";
	// 	//_getch(); 
}