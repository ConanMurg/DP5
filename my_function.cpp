#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <linux/usb/functionfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <iomanip> // For std::hex and std::setw

#define EP0     "/dev/ffs-my_function/ep0"
#define EP_IN   "/dev/ffs-my_function/ep1"   // Bulk IN Endpoint
#define EP_OUT  "/dev/ffs-my_function/ep2"   // Bulk OUT Endpoint

#define REQUEST_PACKET_SIZE   8
#define RESPONSE_PACKET_SIZE  72

// Function to calculate checksum
uint16_t calculate_checksum(const uint8_t* data, size_t length) {
    uint32_t sum = 0; // Use 32-bit to prevent overflow
    for (size_t i = 0; i < length; ++i) {
        sum += data[i];
    }
    // Calculate two's complement
    uint16_t checksum = static_cast<uint16_t>((~sum + 1) & 0xFFFF);
    return checksum;
}

// Function to verify checksum
bool verify_checksum(const uint8_t* data, size_t length, uint16_t received_checksum) {
    uint16_t calculated_checksum = calculate_checksum(data, length);
    uint32_t total = calculated_checksum + received_checksum;
    return (total & 0xFFFF) == 0;
}

// Function to construct the response packet with specified status data
void construct_response_packet(uint8_t* response_packet) {
    // Set the fixed fields (Offsets 0-5)
    response_packet[0] = 0xF5; // SYNC1
    response_packet[1] = 0xFA; // SYNC2
    response_packet[2] = 0x80; // PID1
    response_packet[3] = 1;    // PID2
    response_packet[4] = 0;    // LEN MSB
    response_packet[5] = 0x40; // LEN LSB (64 bytes data field)

    // Clear the data field
    memset(&response_packet[6], 0x00, 64); // Offsets 6-69

    // Now set the data field according to the status data
    uint8_t* data_field = &response_packet[6];

    // Offsets 12-15 (Accumulation Time): Tube Runtime (42 seconds)
    uint32_t accumulation_time_ms = 42000; // 42 seconds in milliseconds
    data_field[12] = 0; // D7-D0 (0-99), Acc. Time (0-99, 1ms/count), limited to 99 ms
    uint32_t acc_time_100ms = accumulation_time_ms / 100; // Convert to 100ms units
    data_field[13] = (acc_time_100ms >> 0) & 0xFF;  // LSB
    data_field[14] = (acc_time_100ms >> 8) & 0xFF;  // Byte 2
    data_field[15] = (acc_time_100ms >> 16) & 0xFF; // MSB

    // Offsets 20-23 (Realtime): 42 seconds
    uint32_t realtime_ms = 42000; // 42 seconds in milliseconds
    data_field[20] = (realtime_ms >> 0) & 0xFF;  // LSB
    data_field[21] = (realtime_ms >> 8) & 0xFF;  // Byte 2
    data_field[22] = (realtime_ms >> 16) & 0xFF; // Byte 3
    data_field[23] = (realtime_ms >> 24) & 0xFF; // MSB

    // Offset 24 (Firmware Version): 6.00
    data_field[24] = ((6 & 0x0F) << 4) | (0 & 0x0F); // D7-D4: Major, D3-D0: Minor

    // Offset 25 (FPGA Version): Set to zero
    data_field[25] = 0x00;

    //
