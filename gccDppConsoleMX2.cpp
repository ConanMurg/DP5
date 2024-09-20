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

	// Identify if device is the MX2 device.
    int GetDeviceType()
	{
		int iDeviceType = 0;
		if (chdpp.LibUsb_isConnected) { // send and receive status
			if (chdpp.LibUsb_SendCommand(XMTPT_SEND_STATUS_MX2)) {	// request status
				iDeviceType = chdpp.iDeviceType;
				cout << "Device: " << iDeviceType << endl;
				return iDeviceType;
			} 
		} 
		cout << "Can't find Device Type" << endl;
		return iDeviceType;
	}


    void CloseConnection()
	{
		if (chdpp.LibUsb_isConnected) { // send and receive status
			if (chdpp.LibUsb_Close_Connection()) {
				cout << "MX2 device connection closed." << endl;
			}
		}
	}

	bool GetDppStatus()
	{
		if (chdpp.LibUsb_isConnected) { // send and receive status
			if (chdpp.LibUsb_SendCommand(XMTPT_SEND_STATUS_MX2)) {	// request status
				return true;
			} else {
				cout << "\t\tError sending status." << endl;
			}
		} else {
			cout << "Device Not Connected" << endl;
		}
		return false;
	}

	// Return the Status of the DP5 device
	const char* GetDppStatusRet()
	{
		if (chdpp.LibUsb_isConnected) { // send and receive status
			if (chdpp.LibUsb_SendCommand(XMTPT_SEND_STATUS_MX2)) {	// request status
				return chdpp.DppStatusString.c_str();
			} else {
				cout << "Error sending status." << endl;
			}
		} else {
			cout << "Device Not Connected." << endl;
		}

		return "";
	}

	void GetInterlockStatus()
	{
		if (chdpp.LibUsb_isConnected) { // send and receive status
			cout << endl;
			cout << "\tRequesting Interlock Status..." << endl;
			if (chdpp.LibUsb_SendCommand(XMTPT_SEND_TUBE_ILOCK_TABLE_MX2)) {	// request status
				cout << "\t\tInterlock Status sent." << endl;
			} else {
				cout << "\t\tError sending status." << endl;
			}
		} else {
			cout << "Device Not Connected" << endl;
		}
	}

	// Get Fault Record
	void FaultRecord()
	{
		if (chdpp.LibUsb_isConnected) {
			cout << "\tRequesting Fault Record ..." << endl;
			if (chdpp.LibUsb_SendCommand(XMTPT_SEND_FAULT_RECORD_MX2)) {
				cout << "Sent Fault Record Request..." << endl;
			} else {
				cout << "Failed to send Fault Record Request" << endl;
			}
		} else {
			cout << "Device Not Connected" << endl;
		}
	}

	void ReadHVCfg()
	{
		if (chdpp.LibUsb_isConnected) {
			string strCmd;
			cout << "\tRequesting HV and I Configuration..." << endl;
			strCmd = "HVSE=?;CUSE=?;";
			chdpp.SendCommandDataMX2(XMTPT_READ_TEXT_CONFIGURATION_MX2, strCmd);
			//Sleep(1000);
			//cout << "\t\t\tkV: " << chdpp.strHV << endl;
			//cout << "\t\t\tuA: " << chdpp.strI << endl;
		} else {
			cout << "Device Not Connected" << endl;
		}
	}

	void TurnHVOn()
	{
		cout << "\t\t\tTurning Tube ON Now" << endl;
		
		string stringHV;
		string stringI;

		stringHV = "15.00";
		stringI = "15.00";
		
		if (chdpp.LibUsb_isConnected) {

			chdpp.SendMX2_HVandI(stringHV, stringI);
			bTubeOn = true;
		} else {
		cout << "Device not connected" << endl;
		}
	}

	void TurnHVOff()
	{
		cout << "\t\t\tTurning Tube OFF Now" << endl;
		
		string stringHV;
		string stringI;

		stringHV = "0";
		stringI = "0";
		
		if (chdpp.LibUsb_isConnected) {
			chdpp.SendMX2_HVandI(stringHV, stringI);
			bTubeOn = false;
		} else {
		cout << "Device not connected" << endl;
		}
	}


	// void NetFinder()
	// {
	// 	if (chdpp.LibUsb_SendCommand(XMTPT_SEND_NETFINDER_PACKET)) {	// request status
	// 		cout << "\t\t\tNetfinder Packet." << endl;
	// 	} else {
	// 		cout << "\t\t\tError sending status." << endl;
	// 	}
	// }


	// void SendTubeOn()
	// {
	// 	cout << "\t\t\tTurning Tube ON Now" << endl;
		
	// 	string stringHV;
	// 	string stringI;

	// 	stringHV = "15.00";
	// 	stringI = "15.00";
		
		
	// 	chdpp.SendMX2_HVandI(stringHV, stringI);
	// 	GetDppStatus();
	// 	bTubeOn = true;
	// 	for (int i = 0; i < 100; ++i)
	// 		{	
	// 			cout << i << endl;
	// 			//chdpp.SendMX2_HVandI(stringHV, stringI);
	// 			// Sleep(500);
	// 			// KeepAlive();
	// 			GetDppStatus();
	// 			if (! bTubeOn) {
	// 				break;
	// 			}
	// 			Sleep(200);
	// 		}

	// 	TurnHVOff();
	// 	cout << "Turning Tube OFF" << endl;
	// }

	// void TurnHVOn()
	// {
	// 	cout << "\t\t\tTurning Tube ON Now" << endl;
		
	// 	string stringHV;
	// 	string stringI;

	// 	stringHV = "15.00";
	// 	stringI = "15.00";
		
	// 	if (chdpp.LibUsb_isConnected) { // send and receive status

	// 		chdpp.SendMX2_HVandI(stringHV, stringI);
	// 		GetDppStatus();
	// 		bTubeOn = true;
	// 		// for (int i = 0; i < 100; ++i)
	// 		// {	
	// 		// 	cout << i << endl;
	// 		// 	//chdpp.SendMX2_HVandI(stringHV, stringI);
	// 		// 	// Sleep(500);
	// 		// 	// KeepAlive();
	// 		// 	GetDppStatus();
	// 		// 	if (! bTubeOn) {
	// 		// 		break;
	// 		// 	}
	// 		// 	Sleep(200);
	// 		// }
	// 		// TurnHVOff();

	// 		// if (chdpp.LibUsb_ReceiveData()) {
	// 		// 	cout << "\t\t\tReceiving Acknowledgement Packet..." << endl;
				
	// 		// } else {
	// 		// 	cout << "\t\tError receiving Acknowledgement Packet . . ." << endl;
	// 		// }
	// 	} else {
	// 	cout << "Device not connected" << endl;
	// 	}
	// }


	bool TurnVolumeOn()
	{
		cout << "\t\t\tTurning Volume ON Now" << endl;
		string strVol;

		strVol= "ON";
		
		if (chdpp.LibUsb_isConnected) { // send and receive status

			chdpp.SendMX2_Volume(strVol);
			return 1;

		} else {
			cout << "Device not connected" << endl;
		}
		return 0;
		
	}


	bool TurnVolumeOff()
	{
		cout << "\t\t\tTurning Volume OFF Now" << endl;
		string strVol;

		strVol= "OFF";
		
		if (chdpp.LibUsb_isConnected) { // send and receive status

			chdpp.SendMX2_Volume(strVol);
			return 1;

		} else {
			cout << "Device not connected" << endl;
		}
		return 0;
	}


	// void Warmup()
	// {
	// 	cout << "Running Daily Warmup" << endl;
	// 	if (chdpp.LibUsb_isConnected) {
	// 		chdpp.DailyWarmup();
	// 		for (int i = 0; i < 30; ++i)
	// 		{
	// 			ReadHVCfg();
	// 			Sleep(1000);
	// 		}
	// 		TurnHVOff();
	// 	} else {
	// 		cout << "Device Not Connected" << endl;
	// 	}
	// }
}