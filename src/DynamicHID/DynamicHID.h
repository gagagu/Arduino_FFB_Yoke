/*
  Modified by Matthew Heironimus to support HID Report Descriptors to be in 
  standard RAM in addition to program memory (PROGMEM).

  Copyright (c) 2015, Arduino LLC
  Original code (pre-library): Copyright (c) 2011, Peter Barrett

  Permission to use, copy, modify, and/or distribute this software for
  any purpose with or without fee is hereby granted, provided that the
  above copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
  WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
  BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
  OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
  SOFTWARE.
 */

#ifndef DYNAMIC_HID_h
#define DYNAMIC_HID_h

#include <stdint.h>
#include <Arduino.h>

#ifdef _VARIANT_ARDUINO_DUE_X_
  // Definitions for Arduino Due variant.
  // USB_EP_SIZE and TRANSFER_PGM are defined here for SAM-based boards (like Arduino Due)
  // because they aren't available in SAM's USBAPI.h
  #define USB_EP_SIZE 64
  #define TRANSFER_PGM 0x80

  // Include PluggableUSB header for handling USB communication on SAM-based boards.
  #include "USB/PluggableUSB.h"
#else
  // For other boards, include the standard PluggableUSB header.
  #include "PluggableUSB.h"
#endif

// Include the PIDReportHandler for force feedback processing.
#include "PIDReportHandler.h"

#if defined(USBCON) // Ensure USB is enabled

#define _USING_DYNAMIC_HID

// HID Class Request Codes
// These codes define requests sent by the USB host for HID-specific actions.
#define DYNAMIC_HID_GET_REPORT        0x01 // Request to get a report.
#define DYNAMIC_HID_GET_IDLE          0x02 // Request to get the idle rate.
#define DYNAMIC_HID_GET_PROTOCOL      0x03 // Request to get the current protocol.
#define DYNAMIC_HID_SET_REPORT        0x09 // Request to send a report.
#define DYNAMIC_HID_SET_IDLE          0x0A // Request to set the idle rate.
#define DYNAMIC_HID_SET_PROTOCOL      0x0B // Request to set the protocol.

// HID Descriptor Types
#define DYNAMIC_HID_DESCRIPTOR_TYPE         0x21 // HID descriptor type.
#define DYNAMIC_HID_REPORT_DESCRIPTOR_TYPE      0x22 // HID report descriptor type.
#define DYNAMIC_HID_PHYSICAL_DESCRIPTOR_TYPE    0x23 // HID physical descriptor type.

// HID Subclass Definitions
// Subclasses define the type of HID device (e.g., Boot Interface).
#define DYNAMIC_HID_SUBCLASS_NONE 0 // No subclass.
#define DYNAMIC_HID_SUBCLASS_BOOT_INTERFACE 1 // Boot interface subclass (for keyboards/mice).

// HID Protocol Definitions
// Defines the protocols supported by the HID device.
#define DYNAMIC_HID_PROTOCOL_NONE 0 // No protocol.
#define DYNAMIC_HID_PROTOCOL_KEYBOARD 1 // Keyboard protocol.
#define DYNAMIC_HID_PROTOCOL_MOUSE 2 // Mouse protocol.

// Boot Protocol and Report Protocol Definitions
// Determines whether the HID device is using the boot or report protocol.
#define DYNAMIC_HID_BOOT_PROTOCOL  0 // Boot protocol.
#define DYNAMIC_HID_REPORT_PROTOCOL  1 // Report protocol.

// HID Report Type Definitions
// Report types specify the kind of report (Input, Output, or Feature).
#define DYNAMIC_HID_REPORT_TYPE_INPUT   1 // Input report.
#define DYNAMIC_HID_REPORT_TYPE_OUTPUT  2 // Output report.
#define DYNAMIC_HID_REPORT_TYPE_FEATURE 3 // Feature report.

// Number of endpoints used by the device.
#define PID_ENPOINT_COUNT 2

// Define the IN and OUT endpoints for USB communication.
#define PID_ENDPOINT_IN  (pluggedEndpoint)
#define PID_ENDPOINT_OUT (pluggedEndpoint+1)

