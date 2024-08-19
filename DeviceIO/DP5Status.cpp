
#include "DP5Status.h"
#include "stringex.h"
#include "DppConst.h"
#include <time.h>
#include <cstring>
#ifdef _MSC_VER
    #pragma warning(disable:4996)
#endif

CDP5Status::CDP5Status(void)
{
}

CDP5Status::~CDP5Status(void)
{
}


string CDP5Status::MX2TubeTypeEnumToString(MX2TubeTypeEnum mx2ttTubeType)
{
	string strMX2TubeTypeEnum("");
    strMX2TubeTypeEnum = "Tube Type Unknown";
	switch(mx2ttTubeType) {
		case mx2tubetype50kV4W:
            strMX2TubeTypeEnum = "50kV4W";
			break;
		case mx2tubetype50kV10W:
            strMX2TubeTypeEnum = "50kV10W";
			break;
		case mx2tubetype70kV10W:
            strMX2TubeTypeEnum = "70kV10W";
			break;
	}
	return(strMX2TubeTypeEnum);
}

void CDP5Status::Process_MNX_Status(Stat_MNX *STATUS_MNX)
{
		 //called by ParsePacket
    float sngTemp=0.0;
    
    isMiniX2 = true;
    
    STATUS_MNX->FW = STATUS_MNX->RAW[4];
    STATUS_MNX->Build = STATUS_MNX->RAW[5] & 15;
	STATUS_MNX->RAW[3] = STATUS_MNX->RAW[3] & 0x7F;		// uses only 7 bits
//    STATUS_MNX->SN = (long)(STATUS_MNX->RAW[3] & 0x7F) * (2 ^ 24) + (long)(STATUS_MNX->RAW[2]) * (2 ^ 16) + (long)(STATUS_MNX->RAW[1]) * 256 + (long)(STATUS_MNX->RAW[0])
	STATUS_MNX->SN = (long)DppUtil.LongWordToDouble(0, STATUS_MNX->RAW);

    sngTemp = (float)((STATUS_MNX->RAW[15] & 15)) * (float)(256.0) + (float)(STATUS_MNX->RAW[14]);
    STATUS_MNX->PWR_IN = (float)(sngTemp / 1000.0 * 3.26);
    
    sngTemp = (float)((STATUS_MNX->RAW[13] & 15)) * (float)(256.0) + (float)(STATUS_MNX->RAW[12]);
    STATUS_MNX->PWR_XRAY = (float)(sngTemp / 1000.0 * 3.26);

    sngTemp = (float)((STATUS_MNX->RAW[11] & 15)) * (float)(256.0) + (float)(STATUS_MNX->RAW[10]);
    STATUS_MNX->LAMP_I = (float)(sngTemp / 1000.0 * 12.44);    // in mA
    
    sngTemp = (float)((STATUS_MNX->RAW[9] & 15)) * (float)(256.0) + (float)(STATUS_MNX->RAW[8]);
    STATUS_MNX->I_MON_RAW = (float)(sngTemp / 1000.0);                                 // monitor, in V

    sngTemp = (float)((STATUS_MNX->RAW[7] & 15)) * (float)(256.0) + (float)(STATUS_MNX->RAW[6]);
    STATUS_MNX->HV_MON_RAW = (float)(sngTemp / 1000.0);                                 // monitor, in V

    STATUS_MNX->HV_EN = DppUtil.ByteToBool(STATUS_MNX->RAW[16] & 0x80);          // D7
    STATUS_MNX->PWR_XRAY_FLAG = DppUtil.ByteToBool(STATUS_MNX->RAW[16] & 0x20);  // D5
    STATUS_MNX->INTERLOCK_STATE = (STATUS_MNX->RAW[16] & 15);  // D3-D0
    
    sngTemp = (float)(STATUS_MNX->RAW[17]);
	if (sngTemp > 127) { // 8-bit signed
        sngTemp = sngTemp - 256;
	}
    STATUS_MNX->Temp = sngTemp;
    
    STATUS_MNX->LAST_FAULT = (STATUS_MNX->RAW[18] & 15);							    // D3-D0
    STATUS_MNX->SPEAKER_EN = DppUtil.ByteToBool(STATUS_MNX->RAW[18] & 128);			    // D7
    STATUS_MNX->DISABLE_FAULT_CHECKS = DppUtil.ByteToBool(STATUS_MNX->RAW[18] & 64);    // D6
    STATUS_MNX->DISABLE_LIMIT_CHECKS = DppUtil.ByteToBool(STATUS_MNX->RAW[18] & 32);    // D5
    STATUS_MNX->ULTRAMINI_PRESENT = DppUtil.ByteToBool(STATUS_MNX->RAW[18] & 16);       // D4

    STATUS_MNX->WARMUP_STEP = STATUS_MNX->RAW[19];
    STATUS_MNX->WARMUP_TIME = STATUS_MNX->RAW[20] + (STATUS_MNX->RAW[21] & 0x7F) * 256;   // avoid overflow
    
	STATUS_MNX->RAW[25] = STATUS_MNX->RAW[25] & 0x7F;  //uses only 7 bits
//    STATUS_MNX->RUNTIME = (long)(STATUS_MNX->RAW[25] & 0x7F) * (2 ^ 24) + (long)(STATUS_MNX->RAW[24]) * (2 ^ 16) + (long)(STATUS_MNX->RAW[23]) * 256 + (long)(STATUS_MNX->RAW[22]);
    STATUS_MNX->RUNTIME = (long)DppUtil.LongWordToDouble(22, STATUS_MNX->RAW);
    
    //STATUS_MNX->HVScale = Format((STATUS_MNX->RAW[26] * 256 + STATUS_MNX->RAW[27]) / 256, "##0.000")
	STATUS_MNX->HVScale = float((STATUS_MNX->RAW[26] * 256.0 + STATUS_MNX->RAW[27]) / 256.0);
    STATUS_MNX->HV_MON = STATUS_MNX->HV_MON_RAW * STATUS_MNX->HVScale;    // in kV
    
    //STATUS_MNX->IScale = Format((STATUS_MNX->RAW[28] * 256 + STATUS_MNX->RAW[29]) / 256, "##0.000")
    STATUS_MNX->IScale = float((STATUS_MNX->RAW[28] * 256.0 + STATUS_MNX->RAW[29]) / 256.0);
    STATUS_MNX->I_MON = STATUS_MNX->I_MON_RAW * STATUS_MNX->IScale;    // in uA

		// Tube P/N, S/N and Description are in tube table
		// Mini-X S/N 
}

// create packet for existing functions to decode
void CDP5Status::MakeFaultMX2Packet(FaultTableMX2PacketType mx2Packet, Packet_In *MX2PIN, unsigned char PIN_buffer[], long idxStart, long idxEnd)
{
	const unsigned char pktPID1_RCV_STATUS = 0x80;
	const unsigned char pktPID1_RCV_SCOPE_MISC = 0x0D;
	const unsigned char pktRCVPT_MX2_STATUS = 0x02;
	const unsigned char pktRCVPT_MX2_TUBE_ILOCK_TABLE = 0x0D;
	const unsigned char pktRCVPT_MX2_WARMUP_TABLE = 0x0E;
	const unsigned char pktRCVPT_MX2_TIMESTAMP_RECORD = 0x0F;
	long idxDataIn;
	long idxDataOut=0;
	
	for(idxDataIn=idxStart;idxDataIn<=idxEnd;idxDataIn++)
	{
		MX2PIN->DATA[idxDataOut] = PIN_buffer[idxDataIn];
		idxDataOut++;
	}

	switch(mx2Packet) {
		case ftptTimestamp:
			MX2PIN->PID1 = pktPID1_RCV_SCOPE_MISC;
			MX2PIN->PID2 = pktRCVPT_MX2_TIMESTAMP_RECORD;
			MX2PIN->LEN = 0x0F;
			break;
		case ftptStatus:
			MX2PIN->PID1 = pktPID1_RCV_STATUS;
			MX2PIN->PID2 = pktRCVPT_MX2_STATUS;
			MX2PIN->LEN = 0x40;
			break;
		case ftptTube:
			MX2PIN->PID1 = pktPID1_RCV_SCOPE_MISC;
			MX2PIN->PID2 = pktRCVPT_MX2_TUBE_ILOCK_TABLE;
			MX2PIN->LEN = 0x5E;
			break;
		case ftptWarmup:
			MX2PIN->PID1 = pktPID1_RCV_SCOPE_MISC;
			MX2PIN->PID2 = pktRCVPT_MX2_WARMUP_TABLE;
			MX2PIN->LEN = 0x30;
			break;
	}
}

