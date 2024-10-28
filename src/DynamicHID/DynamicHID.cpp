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

#include "DynamicHID.h"

#if defined(USBCON)

#ifdef _VARIANT_ARDUINO_DUE_X_
#define USB_SendControl USBD_SendControl
#define USB_Send USBD_Send
#define USB_Recv USBD_Recv
#define USB_RecvControl USBD_RecvControl
#define USB_Available USBD_Available
#endif

// Singleton pattern for the DynamicHID_ class.
// Ensures that only one instance of the DynamicHID_ class exists.
DynamicHID_& DynamicHID()
{
    static DynamicHID_ obj; // Static object created once and returned on each call.
    return obj;
}

// Returns the HID interface and sends HID descriptors to the host.
int DynamicHID_::getInterface(uint8_t* interfaceCount)
{
    *interfaceCount += 1; // Increment the number of interfaces by 1.
    
    // Define the HID interface descriptor.
    DYNAMIC_HIDDescriptor hidInterface = {
        D_INTERFACE(pluggedInterface, PID_ENPOINT_COUNT, USB_DEVICE_CLASS_HUMAN_INTERFACE, DYNAMIC_HID_SUBCLASS_NONE, DYNAMIC_HID_PROTOCOL_NONE),
        D_HIDREPORT(descriptorSize),
        D_ENDPOINT(USB_ENDPOINT_IN(PID_ENDPOINT_IN), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01),
        D_ENDPOINT(USB_ENDPOINT_OUT(PID_ENDPOINT_OUT), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    
    // Send the descriptor to the host.
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

// Handles a request for an HID descriptor from the host.
int DynamicHID_::getDescriptor(USBSetup& setup)
{
    // Check if the request is of the correct type.
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
    if (setup.wValueH != DYNAMIC_HID_REPORT_DESCRIPTOR_TYPE) { return 0; }

    // Check if the interface number is correct.
    if (setup.wIndex != pluggedInterface) { return 0; }

    // Iterate through all HID sub-descriptors and send them to the host.
    int total = 0;
    DynamicHIDSubDescriptor* node;
    for (node = rootNode; node; node = node->next) {
        int res = USB_SendControl(0, node->data, node->length); // Send the descriptor to the host.
        if (res == -1)
            return -1; // Abort on error.
        total += res;

        // Optionally send additional PID data.
        res = USB_SendControl(TRANSFER_PGM, node->pid_data, node->pid_length);
        if (res == -1)
            return -1; // Abort on error.
        total += res;
    }

    // Reset the protocol to the default (report mode).
    protocol = DYNAMIC_HID_REPORT_PROTOCOL;

    return total; // Return the total amount of data sent.
}

// Returns the short name of the device based on descriptor size.
uint8_t DynamicHID_::getShortName(char *name)
{
    name[0] = 'H'; // 'HID' is used as a standard prefix.
    name[1] = 'I';
    name[2] = 'D';
    // Adds additional characters based on descriptor size.
    name[3] = 'A' + (descriptorSize & 0x0F);
    name[4] = 'A' + ((descriptorSize >> 4) & 0x0F);
    return 5; // Returns the length of the generated name (5 characters).
}

// Adds a new descriptor to the linked list of sub-descriptors.
void DynamicHID_::AppendDescriptor(DynamicHIDSubDescriptor *node)
{
    // If no descriptors exist, set the new node as the root.
    if (!rootNode) {
        rootNode = node;
    } else {
        // Traverse the list to find the end.
        DynamicHIDSubDescriptor *current = rootNode;
        while (current->next) {
            current = current->next;
        }
        current->next = node; // Add the new node at the end.
    }
    // Update the total size of the descriptors.
    descriptorSize += node->length;
    descriptorSize += node->pid_length;
}

// Sends a HID report to the host.
int DynamicHID_::SendReport(uint8_t id, const void* data, int len)
{
    // Create an array containing the report ID and the data.
    uint8_t p[len + 1];
    p[0] = id;
    memcpy(&p[1], data, len); // Copy the data into the array.
    
    // Send the data through the IN endpoint.
    return USB_Send(PID_ENDPOINT_IN | TRANSFER_RELEASE, p, len + 1);
}

// Receives data from the USB OUT endpoint and stores it in the data buffer.
int DynamicHID_::RecvData(byte* data)
{
    int count = 0;
    // Read all available data from the USB buffer.
    while (usb_Available() > 0) {
        data[count++] = USB_Recv(PID_ENDPOINT_OUT);
    }
    return count; // Return the number of bytes received.
}

// This function checks if data is available and reads it when present.
void DynamicHID_::RecvfromUsb()
{
    if (usb_Available() > 0) {
        uint8_t out_ffbdata[64]; // Data buffer.
        uint16_t len = USB_Recv(PID_ENDPOINT_OUT, &out_ffbdata, 64); // Read up to 64 bytes.
        if (len >= 0) {
            // Pass the received data to the PID handler for further processing.
            pidReportHandler.UppackUsbData(out_ffbdata, len);
        }
    }
}

// This function handles GET_REPORT requests from the host.
bool DynamicHID_::GetReport(USBSetup& setup) {
    uint8_t report_id = setup.wValueL;
    uint8_t report_type = setup.wValueH;

    if (report_type == DYNAMIC_HID_REPORT_TYPE_INPUT) {
        // Send the PID status report back.
        USB_SendControl(TRANSFER_RELEASE, pidReportHandler.getPIDStatus(), sizeof(USB_FFBReport_PIDStatus_Input_Data_t));
    }
    if (report_type == DYNAMIC_HID_REPORT_TYPE_OUTPUT) {
        // No implementation for OUTPUT types in this example.
    }
    if (report_type == DYNAMIC_HID_REPORT_TYPE_FEATURE) {
        // Example for FEATURE reports:
        if (report_id == 6) {
            delayMicroseconds(500);
            USB_SendControl(TRANSFER_RELEASE, pidReportHandler.getPIDBlockLoad(), sizeof(USB_FFBReport_PIDBlockLoad_Feature_Data_t));
            pidReportHandler.pidBlockLoad.reportId = 0; // Reset the report ID.
            return true;
        }
        if (report_id == 7) {
            // Send information about the PID memory pool.
            USB_FFBReport_PIDPool_Feature_Data_t ans;
            ans.reportId = report_id;
            ans.ramPoolSize = 0xffff;
            ans.maxSimultaneousEffects = MAX_EFFECTS;
            ans.memoryManagement = 3;
            USB_SendControl(TRANSFER_RELEASE, &ans, sizeof(USB_FFBReport_PIDPool_Feature_Data_t));
            return true;
        }
    }
    return false; // Report not recognized or supported.
}

// Processes SET_REPORT requests from the host.
bool DynamicHID_::SetReport(USBSetup& setup) {
    uint8_t report_id = setup.wValueL;
    uint8_t report_type = setup.wValueH;
    uint16_t length = setup.wLength;
    uint8_t data[10]; // Buffer for incoming data.

    if (report_type == DYNAMIC_HID_REPORT_TYPE_FEATURE) {
        if (length == 0) {
            USB_RecvControl(&data, length); // Receive control data.
            return true;
        }
        if (report_id == 5) {
            USB_FFBReport_CreateNewEffect_Feature_Data_t ans;
            USB_RecvControl(&ans, sizeof(USB_FFBReport_CreateNewEffect_Feature_Data_t));
            pidReportHandler.CreateNewEffect(&ans); // Create a new effect based on the received data.
        }
        return true;
    }
    return false; // Report not supported.
}

// This function processes general USB setup requests from the host.
bool DynamicHID_::setup(USBSetup& setup)
{
    if (pluggedInterface != setup.wIndex) {
        return false; // Wrong interface, ignore the request.
    }

    uint8_t request = setup.bRequest;
    uint8_t requestType = setup.bmRequestType;

    if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
        if (request == DYNAMIC_HID_GET_REPORT) {
            GetReport(setup); // Process GET_REPORT request.
            return true;
        }
        if (request == DYNAMIC_HID_GET_PROTOCOL) {
            // TODO: Send protocol to the host.
            return true;
        }
        if (request == DYNAMIC_HID_GET_IDLE) {
            // TODO: Send idle status to the host.
        }
    }

    if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
        if (request == DYNAMIC_HID_SET_PROTOCOL) {
            // The USB host tells us if we are in boot or report mode.
            protocol = setup.wValueL; // Update the protocol based on host request.
            return true;
        }
        if (request == DYNAMIC_HID_SET_IDLE) {
            idle = setup.wValueL; // Update the idle rate based on host request.
            return true;
        }
        if (request == DYNAMIC_HID_SET_REPORT) {
            SetReport(setup); // Process SET_REPORT request.
            return true;
        }
    }
    return false; // Request not handled.
}

// Constructor for the DynamicHID_ class, initializing the interface and endpoints.
DynamicHID_::DynamicHID_(void) : PluggableUSBModule(PID_ENPOINT_COUNT, 1, epType),
                   rootNode(NULL), descriptorSize(0),
                   protocol(DYNAMIC_HID_REPORT_PROTOCOL), idle(1)
{
    // Initialize the endpoints for IN and OUT.
	epType[0] = EP_TYPE_INTERRUPT_IN;
	epType[1] = EP_TYPE_INTERRUPT_OUT;
    PluggableUSB().plug(this); // Register this class as a USB device.
}

int DynamicHID_::begin(void)
{
	return 0;
}

bool DynamicHID_::usb_Available() {
	return USB_Available(PID_ENDPOINT_OUT);
}

#endif /* if defined(USBCON) */
