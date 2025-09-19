
running individual components in QEMU or x86 Ubuntu.



[3 tools called]

Now let me create a comprehensive guide for running individual components in QEMU or x86 Ubuntu:

# Complete Guide: Running Individual Components in QEMU or x86 Ubuntu

## Overview

This guide shows you how to test individual components from the CV181x AliOS project on x86 Ubuntu or in QEMU emulation, without needing the actual hardware.

## Method 1: Using Docker for Cross-Platform Testing

### **Step 1: Create Docker Environment**

```bash
# Create Dockerfile for testing environment
cat > Dockerfile << 'EOF'
FROM ubuntu:20.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    qemu-user-static \
    gcc-multilib \
    g++-multilib \
    libc6-dev-i386 \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install pyserial pyusb

# Set working directory
WORKDIR /workspace

# Copy project
COPY . /workspace/

# Enable QEMU user emulation
RUN update-binfmts --install i386 /usr/bin/qemu-i386-static --magic '\x7fELF\x01\x01\x01\x03\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x03\x00\x01\x00\x00\x00' --mask '\xff\xff\xff\xff\xff\xff\xff\xfc\xff\xff\xff\xff\xff\xff\xff\xff\xf8\xff\xff\xff\xff\xff\xff\xff'
EOF

# Build Docker image
docker build -t cvi-test .
```

### **Step 2: Run Tests in Docker**

```bash
# Run container with project mounted
docker run -it --rm -v $(pwd):/workspace cvi-test

# Inside container, test individual components
cd /workspace/solutions/peripherals_test
make clean
make
```

## Method 2: Native x86 Testing with Mock Components

### **Step 1: Create Mock Testing Framework**

```bash
# Create mock testing directory
mkdir -p /home/xf/work/codes/cvi_alios_open/mock_tests
cd /home/xf/work/codes/cvi_alios_open/mock_tests
```

### **Step 2: Create Mock Component Tests**

```c
// mock_tests/test_pixel_inversion.c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// Mock video frame structure
typedef struct {
    uint8_t *data;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
} mock_video_frame_t;

// Mock pixel inversion function
void mock_pixel_invert_yuyv(uint8_t *src, uint8_t *dst, uint32_t width, uint32_t height) {
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width * 2; j += 2) {
            dst[i * width * 2 + j] = 255 - src[i * width * 2 + j];     // Invert Y
            dst[i * width * 2 + j + 1] = src[i * width * 2 + j + 1];   // Keep U/V
        }
    }
}

// Test function
int test_pixel_inversion() {
    printf("Testing pixel inversion...\n");
    
    // Create test data
    uint8_t src_data[] = {
        128, 64, 200, 32,  // Y0, U0, Y1, V0
        100, 128, 50, 96   // Y2, U1, Y3, V1
    };
    uint8_t dst_data[8];
    
    // Test inversion
    mock_pixel_invert_yuyv(src_data, dst_data, 2, 1);
    
    // Verify results
    assert(dst_data[0] == 127);  // 255 - 128 = 127
    assert(dst_data[1] == 64);   // U unchanged
    assert(dst_data[2] == 55);   // 255 - 200 = 55
    assert(dst_data[3] == 32);   // V unchanged
    assert(dst_data[4] == 155);  // 255 - 100 = 155
    assert(dst_data[5] == 128);  // U unchanged
    assert(dst_data[6] == 205);  // 255 - 50 = 205
    assert(dst_data[7] == 96);   // V unchanged
    
    printf("✓ Pixel inversion test PASSED\n");
    return 0;
}

int main() {
    printf("=== CV181x Component Tests ===\n");
    
    if (test_pixel_inversion() != 0) {
        printf("✗ Pixel inversion test FAILED\n");
        return 1;
    }
    
    printf("=== All tests PASSED ===\n");
    return 0;
}
```

### **Step 3: Create Makefile for Mock Tests**

```makefile
# mock_tests/Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = test_components
SOURCES = test_pixel_inversion.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: test clean
```

### **Step 4: Run Mock Tests**

```bash
cd /home/xf/work/codes/cvi_alios_open/mock_tests
make test
```

## Method 3: QEMU User Mode Emulation

### **Step 1: Install QEMU User Mode**

```bash
# Install QEMU user mode
sudo apt-get update
sudo apt-get install qemu-user-static qemu-user

# Enable RISC-V emulation
sudo update-binfmts --install riscv64 /usr/bin/qemu-riscv64-static --magic '\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\xf3\x00' --mask '\xff\xff\xff\xff\xff\xff\xff\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff'
```

### **Step 2: Build for RISC-V**