string CDP5Status::Process_MNX_Fault_Record(Packet_In PIN)
{
    string strFault("");
	unsigned char PIN_buffer[520];
    long idxData;
	Packet_In MX2PIN;
	Stat_MNX STATUS_MNX2;
	TubeInterlockTableType TubeInterlockTableMX2;
	MiniX2WarmUpTable WarmUpTableMX2;
	string strNowDTS("");
	float flTemp=0.0;
	stringex strfn;

    strFault = "======= Fault Record ===========\r\n";
    strFault += "======= Unit Info ===========\r\n";
	strNowDTS = GetNowTimeString();
    strFault += "Fault Record read at: " + strNowDTS + "\r\n";

	//-----------------------------------------------------------------
	//---- Save the data to local storage -----------------------------
	//-----------------------------------------------------------------
	memset(PIN_buffer,0,sizeof(PIN_buffer));
	for(idxData=0; idxData<PIN.LEN; idxData++) {
        PIN_buffer[idxData] = PIN.DATA[idxData];	// copy the fault record packet
	}

	//-----------------------------------------------------------------
	//---- Get the timestamp table ------------------------------------
	//-----------------------------------------------------------------
	memset(MX2PIN.DATA,0,sizeof(MX2PIN.DATA));
	MakeFaultMX2Packet(ftptTimestamp, &MX2PIN, PIN_buffer, 1, 15);
	time_t ttTimeStamp;		// holds tm for calcs, not used here
	strFault += "Fault occurred at: ";
	strFault += Process_MNX_Timestamp(MX2PIN, &ttTimeStamp);
	strFault += "\r\n";

	//-----------------------------------------------------------------
	//---- Get the status ---------------------------------------------
	//-----------------------------------------------------------------
	//memset(MX2PIN.DATA,0,sizeof(MX2PIN.DATA));
	MakeFaultMX2Packet(ftptStatus, &MX2PIN, PIN_buffer, 18, 81);
	for(idxData=0; idxData<MX2PIN.LEN; idxData++) {		// load the raw data into the status packet
        STATUS_MNX2.RAW[idxData] = MX2PIN.DATA[idxData];	// copy the fault record packet
	}
	Process_MNX_Status(&STATUS_MNX2);
	strFault += MiniX2_StatusToString(STATUS_MNX2);

	//////////-----------------------------------------------------------------
	//////////---- Get the Tube Table -----------------------------------------
	//////////-----------------------------------------------------------------
    //strFault += "======= Tube Table ===========\r\n";
	memset(MX2PIN.DATA,0,sizeof(MX2PIN.DATA));
	MakeFaultMX2Packet(ftptTube, &MX2PIN, PIN_buffer, 82, 175);
	strFault += Process_MNX_Tube_Table(MX2PIN, &TubeInterlockTableMX2);

	//-----------------------------------------------------------------
	//---- Get the Warmup Table ---------------------------------------
	//-----------------------------------------------------------------
    //strFault += "======= Warmup Table ===========\r\n";
	memset(MX2PIN.DATA,0,sizeof(MX2PIN.DATA));
	MakeFaultMX2Packet(ftptWarmup, &MX2PIN, PIN_buffer, 176, 223);
	strFault += Process_MNX_Warmup_Table(MX2PIN, &WarmUpTableMX2, WarmUpTableTypeMX2);

    strFault += "======= Additional Fault Info ===========\r\n";

    flTemp = (float)((((float)(PIN_buffer[226] & 127) * 256.0) + (float)PIN_buffer[227]) / 256.0);
	strFault += "Tube HV Setpoint: " + strfn.Format("%0.2fkV\r\n",flTemp);

    flTemp = (float)((((float)(PIN_buffer[228] & 127) * 256.0) + (float)PIN_buffer[229]) / 256.0);
    strFault += "Tube Current Setpoint: " + strfn.Format("%0.2fuA\r\n", flTemp);

	return(strFault);
}

string CDP5Status::MiniX2FaultToString(unsigned char idxFault)
{
    string strFault("Unknown");
	switch(idxFault) {
		case 0:
			strFault = "Interlock: CLOSED";
			break;
		case 1:
			strFault = "Interlock: OPEN";
			break;
		case 2:
			strFault = "Interlock: SHORTED";
			break;
		case 3:
			strFault = "Supply: VIN UV";
			break;
		case 4:
			strFault = "Supply: VIN OV";
			break;
		case 5:
			strFault = "Tube: HVMON LOW";
			break;
		case 6:
			strFault = "Tube: HVMON HIGH";
			break;
		case 7:
			strFault = "Tube: IMON LOW";
			break;
		case 8:
			strFault = "Tube: IMON HIGH";
			break;
		case 9:
			strFault = "USB: Disconnected";
			break;
		case 10:
			strFault = "USB: Stopped Talking";
			break;
		case 11:
			strFault = "Warmup: Done";
			break;
		case 12:
			strFault = "TBD";
			break;
		case 13:
			strFault = "TBD";
			break;
		case 14:
			strFault = "TBD";
			break;
		case 15:
			strFault = "TBD";
			break;
	}
	return(strFault);
}

string CDP5Status::MiniX2_StatusToString(Stat_MNX STATUS_MNX)
{
    string strStatus("");
	long H;
	long m;
	long S;
	stringex strfn;

    if (STATUS_MNX.SN == 0) { 
		return(strStatus);
	}
	
    strStatus += "======= Mini-X2 Status ===========\r\n";
	strStatus += "Mini-X2 Serial Number: " + strfn.Format("%04d\r\n", STATUS_MNX.SN);
    strStatus += "Firmware Version: " + strfn.Format("%d.",STATUS_MNX.FW / 16) + strfn.Format("%02d.",STATUS_MNX.FW & 15) + strfn.Format("%02d\r\n", STATUS_MNX.Build);
    strStatus += "Supply Voltage (Mini-X): " + strfn.Format("%0.2fV\r\n", STATUS_MNX.PWR_IN);
    strStatus += "Supply Voltage (to tube): " + strfn.Format("%0.2fV\r\n", STATUS_MNX.PWR_XRAY);
    strStatus += "Tube HV Mon: " + strfn.Format("%0.2fkV\r\n", STATUS_MNX.HV_MON);
    strStatus += "Tube HV Mon (raw): " + strfn.Format("%0.3fV\r\n", STATUS_MNX.HV_MON_RAW);
    strStatus += "Tube Current Mon: " + strfn.Format("%0.2fuA\r\n", STATUS_MNX.I_MON);
    strStatus += "Tube Current Mon (raw): " + strfn.Format("%0.3fV\r\n", STATUS_MNX.I_MON_RAW);
    strStatus += "Interlock Current: " + strfn.Format("%0.2fmA\r\n", STATUS_MNX.LAMP_I);
    strStatus += "Board Temp: " + strfn.Format("%0.0f°C\r\n", STATUS_MNX.Temp);
    if (STATUS_MNX.HV_EN) {
        strStatus += "HV_EN State: Enabled\r\n";
    } else {
        strStatus += "HV_EN State: Disabled\r\n";
    }
    strStatus += "Current Fault: " + MiniX2FaultToString(STATUS_MNX.INTERLOCK_STATE) + "\r\n";
    if (STATUS_MNX.LAST_FAULT > 0) {
        strStatus += "Previous Fault: " + MiniX2FaultToString(STATUS_MNX.LAST_FAULT) + "\r\n";
    } else {
        strStatus += "Previous Fault: none\r\n";
    }
    
    H = STATUS_MNX.RUNTIME / 3600;
    m = (STATUS_MNX.RUNTIME - H * 3600) / 60;
    S = STATUS_MNX.RUNTIME - H * 3600 - m * 60;
    strStatus += "Tube Runtime: " + strfn.Format("%dh", H) + " " + strfn.Format("%02dm", m) + " " + strfn.Format("%02ds\r\n", S);
    
    if (STATUS_MNX.DISABLE_LIMIT_CHECKS) {
        strStatus += "Command Limit Checks: Disabled\r\n";
    } else {
        strStatus += "Command Limit Checks: Enabled\r\n";
    }
    
    if (STATUS_MNX.DISABLE_FAULT_CHECKS) {
        strStatus += "Fault Checks: Disabled\r\n";
    } else {
        strStatus += "Fault Checks: Enabled\r\n";
    }
    
    if (STATUS_MNX.SPEAKER_EN) {
        strStatus += "Speaker: Enabled\r\n";
    } else {
        strStatus += "Speaker: Disabled\r\n";
    }
    
    if (STATUS_MNX.WARMUP_STEP > 127) {    // MSB is set if warmup is running
        strStatus += "Warmup Step: " + strfn.Format("%d\r\n", STATUS_MNX.WARMUP_STEP & 15);
        strStatus += "Warmup step time: " + strfn.Format("%d\r\n", STATUS_MNX.WARMUP_TIME);
    } else {
        strStatus += "Warmup: not running\r\n";
    }
	return(strStatus);
}

