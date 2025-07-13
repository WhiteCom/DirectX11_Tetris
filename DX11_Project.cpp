// DX11_Project.cpp : Defines the entry point for the application.
//

#include "DX11_Project.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")


#define MAX_LOADSTRING 100

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    //UNREFERENCED_PARAMETER(hPrevInstance);
    //UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    srand(time(NULL)); // Initialize Random Seed

    if (FAILED(InitWindow(hInstance, nCmdShow)))
        return 0;
    
    if (FAILED(InitDevice()))
    {
        CleanupDevice();
        return 0;
    }

    StartGame();

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Update();
            Render();
        }
    }

    CleanupDevice();
    return (int) msg.wParam;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
        if (true == g_gameOver)
        {
            if (wParam == 'R')
                StartGame();
        }
        else
        {
            switch (wParam)
            {
            case VK_LEFT:
                if (false == CheckCollision(g_currentPiece.x - 1, g_currentPiece.y, g_currentPiece.shape))
                    g_currentPiece.x--;
                break;

            case VK_RIGHT:
                if (false == CheckCollision(g_currentPiece.x + 1, g_currentPiece.y, g_currentPiece.shape))
                    g_currentPiece.x++;
                break;

            case VK_DOWN:
                if (false == CheckCollision(g_currentPiece.x, g_currentPiece.y + 1, g_currentPiece.shape))
                    g_currentPiece.y++;
                break;

            case VK_UP:
                RotatePiece();
                break;
            }
        }
        break;

    case WM_CLOSE:
        {
            int result = MessageBox(NULL ,L"Really Quit?", L"Alarm", MB_YESNOCANCEL);
            if (result == IDYES)
            {
                PostQuitMessage(0);
            }
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//
// Background Function
//
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASSEX wcex =
    {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW,
        WndProc,
        0, 
        0,
        hInstance,
        nullptr,
        LoadCursor(nullptr, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        nullptr,
        TEXT("TetrisWindowClass"),
        nullptr
    };

    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    g_hInstance = hInstance;
    RECT rc = { 0, 0, g_windowWidth, g_windowHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindow(
        TEXT("TetrisWindowClass"),
        TEXT("DirectX 11 - Tetris Project"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);
    return S_OK;
}

// Initialize DirectX Device
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    // Set Swap Chain
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    //
    // Create Device And Swap Chain
    //
    hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        nullptr,
        &g_pImmediateContext
    );

    if (FAILED(hr))
        return hr;

    //
    // Set Render Target View
    //
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

    //
    // Set Viewport
    //
    D3D11_VIEWPORT vp = { 0.0f, 0.0f, (FLOAT)width, (FLOAT)height, 0.0f, 1.0f };
    g_pImmediateContext->RSSetViewports(1, &vp);

    //
    // Set Rasterizer State ( Fill Mode )
    //
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;
    hr = g_pd3dDevice->CreateRasterizerState(&rasterDesc, &g_pRasterizerStateSolid);
    if (FAILED(hr))
        return hr;

    //
    // Set Rasterizer State ( WireFrame Mode )
    //
    //rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    hr = g_pd3dDevice->CreateRasterizerState(&rasterDesc, &g_pRasterizerStateWireframe);
    if (FAILED(hr))
        return hr;

    //
    // Shader Code (HLSL)
    //
    const char* shader_code =
        "cbuffer ConstantBuffer : register( b0 ) \n"
        "{ \n"
        "   matrix WorldViewProj; \n"
        "   float4 Color; \n"
        "} \n"
        "struct VS_OUTPUT \n"
        "{ \n"
        "   float4 Pos : SV_POSITION; \n"
        "}; \n"
        "VS_OUTPUT VS( float4 Pos : POSITION ) \n"
        "{ \n"
        "   VS_OUTPUT output = (VS_OUTPUT)0; \n"
        "   output.Pos = mul( Pos, WorldViewProj ); \n"
        "   return output; \n"
        "} \n"
        "float4 PS( VS_OUTPUT input ) : SV_Target \n"
        "{ \n"
        "   return Color; \n"
        "} \n";

    //
    // Create Vertex Shader And Compile
    //
    ID3DBlob* pVSBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompile(shader_code, strlen(shader_code), NULL, NULL, NULL, "VS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Vertex Shader Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        return hr;
    }

    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr))
    {
        if(pVSBlob)
            pVSBlob->Release();
        return hr;
    }

    //
    // Create Input Layout
    //
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = g_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pInputLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    g_pImmediateContext->IASetInputLayout(g_pInputLayout);

    //
    // Create Pixel Shader And Compile
    //
    ID3DBlob* pPSBlob = nullptr;
    hr = D3DCompile(shader_code, strlen(shader_code), NULL, NULL, NULL, "PS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            MessageBoxA(nullptr, (char*)pErrorBlob->GetBufferPointer(), "Pixel Shader Compile Error", MB_OK);
            pErrorBlob->Release();
        }
        else
            MessageBoxA(nullptr, "Failed to compile pixel shader.", "Error", MB_OK | MB_ICONERROR);

        return hr;
    }

    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    //
    // Create Vertex Buffer ( Square Block )
    //
    SimpleVertex vertices[] = {
        { DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f) }, { DirectX::XMFLOAT4(1.0f, 0.0f, 0.5f, 1.0f) }, { DirectX::XMFLOAT4(0.0f, 1.0f, 0.5f, 1.0f) },
        { DirectX::XMFLOAT4(0.0f, 1.0f, 0.5f, 1.0f) }, { DirectX::XMFLOAT4(1.0f, 0.0f, 0.5f, 1.0f) }, { DirectX::XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f) }
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA InitData = { vertices, 0, 0 };
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    //
    // Create Constant Buffer
    //
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
    if (FAILED(hr))
        return hr;

    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return S_OK;
}

