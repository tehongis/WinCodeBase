#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

#define TIMER_ID 1
#define TIMER_INTERVAL 100

#define PIXEL_WIDTH 320
#define PIXEL_HEIGHT 256
#define PIXEL_SCALE 3

// Add this global buffer for the bitmap (24-bit RGB)
uint8_t framebuffer[PIXEL_WIDTH * PIXEL_HEIGHT * 3];

int RunGameLogic() {
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        
        case WM_CREATE: {
            // Start a timer when the window is created
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            return 0;            
        }
        
        case WM_TIMER: {
            if (wParam == TIMER_ID) {
                RunGameLogic();
                InvalidateRect(hwnd, NULL, FALSE); // Request repaint
            }
            return 0;
        }
        
        case WM_PAINT: {
            
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);

            HBITMAP hBitmap = CreateCompatibleBitmap(hdc, PIXEL_WIDTH, PIXEL_HEIGHT);
            SelectObject(memDC, hBitmap);

            // Get the client rectangle
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            // Fill the background with a solid color (e.g., white)
            HBRUSH hBrush = CreateSolidBrush(RGB(155, 155, 155)); // Gray
            FillRect(hdc, &clientRect, hBrush);
            DeleteObject(hBrush);

            // Set text color and background mode
            SetTextColor(hdc, RGB(0, 0, 0));
            SetBkMode(hdc, TRANSPARENT);
            
            // printf("PC: 0x%04X\n", cpu.PC);
            LPTSTR buffer[100];
            LPTSTR* label = TEXT("PC: ");
            // Format the string with a hexadecimal number

            int tempnumber = 200;

            wsprintf(buffer, TEXT("%s 0x%04X"), label, tempnumber);
            TextOut(hdc, 8, 8, buffer, lstrlen(buffer));

            // Fill framebuffer
            for (int index = 0; index < PIXEL_WIDTH * PIXEL_HEIGHT; index++) {
                uint8_t val = rand() % 0xff;
                
                // Extract raw values
                uint8_t red_raw   =  val       & 0b00000111;        // bits 0–2
                uint8_t green_raw = (val >> 3) & 0b00000111;        // bits 3–5
                uint8_t blue_raw  = (val >> 6) & 0b00000011;        // bits 6–7
                
                // Scale to 0–255 range (optional)
                uint8_t red   = (red_raw   * 255) / 7;
                uint8_t green = (green_raw * 255) / 7;
                uint8_t blue  = (blue_raw  * 255) / 3;            
                
                int idx = index * 3;
                framebuffer[idx + 0] = blue;
                framebuffer[idx + 1] = green;
                framebuffer[idx + 2] = red;
            }
            
            // Prepare BITMAPINFO
            BITMAPINFO bmi = {0};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = PIXEL_WIDTH;
            bmi.bmiHeader.biHeight = -PIXEL_HEIGHT; // negative for top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 24;
            bmi.bmiHeader.biCompression = BI_RGB;
            
            // Blit to memDC
            SetDIBitsToDevice(
                memDC,
                0, 0, PIXEL_WIDTH, PIXEL_HEIGHT,
                0, 0, 0, PIXEL_HEIGHT,
                framebuffer,
                &bmi,
                DIB_RGB_COLORS
            );

            // Blit to screen scaled
            StretchBlt(hdc, 0, 0, PIXEL_WIDTH*PIXEL_SCALE, PIXEL_HEIGHT*PIXEL_SCALE, memDC, 0, 0, PIXEL_WIDTH, PIXEL_HEIGHT, SRCCOPY);

            EndPaint(hwnd, &ps);
            return 0;            
        }
        
        case WM_DESTROY: {
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
    
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    // Initialize random seed using the current time
    srand((unsigned) time(NULL));

    const char CLASS_NAME[] = "SampleWindowClass";
    
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Pixel Drawing Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, PIXEL_WIDTH * PIXEL_SCALE, PIXEL_HEIGHT * PIXEL_SCALE,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) return 0;
    
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
