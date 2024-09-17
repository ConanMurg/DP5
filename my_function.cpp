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

#define EP0     "/dev/ffs-my_function/ep0"
#define EP_IN   "/dev/ffs-my_function/ep1"   // Bulk IN Endpoint
#define EP_OUT  "/dev/ffs-my_function/ep2"   // Bulk OUT Endpoint

#define REQUEST_PACKET_SIZE   8
#define RESPONSE_PACKET_SIZE  72

// Function to calculate checksum
uint16_t calculate_checksum(const uint8_t* data, size_t length) {
    uint16_t checksum = 0;
    for (size_t i = 0; i < length; ++i) {
        checksum += data[i];
    }
    checksum = ~checksum + 1;
    return checksum;
}

// Function to construct the response packet
void construct_response_packet(uint8_t* response_packet) {
    // Set the fixed fields
    response_packet[0] = 0xF5; // SYNC1
    response_packet[1] = 0xFA; // SYNC2
    response_packet[2] = 0x80; // PID1
    response_packet[3] = 1;    // PID2
    response_packet[4] = 0;    // LEN MSB
    response_packet[5] = 0x40; // LEN LSB (64 bytes data field)

    // Data Field (fill with your status data)
    // For demonstration, we'll fill it with zeros
    memset(&response_packet[6], 0x00, 64); // Data Field (offsets 6-69)

    // Calculate checksum over offsets 0-69
    uint16_t checksum = calculate_checksum(response_packet, 70);
    response_packet[70] = (checksum >> 8) & 0xFF; // CHKSUM MSB
    response_packet[71] = checksum & 0xFF;        // CHKSUM LSB
}