void CleanupDevice()
{
    if (g_pImmediateContext)
        g_pImmediateContext->ClearState();

    if (g_pRasterizerStateSolid)
        g_pRasterizerStateSolid->Release();

    if (g_pRasterizerStateWireframe)
        g_pRasterizerStateWireframe->Release();

    if (g_pConstantBuffer)
        g_pConstantBuffer->Release();

    if (g_pVertexBuffer)
        g_pVertexBuffer->Release();

    if (g_pInputLayout)
        g_pInputLayout->Release();

    if (g_pVertexShader)
        g_pVertexShader->Release();

    if (g_pPixelShader)
        g_pPixelShader->Release();

    if (g_pRenderTargetView)
        g_pRenderTargetView->Release();

    if (g_pSwapChain)
        g_pSwapChain->Release();

    if (g_pImmediateContext)
        g_pImmediateContext->Release();

    if (g_pd3dDevice)
        g_pd3dDevice->Release();
}

void Render()
{
    //
    // Set Background Color & Clear Scene
    //
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

    // Set Shader
    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

    // Set Projection Matrix (for 2D Rendering)
    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, g_windowWidth, g_windowHeight, 0.0f, 0.1f, 100.0f);

    ConstantBuffer cb;

    // Draw Board Edge
    g_pImmediateContext->RSSetState(g_pRasterizerStateWireframe); // Change Wire-frame mode
    DirectX::XMMATRIX borderWorld = DirectX::XMMatrixScaling(BOARD_WIDTH * BLOCK_SIZE, BOARD_HEIGHT * BLOCK_SIZE, 1.0f);
    cb.wvp = DirectX::XMMatrixTranspose(borderWorld * projection);
    cb.color = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); // Gray Color
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    g_pImmediateContext->Draw(6, 0); // Draw Rectangle Edge

    // Restore For Draw Block
    g_pImmediateContext->RSSetState(g_pRasterizerStateSolid); // 채우기 모드로 복구

    // Draw Block Stacks
    for (int y = 0; y < BOARD_HEIGHT; ++y)
    {
        for (int x = 0; x < BOARD_WIDTH; ++x)
        {
            if (g_board[y][x] != 0) // 빈 공간 아닌경우
            {
                DirectX::XMMATRIX world = DirectX::XMMatrixScaling(BLOCK_SIZE, BLOCK_SIZE, 1.0f) * DirectX::XMMatrixTranslation(x * BLOCK_SIZE, y * BLOCK_SIZE, 0);
                cb.wvp = DirectX::XMMatrixTranspose(world * projection);
                cb.color = COLORS[g_board[y][x]];
                g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
                g_pImmediateContext->Draw(6, 0);
            }
        }
    }

    // Draw Moving Block
    if (false == g_gameOver)
    {
        cb.color = COLORS[g_currentPiece.type + 1];
        for (int y = 0; y < 4; ++y)
        {
            for (int x = 0; x < 4; ++x)
            {
                if (g_currentPiece.shape[y][x] != 0)
                {
                    DirectX::XMMATRIX world = DirectX::XMMatrixScaling(BLOCK_SIZE, BLOCK_SIZE, 1.0f) * DirectX::XMMatrixTranslation((g_currentPiece.x + x) * BLOCK_SIZE, (g_currentPiece.y + y) * BLOCK_SIZE, 0);
                    cb.wvp = DirectX::XMMatrixTranspose(world * projection);
                    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
                    g_pImmediateContext->Draw(6, 0);
                }
            }
        }
    }

    // Present Result Scene
    g_pSwapChain->Present(1, 0);
}

