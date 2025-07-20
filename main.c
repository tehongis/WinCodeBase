#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdio.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

// Globals
IDXGISwapChain* swapChain;
ID3D11Device* device;
ID3D11DeviceContext* deviceContext;
ID3D11RenderTargetView* renderTargetView;
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitD3D(HWND hwnd);
void LoadShaders();
void CleanD3D();
void Update();
void Render();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    const char CLASS_NAME[] = "GameWindowClass";

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    hwnd = CreateWindowEx(0, CLASS_NAME, "Game Engine DX11", WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                          NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    InitD3D(hwnd);
    LoadShaders();

    ZeroMemory(&msg, sizeof(MSG));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            Update();
            Render();
        }
    }

    CleanD3D();
    return (int) msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitD3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC scd;
    ID3D11Texture2D* backBuffer;
    D3D11_VIEWPORT viewport;

    ZeroMemory(&scd, sizeof(scd));
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = 800;
    scd.BufferDesc.Height = 600;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
                                  NULL, 0, D3D11_SDK_VERSION,
                                  &scd, &swapChain, &device, NULL, &deviceContext);

    swapChain->lpVtbl->GetBuffer(swapChain, 0, &IID_ID3D11Texture2D, (void**)&backBuffer);
    device->lpVtbl->CreateRenderTargetView(device, (ID3D11Resource*)backBuffer, NULL, &renderTargetView);
    backBuffer->lpVtbl->Release(backBuffer);

    deviceContext->lpVtbl->OMSetRenderTargets(deviceContext, 1, &renderTargetView, NULL);

    ZeroMemory(&viewport, sizeof(viewport));
    viewport.Width = 800;
    viewport.Height = 600;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->lpVtbl->RSSetViewports(deviceContext, 1, &viewport);
}

void LoadShaders() {
    ID3DBlob* vsBlob = NULL;
    ID3DBlob* psBlob = NULL;
    ID3DBlob* errorBlob = NULL;
    HRESULT hr;

    hr = D3DCompileFromFile("shader.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            printf("Vertex Shader Error: %s\n", (char*)errorBlob->lpVtbl->GetBufferPointer(errorBlob));
            errorBlob->lpVtbl->Release(errorBlob);
        }
        return;
    }

    hr = D3DCompileFromFile("shader.hlsl", NULL, NULL, "ps_main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            printf("Pixel Shader Error: %s\n", (char*)errorBlob->lpVtbl->GetBufferPointer(errorBlob));
            errorBlob->lpVtbl->Release(errorBlob);
        }
        return;
    }

    device->lpVtbl->CreateVertexShader(device, vsBlob->lpVtbl->GetBufferPointer(vsBlob),
                                       vsBlob->lpVtbl->GetBufferSize(vsBlob), NULL, &vertexShader);
    device->lpVtbl->CreatePixelShader(device, psBlob->lpVtbl->GetBufferPointer(psBlob),
                                      psBlob->lpVtbl->GetBufferSize(psBlob), NULL, &pixelShader);

    deviceContext->lpVtbl->VSSetShader(deviceContext, vertexShader, NULL, 0);
    deviceContext->lpVtbl->PSSetShader(deviceContext, pixelShader, NULL, 0);

    vsBlob->lpVtbl->Release(vsBlob);
    psBlob->lpVtbl->Release(psBlob);
}

void Update() {
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        printf("Up pressed\n");
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        printf("Down pressed\n");
    }
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        PostQuitMessage(0);
    }
}

void Render() {
    float clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f };
    deviceContext->lpVtbl->ClearRenderTargetView(deviceContext, renderTargetView, clearColor);
    swapChain->lpVtbl->Present(swapChain, 1, 0);
}

void CleanD3D() {
    if (vertexShader) vertexShader->lpVtbl->Release(vertexShader);
    if (pixelShader) pixelShader->lpVtbl->Release(pixelShader);
    if (renderTargetView) renderTargetView->lpVtbl->Release(renderTargetView);
    if (swapChain) swapChain->lpVtbl->Release(swapChain);
    if (deviceContext) deviceContext->lpVtbl->Release(deviceContext);
    if (device) device->lpVtbl->Release(device);
}