////===================================================================================
//// Here's the tube table structure from the Mini-X embedded code
////struct MNX_TT {                     // 64-byte tube and interlock table
////    unsigned char PN[20];           // 0-19: 20-character P/N
////    unsigned char SN[12];           // 20-31: 12-character S/N
////    unsigned char HVMIN;            // 32
////    unsigned char HVMAX;            // 33
////    unsigned char IMIN;             // 34
////    unsigned int  IMAX;             // 35-36
////    unsigned char PMAX;             // 37
////    unsigned char HVMIN_alt;        // 38
////    unsigned char HVMAX_alt;        // 39
////    unsigned char IMIN_alt;         // 40
////    unsigned int  IMAX_alt;         // 41-42
////    unsigned char PMAX_alt;         // 43
////    unsigned int  HVSCALE;          // 44-45
////    unsigned int  ISCALE;           // 46-47
////    unsigned char MNX_Interlock_V;      // 48: 19.5mV/LSB
////    unsigned int  MNX_Interlock_MinI;   // 49-50: 199uA/LSB
////    unsigned int  MNX_Interlock_MaxI;   // 51-52: 199uA/LSB
////    unsigned char MISC[9];          // 53-61
////    unsigned int  CHKSUM;           // 62-63
////    };
////===================================================================================
string CDP5Status::Process_MNX_Tube_Table(Packet_In PIN, TubeInterlockTableType *TubeInterlockTable)
{
//// This is called by Form1.ParsePacket
//    Dim L As Long
    long idxData;
    string strTubeTable("");
	string strTemp("");
	stringex strfn;
    strTubeTable += "======= Tube Table ===========\r\n";

	//-----------------------------------------------------------------
	//---- Tube Part Number -------------------------------------------
	//-----------------------------------------------------------------
    strTemp = "";
	for(idxData=0;idxData<=19;idxData++) {
		if (PIN.DATA[idxData] > 0) {
			strTemp += strfn.Format("%c",PIN.DATA[idxData]);
		} else {
			break;
		}
	}
    TubeInterlockTable->PartNumber = strTemp;
	strTubeTable += "Tube Part Number: " + strTemp;
	strTubeTable += "\r\n";
	//-----------------------------------------------------------------
	//---- Tube Serial Number -----------------------------------------
	//-----------------------------------------------------------------
    strTemp = "";
	for(idxData=20;idxData<=31;idxData++) {
		if (PIN.DATA[idxData] > 0) {
			strTemp += strfn.Format("%c",PIN.DATA[idxData]);
		} else {
			break;
		}
	}
	TubeInterlockTable->SerialNumber = strTemp;
	strTubeTable += "Tube Serial Number: " + TubeInterlockTable->SerialNumber;
	strTubeTable += "\r\n";
	//-----------------------------------------------------------------
	//---- Tube Description -------------------------------------------
	//-----------------------------------------------------------------
    strTemp = "";
	for(idxData=62;idxData<=93;idxData++) {
		if (PIN.DATA[idxData] > 0) {
			strTemp += strfn.Format("%c",PIN.DATA[idxData]);
		} else {
			break;
		}
	}
    TubeInterlockTable->Description = strTemp;
    strTubeTable += "Tube Description: " + strTemp;
	strTubeTable += "\r\n";
	//-----------------------------------------------------------------
	TubeInterlockTable->HVmin_kV = (float)PIN.DATA[32];		// 8-bit HVMin
	strTubeTable += strfn.Format("HVmin: %0.0fkV\r\n", TubeInterlockTable->HVmin_kV);
	//-----------------------------------------------------------------
    TubeInterlockTable->HVmax_kV = (float)PIN.DATA[33];		// 8-bit HVMax                      
	strTubeTable += strfn.Format("HVmax: %0.0fkV\r\n", TubeInterlockTable->HVmax_kV);
	//-----------------------------------------------------------------
    TubeInterlockTable->Imin_uA = (float)PIN.DATA[34];      // 8-bit Imin                
	strTubeTable += strfn.Format("Imin: %0.0fuA\r\n", TubeInterlockTable->Imin_uA);
	//-----------------------------------------------------------------
    TubeInterlockTable->Imax_uA = (float)(PIN.DATA[35] * 256 + PIN.DATA[36]);   // 16 bit Imax, 35 MSB, 36 LSB
	strTubeTable += strfn.Format("Imax: %0.0fuA\r\n", TubeInterlockTable->Imax_uA);
	//-----------------------------------------------------------------
    TubeInterlockTable->Pmax_Anode_W = (float)((float)PIN.DATA[37] / 4.0);	// 8-bit 6.2 fixed point Pmax
	strTubeTable += strfn.Format("Pmax anode: %0.2fW\r\n", TubeInterlockTable->Pmax_Anode_W);
	//-----------------------------------------------------------------
    TubeInterlockTable->HVscale_kVperV = (float)(((float)PIN.DATA[44] * 256.0 + (float)PIN.DATA[45]) / 256.0);	// 16 bit 8.8 fixed point, 44 MSB, 45 LSB
	strTubeTable += strfn.Format("HVscale: %0.2fkV/V\r\n", TubeInterlockTable->HVscale_kVperV);
	//-----------------------------------------------------------------
    TubeInterlockTable->Iscale_uAperV = (float)(((float)PIN.DATA[46] * 256.0 + (float)PIN.DATA[47]) / 256.0);	// 16 bit 8.8 fixed point, 46 MSB, 47 LSB
	strTubeTable += strfn.Format("Iscale: %0.2fuA/V\r\n", TubeInterlockTable->Iscale_uAperV);
	//-----------------------------------------------------------------

	strTubeTable += "======= Interlock Table ===========\r\n";

	//-----------------------------------------------------------------
	TubeInterlockTable->Interlock_Voltage_V = (float)((float)((float)PIN.DATA[48]) * 0.02);	// 20.0mV/LSB, 8 bit
	strTubeTable += strfn.Format("Interlock Voltage: %0.2fV\r\n", TubeInterlockTable->Interlock_Voltage_V);
	//-----------------------------------------------------------------
    TubeInterlockTable->Interlock_min_I_mA = (float)(((float)PIN.DATA[49] * 256.0 + (float)PIN.DATA[50]) * 0.01244);	// 12.44uA/LSB, 12 bit, 49 MSB, 50 LSB
	strTubeTable += strfn.Format("Interlock Current Min: %0.2fmA\r\n", TubeInterlockTable->Interlock_min_I_mA);
	//-----------------------------------------------------------------
    TubeInterlockTable->Interlock_max_I_mA = (float)(((float)PIN.DATA[51] * 256.0 + (float)PIN.DATA[52]) * 0.01244);	// 12.44uA/LSB, 12 bit, 51 MSB, 52 LSB
	strTubeTable += strfn.Format("Interlock Current Max: %0.2fmA\r\n", TubeInterlockTable->Interlock_max_I_mA);
	//-----------------------------------------------------------------
    TubeInterlockTable->Tube_Supply_Min_V = (float)(((float)PIN.DATA[53]) / 16.0);	// 8-bit 4.4 fixed point Tube supply min
	strTubeTable += strfn.Format("Tube Supply Min: %0.2fV\r\n", TubeInterlockTable->Tube_Supply_Min_V);
	//-----------------------------------------------------------------
    TubeInterlockTable->Tube_Supply_Max_V = (float)(((float)PIN.DATA[54]) / 16.0);	// 8-bit 4.4 fixed point Tube supply max
	strTubeTable += strfn.Format("Tube Supply Max: %0.2fV\r\n", TubeInterlockTable->Tube_Supply_Max_V);
    // 55-61: misc (unused)
	//-----------------------------------------------------------------
	return(strTubeTable);
}

string CDP5Status::Process_MNX_Warmup_Table(Packet_In PIN, MiniX2WarmUpTable *WarmUpTable, MX2WarmupTableType WarmUpTableType)
{
    long idxWUEntry=0;
    string strTable("");
	long lWup;
    string strErrorHelp("");
	stringex strfn;

    strErrorHelp = "Checking WarmUpTable Struct";
    //the table values are stored in variant arrays to allow quick array init
    strErrorHelp = "WarmUpTable Struct needs init";
    switch(WarmUpTableType) {
        case mx2tbltyp50kV10W:
            strErrorHelp = "Create50kV10W_WarmUpTable WarmUpTable";
            Create50kV10W_WarmUpTable(WarmUpTable);
			break;
        case mx2tbltyp70kV10W:
            strErrorHelp = "Create70kV10W_WarmUpTable WarmUpTable";
            Create70kV10W_WarmUpTable(WarmUpTable);
			break;
        case mx2tbltyp50kV4W:
            strErrorHelp = "Create50kV4W_WarmUpTable WarmUpTable";
            Create50kV4W_WarmUpTable(WarmUpTable);
			break;
        case mx2tbltypCustom:
            strErrorHelp = "Create50kV10W_WarmUpTable WarmUpTable";
            Create50kV10W_WarmUpTable(WarmUpTable);
			break;
        default:
            strErrorHelp = "Not initialized";
			break;
    }
    strErrorHelp = "WarmUpTable Struct init done";
//    
    strErrorHelp = "Loading WarmUpTable data from PIN";
	for(idxWUEntry = 0; idxWUEntry < 12; idxWUEntry++) {
        WarmUpTable->fltWarmUpVoltage[idxWUEntry] = (float)((long)(PIN.DATA[idxWUEntry * 4]));
        WarmUpTable->fltWarmUpCurrent[idxWUEntry] = (float)((long)(PIN.DATA[idxWUEntry * 4 + 1]));
        WarmUpTable->fltWarmUpTime[idxWUEntry] = (float)((long)(PIN.DATA[idxWUEntry * 4 + 2]) * 256 + (long)(PIN.DATA[idxWUEntry * 4 + 3]));
    }
    strErrorHelp = "Set Warmup type (if any)";
    WarmUpTable->WarmUpTableType = WarmUpTableType;
    strErrorHelp = "Calc daily warmup total";
    WarmUpTable->DailyWarmupTotalTime = CalcWarmupTotalTime(WarmUpTable->fltWarmUpTime, DailyWarmupIndexStart);
    strErrorHelp = "Calc monthly warmup total";
    WarmUpTable->MonthlyWarmupTotalTime = CalcWarmupTotalTime(WarmUpTable->fltWarmUpTime, MonthlyWarmupIndexStart);

    strTable += "======= Warmup Table ===========\r\n";
    strErrorHelp = "Creating Warmup Table Display";
	for(idxWUEntry = 0; idxWUEntry < 12; idxWUEntry++) {
        if (idxWUEntry < 6) {
            strTable += "Daily Warmup Step #" + strfn.Format("%01d: ", idxWUEntry + 1);
        } else {
            strTable += "Monthly Warmup Step #" + strfn.Format("%01d: ", idxWUEntry - 5);
        }
        strTable += strfn.Format("%0.0fkV, ", WarmUpTable->fltWarmUpVoltage[idxWUEntry]);
        strTable += strfn.Format("%0.0fuA, ", WarmUpTable->fltWarmUpCurrent[idxWUEntry]);
        strTable += strfn.Format("%0.0fsec\r\n", WarmUpTable->fltWarmUpTime[idxWUEntry]);
        if (idxWUEntry == 5) {
            lWup = (long)(WarmUpTable->DailyWarmupTotalTime);
            //MsgBox lWup
            strTable += "Daily Warmup Total time = " + strfn.Format("%01dm ", lWup / 60) + strfn.Format("%02ds\r\n", lWup % 60);
        } else if (idxWUEntry == 11) {
            lWup = (long)(WarmUpTable->MonthlyWarmupTotalTime);
            //MsgBox lWup
            strTable += "Monthly Warmup Total time = " + strfn.Format("%01dm ", lWup / 60) + strfn.Format("%02ds\r\n", lWup % 60);
        }
    }
	return(strTable);
}

string CDP5Status::GetNowTimeString()
{
	char buffer[256];
	time_t tNow;
    struct tm tmNow;
	string strFormat("%m/%d/%Y %I:%M:%S %p"); 
	stringex strfn;
	string strDTS("");

    tNow = time(NULL);
    tmNow = *localtime(&tNow);
	strftime(buffer, sizeof(buffer), strFormat.c_str(), &tmNow);
	strDTS = strfn.Format("%s",buffer);
	return(strDTS);
}