// Structure for HID Descriptor
// Contains the necessary fields for an HID descriptor, including
// length, type, address, version, and report descriptor information.
typedef struct
{
  uint8_t len;      // Length of the descriptor (9 bytes).
  uint8_t dtype;    // Descriptor type (0x21).
  uint8_t addr;     // Address.
  uint8_t versionL; // Low byte of version (1.01).
  uint8_t versionH; // High byte of version (1.01).
  uint8_t country;  // Country code.
  uint8_t desctype; // Descriptor type (0x22 for report descriptor).
  uint8_t descLenL; // Low byte of descriptor length.
  uint8_t descLenH; // High byte of descriptor length.
} DYNAMIC_HIDDescDescriptor;

// Structure for HID Interface Descriptor
// Combines an interface descriptor, HID descriptor, and endpoint descriptors
// for both IN and OUT communication.
typedef struct 
{
  InterfaceDescriptor hid;                // HID interface descriptor.
  DYNAMIC_HIDDescDescriptor   desc;       // HID descriptor.
  EndpointDescriptor  in;                 // IN endpoint descriptor.
  EndpointDescriptor  out;                // OUT endpoint descriptor.
} DYNAMIC_HIDDescriptor;

// Class for Handling Sub-Descriptors
// Each HID sub-descriptor represents a report or feature used by the device.
class DynamicHIDSubDescriptor {
public:
  DynamicHIDSubDescriptor *next = NULL;  // Pointer to the next descriptor in the chain.
  // Constructor for initializing a sub-descriptor.
  DynamicHIDSubDescriptor(const void *d, const uint16_t l, const void* pid_d, const uint16_t pid_l, const bool ipm = true) 
      : data(d), length(l), pid_data(pid_d), pid_length(pid_l), inProgMem(ipm) { }

  const void* data;       // Pointer to the descriptor data.
  const void* pid_data;   // Pointer to the PID-specific data.
  const uint16_t length;  // Length of the descriptor data.
  const uint16_t pid_length; // Length of the PID-specific data.
  const bool inProgMem;   // Indicates whether the data is in program memory.
};

// Main Dynamic HID Class
// This class defines the dynamic HID device and handles all USB-related communication.
class DynamicHID_ : public PluggableUSBModule
{
public:
  // Constructor for initializing the HID device.
  DynamicHID_(void);
  
  // Starts the USB communication.
  int begin(void);
  
  // Checks if there is data available from the USB host.
  bool usb_Available();
  
  // Sends a report to the host.
  int SendReport(uint8_t id, const void* data, int len);
  
  // Receives data from the USB OUT endpoint.
  int RecvData(byte* data);
  
  // Receives data from the USB and processes it.
  void RecvfromUsb();
  
  // Appends a new descriptor to the HID device.
  void AppendDescriptor(DynamicHIDSubDescriptor* node);
  
  PIDReportHandler pidReportHandler;  // Object for handling PID reports (force feedback).

protected:
  // Implementation of PluggableUSBModule - gets the interface descriptor.
  int getInterface(uint8_t* interfaceCount);
  
  // Handles descriptor requests from the host.
  int getDescriptor(USBSetup& setup);
  
  // Processes GET_REPORT requests from the host.
  bool GetReport(USBSetup& setup);
  
  // Processes SET_REPORT requests from the host.
  bool SetReport(USBSetup& setup);
  
  // Processes general USB setup requests.
  bool setup(USBSetup& setup);
  
  // Returns a short name for the device.
  uint8_t getShortName(char* name);

private:
  // Endpoint types for communication (IN and OUT).
  #ifdef _VARIANT_ARDUINO_DUE_X_
      uint16_t epType[2]; // Endpoint types for SAM-based boards.
  #else
      uint8_t epType[2];  // Endpoint types for other boards.
  #endif

  DynamicHIDSubDescriptor* rootNode;  // Root node for the descriptor chain.
  uint16_t descriptorSize;            // Total size of the descriptors.

  uint8_t protocol;   // Current protocol (boot or report).
  uint8_t idle;       // Idle rate for HID reports.
};

// Replacement for a global singleton instance of the HID class.
// This function ensures proper initialization order when the class is used.
DynamicHID_& DynamicHID();

// Macro for defining a HID report descriptor.
// This macro simplifies the creation of a HID descriptor with the necessary fields.
#define D_HIDREPORT(length) { 9, 0x21, 0x11, 0x01, 0, 1, 0x22, lowByte(length), highByte(length) }

#endif // USBCON

#endif // DYNAMIC_HID_h
