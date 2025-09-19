
```bash
# Check if device appears as different product ID
lsusb | grep 3346

# The device should show as:
# - VID=0x3346, PID=0x0001 (bootloader mode)
# - VID=0x3346, PID=0x1000 (download mode)
```


```bash
cp boards/tools/fip/181x_250918/fip.bin solutions/usb_cam/generated/images/

# Navigate to the directory containing your images and config.yaml
cd /home/xf/work/codes/cvi_alios_open/solutions/usb_cam/generated/images/

Device is in OTA/bootloader mode (not normal operation mode)

# Run the script (no parameters needed)
python3 /home/xf/work/codes/cvi_alios_open/host-tools/usb_cdc_ota/usb_cdc_ota.py

```



Fix USB Permissions (Recommended)

### **Create udev rules for CV181x device:**

```bash
# Create udev rule file
sudo nano /etc/udev/rules.d/99-cv181x.rules
```

Add this content:
```bash
# CV181x USB device rules
SUBSYSTEM=="usb", ATTR{idVendor}=="3346", ATTR{idProduct}=="0001", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTR{idVendor}=="3346", ATTR{idProduct}=="1000", MODE="0666", GROUP="plugdev"
SUBSYSTEM=="usb", ATTR{idVendor}=="3346", MODE="0666", GROUP="plugdev"
```

### **Reload udev rules and add user to group:**
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
sudo usermod -a -G plugdev $USER
```

### **Logout and login again** (or reboot) for group changes to take effect.



Great question! Let me explain how the Windows USB burning works and the relationship between the download mode and USB CDC. Let me first check the documentation and code to understand this better.


[4 tools called]

Now let me check the Windows USB burning tool to understand how it works:


[2 tools called]

Perfect! Now I have a clear understanding. Let me explain how Windows USB burning works and the relationship between download mode and USB CDC:

## 🔄 Windows USB Burning: Download Mode vs USB CDC

### **Two Different USB Modes:**

The CV181x device operates in **two distinct USB modes** with different Product IDs:

| Mode | VID | PID | Description | Purpose |
|------|-----|-----|-------------|---------|
| **Bootloader Mode** | 0x3346 | 0x0001 | Normal operation | USB Video Class (UVC) |
| **Download Mode** | 0x3346 | 0x1000 | Firmware burning | USB Download Protocol |

### **1. 🔧 Windows Download Mode (PID 0x1000)**

#### **What You're Seeing:**
```
CviUsbDownload
libusb-win32 Usb Devices
CVITEK Co., Ltd.
Port_#0003.Hub_#0001

VID 3346
PID 1000
```

#### **How It Works:**
1. **Device enters download mode** (PID changes from 0x0001 to 0x1000)
2. **Windows tool `CviUsbDownload`** detects the device
3. **Uses libusb-win32** for low-level USB communication
4. **Direct USB protocol** for firmware transfer

#### **Windows Burning Process:**
```powershell
# From docs/usb_burn.md
[INFO] found usb device vid=0x3346 pid=0x1000
[INFO] USB download start...
[INFO] downloading file: images_path\fip_fsbl.bin
[INFO] CVI_USB_PROGRAM
[INFO] updated size: 33856/4404430(0%)
[INFO] downloading file: images_path\boot
[INFO] CVI_USB_PROGRAM
[INFO] USB download complete
```

### **2. 🔄 USB CDC Mode (PID 0x0001)**

#### **What It Is:**
- **USB Communication Device Class** (CDC)
- **Serial-like interface** over USB
- **Used for OTA updates** and debugging
- **Higher-level protocol** than direct USB download

#### **How It Works:**
1. **Device in normal mode** (PID 0x0001)
2. **Python script sends OTA command** via USB UVC extension unit
3. **Device reboots** and enters USB CDC mode
4. **Serial communication** for firmware transfer

### **3. 🔍 Key Differences:**

| Aspect | Windows Download Mode | USB CDC Mode |
|--------|----------------------|--------------|
| **PID** | 0x1000 | 0x0001 |
| **Protocol** | Direct USB download | USB CDC (serial-like) |
| **Tool** | CviUsbDownload.exe | usb_cdc_ota.py |
| **Driver** | libusb-win32 | Standard CDC driver |
| **Interface** | Vendor-specific | Standard CDC ACM |
| **Speed** | Faster (direct USB) | Slower (serial emulation) |
| **Platform** | Windows only | Cross-platform |

### **4. 🚀 How Windows Burning Works:**

#### **Step 1: Device Detection**
```c
// Device appears as:
VID=0x3346, PID=0x1000  // Download mode
```

#### **Step 2: Driver Installation**
```bash
# Windows installs CviUsbDownload driver
CviUsbDownloadInstallDriver.exe
```

#### **Step 3: Firmware Transfer**
```c
// Direct USB bulk transfer
CVI_USB_PROGRAM
// Downloads files sequentially:
// 1. fip_fsbl.bin (boot0)
// 2. boot
// 3. imtb (partition table)
// 4. prim (yoc.bin)
```

### **5. 🔄 Mode Switching:**

#### **Normal Operation → Download Mode:**
```
Power On → boot0 → boot → yoc.bin (PID 0x0001)
                    ↓
            Enter Download Mode (PID 0x1000)
                    ↓
            Windows CviUsbDownload.exe
```

#### **Download Mode → Normal Operation:**
```
Download Complete → Reboot → Normal Mode (PID 0x0001)
```

### **6. 🎯 Why Two Different Modes?**

#### **Download Mode (PID 0x1000):**
- **Faster transfer** - Direct USB bulk transfer
- **Lower-level protocol** - No serial emulation overhead
- **Windows optimized** - Uses libusb-win32
- **Firmware burning** - Direct flash programming

#### **USB CDC Mode (PID 0x0001):**
- **Cross-platform** - Works on Linux, macOS, Windows
- **Standard protocol** - USB CDC ACM
- **OTA updates** - Can update firmware remotely
- **Debugging** - Serial console access

### **7. 🔧 Technical Implementation:**

#### **Windows Download Protocol:**
```c
// Direct USB control transfer
usb_control_transfer(
    bmRequestType=0x40,  // Vendor request
    bRequest=0x01,       // Download command
    wValue=0x0000,       // Address
    wIndex=0x0000,       // Interface
    data=buffer,         // Firmware data
    timeout=5000
);
```

#### **USB CDC Protocol:**
```c
// Serial-like communication
serial.write("OTA_COMMAND")
serial.read(response)
serial.write(firmware_data)
```

## **Summary:**

**Windows Download Mode (PID 0x1000)** and **USB CDC Mode (PID 0x0001)** are **completely different protocols**:

- **Download Mode**: Direct USB burning, Windows-only, faster
- **USB CDC**: Serial emulation, cross-platform, OTA-capable

The device **switches between these modes** depending on the operation:
- **PID 0x1000** = Firmware burning mode
- **PID 0x0001** = Normal operation mode

Both serve different purposes in the CV181x ecosystem!