int main() {
    // Open the control endpoint (ep0)
    int ep0_fd = open(EP0, O_RDWR);
    if (ep0_fd < 0) {
        std::cerr << "Failed to open EP0: " << strerror(errno) << std::endl;
        return 1;
    }

    // Define USB descriptors
    struct {
        struct usb_functionfs_descs_head_v2 header;
        __le32 fs_count;
        __le32 hs_count;
        struct {
            struct usb_interface_descriptor intf;
            struct usb_endpoint_descriptor_no_audio ep_out;
            struct usb_endpoint_descriptor_no_audio ep_in;
        } __attribute__((packed)) fs_descriptors;
        struct {
            struct usb_interface_descriptor intf;
            struct usb_endpoint_descriptor_no_audio ep_out;
            struct usb_endpoint_descriptor_no_audio ep_in;
        } __attribute__((packed)) hs_descriptors;
    } __attribute__((packed)) descriptors;

    memset(&descriptors, 0, sizeof(descriptors));

    // Fill the descriptors
    descriptors.header.magic = htole32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2);
    descriptors.header.length = htole32(sizeof(descriptors));
    descriptors.header.flags = htole32(FUNCTIONFS_HAS_FS_DESC | FUNCTIONFS_HAS_HS_DESC);

    descriptors.fs_count = htole32(3); // Interface + 2 endpoints
    descriptors.hs_count = htole32(3); // Interface + 2 endpoints

    // Full Speed descriptors
    descriptors.fs_descriptors.intf.bLength = sizeof(struct usb_interface_descriptor);
    descriptors.fs_descriptors.intf.bDescriptorType = USB_DT_INTERFACE;
    descriptors.fs_descriptors.intf.bInterfaceNumber = 0;
    descriptors.fs_descriptors.intf.bAlternateSetting = 0;
    descriptors.fs_descriptors.intf.bNumEndpoints = 2;
    descriptors.fs_descriptors.intf.bInterfaceClass = 0xFF;      // Vendor Specific
    descriptors.fs_descriptors.intf.bInterfaceSubClass = 0xFF;
    descriptors.fs_descriptors.intf.bInterfaceProtocol = 0xFF;
    descriptors.fs_descriptors.intf.iInterface = 1;

    descriptors.fs_descriptors.ep_out.bLength = sizeof(struct usb_endpoint_descriptor_no_audio);
    descriptors.fs_descriptors.ep_out.bDescriptorType = USB_DT_ENDPOINT;
    descriptors.fs_descriptors.ep_out.bEndpointAddress = USB_DIR_OUT | 1; // Endpoint 1 OUT
    descriptors.fs_descriptors.ep_out.bmAttributes = USB_ENDPOINT_XFER_BULK;
    descriptors.fs_descriptors.ep_out.wMaxPacketSize = htole16(64);
    descriptors.fs_descriptors.ep_out.bInterval = 0;

    descriptors.fs_descriptors.ep_in.bLength = sizeof(struct usb_endpoint_descriptor_no_audio);
    descriptors.fs_descriptors.ep_in.bDescriptorType = USB_DT_ENDPOINT;
    descriptors.fs_descriptors.ep_in.bEndpointAddress = USB_DIR_IN | 2; // Endpoint 2 IN
    descriptors.fs_descriptors.ep_in.bmAttributes = USB_ENDPOINT_XFER_BULK;
    descriptors.fs_descriptors.ep_in.wMaxPacketSize = htole16(64);
    descriptors.fs_descriptors.ep_in.bInterval = 0;

    // High Speed descriptors (similar to FS but wMaxPacketSize can be larger)
    descriptors.hs_descriptors = descriptors.fs_descriptors;
    descriptors.hs_descriptors.ep_out.wMaxPacketSize = htole16(512);
    descriptors.hs_descriptors.ep_in.wMaxPacketSize = htole16(512);

    // Write descriptors to ep0
    ssize_t bytes_written = write(ep0_fd, &descriptors, sizeof(descriptors));
    if (bytes_written != sizeof(descriptors)) {
        std::cerr << "Failed to write descriptors to EP0: " << strerror(errno) << std::endl;
        close(ep0_fd);
        return 1;
    }

    // Define USB strings
    struct {
        struct usb_functionfs_strings_head header;
        struct {
            __le16 code;
            const char str1[32];
        } __attribute__((packed)) lang0;
    } __attribute__((packed)) strings;

    strings.header.magic = htole32(FUNCTIONFS_STRINGS_MAGIC);
    strings.header.length = htole32(sizeof(strings));
    strings.header.str_count = htole32(1);
    strings.header.lang_count = htole32(1);

    strings.lang0.code = htole16(0x0409); // English (United States)
    strncpy((char*)strings.lang0.str1, "Virtual USB Function", sizeof(strings.lang0.str1));

    // Write strings to ep0
    bytes_written = write(ep0_fd, &strings, sizeof(strings));
    if (bytes_written != sizeof(strings)) {
        std::cerr << "Failed to write strings to EP0: " << strerror(errno) << std::endl;
        close(ep0_fd);
        return 1;
    }

    // Open the bulk IN endpoint (ep1)
    int ep_in_fd = open(EP_IN, O_RDWR);
    if (ep_in_fd < 0) {
        std::cerr << "Failed to open EP IN: " << strerror(errno) << std::endl;
        close(ep0_fd);
        return 1;
    }

    // Open the bulk OUT endpoint (ep2)
    int ep_out_fd = open(EP_OUT, O_RDWR);
    if (ep_out_fd < 0) {
        std::cerr << "Failed to open EP OUT: " << strerror(errno) << std::endl;
        close(ep_in_fd);
        close(ep0_fd);
        return 1;
    }

    uint8_t request_buffer[REQUEST_PACKET_SIZE];
    uint8_t response_packet[RESPONSE_PACKET_SIZE];

    while (true) {
        // Read from EP_OUT
        ssize_t bytes_read = read(ep_out_fd, request_buffer, REQUEST_PACKET_SIZE);
        if (bytes_read < 0) {
            std::cerr << "Failed to read from EP OUT: " << strerror(errno) << std::endl;
            break;
        } else if (bytes_read == 0) {
            continue; // No data received
        }

        // Check if we have the full request packet
        if (bytes_read == REQUEST_PACKET_SIZE) {
            // Check if it matches the "Request Status Packet"
            if (request_buffer[0] == 0xF5 && request_buffer[1] == 0xFA &&
                request_buffer[2] == 1 && request_buffer[3] == 1 &&
                request_buffer[4] == 0 && request_buffer[5] == 0) {

                // Verify checksum
                uint16_t received_checksum = (request_buffer[6] << 8) | request_buffer[7];
                uint16_t calculated_checksum = calculate_checksum(request_buffer, 6);
                if (received_checksum == calculated_checksum) {
                    std::cout << "Received valid Request Status Packet." << std::endl;

                    // Construct the response packet
                    construct_response_packet(response_packet);

                    // Write the response packet to EP_IN
                    bytes_written = write(ep_in_fd, response_packet, RESPONSE_PACKET_SIZE);
                    if (bytes_written < 0) {
                        std::cerr << "Failed to write to EP IN: " << strerror(errno) << std::endl;
                        break;
                    } else if (bytes_written != RESPONSE_PACKET_SIZE) {
                        std::cerr << "Incomplete write to EP IN." << std::endl;
                        break;
                    } else {
                        std::cout << "Sent Status Packet in response." << std::endl;
                    }
                } else {
                    std::cerr << "Invalid checksum in Request Status Packet." << std::endl;
                }
            } else {
                std::cerr << "Received unknown packet." << std::endl;
            }
        } else {
            std::cerr << "Incomplete packet received. Bytes read: " << bytes_read << std::endl;
        }
    }

    // Close file descriptors
    close(ep_out_fd);
    close(ep_in_fd);
    close(ep0_fd);

    return 0;
}
