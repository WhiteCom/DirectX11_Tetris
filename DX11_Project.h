#pragma once

#include "resource.h"

#include "CommonHeader.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

//
// Global Variable
//

#define MAX_TYPES_TETROMINOS 7

//
// Win32 Variable
//
HINSTANCE					g_hInstance = nullptr;
HWND						g_hWnd = nullptr;
const int					g_windowWidth = 400; // ( Game Area + Info Area )
const int					g_windowHeight = 480;

//
// DirectX Variable
//
ID3D11Device*				g_pd3dDevice = nullptr;
ID3D11DeviceContext*		g_pImmediateContext = nullptr;
IDXGISwapChain*				g_pSwapChain = nullptr;
ID3D11RenderTargetView*		g_pRenderTargetView = nullptr;
ID3D11VertexShader*			g_pVertexShader = nullptr;
ID3D11PixelShader*			g_pPixelShader = nullptr;
ID3D11InputLayout*			g_pInputLayout = nullptr;
ID3D11Buffer*				g_pVertexBuffer = nullptr;
ID3D11Buffer*				g_pConstantBuffer = nullptr;
ID3D11RasterizerState*		g_pRasterizerStateSolid = nullptr; // 채워진 사각형용
ID3D11RasterizerState*		g_pRasterizerStateWireframe = nullptr; // 테두리용

// Struct Vertex
struct SimpleVertex
{
	DirectX::XMFLOAT4 Pos;
};

// Constant BUffer Sturcture ( for transfer Shader )
struct ConstantBuffer
{
	DirectX::XMMATRIX wvp;
	DirectX::XMFLOAT4 color;
};

// for Game Logic
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const float BLOCK_SIZE = 20.0f;

int g_board[BOARD_HEIGHT][BOARD_WIDTH] = { 0, }; // Game board ( 0 : empty, 1 ~ 8 : block type )

// Define Tetromino Shapes ( not zero => Block )
const int TETROMINOS[MAX_TYPES_TETROMINOS][4][4] =
{
	{ {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0} }, // Shape : I
	{ {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0} }, // Shape : O
	{ {0, 0, 0, 0}, {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0} }, // Shape : T
	{ {0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0} }, // Shape : S
	{ {0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0} }, // Shape : Z
	{ {0, 0, 0, 0}, {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0} }, // Shape : L
	{ {0, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0} }, // Shape : J
};

// Define Tetromino Color ( 0 : non color, 1 ~ 7 : Block Color )
const DirectX::XMFLOAT4 COLORS[] =
{
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), // 0 : empty (not use)
	DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), // 1 : I ( Cyan )
	DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), // 2 : O ( Yellow )
	DirectX::XMFLOAT4(0.5f, 0.0f, 0.5f, 1.0f), // 3 : T ( Purple )
	DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), // 4 : S ( Green )
	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), // 5 : Z ( Red )
	DirectX::XMFLOAT4(1.0f, 0.64f, 0.0f, 1.0f), // 6 : L ( Orange )
	DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), // 7 : J ( Blue )
};

// Player Block information
struct PlayerPiece
{
	int x = BOARD_WIDTH / 2 - 2;
	int y = 0;
	int shape[4][4];
	int type; // 0 ~ 6
};

PlayerPiece g_currentPiece;
bool g_gameOver = false;
std::chrono::steady_clock::time_point g_lastFallTime = std::chrono::steady_clock::now();
float g_fallInterval = 0.5f; // Interval unit : seconds

//
// Background Function
//
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
void Render();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//
// Game Function
// 
void StartGame();
void NewPiece();
bool CheckCollision(int pieceX, int pieceY, int shape[4][4]);
void MergePiece();
void RotatePiece();
void ClearLines();
void Update();