string CDP5Status::Process_MNX_Timestamp(Packet_In PIN, time_t *ttTimeStamp)
{
	string strMNX_Timestamp("");
	unsigned char Timestamp_flags;
	unsigned char LastOnTime[4];
	long Timetag_lag = 0;
	//====== time Processing ===================
	time_t ttNow;							// World Time
    struct tm tmLastOnTime;
	time_t ttTimeWithLag;
	char buffer[256];
	struct tm tmLastOnTime2;
	string strFormat("%m/%d/%Y %I:%M:%S %p"); 
	stringex strfn;

	//====== time Processing ===================
	ttNow = time(NULL);						// Get World Time
	tmLastOnTime = *localtime(&ttNow);		// init with current local time
    Timestamp_flags = PIN.DATA[0];
    if (Timestamp_flags < 128) {
		// set to the LastOnTime
		tmLastOnTime.tm_year = PIN.DATA[3] + 100;	// tm year to starts at 1900
		tmLastOnTime.tm_mon = PIN.DATA[1] - 1;		// tm month to starts at 0
		tmLastOnTime.tm_mday = PIN.DATA[2];
		tmLastOnTime.tm_hour = PIN.DATA[4];
		tmLastOnTime.tm_min = PIN.DATA[5];
		tmLastOnTime.tm_sec = PIN.DATA[6];
        //Timetag_lag = (long)(PIN.DATA[7] & 0x7F) * (2 ^ 24) + (long)(PIN.DATA[8]) * (2 ^ 16) + (long)(PIN.DATA[9]) * 256 + (long)(PIN.DATA[10]);
		PIN.DATA[7] = PIN.DATA[7] & 0x7F;	// msb uses only 7 bits
		LastOnTime[3] = PIN.DATA[7];
		LastOnTime[2] = PIN.DATA[8];
		LastOnTime[1] = PIN.DATA[9];
		LastOnTime[0] = PIN.DATA[10];
		Timetag_lag = (long)DppUtil.LongWordToDouble(0, LastOnTime);
		ttTimeWithLag = mktime(&tmLastOnTime) + Timetag_lag;
		tmLastOnTime2 = *localtime(&ttTimeWithLag);
		*ttTimeStamp = ttTimeWithLag;
		//*tmTimeStamp = *localtime(&ttTimeWithLag);

		//tmTimeStamp = &tmLastOnTime2;
		strftime(buffer, sizeof(buffer), strFormat.c_str(), &tmLastOnTime2);
		strMNX_Timestamp = strfn.Format("%s",buffer);
		//printf("%s\n",buffer);
    }
	return(strMNX_Timestamp);
}

//converts minix controller status codes (mxmStatusInd) to text strings
string CDP5Status::GetMiniXStatusString(unsigned char monitorMiniXStatus)
{
	string strGetMiniXStatus("");
	switch(monitorMiniXStatus) {
		case mxstNoStatus:
            strGetMiniXStatus = "No Status";
			break;
        case mxstDriversNotLoaded:
            strGetMiniXStatus = "Drivers not loaded, please install MiniX drivers.";
			break;
        case mxstMiniXApplicationReady:
            strGetMiniXStatus = "MiniX application ready.";
			break;
        case mxstPortCLOSED:
            strGetMiniXStatus = "Port CLOSED";
			break;
        case mxstNoDevicesAttached:
            strGetMiniXStatus = "No devices attached";
			break;
        case mxstMiniXControllerSelected:
            strGetMiniXStatus = "MiniX Controller selected";
			break;
        case mxstMiniXControllerReady:
            strGetMiniXStatus = "MiniX Controller ready";
			break;
        case mxstMiniXControllerFailedToOpen:
            strGetMiniXStatus = "MiniX Controller failed to open";
			break;
        case mxstNoDeviceSelected:
            strGetMiniXStatus = "No device selected";
			break;
        case mxstRequestedVoltageOutOfRange:
            strGetMiniXStatus = "Requested voltage out of range.  Please enter voltage in range.";
			break;
        case mxstRequestedCurrentOutOfRange:
            strGetMiniXStatus = "Requested current out of range.  Please enter current in range.";
			break;
        case mxstConnectingToMiniX:
            strGetMiniXStatus = "Connecting to MiniX. Please wait...";
			break;
        case mxstUpdatingSettings:
            strGetMiniXStatus = "Updating settings. Please wait...";
			break;
        case mxstMiniXReady:
            strGetMiniXStatus = "Mini-X Ready";
			break;
		default:
            strGetMiniXStatus = "Status Unknown";
			break;
	}
	return (strGetMiniXStatus);
}

void CDP5Status::ReadMiniXSetup50kv4W(MiniX_Constant *MiniXConstant)
{
    MiniXConstant->strTubeType = "Mini-X2 50kV 4W";             // Tube Type Name
    MiniXConstant->lTubeTypeID = 1;                                // Tube Type Identifier Index
    MiniXConstant->strControllerType = "Mini-X2 Controller";      // Controller Type Name
    MiniXConstant->strCtrlTypeID = "Mini-X2";                         // Controller Type Short Name
    MiniXConstant->dblHighVoltageConversionFactor = 12.5;          // High Voltage Conversion Factor
    MiniXConstant->dblHighVoltageMin = 10.0;                        // High Voltage Min
    MiniXConstant->dblHighVoltageMax = 50.0;                        // High Voltage Max
    MiniXConstant->dblDefaultdblHighVoltage = 15.0;                 // Default High Voltage kV
    MiniXConstant->dblCurrentMin = 5.0;                             // Current Min
    MiniXConstant->dblCurrentMax = 200.0;                           // Current Max
    MiniXConstant->dblDefaultCurrent = 15.0;                        // Default Current
    MiniXConstant->dblWattageMax = 4.0;                             // Wattage Max
}

void CDP5Status::ReadMiniXSetup50kv10W(MiniX_Constant *MiniXConstant)
{
    MiniXConstant->strTubeType = "Mini-X2 50kV 10W";            // Tube Type Name
    MiniXConstant->lTubeTypeID = 2;                                // Tube Type Identifier Index
    MiniXConstant->strControllerType = "Mini-X2 Controller";   // Controller Type Name
    MiniXConstant->strCtrlTypeID = "Mini-X2";                      // Controller Type Short Name
    MiniXConstant->dblHighVoltageConversionFactor = 12.5;          // High Voltage Conversion Factor
    MiniXConstant->dblHighVoltageMin = 10.0;                        // High Voltage Min
    MiniXConstant->dblHighVoltageMax = 50.0;                        // High Voltage Max
    MiniXConstant->dblDefaultdblHighVoltage = 15.0;                 // Default High Voltage kV
    MiniXConstant->dblCurrentMin = 5.0;                             // Current Min
    MiniXConstant->dblCurrentMax = 200.0;                           // Current Max
    MiniXConstant->dblDefaultCurrent = 15.0;                        // Default Current
    MiniXConstant->dblWattageMax = 10.0;                            // Wattage Max
}

void CDP5Status::ReadMiniXSetup70kv10W(MiniX_Constant *MiniXConstant)
{
    MiniXConstant->strTubeType = "Mini-X2 70kV 10W";            // Tube Type Name
    MiniXConstant->lTubeTypeID = 0;                                // Tube Type Identifier Index
    MiniXConstant->strControllerType = "Mini-X2 Controller";      // Controller Type Name
    MiniXConstant->strCtrlTypeID = "Mini-X2";                         // Controller Type Short Name
    MiniXConstant->dblHighVoltageConversionFactor = 20.0;           // High Voltage Conversion Factor
    MiniXConstant->dblHighVoltageMin = 35.0;                        // High Voltage Min
    MiniXConstant->dblHighVoltageMax = 70.0;                        // High Voltage Max
    MiniXConstant->dblDefaultdblHighVoltage = 45.0;                 // Default High Voltage kV
    MiniXConstant->dblCurrentMin = 10.0;                            // Current Min
    MiniXConstant->dblCurrentMax = 143.0;                           // Current Max
    MiniXConstant->dblDefaultCurrent = 15.0;                        // Default Current
    MiniXConstant->dblWattageMax = 10.0;                            // Wattage Max
}

string CDP5Status::MxDblDispFormat(double dblValue)
{
	stringex strfn;
	string strDisplay("");
	strDisplay = strfn.Format("%.0f",dblValue);
	return(strDisplay);
}

long CDP5Status::CalcWarmupTotalTime(float fltWarmUpTable[], long WarmupIndexStart, long NumberWarmupIndexes)
{
    long idxWarmup = 0;
    float sngTotal = 0.0;
	for(idxWarmup = WarmupIndexStart; idxWarmup < (WarmupIndexStart + NumberWarmupIndexes); idxWarmup++)
	{
        sngTotal = sngTotal + fltWarmUpTable[idxWarmup];
    }
	return((long)sngTotal);
}

// 50kV10W Daily warm-up 0-5,Monthly warm-up 6-11
void CDP5Status::Create50kV10W_WarmUpTable(MiniX2WarmUpTable *WarmUpTable)
{
	float fltWarmUpVoltage[12] = {25.0, 30.0, 40.0, 40.0, 50.0, 20.0, 25.0, 30.0, 40.0, 40.0, 50.0, 20.0};
	float fltWarmUpCurrent[12] = {20.0, 50.0, 50.0, 100.0, 200.0, 200.0, 20.0, 50.0, 50.0, 100.0, 200.0, 200.0};
	float fltWarmUpTime[12] = {5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 300.0, 300.0, 300.0, 600.0, 600.0, 600.0};
	for(int i=0;i<12;i++){
		WarmUpTable->fltWarmUpVoltage[i] = fltWarmUpVoltage[i];
		WarmUpTable->fltWarmUpCurrent[i] = fltWarmUpCurrent[i];
		WarmUpTable->fltWarmUpTime[i] = fltWarmUpTime[i];
	}
    WarmUpTable->WarmUpTableType = mx2tbltyp50kV10W;
    WarmUpTable->DailyWarmupTotalTime = CalcWarmupTotalTime(WarmUpTable->fltWarmUpTime, DailyWarmupIndexStart);
    WarmUpTable->MonthlyWarmupTotalTime = CalcWarmupTotalTime(WarmUpTable->fltWarmUpTime, MonthlyWarmupIndexStart);
}