//
// Game Function
// 
void StartGame()
{
    // Init Board
    for (int y = 0; y < BOARD_HEIGHT; ++y)
    {
        for (int x = 0; x < BOARD_WIDTH; ++x)
            g_board[y][x] = 0;
    }

    g_gameOver = false;
    SetWindowText(g_hWnd, L"DirectX 11 Tetris");
    NewPiece();
}

void NewPiece()
{
    g_currentPiece.x = BOARD_WIDTH / 2 - 2;
    g_currentPiece.y = 0;
    g_currentPiece.type = (rand() % MAX_TYPES_TETROMINOS);
    memcpy(g_currentPiece.shape, TETROMINOS[g_currentPiece.type], sizeof(int) * 16);

    // Check Collision - Game Over Condition
    if (true == CheckCollision(g_currentPiece.x, g_currentPiece.y, g_currentPiece.shape))
    {
        g_gameOver = true;
        SetWindowText(g_hWnd, L"게임 오버! 'R' 을 눌러 다시 시작");
    }
}

bool CheckCollision(int pieceX, int pieceY, int shape[4][4])
{
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            if (shape[y][x] != 0)
            {
                int boardX = pieceX + x;
                int boardY = pieceY + y;

                // 벽 충돌 검사
                if (boardX < 0 || boardX >= BOARD_WIDTH || boardY >= BOARD_HEIGHT)
                {
                    return true;
                }

                // 다른 블록과 충돌검사 (y < 0 이면 충돌검사 안함)
                if (boardY >= 0 && g_board[boardY][boardX] != 0)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void MergePiece()
{
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            if (g_currentPiece.shape[y][x] != 0)
            {
                // 보드에 저장할 때는 타입에 1을 더해 색상 인덱스와 맞춤 (1~7)
                g_board[g_currentPiece.y + y][g_currentPiece.x + x] = g_currentPiece.type + 1;
            }
        }
    }
}

void RotatePiece()
{
    int tempShape[4][4] = { {0,}, };
    memcpy(tempShape, g_currentPiece.shape, sizeof(int) * 16);

    // 90도 시계방향 회전
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            g_currentPiece.shape[y][x] = tempShape[3 - x][y];
        }
    }

    // 회전 후 충돌하면 원상복구
    if (true == CheckCollision(g_currentPiece.x, g_currentPiece.y, g_currentPiece.shape))
    {
        memcpy(g_currentPiece.shape, tempShape, sizeof(int) * 16);
    }
}

void ClearLines()
{
    for (int y = BOARD_HEIGHT - 1; y >= 0; --y)
    {
        bool lineFull = true;
        for (int x = 0; x < BOARD_WIDTH; ++x)
        {
            if (g_board[y][x] == 0)
            {
                lineFull = false;
                break;
            }
        }

        if (true == lineFull)
        {
            for (int k = y; k > 0; --k)
            {
                for (int x = 0; x < BOARD_WIDTH; ++x)
                {
                    g_board[k][x] = g_board[k - 1][x];
                }
            }

            // 가장 윗줄은 비움
            for (int x = 0; x < BOARD_WIDTH; ++x)
            {
                g_board[0][x] = 0;
            }
            ++y; // 한 줄 내렸으니, 같은줄을 다시 검사.
        }
    }

}

void Update()
{
    // 게임 오버 상태에서는 업데이트 중지
    if (g_gameOver)
        return;

    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = currentTime - g_lastFallTime;

    if (elapsed.count() > g_fallInterval)
    {
        if (false == CheckCollision(g_currentPiece.x, g_currentPiece.y + 1, g_currentPiece.shape))
        {
            g_currentPiece.y++;
        }
        else
        {
            MergePiece();
            ClearLines();
            NewPiece();
        }
        g_lastFallTime = currentTime;
    }
    
}