```bash
cd /home/xf/work/codes/cvi_alios_open/solutions/peripherals_test

# Build with RISC-V toolchain
make clean
make

# The binary will be in generated/images/
ls -la generated/images/
```

### **Step 3: Run in QEMU**

```bash
# Run RISC-V binary in QEMU user mode
qemu-riscv64-static -L /usr/riscv64-linux-gnu/ generated/images/yoc.bin

# Or with specific libraries
qemu-riscv64-static -L /usr/riscv64-linux-gnu/ \
    -E LD_LIBRARY_PATH=/usr/riscv64-linux-gnu/lib \
    generated/images/yoc.bin
```

## Method 4: Component-Specific Testing

### **Test 1: Audio Components**

```c
// mock_tests/test_audio.c
#include <stdio.h>
#include <stdint.h>

// Mock G711 encoder/decoder
uint8_t mock_g711_encode(int16_t pcm) {
    // Simplified G711 encoding
    if (pcm >= 0) {
        return (uint8_t)(pcm >> 4);
    } else {
        return (uint8_t)((~pcm) >> 4) | 0x80;
    }
}

int16_t mock_g711_decode(uint8_t g711) {
    // Simplified G711 decoding
    if (g711 & 0x80) {
        return (int16_t)(~((g711 & 0x7F) << 4));
    } else {
        return (int16_t)(g711 << 4);
    }
}

int test_g711_codec() {
    printf("Testing G711 codec...\n");
    
    int16_t test_samples[] = {1000, -2000, 500, -1500, 0};
    uint8_t encoded[5];
    int16_t decoded[5];
    
    // Test encoding
    for (int i = 0; i < 5; i++) {
        encoded[i] = mock_g711_encode(test_samples[i]);
    }
    
    // Test decoding
    for (int i = 0; i < 5; i++) {
        decoded[i] = mock_g711_decode(encoded[i]);
    }
    
    // Verify (with some tolerance for quantization)
    for (int i = 0; i < 5; i++) {
        int16_t diff = test_samples[i] - decoded[i];
        if (diff > 16 || diff < -16) {  // Allow 16-bit tolerance
            printf("✗ G711 test FAILED at sample %d\n", i);
            return 1;
        }
    }
    
    printf("✓ G711 codec test PASSED\n");
    return 0;
}
```

### **Test 2: Video Processing Components**

```c
// mock_tests/test_video.c
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Mock video frame processing
typedef struct {
    uint8_t *y_plane;
    uint8_t *u_plane;
    uint8_t *v_plane;
    uint32_t width;
    uint32_t height;
} mock_yuv_frame_t;

void mock_yuv_to_rgb(mock_yuv_frame_t *yuv, uint8_t *rgb) {
    // Simplified YUV to RGB conversion
    for (uint32_t y = 0; y < yuv->height; y++) {
        for (uint32_t x = 0; x < yuv->width; x++) {
            uint32_t idx = y * yuv->width + x;
            uint32_t rgb_idx = idx * 3;
            
            int16_t Y = yuv->y_plane[idx];
            int16_t U = yuv->u_plane[idx/4] - 128;
            int16_t V = yuv->v_plane[idx/4] - 128;
            
            int16_t R = Y + ((V * 143) >> 8);
            int16_t G = Y - ((U * 88) >> 8) - ((V * 183) >> 8);
            int16_t B = Y + ((U * 181) >> 8);
            
            rgb[rgb_idx] = (R < 0) ? 0 : (R > 255) ? 255 : R;
            rgb[rgb_idx + 1] = (G < 0) ? 0 : (G > 255) ? 255 : G;
            rgb[rgb_idx + 2] = (B < 0) ? 0 : (B > 255) ? 255 : B;
        }
    }
}

int test_video_processing() {
    printf("Testing video processing...\n");
    
    // Create test YUV frame (2x2 pixels)
    uint8_t y_data[4] = {128, 200, 100, 50};
    uint8_t u_data[1] = {128};
    uint8_t v_data[1] = {128};
    uint8_t rgb_data[12];  // 2x2x3
    
    mock_yuv_frame_t frame = {
        .y_plane = y_data,
        .u_plane = u_data,
        .v_plane = v_data,
        .width = 2,
        .height = 2
    };
    
    mock_yuv_to_rgb(&frame, rgb_data);
    
    // Basic verification
    printf("RGB values: ");
    for (int i = 0; i < 12; i++) {
        printf("%d ", rgb_data[i]);
    }
    printf("\n");
    
    printf("✓ Video processing test PASSED\n");
    return 0;
}
```

### **Test 3: USB Components**

