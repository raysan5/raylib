#include "nxusb.h"

#include <switch.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/iosupport.h>

#define U64_MAX	((u64)~0ULL)
#define TOTAL_INTERFACES 4
#define TOTAL_ENDPOINTS 4
#define EP_IN 0
#define EP_OUT 1

typedef enum {
    UsbDirectionRead  = 0,
    UsbDirectionWrite = 1,
} UsbDirection;

typedef struct {
    struct usb_interface_descriptor *interface_desc;
    struct usb_endpoint_descriptor *endpoint_desc[4];
    const char *string_descriptor;
} UsbInterfaceDesc;

typedef struct {
    UsbDsEndpoint *endpoint;
    u8 *buffer;
    RwLock lock;
} UsbCommsEndpoint;

typedef struct {
    RwLock lock;
    bool initialized;
    UsbDsInterface* interface;
    u32 endpoint_number;
    UsbCommsEndpoint endpoint[TOTAL_ENDPOINTS];
} UsbCommsInterface;

static bool g_usbCommsInitialized = false;
static UsbCommsInterface g_usbCommsInterfaces[TOTAL_INTERFACES];
static RwLock g_usbCommsLock;
static int ep_in = 1;
static int ep_out = 1;

static Result UsbCommsInterfaceInit(u32 intf_ind, const UsbInterfaceDesc *info)
{
    Result rc = 0;
    UsbCommsInterface *interface = &g_usbCommsInterfaces[intf_ind];

    u8 index = 0;
    if (info->string_descriptor != NULL)
    {
        usbDsAddUsbStringDescriptor(&index, info->string_descriptor);
    }
    info->interface_desc->iInterface = index;

    struct usb_ss_endpoint_companion_descriptor endpoint_companion = {
            .bLength = sizeof(struct usb_ss_endpoint_companion_descriptor),
            .bDescriptorType = USB_DT_SS_ENDPOINT_COMPANION,
            .bMaxBurst = 0x0F,
            .bmAttributes = 0x00,
            .wBytesPerInterval = 0x00,
    };

    interface->initialized = 1;

    //The buffer for PostBufferAsync commands must be 0x1000-byte aligned.
    for (u32 i = 0; i < interface->endpoint_number; i++)
    {
        interface->endpoint[i].buffer = (u8*)memalign(0x1000, 0x1000);
        if (interface->endpoint[i].buffer == NULL)
        {
            rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
            break;
        }
        memset(interface->endpoint[i].buffer, 0, 0x1000);
    }
    if (R_FAILED(rc)) return rc;

    rc = usbDsRegisterInterface(&interface->interface);
    if (R_FAILED(rc)) return rc;

    info->interface_desc->bInterfaceNumber = interface->interface->interface_index;
    for (u32 i = 0; i < interface->endpoint_number; i++)
    {
        if((info->endpoint_desc[i]->bEndpointAddress & USB_ENDPOINT_IN) != 0)
        {
            info->endpoint_desc[i]->bEndpointAddress |= ep_in;
            ep_in++;
        }
        else
        {
            info->endpoint_desc[i]->bEndpointAddress |= ep_out;
            ep_out++;
        }
    }

    // Full Speed Config
    rc = usbDsInterface_AppendConfigurationData(interface->interface, UsbDeviceSpeed_Full, info->interface_desc, USB_DT_INTERFACE_SIZE);
    if (R_FAILED(rc)) return rc;

    for (u32 i = 0; i < interface->endpoint_number; i++)
    {
        if(info->endpoint_desc[i]->bmAttributes == USB_TRANSFER_TYPE_BULK)
            info->endpoint_desc[i]->wMaxPacketSize = 0x40;
        rc = usbDsInterface_AppendConfigurationData(interface->interface, UsbDeviceSpeed_Full, info->endpoint_desc[i], USB_DT_ENDPOINT_SIZE);
        if (R_FAILED(rc)) return rc;
    }

    // High Speed Config
    rc = usbDsInterface_AppendConfigurationData(interface->interface, UsbDeviceSpeed_High, info->interface_desc, USB_DT_INTERFACE_SIZE);
    if (R_FAILED(rc)) return rc;

    for (u32 i = 0; i < interface->endpoint_number; i++)
    {
        if(info->endpoint_desc[i]->bmAttributes == USB_TRANSFER_TYPE_BULK)
            info->endpoint_desc[i]->wMaxPacketSize = 0x200;
        rc = usbDsInterface_AppendConfigurationData(interface->interface, UsbDeviceSpeed_High, info->endpoint_desc[i], USB_DT_ENDPOINT_SIZE);
        if (R_FAILED(rc)) return rc;
    }

    // Super Speed Config
    rc = usbDsInterface_AppendConfigurationData(interface->interface, UsbDeviceSpeed_Super, info->interface_desc, USB_DT_INTERFACE_SIZE);
    if (R_FAILED(rc)) return rc;

    for (u32 i = 0; i < interface->endpoint_number; i++)
    {
        if(info->endpoint_desc[i]->bmAttributes == USB_TRANSFER_TYPE_BULK)
            info->endpoint_desc[i]->wMaxPacketSize = 0x400;
        rc = usbDsInterface_AppendConfigurationData(interface->interface, UsbDeviceSpeed_Super, info->endpoint_desc[i], USB_DT_ENDPOINT_SIZE);
        if (R_FAILED(rc)) return rc;
        rc = usbDsInterface_AppendConfigurationData(interface->interface, UsbDeviceSpeed_Super, &endpoint_companion, USB_DT_SS_ENDPOINT_COMPANION_SIZE);
        if (R_FAILED(rc)) return rc;
    }

    //Setup endpoints.
    for (u32 i = 0; i < interface->endpoint_number; i++)
    {
        rc = usbDsInterface_RegisterEndpoint(interface->interface, &interface->endpoint[i].endpoint, info->endpoint_desc[i]->bEndpointAddress);
        if (R_FAILED(rc)) return rc;
    }

    rc = usbDsInterface_EnableInterface(interface->interface);
    if (R_FAILED(rc)) return rc;

    return rc;
}

