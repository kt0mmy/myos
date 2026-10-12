#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <deque>
#include "queue.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.hpp"
#include "pci.hpp"
#include "logger.hpp"
#include "interrupt.hpp"
#include "mouse.hpp"
#include "memory_map.hpp"
#include "memory_manager.hpp"
#include "layer.hpp"
#include "paging.hpp"
#include "asmfunc.h"
#include "segment.hpp"
#include "timer.hpp"
#include "shadow_buffer.hpp"
#include "panel.hpp"
#include "usb/memory.hpp"
#include "usb/device.hpp"
#include "usb/classdriver/mouse.hpp"
#include "usb/xhci/xhci.hpp"
#include "usb/xhci/trb.hpp"

// .bss or .dataセクション
// プログラムと合わせてkEfiLoaderDataにロードされる
alignas(16) uint8_t kernel_main_stack[1024 * 1024]; // 1MB

/**
 * NOTE:
 * echo '#include <array>' > test.cpp
 * clang++ -H -I/home/t0mmy/osbook/devenv/x86_64-elf/include/c++/v1 --target=x86_64-elf -std=c++17 -c test.cpp 2>&1
 *
 * array→iterator→__functional_base→typeinfo/exception→new という依存関係がある。
 * したがって配置newは不要
 *
 */
void operator delete(void *obj) noexcept {}

/**
 * @retval 0   成功
 * @retval 非0 失敗
 */
int WritePixel(const FrameBuferConfig &config, int x, int y, const PixelColor &c)
{
    const int pixel_position = config.pixels_per_scan_line * y + x;
    uint8_t *p = &config.frame_buffer[4 * pixel_position];

    if (config.pixel_format == kPixelBGRResv8BitPerColor)
    {
        p[0] = c.b;
        p[1] = c.g;
        p[2] = c.r;
    }
    else if (config.pixel_format == kPixelRGBResv8BitPerColor)
    {
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    }
    else
    {
        return -1;
    }

    return 0;
}

// class のサイズ？
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter *pixel_writer;

int printk(const char *format, ...)
{
    va_list ap;
    int result;
    char s[1024];

    va_start(ap, format);
    result = vsprintf(s, format, ap);
    va_end(ap);

    console->PutString(s);
    return result;
}

void SwitchEhci2Xhci(const pci::Device &xhc_dev)
{
    bool intel_ehc_exist = false;
    for (int i = 0; i < pci::num_device; i++)
    {
        intel_ehc_exist = pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x20u) && pci::ReadVendorId(pci::devices[i]) == 0x8086;
        if (intel_ehc_exist)
            break;
    }

    if (!intel_ehc_exist)
        return;

    uint32_t superspeed_ports = pci::ReadConfReg(xhc_dev, 0xdc);
    pci::WriteConfReg(xhc_dev, 0xd8, superspeed_ports);

    uint32_t ehci2xhci_ports = pci::ReadConfReg(xhc_dev, 0xd4);
    pci::WriteConfReg(xhc_dev, 0xd0, ehci2xhci_ports);
    Log(kDebug, "SwitchEhci2Xhci: SS = %02, xHCI = %02x\n", superspeed_ports, ehci2xhci_ports);
}

std::deque<Message> *main_queue;

unsigned int mouse_layer_id;
Vector2D<int> screen_size;
Vector2D<int> mouse_position;
void MouseObserver(uint8_t buttons, int8_t displacement_x, int8_t displacement_y)
{
    /*
    ボタンが押されているかどうかを、前の状態として保持する
    */
    static unsigned int mouse_drag_layer_id = 0;
    static uint8_t previous_buttons = 0;

    const auto old_pos = mouse_position;
    auto new_pos = old_pos + Vector2D<int>{displacement_x, displacement_y};
    new_pos = ElementMin(new_pos, screen_size + Vector2D<int>{-1, -1});
    mouse_position = ElementMax(new_pos, {0, 0});

    const auto pos_diff = mouse_position - old_pos;
    layer_manager->Move(mouse_layer_id, mouse_position);

    const bool previous_left_pressed = (previous_buttons & 0x01);
    const bool left_pressed = (buttons & 0x01);

    // 初めて押されたとき
    if (!previous_left_pressed && left_pressed)
    {
        auto layer = layer_manager->FindLayerByPosition(mouse_position, mouse_layer_id);
        if (layer && layer->IsDraggable())
        {
            mouse_drag_layer_id = layer->ID();
        }
    }
    else if (previous_left_pressed && left_pressed)
    {
        if (mouse_drag_layer_id > 0)
        {
            layer_manager->MoveRelative(mouse_drag_layer_id, pos_diff);
        }
    }
    else if (previous_left_pressed && !left_pressed)
    {
        mouse_drag_layer_id = 0;
    }

    previous_buttons = buttons;
}


std::shared_ptr<Window> main_window;
unsigned int main_window_layer_id;

usb::xhci::Controller *xhc;

// NOTE: マングリングを防ぐ
extern "C" void KernelMainNewStack(const FrameBuferConfig &frame_buffer_config_ref, const MemoryMap &memory_map_ref)
{
    // 引数で渡されたデータをスタック領域に保持する
    FrameBuferConfig frame_buffer_config{frame_buffer_config_ref};
    MemoryMap memory_map{memory_map_ref};

    InitializeGraphics(frame_buffer_config_ref);
    InitializeConsole();

    InitializeSegmentation();
    InitializePaging();
    InitializeMemoryManager(memory_map);

    InitializeLAPICTimer();

    const int kFrameWidth = frame_buffer_config.horizontal_resolution;
    const int kFrameHeight = frame_buffer_config.vertical_resolution;

    const std::array available_memory_types{
        MemoryType::kEfiBootServicesCode,
        MemoryType::kEfiBootServicesData,
        MemoryType::kEfiConventionalMemory,
    };

    ::main_queue = new std::deque<Message>(32);
    InitializeInterrupt(main_queue);

    InitializePCI();
    usb::xhci::Initialize();

    InitializeLayer();


    usb::HIDMouseDriver::default_observer = MouseObserver;

    char str[128];
    unsigned int count = 0;

    while (true)
    {
        ++count;
        sprintf(str, "%010u", count);
        FillRectangle(*main_window->Writer(), {24, 28}, {8 * 10, 16}, {0xc6, 0xc6, 0xc6});
        WriteString(*main_window->Writer(), {24, 28}, str, {0, 0, 0});
        layer_manager->Draw(main_window_layer_id);

        __asm__("cli");
        if (main_queue->size() == 0)
        {
            __asm__("sti");
            continue;
        }

        Message msg = main_queue->front();
        main_queue->pop_front();
        __asm__("sti");

        switch (msg.type)
        {
        case Message::kInterruptXHCI:
            usb::xhci::ProcessEvents();
            break;
        default:
            Log(kError, "Unknown message type: %d\n", msg.type);
            break;
        }
    }

    // NOTE: この間にマウスを動かしておくと、sti命令のあとに移動
    // 割り込み禁止中、割り込みが保留されている
    // __asm__("cli");
    // for (int i = 0; i < 500; i++)
    // {
    //     Log(kError, "xHC starting: %s\n", i);
    // }
    // __asm__("sti");

    printk("Hello, MyOS!");
    while (1)
        __asm__("hlt");
}

extern "C" void __cxa_pure_virtual()
{
    while (1)
        __asm__("hlt");
}