// 50kV4W Daily warm-up 0-5,Monthly warm-up 6-11
void CDP5Status::Create50kV4W_WarmUpTable(MiniX2WarmUpTable *WarmUpTable)
{
	float fltWarmUpVoltage[12] = {25.0, 30.0, 40.0, 40.0, 50.0, 20.0, 25.0, 30.0, 40.0, 40.0, 50.0, 20.0};
    float fltWarmUpCurrent[12] = {20.0, 50.0, 50.0, 100.0, 80.0, 200.0, 20.0, 50.0, 50.0, 100.0, 80.0, 200.0};
    float fltWarmUpTime[12] = {5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 300.0, 300.0, 300.0, 600.0, 600.0, 600.0};
	for(int i=0;i<12;i++){
		WarmUpTable->fltWarmUpVoltage[i] = fltWarmUpVoltage[i];
		WarmUpTable->fltWarmUpCurrent[i] = fltWarmUpCurrent[i];
		WarmUpTable->fltWarmUpTime[i] = fltWarmUpTime[i];
	}
    WarmUpTable->WarmUpTableType = mx2tbltyp50kV4W;
    WarmUpTable->DailyWarmupTotalTime = CalcWarmupTotalTime(WarmUpTable->fltWarmUpTime, DailyWarmupIndexStart);
    WarmUpTable->MonthlyWarmupTotalTime = CalcWarmupTotalTime(WarmUpTable->fltWarmUpTime, MonthlyWarmupIndexStart);
}

// 70kV10W Daily warm-up 0-5,Monthly warm-up 6-11
void CDP5Status::Create70kV10W_WarmUpTable(MiniX2WarmUpTable *WarmUpTable)
{
    float fltWarmUpVoltage[12] = {10.0, 35.0, 50.0, 50.0, 60.0, 70.0, 10.0, 25.0, 40.0, 50.0, 50.0, 60.0};
    float fltWarmUpCurrent[12] = {20.0, 20.0, 100.0, 200.0, 166.0, 143.0, 50.0, 50.0, 50.0, 100.0, 200.0, 266.0};
    float fltWarmUpTime[12] = {5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 300.0, 300.0, 300.0, 300.0, 300.0, 300.0};
	for(int i=0;i<12;i++){
		WarmUpTable->fltWarmUpVoltage[i] = fltWarmUpVoltage[i];
		WarmUpTable->fltWarmUpCurrent[i] = fltWarmUpCurrent[i];
		WarmUpTable->fltWarmUpTime[i] = fltWarmUpTime[i];
	}
    WarmUpTable->WarmUpTableType = mx2tbltyp70kV10W;
    WarmUpTable->DailyWarmupTotalTime = CalcWarmupTotalTime(WarmUpTable->fltWarmUpTime, DailyWarmupIndexStart);
    WarmUpTable->MonthlyWarmupTotalTime = CalcWarmupTotalTime(WarmUpTable->fltWarmUpTime, MonthlyWarmupIndexStart);
}

void CDP5Status::Process_Status(DP4_FORMAT_STATUS *m_DP5_Status)
{
	bool bDMCA_LiveTime = false;
	unsigned int uiFwBuild = 0;

	m_DP5_Status->DEVICE_ID = m_DP5_Status->RAW[39];
    m_DP5_Status->FastCount = DppUtil.LongWordToDouble(0, m_DP5_Status->RAW);
    m_DP5_Status->SlowCount = DppUtil.LongWordToDouble(4, m_DP5_Status->RAW);
    m_DP5_Status->GP_COUNTER = DppUtil.LongWordToDouble(8, m_DP5_Status->RAW);
    m_DP5_Status->AccumulationTime = (float)m_DP5_Status->RAW[12] * 0.001 + (float)(m_DP5_Status->RAW[13] + (float)m_DP5_Status->RAW[14] * 256.0 + (float)m_DP5_Status->RAW[15] * 65536.0) * 0.1;
	m_DP5_Status->RealTime = ((double)m_DP5_Status->RAW[20] + ((double)m_DP5_Status->RAW[21] * 256.0) + ((double)m_DP5_Status->RAW[22] * 65536.0) + ((double)m_DP5_Status->RAW[23] * 16777216.0)) * 0.001;

	m_DP5_Status->Firmware = m_DP5_Status->RAW[24];
    m_DP5_Status->FPGA = m_DP5_Status->RAW[25];
	
	if (m_DP5_Status->Firmware > 0x65) {
		m_DP5_Status->Build = m_DP5_Status->RAW[37] & 0xF;		//Build # added in FW6.06
	} else {
		m_DP5_Status->Build = 0;
	}

	//Firmware Version:  6.07  Build:  0 has LiveTime and PREL
	//DEVICE_ID 0=DP5,1=PX5,2=DP5G,3=MCA8000D,4=TB5
	if (m_DP5_Status->DEVICE_ID == dppMCA8000D) {
		if (m_DP5_Status->Firmware >= 0x67) {
			bDMCA_LiveTime = true;
		}
	}

	if (bDMCA_LiveTime) {
		m_DP5_Status->LiveTime = ((double)m_DP5_Status->RAW[16] + ((double)m_DP5_Status->RAW[17] * 256.0) + ((double)m_DP5_Status->RAW[18] * 65536.0) + ((double)m_DP5_Status->RAW[19] * 16777216.0)) * 0.001;
	} else {
		m_DP5_Status->LiveTime = 0;
	}

    if (m_DP5_Status->RAW[29] < 128) {
        m_DP5_Status->SerialNumber = (unsigned long)DppUtil.LongWordToDouble(26, m_DP5_Status->RAW);
    } else {
        m_DP5_Status->SerialNumber = -1;
    }
    
	// m_DP5_Status->HV = (double)(m_DP5_Status->RAW[31] + (m_DP5_Status->RAW[30] & 15) * 256) * 0.5;					// 0.5V/count

	if (m_DP5_Status->RAW[30] < 128)  {    // not negative
        m_DP5_Status->HV = ((double)m_DP5_Status->RAW[31] + ((double)m_DP5_Status->RAW[30] * 256.0)) * 0.5;  // 0.5V/count
	} else {
		m_DP5_Status->HV = (((double)m_DP5_Status->RAW[31] + ((double)m_DP5_Status->RAW[30] * 256)) - 65536.0) * 0.5; // 0.5V/count
	}
      
	m_DP5_Status->DET_TEMP = (double)((m_DP5_Status->RAW[33]) + (m_DP5_Status->RAW[32] & 15) * 256) * 0.1; // - 273.16;		// 0.1K/count
    m_DP5_Status->DP5_TEMP = m_DP5_Status->RAW[34] - ((m_DP5_Status->RAW[34] & 128) * 2);

    m_DP5_Status->PresetRtDone = ((m_DP5_Status->RAW[35] & 128) == 128);

	//unsigned char:35 BIT:D6 
	// == Preset LiveTime Done for MCA8000D
	// == FAST Thresh locked for other dpp devices
	m_DP5_Status->PresetLtDone = false;
	m_DP5_Status->AFAST_LOCKED = false;
	if (bDMCA_LiveTime) {		// test for MCA8000D
		m_DP5_Status->PresetLtDone = ((m_DP5_Status->RAW[35] & 64) == 64);
	} else {
		m_DP5_Status->AFAST_LOCKED = ((m_DP5_Status->RAW[35] & 64) == 64);
	}
    m_DP5_Status->MCA_EN = ((m_DP5_Status->RAW[35] & 32) == 32);
    m_DP5_Status->PRECNT_REACHED = ((m_DP5_Status->RAW[35] & 16) == 16);
    m_DP5_Status->SCOPE_DR = ((m_DP5_Status->RAW[35] & 4) == 4);
    m_DP5_Status->DP5_CONFIGURED = ((m_DP5_Status->RAW[35] & 2) == 2);

    m_DP5_Status->AOFFSET_LOCKED = ((m_DP5_Status->RAW[36] & 128) == 0);  // 0=locked, 1=searching
    m_DP5_Status->MCS_DONE = ((m_DP5_Status->RAW[36] & 64) == 64);

    m_DP5_Status->b80MHzMode = ((m_DP5_Status->RAW[36] & 2) == 2);
    m_DP5_Status->bFPGAAutoClock = ((m_DP5_Status->RAW[36] & 1) == 1);

    m_DP5_Status->PC5_PRESENT = ((m_DP5_Status->RAW[38] & 128) == 128);
    if  (m_DP5_Status->PC5_PRESENT) {
        m_DP5_Status->PC5_HV_POL = ((m_DP5_Status->RAW[38] & 64) == 64);
        m_DP5_Status->PC5_8_5V = ((m_DP5_Status->RAW[38] & 32) == 32);
    } else {
        m_DP5_Status->PC5_HV_POL = false;
        m_DP5_Status->PC5_8_5V = false;
    }

	if (m_DP5_Status->Firmware >= 0x65) {		// reboot flag added FW6.05
		if ((m_DP5_Status->RAW[36] & 32) == 32) {
            m_DP5_Status->ReBootFlag = true;
		} else {
			m_DP5_Status->ReBootFlag = false;
		}
	} else {
        m_DP5_Status->ReBootFlag = false;
	}

	m_DP5_Status->TEC_Voltage = (((double)(m_DP5_Status->RAW[40] & 15) * 256.0) + (double)(m_DP5_Status->RAW[41])) / 758.5;
	m_DP5_Status->DPP_ECO = m_DP5_Status->RAW[49];
	m_DP5_Status->bScintHas80MHzOption = false;
	m_DP5_Status->DPP_options = (m_DP5_Status->RAW[42] & 15);
	m_DP5_Status->HPGe_HV_INH = false;
	m_DP5_Status->HPGe_HV_INH_POL = false;
	m_DP5_Status->AU34_2 = false;
	m_DP5_Status->isAscInstalled = false;
	m_DP5_Status->isDP5_RevDxGains = false;
	if (m_DP5_Status->DEVICE_ID == dppPX5) {
		if (m_DP5_Status->DPP_options == PX5_OPTION_HPGe_HVPS) {
			m_DP5_Status->HPGe_HV_INH = ((m_DP5_Status->RAW[42] & 32) == 32);
			m_DP5_Status->HPGe_HV_INH_POL = ((m_DP5_Status->RAW[42] & 16) == 16);
			if (m_DP5_Status->DPP_ECO == 1) {
				m_DP5_Status->isAscInstalled = true;
				m_DP5_Status->AU34_2 = ((m_DP5_Status->RAW[42] & 64) == 64);
			}
		}
	} else if ((m_DP5_Status->DEVICE_ID == dppDP5G) || (m_DP5_Status->DEVICE_ID == dppTB5)) {
		if ((m_DP5_Status->DPP_ECO == 1) || (m_DP5_Status->DPP_ECO == 2)) {
			// DPP_ECO == 2 80MHz option added 20150409
			m_DP5_Status->bScintHas80MHzOption = true;
		}
	} else if (m_DP5_Status->DEVICE_ID == dppDP5) {
		uiFwBuild = m_DP5_Status->Firmware;
		uiFwBuild = uiFwBuild << 8;
		uiFwBuild = uiFwBuild + m_DP5_Status->Build;
		// uiFwBuild - firmware with build for comparison
		if (uiFwBuild >= 0x686) {
			// 0xFF Value indicates old Analog Gain Count of 16
			// Values < 0xFF indicate new gain count of 24 and new board rev
			// "DP5 G3 Configuration P" will not be used (==0xFF)
			if (m_DP5_Status->DPP_ECO < 0xFF) {		
				m_DP5_Status->isDP5_RevDxGains = true;
			}
			
		}

	}
}