static Result UsbCommsTransfer(UsbCommsEndpoint *ep, UsbDirection dir, const void* buffer, size_t size, u64 timeout, size_t *transferredSize)
{
    Result rc=0;
    u32 urbId=0;
    u32 chunksize=0;
    u8 transfer_type=0;
    u8 *bufptr = (u8*)buffer;
    u8 *transfer_buffer = NULL;
    u32 tmp_transferredSize = 0;
    size_t total_transferredSize=0;
    UsbDsReportData reportdata;

    //Makes sure endpoints are ready for data-transfer / wait for init if needed.
    rc = usbDsWaitReady(U64_MAX);
    if (R_FAILED(rc)) return rc;

    while(size)
    {
        //When bufptr isn't page-aligned copy the data into g_usbComms_endpoint_in_buffer and transfer that, otherwise use the bufptr directly.
        if(((u64)bufptr) & 0xfff)
        {
            transfer_buffer = ep->buffer;
            memset(ep->buffer, 0, 0x1000);

            chunksize = 0x1000;
            //After this transfer, bufptr will be page-aligned (if size is large enough for another transfer).
            chunksize-= ((u64)bufptr) & 0xfff;
            if (size<chunksize) chunksize = size;

            if(dir == UsbDirectionWrite)
                memcpy(ep->buffer, bufptr, chunksize);

            transfer_type = 0;
        }
        else
        {
            transfer_buffer = bufptr;
            chunksize = size;
            transfer_type = 1;
        }

        //Start transfer.
        rc = usbDsEndpoint_PostBufferAsync(ep->endpoint, transfer_buffer, chunksize, &urbId);
        if(R_FAILED(rc)) return rc;

        //Wait for the transfer to finish.
        rc = eventWait(&ep->endpoint->CompletionEvent, timeout);

        if (R_FAILED(rc))
        {
            usbDsEndpoint_Cancel(ep->endpoint);
            eventWait(&ep->endpoint->CompletionEvent, U64_MAX);
            eventClear(&ep->endpoint->CompletionEvent);
            return rc;
        }
        eventClear(&ep->endpoint->CompletionEvent);

        rc = usbDsEndpoint_GetReportData(ep->endpoint, &reportdata);
        if (R_FAILED(rc)) return rc;

        rc = usbDsParseReportData(&reportdata, urbId, NULL, &tmp_transferredSize);
        if (R_FAILED(rc)) return rc;

        if (tmp_transferredSize > chunksize) tmp_transferredSize = chunksize;

        total_transferredSize+= (size_t)tmp_transferredSize;

        if ((transfer_type==0) && (dir == UsbDirectionRead))
            memcpy(bufptr, transfer_buffer, tmp_transferredSize);

        bufptr+= tmp_transferredSize;
        size-= tmp_transferredSize;

        if (tmp_transferredSize < chunksize) break;
    }

    if (transferredSize) *transferredSize = total_transferredSize;

    return rc;
}

static void UsbCommsInterfaceFree(UsbCommsInterface *interface)
{
    rwlockWriteLock(&interface->lock);
    if (!interface->initialized)
    {
        rwlockWriteUnlock(&interface->lock);
        return;
    }

    interface->initialized = 0;
    interface->interface = NULL;

    for (u32 i = 0; i < interface->endpoint_number; i++)
    {
        rwlockWriteLock(&interface->endpoint[i].lock);
        interface->endpoint[i].endpoint = NULL;
        free(interface->endpoint[i].buffer);
        interface->endpoint[i].buffer = NULL;
        rwlockWriteUnlock(&interface->endpoint[i].lock);
    }

    rwlockWriteUnlock(&interface->lock);
}