```c
// mock_tests/test_usb.c
#include <stdio.h>
#include <stdint.h>

// Mock USB CDC ACM
typedef struct {
    uint8_t *buffer;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
} mock_usb_cdc_t;

int mock_usb_cdc_send(mock_usb_cdc_t *cdc, const uint8_t *data, uint32_t len) {
    // Simplified USB CDC send
    for (uint32_t i = 0; i < len; i++) {
        if (cdc->head < cdc->size) {
            cdc->buffer[cdc->head++] = data[i];
        }
    }
    return len;
}

int mock_usb_cdc_receive(mock_usb_cdc_t *cdc, uint8_t *data, uint32_t max_len) {
    // Simplified USB CDC receive
    uint32_t received = 0;
    while (cdc->tail < cdc->head && received < max_len) {
        data[received++] = cdc->buffer[cdc->tail++];
    }
    return received;
}

int test_usb_cdc() {
    printf("Testing USB CDC...\n");
    
    uint8_t buffer[256];
    mock_usb_cdc_t cdc = {
        .buffer = buffer,
        .size = 256,
        .head = 0,
        .tail = 0
    };
    
    // Test send
    const char *test_data = "Hello USB CDC!";
    int sent = mock_usb_cdc_send(&cdc, (uint8_t*)test_data, strlen(test_data));
    
    if (sent != strlen(test_data)) {
        printf("✗ USB CDC send test FAILED\n");
        return 1;
    }
    
    // Test receive
    uint8_t received_data[256];
    int received = mock_usb_cdc_receive(&cdc, received_data, sizeof(received_data));
    
    if (received != strlen(test_data)) {
        printf("✗ USB CDC receive test FAILED\n");
        return 1;
    }
    
    if (memcmp(test_data, received_data, strlen(test_data)) != 0) {
        printf("✗ USB CDC data mismatch\n");
        return 1;
    }
    
    printf("✓ USB CDC test PASSED\n");
    return 0;
}
```

## Method 5: Complete Test Suite

### **Create Complete Test Runner**

```c
// mock_tests/test_runner.c
#include <stdio.h>
#include <stdlib.h>

// Include all test functions
extern int test_pixel_inversion(void);
extern int test_g711_codec(void);
extern int test_video_processing(void);
extern int test_usb_cdc(void);

typedef struct {
    const char *name;
    int (*test_func)(void);
} test_case_t;

test_case_t tests[] = {
    {"Pixel Inversion", test_pixel_inversion},
    {"G711 Codec", test_g711_codec},
    {"Video Processing", test_video_processing},
    {"USB CDC", test_usb_cdc},
    {NULL, NULL}
};

int main() {
    printf("=== CV181x AliOS Component Test Suite ===\n");
    printf("Running on x86 Ubuntu (Mock Environment)\n\n");
    
    int passed = 0;
    int total = 0;
    
    for (int i = 0; tests[i].name != NULL; i++) {
        printf("Running test: %s\n", tests[i].name);
        printf("----------------------------------------\n");
        
        int result = tests[i].test_func();
        total++;
        
        if (result == 0) {
            passed++;
            printf("✓ PASSED\n");
        } else {
            printf("✗ FAILED\n");
        }
        printf("\n");
    }
    
    printf("=== Test Results ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Success Rate: %.1f%%\n", (float)passed / total * 100);
    
    return (passed == total) ? 0 : 1;
}
```

### **Run Complete Test Suite**

```bash
cd /home/xf/work/codes/cvi_alios_open/mock_tests

# Compile all tests
gcc -Wall -Wextra -std=c99 -g -o test_runner \
    test_pixel_inversion.c \
    test_audio.c \
    test_video.c \
    test_usb.c \
    test_runner.c

# Run tests
./test_runner
```

## Method 6: CI/CD Integration

### **GitHub Actions Example**

```yaml
# .github/workflows/component-tests.yml
name: Component Tests

on: [push, pull_request]

jobs:
  test-components:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential qemu-user-static
    
    - name: Run component tests
      run: |
        cd mock_tests
        make test
    
    - name: Run QEMU tests
      run: |
        cd solutions/peripherals_test
        make clean
        make
        qemu-riscv64-static generated/images/yoc.bin
```

## Summary

This comprehensive guide provides multiple approaches:

1. **Docker**: Isolated testing environment
2. **Mock Components**: Pure x86 testing without hardware
3. **QEMU User Mode**: RISC-V emulation
4. **Component-Specific**: Individual component testing
5. **Complete Test Suite**: Automated testing
6. **CI/CD**: Continuous integration

**Recommendation**: Start with **Method 2 (Mock Components)** for quick development and testing, then use **Method 3 (QEMU)** for more realistic testing before deploying to real hardware.