string CDP5Status::DP5_Dx_OptionFlags(unsigned char DP5_Dx_Options) {
	unsigned char D7D6;
	unsigned char D5D4;
	unsigned char D3D0;
	string strRev("");
	stringex strfn;

	//D7-D6: 0 = DP5 Rev D
	//       1 = DP5 Rev E (future)
	//       2 = DP5 Rev F (future)
	//       3 = DP5 Rev G (future)
	D7D6 = ((DP5_Dx_Options >> 6) & 0x03) + 'D';
	//D5-D4: minor rev, 0-3 (i.e. Rev D0, D1 etc.)
	D5D4 = ((DP5_Dx_Options >> 4) & 0x03);
	//D3-D0: Configuration 0 = A, 1=B, 2=C... 15=P.
	D3D0 = (DP5_Dx_Options & 0x0F) + 'A';

	strRev = strfn.Format("DP5 Rev %c%d Configuration %c",D7D6,D5D4,D3D0);
	return(strRev);

}

string CDP5Status::ShowStatusValueStrings(DP4_FORMAT_STATUS m_DP5_Status) 
{ 
    string strConfig("");
    string strTemp;
	string strIntPart;
	string strFracPart;
	stringex strfn;

	strTemp = GetDeviceNameFromVal(m_DP5_Status.DEVICE_ID);
	strConfig = "Device Type: " + strTemp + "\r\n";
	strTemp = strfn.Format("Serial Number: %lu\r\n",m_DP5_Status.SerialNumber);	//SerialNumber
	strConfig += strTemp;
	strTemp = "Firmware: " + DppUtil.BYTEVersionToString(m_DP5_Status.Firmware);   
	strConfig += strTemp;
	if (m_DP5_Status.Firmware > 0x65) {
		strTemp = strfn.Format("  Build:  %d\r\n", m_DP5_Status.Build);
		strConfig += strTemp;
	} else {
		strConfig += "\r\n";
	}		
	strTemp = "FPGA: " + DppUtil.BYTEVersionToString(m_DP5_Status.FPGA) + "\r\n"; 
	strConfig += strTemp;
	if (m_DP5_Status.DEVICE_ID != dppMCA8000D) {
		strTemp = strfn.Format("Fast Count: %.0f\r\n",m_DP5_Status.FastCount);   	//FastCount
		strConfig += strTemp;
	}
	strTemp = strfn.Format("Slow Count: %.0f\r\n",m_DP5_Status.SlowCount);   	//SlowCount
	strConfig += strTemp;
	strTemp = strfn.Format("GP Count: %.0f\r\n",m_DP5_Status.GP_COUNTER);   	//GP Count
	strConfig += strTemp;

	if (m_DP5_Status.DEVICE_ID != dppMCA8000D) {
		strTemp = strfn.Format("Accumulation Time: %.0f\r\n",m_DP5_Status.AccumulationTime);	    //AccumulationTime
		strConfig += strTemp;
	}

	strTemp = strfn.Format("Real Time: %.0f\r\n",m_DP5_Status.RealTime);	    //RealTime
	strConfig += strTemp;

	if (m_DP5_Status.DEVICE_ID == dppMCA8000D) {
		strTemp = strfn.Format("Live Time: %.0f\r\n",m_DP5_Status.LiveTime);	    //RealTime
		strConfig += strTemp;
	}

	return strConfig;
}

string CDP5Status::PX5_OptionsString(DP4_FORMAT_STATUS m_DP5_Status)
{
	string strOptions("");
	string strValue("");

	if (m_DP5_Status.DEVICE_ID == dppPX5) {
        //m_DP5_Status.DPP_options = 1;
        //m_DP5_Status.HPGe_HV_INH = true;
        //m_DP5_Status.HPGe_HV_INH_POL = true;
        if (m_DP5_Status.DPP_options > 0) {
            //===============PX5 Options==================
            strOptions += "PX5 Options: ";
            if ((m_DP5_Status.DPP_options & 1) == 1) {
                strOptions += "HPGe HVPS\r\n";
            } else {
                strOptions += "Unknown\r\n";
            }
            //===============HPGe HVPS HV Status==================
            strOptions += "HPGe HV: ";
            if (m_DP5_Status.HPGe_HV_INH) {
                strOptions += "not inhibited\r\n";
            } else {
                strOptions += "inhibited\r\n";
            }
            //===============HPGe HVPS Inhibit Status==================
            strOptions += "INH Polarity: ";
            if (m_DP5_Status.HPGe_HV_INH_POL) {
                strOptions += "high\r\n";
            } else {
                strOptions += "low\r\n";
            }
        } else {
            strOptions += "PX5 Options: None\r\n";  //           strOptions += "No Options Installed"
        }
    }
	return strOptions; 
}

string CDP5Status::GetStatusValueStrings(DP4_FORMAT_STATUS m_DP5_Status) 
{ 
    string strConfig("");
    string strTemp;
	string strIntPart;
	string strFracPart;
	stringex strfn;

	strTemp = GetDeviceNameFromVal(m_DP5_Status.DEVICE_ID);
	strConfig = "Device Type: " + strTemp + "\r\n";
	strTemp = strfn.Format("Serial Number: %lu\r\n",m_DP5_Status.SerialNumber);	//SerialNumber
	strConfig += strTemp;
	strTemp = "Firmware: " + DppUtil.BYTEVersionToString(m_DP5_Status.Firmware);   
	strConfig += strTemp;
	if (m_DP5_Status.Firmware > 0x65) {
		strTemp = strfn.Format("  Build:  %d\r\n", m_DP5_Status.Build);
		strConfig += strTemp;
	} else {
		strConfig += "\r\n";
	}
	strTemp = "FPGA: " + DppUtil.BYTEVersionToString(m_DP5_Status.FPGA) + "\r\n"; 
	strConfig += strTemp;
	if (m_DP5_Status.DEVICE_ID != dppMCA8000D) {
		strTemp = strfn.Format("Fast Count: %.0f\r\n",m_DP5_Status.FastCount);   	//FastCount
		strConfig += strTemp;
	}
	strTemp = strfn.Format("Slow Count: %.0f\r\n",m_DP5_Status.SlowCount);   	//SlowCount
	strConfig += strTemp;
	strTemp = strfn.Format("GP Count: %.0f\r\n",m_DP5_Status.GP_COUNTER);   	//GP Count
	strConfig += strTemp;

	if (m_DP5_Status.DEVICE_ID != dppMCA8000D) {
		strTemp = strfn.Format("Accumulation Time: %.0f\r\n",m_DP5_Status.AccumulationTime);	    //AccumulationTime
		strConfig += strTemp;
	}

	strTemp = strfn.Format("Real Time: %.0f\r\n",m_DP5_Status.RealTime);	    //RealTime
	strConfig += strTemp;

	if (m_DP5_Status.DEVICE_ID == dppMCA8000D) {
		strTemp = strfn.Format("Live Time: %.0f\r\n",m_DP5_Status.LiveTime);	    //RealTime
		strConfig += strTemp;
	}

   if ((m_DP5_Status.DEVICE_ID != dppDP5G) && (m_DP5_Status.DEVICE_ID != dppTB5) && (m_DP5_Status.DEVICE_ID != dppMCA8000D)) {
        strTemp = strfn.Format("Detector Temp: %.0fK\r\n",m_DP5_Status.DET_TEMP);		//"##0�C") ' round to nearest degree
		strConfig += strTemp;
		strTemp = strfn.Format("Detector HV: %.0fV\r\n",m_DP5_Status.HV);
		strConfig += strTemp;
		strTemp = strfn.Format("Board Temp: %d�C\r\n",(int)m_DP5_Status.DP5_TEMP);
		strConfig += strTemp;
	} else if ((m_DP5_Status.DEVICE_ID == dppDP5G) || (m_DP5_Status.DEVICE_ID == dppTB5)) {		// GAMMARAD5,TB5
		if (m_DP5_Status.DET_TEMP > 0) {
			strTemp = strfn.Format("Detector Temp: %.1fK\r\n",m_DP5_Status.DET_TEMP);
			strConfig += strTemp;
		} else {
			strConfig += "";
		}
		strTemp = strfn.Format("HV Set: %.0fV\r\n",m_DP5_Status.HV);
		strConfig += strTemp;
	} else if (m_DP5_Status.DEVICE_ID == dppMCA8000D) {		// Digital MCA
		strTemp = strfn.Format("Board Temp: %d�C\r\n",(int)m_DP5_Status.DP5_TEMP);
		strConfig += strTemp;
	}
	if (m_DP5_Status.DEVICE_ID == dppPX5) {
		strTemp = PX5_OptionsString(m_DP5_Status);
		strConfig += strTemp;
		strTemp = strfn.Format("TEC V: %.3fV\r\n",m_DP5_Status.TEC_Voltage);
		strConfig += strTemp;
	}
	return strConfig;
}