static size_t UsbTransfer(u32 interface, u32 endpoint, UsbDirection dir, void* buffer, size_t size, u64 timeout)
{
    size_t transferredSize=-1;
    u32 state=0;
    Result rc, rc2;
    bool initialized;

    UsbCommsInterface *inter = &g_usbCommsInterfaces[interface];
    UsbCommsEndpoint *ep = &inter->endpoint[endpoint];
    rwlockReadLock(&inter->lock);
    initialized = inter->initialized;
    rwlockReadUnlock(&inter->lock);
    if (!initialized) return 0;

    rwlockWriteLock(&ep->lock);
    rc = UsbCommsTransfer(ep, dir, buffer, size, timeout, &transferredSize);
    rwlockWriteUnlock(&ep->lock);
    if (R_FAILED(rc))
    {
        rc2 = usbDsGetState(&state);
        if (R_SUCCEEDED(rc2))
        {
            if (state!=5)
            {
                rwlockWriteLock(&ep->lock);
                // If state changed during transfer, try again. usbDsWaitReady() will be called from this.
                rc = UsbCommsTransfer(ep, dir, buffer, size, timeout, &transferredSize);
                rwlockWriteUnlock(&ep->lock);
            }
        }
        if (R_FAILED(rc))
        {
            transferredSize = 0;
        }
    }
    return transferredSize;
}

/*
 * REDIRECT IO FUNCTIONS
 */

static ssize_t _write_stdout(struct _reent *r,void *fd,const char *ptr, size_t len)
{
    return UsbTransfer(0, EP_IN, UsbDirectionWrite, (void*)ptr, len, U64_MAX);
}

static const devoptab_t dotab_stdout = {
        "usb",
        0,
        NULL,
        NULL,
        _write_stdout,
        NULL,
        NULL,
        NULL
};

static void RedirectOutput(void)
{
    devoptab_list[STD_OUT] = &dotab_stdout;
    devoptab_list[STD_ERR] = &dotab_stdout;
    setvbuf(stdout, NULL , _IONBF, 0);
    setvbuf(stderr, NULL , _IONBF, 0);
}

/*
 * PUBLIC FUNCTIONS
 */

