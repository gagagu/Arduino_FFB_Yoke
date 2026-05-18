/*
  DynamicHID.cpp – improved version
  Original: Copyright (c) 2015, Arduino LLC / Matthew Heironimus (ISC Licence)
*/

#include "DynamicHID.h"

#if defined(USBCON)

#ifdef _VARIANT_ARDUINO_DUE_X_
  #define USB_SendControl  USBD_SendControl
  #define USB_Send         USBD_Send
  #define USB_Recv         USBD_Recv
  #define USB_RecvControl  USBD_RecvControl
  #define USB_Available    USBD_Available
#endif

// Singleton
DynamicHID_& DynamicHID()
{
    static DynamicHID_ obj;
    return obj;
}

int DynamicHID_::getInterface(uint8_t* interfaceCount)
{
    *interfaceCount += 1;
    DYNAMIC_HIDDescriptor hidInterface = {
        D_INTERFACE(pluggedInterface, PID_ENDPOINT_COUNT,
                    USB_DEVICE_CLASS_HUMAN_INTERFACE,
                    DYNAMIC_HID_SUBCLASS_NONE,
                    DYNAMIC_HID_PROTOCOL_NONE),
        D_HIDREPORT(descriptorSize),
        D_ENDPOINT(USB_ENDPOINT_IN(PID_ENDPOINT_IN),   USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01),
        D_ENDPOINT(USB_ENDPOINT_OUT(PID_ENDPOINT_OUT), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

int DynamicHID_::getDescriptor(USBSetup& setup)
{
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) return 0;
    if (setup.wValueH       != DYNAMIC_HID_REPORT_DESCRIPTOR_TYPE)      return 0;
    if (setup.wIndex        != pluggedInterface)                         return 0;

    int total = 0;
    for (DynamicHIDSubDescriptor* node = rootNode; node; node = node->next) {
        int res = USB_SendControl(0, node->data, node->length);
        if (res == -1) return -1;
        total += res;
        res = USB_SendControl(TRANSFER_PGM, node->pid_data, node->pid_length);
        if (res == -1) return -1;
        total += res;
    }
    protocol = DYNAMIC_HID_REPORT_PROTOCOL;
    return total;
}

uint8_t DynamicHID_::getShortName(char* name)
{
    name[0] = 'H'; name[1] = 'I'; name[2] = 'D';
    name[3] = 'A' + (descriptorSize & 0x0F);
    name[4] = 'A' + ((descriptorSize >> 4) & 0x0F);
    return 5;
}

void DynamicHID_::AppendDescriptor(DynamicHIDSubDescriptor* node)
{
    if (!rootNode) {
        rootNode = node;
    } else {
        DynamicHIDSubDescriptor* current = rootNode;
        while (current->next) current = current->next;
        current->next = node;
    }
    descriptorSize += node->length;
    descriptorSize += node->pid_length;
}

// BUG FIX: original used a VLA  uint8_t p[len + 1]  which is a GCC extension,
// not standard C++, and silently overflows the stack on unexpected len values.
// Fixed to a fixed-size buffer with an explicit length guard.
int DynamicHID_::SendReport(uint8_t id, const void* data, int len)
{
    const int MAX_REPORT = 64;
    if (len > MAX_REPORT - 1) return -1;   // guard: drop oversized reports

    uint8_t p[MAX_REPORT];
    p[0] = id;
    memcpy(&p[1], data, len);
    return USB_Send(PID_ENDPOINT_IN | TRANSFER_RELEASE, p, len + 1);
}

// BUG FIX: no bounds check on caller's buffer.
// Added maxLen parameter so callers can tell us how big their buffer is.
int DynamicHID_::RecvData(byte* data, uint8_t maxLen)
{
    int count = 0;
    while (count < maxLen && usb_Available()) {
        data[count++] = USB_Recv(PID_ENDPOINT_OUT);
    }
    return count;
}

// BUG FIX: len is uint16_t so  len >= 0  was always true – error packets
// were forwarded to the PID handler instead of being discarded.
// Changed to  len > 0.
void DynamicHID_::RecvfromUsb()
{
    if (usb_Available()) {
        uint8_t  out_ffbdata[64];
        uint16_t len = USB_Recv(PID_ENDPOINT_OUT, out_ffbdata, sizeof(out_ffbdata));
        if (len > 0) {
            pidReportHandler.UppackUsbData(out_ffbdata, len);
        }
    }
}

bool DynamicHID_::GetReport(USBSetup& setup)
{
    uint8_t report_id   = setup.wValueL;
    uint8_t report_type = setup.wValueH;

    if (report_type == DYNAMIC_HID_REPORT_TYPE_INPUT) {
        USB_SendControl(TRANSFER_RELEASE,
                        pidReportHandler.getPIDStatus(),
                        sizeof(USB_FFBReport_PIDStatus_Input_Data_t));
        return true;
    }

    if (report_type == DYNAMIC_HID_REPORT_TYPE_FEATURE) {
        if (report_id == 6) {
            delayMicroseconds(500);
            USB_SendControl(TRANSFER_RELEASE,
                            pidReportHandler.getPIDBlockLoad(),
                            sizeof(USB_FFBReport_PIDBlockLoad_Feature_Data_t));
            pidReportHandler.pidBlockLoad.reportId = 0;
            return true;
        }
        if (report_id == 7) {
            // BUG FIX: original hardcoded ramPoolSize = 0xFFFF.
            // The host uses this value to decide how many effects it can allocate.
            // Reporting 0xFFFF (65535 bytes) instead of the true MEMORY_SIZE causes
            // the host to believe far more slots are available, leading to effects
            // being requested that the device can never actually service.
            USB_FFBReport_PIDPool_Feature_Data_t pool;
            pool.reportId               = 7;
            pool.ramPoolSize            = MEMORY_SIZE;  // ← actual pool size
            pool.maxSimultaneousEffects = MAX_EFFECTS;
            pool.memoryManagement       = 3;
            USB_SendControl(TRANSFER_RELEASE, &pool, sizeof(pool));
            return true;
        }
    }
    return false;
}

bool DynamicHID_::SetReport(USBSetup& setup)
{
    uint8_t  report_id   = setup.wValueL;
    uint8_t  report_type = setup.wValueH;
    uint16_t length      = setup.wLength;

    if (report_type == DYNAMIC_HID_REPORT_TYPE_FEATURE) {
        // BUG FIX: original called USB_RecvControl(&data, 0) when length==0,
        // which is a no-op but still exercised the USB stack unnecessarily.
        if (length == 0) return true;

        if (report_id == 5) {
            USB_FFBReport_CreateNewEffect_Feature_Data_t ans;
            USB_RecvControl(&ans, sizeof(ans));
            pidReportHandler.CreateNewEffect(&ans);
        }
        return true;
    }
    return false;
}

bool DynamicHID_::setup(USBSetup& setup)
{
    if (pluggedInterface != setup.wIndex) return false;

    uint8_t request     = setup.bRequest;
    uint8_t requestType = setup.bmRequestType;

    if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
        if (request == DYNAMIC_HID_GET_REPORT) {
            return GetReport(setup);
        }
        if (request == DYNAMIC_HID_GET_PROTOCOL) {
            // Send current protocol (boot=0 / report=1) to host.
            // Required by HID spec; missing response can cause enumeration issues.
            USB_SendControl(TRANSFER_RELEASE, &protocol, 1);
            return true;
        }
        if (request == DYNAMIC_HID_GET_IDLE) {
            USB_SendControl(TRANSFER_RELEASE, &idle, 1);
            return true;
        }
    }

    if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
        if (request == DYNAMIC_HID_SET_PROTOCOL) {
            protocol = setup.wValueL;
            return true;
        }
        if (request == DYNAMIC_HID_SET_IDLE) {
            idle = setup.wValueL;
            return true;
        }
        if (request == DYNAMIC_HID_SET_REPORT) {
            return SetReport(setup);
        }
    }
    return false;
}

DynamicHID_::DynamicHID_(void)
    : PluggableUSBModule(PID_ENDPOINT_COUNT, 1, epType),
      rootNode(NULL), descriptorSize(0),
      protocol(DYNAMIC_HID_REPORT_PROTOCOL), idle(1)
{
    epType[0] = EP_TYPE_INTERRUPT_IN;
    epType[1] = EP_TYPE_INTERRUPT_OUT;
    PluggableUSB().plug(this);
}

int  DynamicHID_::begin(void)      { return 0; }
bool DynamicHID_::usb_Available()  { return USB_Available(PID_ENDPOINT_OUT); }

#endif // USBCON