void CDP5Status::Process_Diagnostics(Packet_In PIN, DiagDataType *dd, int device_type)
{
    long idxVal;
    string strVal;
	double DP5_ADC_Gain[10];  // convert each ADC count to engineering units - values calculated in FORM.LOAD
	double PC5_ADC_Gain[3];
	double PX5_ADC_Gain[12];
    stringex strfn;

    DP5_ADC_Gain[0] = 1.0 / 0.00286;                // 2.86mV/C
    DP5_ADC_Gain[1] = 1.0;                          // Vdd mon (out-of-scale)
    DP5_ADC_Gain[2] = (30.1 + 20.0) / 20.0;           // PWR
    DP5_ADC_Gain[3] = (13.0 + 20.0) / 20.0;            // 3.3V
    DP5_ADC_Gain[4] = (4.99 + 20.0) / 20.0;         // 2.5V
    DP5_ADC_Gain[5] = 1.0;                          // 1.2V
    DP5_ADC_Gain[6] = (35.7 + 20.0) / 20.0;          // 5.5V
    DP5_ADC_Gain[7] = (35.7 + 75.0) / 35.7;        // -5.5V (this one is tricky)
    DP5_ADC_Gain[8] = 1.0;                          // AN_IN
    DP5_ADC_Gain[9] = 1.0;                          // VREF_IN
    
    PC5_ADC_Gain[0] = 500.0;                        // HV: 1500V/3V
    PC5_ADC_Gain[1] = 100.0;                        // TEC: 300K/3V
    PC5_ADC_Gain[2] = (20.0 + 10.0) / 10.0;            // +8.5/5V

	//PX5_ADC_Gain[0] = (30.1 + 20.0) / 20.0;          // PWR
	PX5_ADC_Gain[0] = (69.8 + 20.0) / 20.0;          // 9V (was originally PWR)
    PX5_ADC_Gain[1] = (13.0 + 20.0) / 20.0;            // 3.3V
    PX5_ADC_Gain[2] = (4.99 + 20.0) / 20.0;          // 2.5V
    PX5_ADC_Gain[3] = 1.0;                         // 1.2V
    PX5_ADC_Gain[4] = (30.1 + 20.0) / 20.0;          // 5V
    PX5_ADC_Gain[5] = (10.7 + 75.0) / 10.7;         // -5V (this one is tricky)
    PX5_ADC_Gain[6] = (64.9 + 20.0) / 20.0;          // +PA
    PX5_ADC_Gain[7] = (10.7 + 75) / 10.7;        // -PA
    PX5_ADC_Gain[8] = (16.0 + 20.0) / 20.0;            // +TEC
    PX5_ADC_Gain[9] = 500.0;                       // HV: 1500V/3V
    PX5_ADC_Gain[10] = 100.0;                       // TEC: 300K/3V
    PX5_ADC_Gain[11] = 1.0 / 0.00286;               // 2.86mV/C

    dd->Firmware = PIN.DATA[0];
    dd->FPGA = PIN.DATA[1];
    strVal = "0x0" + FmtHex(PIN.DATA[2], 2) + FmtHex(PIN.DATA[3], 2) + FmtHex(PIN.DATA[4], 2);
    dd->SRAMTestData = strtol(strVal.c_str(),NULL,0);
    dd->SRAMTestPass = (dd->SRAMTestData == 0xFFFFFF);
    dd->TempOffset = PIN.DATA[180] + 256 * (PIN.DATA[180] > 127);  // 8-bit signed value

	if ((device_type == dppDP5) || (device_type == dppDP5X)) {
		for(idxVal=0;idxVal<10;idxVal++){
			dd->ADC_V[idxVal] = (float)((((PIN.DATA[5 + idxVal * 2] & 3) * 256) + PIN.DATA[6 + idxVal * 2]) * 2.44 / 1024.0 * DP5_ADC_Gain[idxVal]); // convert counts to engineering units (C or V)
		}
		dd->ADC_V[7] = dd->ADC_V[7] + dd->ADC_V[6] * (float)(1.0 - DP5_ADC_Gain[7]);  // -5.5V is a function of +5.5V
		dd->strTempRaw = strfn.Format("%   #.0f0C", dd->ADC_V[0] - 271.3);
		dd->strTempCal = strfn.Format("%   #.0f0C", (dd->ADC_V[0] - 280.0 + dd->TempOffset));
	} else if (device_type == dppPX5) {
        for(idxVal=0;idxVal<11;idxVal++){
            dd->ADC_V[idxVal] = (float)((((PIN.DATA[5 + idxVal * 2] & 15) * 256) + PIN.DATA[6 + idxVal * 2]) * 3.0 / 4096.0 * PX5_ADC_Gain[idxVal]);   // convert counts to engineering units (C or V)
		}
        dd->ADC_V[11] = (float)((((PIN.DATA[5 + 11 * 2] & 3) * 256) + PIN.DATA[6 + 11 * 2]) * 3.0 / 1024.0 * PX5_ADC_Gain[11]);  // convert counts to engineering units (C or V)
        dd->ADC_V[5] = (float)(dd->ADC_V[5] - (3.0 * PX5_ADC_Gain[5]) + 3.0); // -5V uses +3VR
        dd->ADC_V[7] = (float)(dd->ADC_V[7] - (3.0 * PX5_ADC_Gain[7]) + 3.0); // -PA uses +3VR
 		dd->strTempRaw = strfn.Format("%#.1fC", dd->ADC_V[11] - 271.3);
		dd->strTempCal = strfn.Format("%#.1fC", (dd->ADC_V[11] - 280.0 + dd->TempOffset));
	}	
	
    dd->PC5_PRESENT = false;  // assume no PC5, then check to see if there are any non-zero bytes
	for(idxVal=25;idxVal<=38;idxVal++) {
		if (PIN.DATA[idxVal] > 0) {
            dd->PC5_PRESENT = true;
            break;
        }
    }

    if (dd->PC5_PRESENT) {
		for(idxVal=0;idxVal<=2;idxVal++) {
			dd->PC5_V[idxVal] = (float)((((PIN.DATA[25 + idxVal * 2] & 15) * 256) + PIN.DATA[26 + idxVal * 2]) * 3.0 / 4096.0 * PC5_ADC_Gain[idxVal]); // convert counts to engineering units (C or V)
		}
		if (PIN.DATA[34] < 128) {
			dd->PC5_SN = (unsigned long)DppUtil.LongWordToDouble(31, PIN.DATA);
		} else {
			dd->PC5_SN = -1; // no PC5 S/N
		}
		if ((PIN.DATA[35] == 255) && (PIN.DATA[36] == 255)) {
			dd->PC5Initialized = false;
			dd->PC5DCAL = 0;
		} else {
			dd->PC5Initialized = true;
			dd->PC5DCAL = (float)(((float)(PIN.DATA[35]) * 256.0 + (float)(PIN.DATA[36])) * 3.0 / 4096.0);
		}
		dd->IsPosHV = ((PIN.DATA[37] & 128) == 128);
		dd->Is8_5VPreAmp = ((PIN.DATA[37] & 64) == 64);
		dd->Sup9VOn = ((PIN.DATA[38] & 8) == 8);
		dd->PreAmpON = ((PIN.DATA[38] & 4) == 4);
		dd->HVOn = ((PIN.DATA[38] & 2) == 2);
		dd->TECOn = ((PIN.DATA[38] & 1) == 1);
	} else {
        for(idxVal=0;idxVal<=2;idxVal++) {
            dd->PC5_V[idxVal] = 0;
		}
        dd->PC5_SN = -1; // no PC5 S/N
        dd->PC5Initialized = false;
        dd->PC5DCAL = 0;
        dd->IsPosHV = false;
        dd->Is8_5VPreAmp = false;
        dd->Sup9VOn = false;
        dd->PreAmpON = false;
        dd->HVOn = false;
        dd->TECOn = false;
    }
	for(idxVal=0;idxVal<=191;idxVal++) {
        dd->DiagData[idxVal] = PIN.DATA[idxVal + 39];
    }
	//string strData;
	//strData = DisplayBufferArray(PIN.DATA, 256);
	//SaveStringDataToFile(strData);
}