bool NxUsbDebuggerInit(void)
{
    u32 num_interfaces = 1;

    struct usb_device_descriptor device_descriptor = {
            .bLength = USB_DT_DEVICE_SIZE,
            .bDescriptorType = USB_DT_DEVICE,
            .bcdUSB = 0x0110,
            .bDeviceClass = 0x00,
            .bDeviceSubClass = 0x00,
            .bDeviceProtocol = 0x00,
            .bMaxPacketSize0 = 0x40,
            .idVendor = 0x057e,
            .idProduct = 0x4000,
            .bcdDevice = 0x0100,
            .bNumConfigurations = 0x01
    };

    struct usb_interface_descriptor serial_interface_descriptor = {
            .bLength = USB_DT_INTERFACE_SIZE,
            .bDescriptorType = USB_DT_INTERFACE,
            .bNumEndpoints = 2,
            .bInterfaceClass = USB_CLASS_VENDOR_SPEC,
            .bInterfaceSubClass = USB_CLASS_VENDOR_SPEC,
            .bInterfaceProtocol = USB_CLASS_VENDOR_SPEC,
    };

    struct usb_endpoint_descriptor serial_endpoint_descriptor_in = {
            .bLength = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType = USB_DT_ENDPOINT,
            .bEndpointAddress = USB_ENDPOINT_IN,
            .bmAttributes = USB_TRANSFER_TYPE_BULK,
            .wMaxPacketSize = 0x200,
    };

    struct usb_endpoint_descriptor serial_endpoint_descriptor_out = {
            .bLength = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType = USB_DT_ENDPOINT,
            .bEndpointAddress = USB_ENDPOINT_OUT,
            .bmAttributes = USB_TRANSFER_TYPE_BULK,
            .wMaxPacketSize = 0x200,
    };

    UsbInterfaceDesc infos[1];
    infos[0].interface_desc = &serial_interface_descriptor;
    infos[0].endpoint_desc[EP_IN] = &serial_endpoint_descriptor_in;
    infos[0].endpoint_desc[EP_OUT] = &serial_endpoint_descriptor_out;
    infos[0].string_descriptor = NULL;

    Result rc = 0;
    rwlockWriteLock(&g_usbCommsLock);

    if (g_usbCommsInitialized)
    {
        rc = MAKERESULT(Module_Libnx, LibnxError_AlreadyInitialized);
    }
    else if (num_interfaces > TOTAL_INTERFACES)
    {
        rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
    }
    else
    {
        rc = usbDsInitialize();

        if (R_SUCCEEDED(rc)) {
            if (hosversionAtLeast(5,0,0))
            {
                u8 iManufacturer, iProduct, iSerialNumber;
                static const u16 supported_langs[1] = {0x0409};
                // Send language descriptor
                rc = usbDsAddUsbLanguageStringDescriptor(NULL, supported_langs, sizeof(supported_langs) / sizeof(u16));
                // Send manufacturer
                if (R_SUCCEEDED(rc)) rc = usbDsAddUsbStringDescriptor(&iManufacturer, "Nintendo");
                // Send product
                if (R_SUCCEEDED(rc)) rc = usbDsAddUsbStringDescriptor(&iProduct, "Nintendo Switch");
                // Send serial number
                if (R_SUCCEEDED(rc)) rc = usbDsAddUsbStringDescriptor(&iSerialNumber, "SerialNumber");

                // Send device descriptors
                device_descriptor.iManufacturer = iManufacturer;
                device_descriptor.iProduct = iProduct;
                device_descriptor.iSerialNumber = iSerialNumber;

                // Full Speed is USB 1.1
                if (R_SUCCEEDED(rc)) rc = usbDsSetUsbDeviceDescriptor(UsbDeviceSpeed_Full, &device_descriptor);

                // High Speed is USB 2.0
                device_descriptor.bcdUSB = 0x0200;
                if (R_SUCCEEDED(rc)) rc = usbDsSetUsbDeviceDescriptor(UsbDeviceSpeed_High, &device_descriptor);

                // Super Speed is USB 3.0
                device_descriptor.bcdUSB = 0x0300;
                // Upgrade packet size to 512
                device_descriptor.bMaxPacketSize0 = 0x09;
                if (R_SUCCEEDED(rc)) rc = usbDsSetUsbDeviceDescriptor(UsbDeviceSpeed_Super, &device_descriptor);

                // Define Binary Object Store
                u8 bos[0x16] = {
                        0x05, // .bLength
                        USB_DT_BOS, // .bDescriptorType
                        0x16, 0x00, // .wTotalLength
                        0x02, // .bNumDeviceCaps

                        // USB 2.0
                        0x07, // .bLength
                        USB_DT_DEVICE_CAPABILITY, // .bDescriptorType
                        0x02, // .bDevCapabilityType
                        0x02, 0x00, 0x00, 0x00, // dev_capability_data

                        // USB 3.0
                        0x0A, // .bLength
                        USB_DT_DEVICE_CAPABILITY, // .bDescriptorType
                        0x03, // .bDevCapabilityType
                        0x00, 0x0E, 0x00, 0x03, 0x00, 0x00, 0x00
                };
                if (R_SUCCEEDED(rc)) rc = usbDsSetBinaryObjectStore(bos, sizeof(bos));
            }

            if (R_SUCCEEDED(rc))
            {
                for (u32 i = 0; i < num_interfaces; i++)
                {
                    UsbCommsInterface *intf = &g_usbCommsInterfaces[i];
                    const UsbInterfaceDesc *info = &infos[i];
                    intf->endpoint_number = info->interface_desc->bNumEndpoints;
                    rwlockWriteLock(&intf->lock);
                    for (u32 j = 0; j < intf->endpoint_number; j++)
                    {
                        rwlockWriteLock(&intf->endpoint[j].lock);
                    }
                    rc = UsbCommsInterfaceInit(i, info);
                    for (u32 j = 0; j < intf->endpoint_number; j++)
                    {
                        rwlockWriteUnlock(&intf->endpoint[j].lock);
                    }
                    rwlockWriteUnlock(&intf->lock);
                    if (R_FAILED(rc)) break;
                }
            }
        }

        if (R_SUCCEEDED(rc) && hosversionAtLeast(5,0,0))
        {
            rc = usbDsEnable();
        }

        if (R_FAILED(rc))
        {
            NxUsbDebuggerEnd();
        }
    }

    if (R_SUCCEEDED(rc))
    {
        g_usbCommsInitialized = true;
    }

    rwlockWriteUnlock(&g_usbCommsLock);

    if (g_usbCommsInitialized) RedirectOutput();
    return g_usbCommsInitialized;
}

void NxUsbDebuggerEnd(void)
{
    rwlockWriteLock(&g_usbCommsLock);

    usbDsExit();

    g_usbCommsInitialized = false;

    rwlockWriteUnlock(&g_usbCommsLock);

    for (u32 i = 0; i < TOTAL_INTERFACES; i++)
    {
        UsbCommsInterfaceFree(&g_usbCommsInterfaces[i]);
    }
}
