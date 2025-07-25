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
ID3D11Buffer* quadVertexBuffer = NULL;
ID3D11InputLayout* inputLayout = NULL;

ID3D11Texture2D* bgTexture = NULL;
ID3D11ShaderResourceView* bgSRV = NULL;
ID3D11SamplerState* samplerState = NULL;

// Add this struct at the top:
typedef struct {
    float position[3];
    float texcoord[2];
} Vertex;

// Example: create a simple checkerboard pattern
unsigned char checkerboard[800 * 600 * 4];
void GenerateCheckerboard() {
    for (int y = 0; y < 600; ++y) {
        for (int x = 0; x < 800; ++x) {
            int i = (y * 800 + x) * 4;
            int c = ((x / 32) % 2) ^ ((y / 32) % 2);
            checkerboard[i + 0] = c ? 255 : 0;   // R
            checkerboard[i + 1] = c ? 255 : 0;   // G
            checkerboard[i + 2] = c ? 255 : 0;   // B
            checkerboard[i + 3] = 255;           // A
        }
    }
}

// Create a texture from a byte array
void CreateBackgroundTexture(unsigned char* pixels, int width, int height) {
    if (bgTexture) { bgTexture->lpVtbl->Release(bgTexture); bgTexture = NULL; }
    if (bgSRV) { bgSRV->lpVtbl->Release(bgSRV); bgSRV = NULL; }

    D3D11_TEXTURE2D_DESC desc = {0};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {0};
    initData.pSysMem = pixels;
    initData.SysMemPitch = width * 4;

    device->lpVtbl->CreateTexture2D(device, &desc, &initData, &bgTexture);
    device->lpVtbl->CreateShaderResourceView(device, (ID3D11Resource*)bgTexture, NULL, &bgSRV);
}

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitD3D(HWND hwnd);
void LoadShaders();
void CreateFullscreenQuad();
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

    // Add after viewport setup:
    D3D11_SAMPLER_DESC sampDesc = {0};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device->lpVtbl->CreateSamplerState(device, &sampDesc, &samplerState);

    // Example: generate and upload checkerboard background
    GenerateCheckerboard();
    CreateBackgroundTexture(checkerboard, 800, 600);
}

void LoadShaders() {
    ID3DBlob* vsBlob = NULL;
    ID3DBlob* psBlob = NULL;
    ID3DBlob* errorBlob = NULL;
    HRESULT hr;

    hr = D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            printf("Vertex Shader Error: %s\n", (char*)errorBlob->lpVtbl->GetBufferPointer(errorBlob));
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

    CreateFullscreenQuad();
}

void CreateFullscreenQuad() {
    Vertex vertices[] = {
        { {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f} },
        { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
        { { 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
    };

    D3D11_BUFFER_DESC bd = {0};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {0};
    initData.pSysMem = vertices;

    device->lpVtbl->CreateBuffer(device, &bd, &initData, &quadVertexBuffer);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

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

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    deviceContext->lpVtbl->IASetInputLayout(deviceContext, inputLayout);
    deviceContext->lpVtbl->IASetVertexBuffers(deviceContext, 0, 1, &quadVertexBuffer, &stride, &offset);
    deviceContext->lpVtbl->IASetPrimitiveTopology(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // Bind background texture and sampler
    deviceContext->lpVtbl->PSSetShaderResources(deviceContext, 0, 1, &bgSRV);
    deviceContext->lpVtbl->PSSetSamplers(deviceContext, 0, 1, &samplerState);

    deviceContext->lpVtbl->Draw(deviceContext, 4, 0);

    swapChain->lpVtbl->Present(swapChain, 1, 0);
}

void CleanD3D() {
    if (vertexShader) vertexShader->lpVtbl->Release(vertexShader);
    if (pixelShader) pixelShader->lpVtbl->Release(pixelShader);
    if (renderTargetView) renderTargetView->lpVtbl->Release(renderTargetView);
    if (swapChain) swapChain->lpVtbl->Release(swapChain);
    if (deviceContext) deviceContext->lpVtbl->Release(deviceContext);
    if (device) device->lpVtbl->Release(device);
    if (quadVertexBuffer) quadVertexBuffer->lpVtbl->Release(quadVertexBuffer);
    if (inputLayout) inputLayout->lpVtbl->Release(inputLayout);
    if (bgTexture) bgTexture->lpVtbl->Release(bgTexture);
    if (bgSRV) bgSRV->lpVtbl->Release(bgSRV);
    if (samplerState) samplerState->lpVtbl->Release(samplerState);
}