string CDP5Status::DiagnosticsToString(DiagDataType dd, int device_type)
{
    long idxVal;
	string strVal;
    string strDiag;
	stringex strfn;
    
    strDiag = "Firmware: " + VersionToStr(dd.Firmware) + "\r\n";
    strDiag += "FPGA: " + VersionToStr(dd.FPGA) + "\r\n";
    strDiag += "SRAM Test: ";
    if (dd.SRAMTestPass) {
        strDiag += "PASS\r\n";
    } else {
        strDiag += "ERROR @ 0x" + FmtHex(dd.SRAMTestData, 6) + "\r\n";
    }

	if ((device_type == dppDP5) || (device_type == dppDP5X)) {
		strDiag += "DP5 Temp (raw): " + dd.strTempRaw + "\r\n";
		strDiag += "DP5 Temp (cal'd): " + dd.strTempCal + "\r\n";
		strDiag += "PWR: " + FmtPc5Pwr(dd.ADC_V[2]) + "\r\n";
		strDiag += "3.3V: " + FmtPc5Pwr(dd.ADC_V[3]) + "\r\n";
		strDiag += "2.5V: " + FmtPc5Pwr(dd.ADC_V[4]) + "\r\n";
		strDiag += "1.2V: " + FmtPc5Pwr(dd.ADC_V[5]) + "\r\n";
		strDiag += "+5.5V: " + FmtPc5Pwr(dd.ADC_V[6]) + "\r\n";
		strDiag += "-5.5V: " + FmtPc5Pwr(dd.ADC_V[7]) + "\r\n";
		strDiag += "AN_IN: " + FmtPc5Pwr(dd.ADC_V[8]) + "\r\n";
		strDiag += "VREF_IN: " + FmtPc5Pwr(dd.ADC_V[9]) + "\r\n";

		strDiag += "\r\n";
		if (dd.PC5_PRESENT) {
			strDiag += "PC5: Present\r\n";
			strVal = strfn.Format("%dV",(int)(dd.PC5_V[0]));
			strDiag += "HV: " + strVal + "\r\n";
			strVal = strfn.Format("%#.1fK",dd.PC5_V[1]);
			strDiag += "Detector Temp: " + strVal + "\r\n";
			strDiag += "+8.5/5V: " + FmtPc5Pwr(dd.PC5_V[2]) + "\r\n";
			if (dd.PC5_SN > -1) {
				strDiag += "PC5 S/N: " + FmtLng(dd.PC5_SN) + "\r\n";
			} else {
				strDiag += "PC5 S/N: none\r\n";
			}
			if (dd.PC5Initialized) {
				strDiag += "PC5 DCAL: " + FmtPc5Pwr(dd.PC5DCAL) + "\r\n";
			} else {
				strDiag += "PC5 DCAL: Uninitialized\r\n";
			}
			strDiag += "PC5 Flavor: ";
			strDiag += IsAorB(dd.IsPosHV, "+HV, ", "-HV, ");
			strDiag += IsAorB(dd.Is8_5VPreAmp, "8.5V preamp", "5V preamp") + "\r\n";
			strDiag += "PC5 Supplies:\r\n";
			strDiag += "9V: " + OnOffStr(dd.Sup9VOn) + "\r\n";
			strDiag += "Preamp: " + OnOffStr(dd.PreAmpON) + "\r\n";
			strDiag += "HV: " + OnOffStr(dd.HVOn) + "\r\n";
			strDiag += "TEC: " + OnOffStr(dd.TECOn) + "\r\n";
		} else {
			strDiag += "PC5: Not Present\r\n";
		}
	} else if (device_type == dppPX5) {
		strDiag += "PX5 Temp (raw): " + dd.strTempRaw + "\r\n";
		strDiag += "PX5 Temp (cal'd): " + dd.strTempCal + "\r\n";
		//strDiag += "PWR: " + FmtPc5Pwr(dd.ADC_V[0]) + "\r\n";
		strDiag += "9V: " + FmtPc5Pwr(dd.ADC_V[0]) + "\r\n";
		strDiag += "3.3V: " + FmtPc5Pwr(dd.ADC_V[1]) + "\r\n";
		strDiag += "2.5V: " + FmtPc5Pwr(dd.ADC_V[2]) + "\r\n";
		strDiag += "1.2V: " + FmtPc5Pwr(dd.ADC_V[3]) + "\r\n";
		strDiag += "+5V: " + FmtPc5Pwr(dd.ADC_V[4]) + "\r\n";
		strDiag += "-5V: " + FmtPc5Pwr(dd.ADC_V[5]) + "\r\n";
		strDiag += "+PA: " + FmtPc5Pwr(dd.ADC_V[6]) + "\r\n";
		strDiag += "-PA: " + FmtPc5Pwr(dd.ADC_V[7]) + "\r\n";
		strDiag += "TEC: " + FmtPc5Pwr(dd.ADC_V[8]) + "\r\n";
		strDiag += "ABS(HV): " + FmtHvPwr(dd.ADC_V[9]) + "\r\n";
		strDiag += "DET_TEMP: " + FmtPc5Temp(dd.ADC_V[10]) + "\r\n";
	}

    strDiag += "\r\nDiagnostic Data\r\n";
    strDiag += "---------------\r\n";
	for(idxVal=0;idxVal<=191;idxVal++) {
        if ((idxVal % 8) == 0) { 
			strDiag += FmtHex(idxVal, 2) + ":";
		}
        strDiag += FmtHex(dd.DiagData[idxVal], 2) + " ";
        if ((idxVal % 8) == 7) { 
			strDiag += "\r\n";
		}
    }
    return (strDiag);
}

string CDP5Status::DiagStrPX5Option(DiagDataType dd, int device_type)
{
    long idxVal;
	string strVal;
    string strDiag;
 	stringex strfn;
   
    strDiag = "Firmware: " + VersionToStr(dd.Firmware) + "\r\n";
    strDiag += "FPGA: " + VersionToStr(dd.FPGA) + "\r\n";
    strDiag += "SRAM Test: ";
    if (dd.SRAMTestPass) {
        strDiag += "PASS\r\n";
    } else {
        strDiag += "ERROR @ 0x" + FmtHex(dd.SRAMTestData, 6) + "\r\n";
    }

	if (device_type == dppPX5) {
		strDiag += "PX5 Temp (raw): " + dd.strTempRaw + "\r\n";
		strDiag += "PX5 Temp (cal'd): " + dd.strTempCal + "\r\n";
		strDiag += "9V: " + FmtPc5Pwr(dd.ADC_V[0]) + "\r\n";
		strDiag += "3.3V: " + FmtPc5Pwr(dd.ADC_V[1]) + "\r\n";
		strDiag += "2.5V: " + FmtPc5Pwr(dd.ADC_V[2]) + "\r\n";
		strDiag += "TDET: " + FmtPc5Pwr(dd.ADC_V[3]) + "\r\n";
		strDiag += "+5V: " + FmtPc5Pwr(dd.ADC_V[4]) + "\r\n";
		strDiag += "-5V: " + FmtPc5Pwr(dd.ADC_V[5]) + "\r\n";
		strDiag += "+PA: " + FmtPc5Pwr(dd.ADC_V[6]) + "\r\n";
		strDiag += "-PA: " + FmtPc5Pwr(dd.ADC_V[7]) + "\r\n";
		strDiag += "TEC: " + FmtPc5Pwr(dd.ADC_V[8]) + "\r\n";
		strDiag += "ABS(HV): " + FmtHvPwr(dd.ADC_V[9]) + "\r\n";
		strDiag += "DET_TEMP: " + FmtPc5Temp(dd.ADC_V[10]) + "\r\n";
	}

    strDiag += "\r\nDiagnostic Data\r\n";
    strDiag += "---------------\r\n";
	for(idxVal=0;idxVal<=191;idxVal++) {
        if ((idxVal % 8) == 0) { 
			strDiag += FmtHex(idxVal, 2) + ":";
		}
        strDiag += FmtHex(dd.DiagData[idxVal], 2) + " ";
        if ((idxVal % 8) == 7) { 
			strDiag += "\r\n";
		}
    }
    return (strDiag);
}

string CDP5Status::FmtHvPwr(float fVal) 
{
	string strVal;
	stringex strfn;
	strVal = strfn.Format("%#.1fV", fVal);	// "#.##0V"
	return strVal;
}

string CDP5Status::FmtPc5Pwr(float fVal) 
{
	string strVal;
	stringex strfn;
	strVal = strfn.Format("%#.3fV", fVal);	// "#.##0V"
	return strVal;
}

string CDP5Status::FmtPc5Temp(float fVal) 
{
	string strVal;
	stringex strfn;
	strVal = strfn.Format("%#.1fK", fVal);	// "#.##0V"
	return strVal;
}

string CDP5Status::FmtHex(long FmtHex, long HexDig) 
{
	string strHex;
	string strFmt;
	stringex strfn;
	strFmt = strfn.Format("%d",HexDig);		// max size of 0 pad
	strFmt = "%0" + strFmt + "X";		// string format specifier
	strHex = strfn.Format(strFmt.c_str(), FmtHex);	// create padded string
	return strHex;
}

string CDP5Status::FmtLng(long lVal) 
{
	string strVal;
	stringex strfn;
	strVal = strfn.Format("%d", lVal);
	return strVal;
}

string CDP5Status::VersionToStr(unsigned char bVersion)
{
	string strVerMajor;
	string strVerMinor;
	string strVer;
	stringex strfn;
	strVerMajor = strfn.Format("%d",((bVersion & 0xF0) / 16));
	strVerMinor = strfn.Format("%02d",(bVersion & 0x0F));
	strVer = strVerMajor + "." + strVerMinor;
	return (strVer);
}

string CDP5Status::OnOffStr(bool bOn)
{
    if (bOn) {
        return("ON");
    } else {
        return("OFF");
    }
}

string CDP5Status::IsAorB(bool bIsA, string strA, string strB)
{
    if (bIsA) {
        return(strA);
    } else {
        return(strB);
    }
}

string CDP5Status::GetDeviceNameFromVal(int DeviceTypeVal) 
{
    string strDeviceType;
	switch(DeviceTypeVal) {
		case 0:
            strDeviceType = "DP5";
			break;
		case 1:
            strDeviceType = "PX5";
			break;
		case 2:
            strDeviceType = "DP5G";
			break;
		case 3:
            strDeviceType = "MCA8000D";
			break;
		case 4:
            strDeviceType = "TB5";
			break;
		case 5:
            strDeviceType = "DP5-X";
			break;
		default:           //if unknown set to DP5
            strDeviceType = "DP5";
			break;
	}
    return strDeviceType;
}

string CDP5Status::DisplayBufferArray(unsigned char buffer[], unsigned long bufSizeIn)
{
    unsigned long i;
	string strVal("");
	string strMsg("");
	stringex strfn;
	for(i=0;i<bufSizeIn;i++) {
		strVal = strfn.Format("%.2X ",buffer[i]);
		strMsg += strVal;
		//if (((i+1) % 16) == 0 ) { 
		//	strMsg += "\r\n";
		//} else 
		if (((i+1) % 8) == 0 ) {
		//	strMsg += "   ";
			//strMsg += "\r\n";
			strMsg += "\n";
		}
	}
	//strMsg += "\n";
	return strMsg;
}

void CDP5Status::SaveStringDataToFile(string strData)
{
	FILE  *out;
	string strFilename;
	string strError;
	stringex strfn;

	strFilename = "vcDP5_Data.txt";

	if ( (out = fopen(strFilename.c_str(),"w")) == (FILE *) NULL)
		strError = strfn.Format("Couldn't open %s for writing.\n", strFilename.c_str());
	else
	{
		fprintf(out,"%s\n",strData.c_str());
	}
	fclose(out);
}













