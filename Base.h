#pragma once
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#ifndef BASE_H
#define BASE_H
#include "D3DUtil.h"
#include "resource.h"
#include <dinput.h>



class Base
{
public:
	Base(HINSTANCE hInstance);
	virtual ~Base(void);
	int Run();
protected:
	HINSTANCE                   g_hInst;
	HWND                        g_hWnd;
	D3D10_DRIVER_TYPE           g_driverType;
	ID3D10Device*               g_pd3dDevice;
	IDXGISwapChain*             g_pSwapChain;
	ID3D10RenderTargetView*     g_pRenderTargetView;
	ID3D10Texture2D*            g_pDepthStencil;
	ID3D10DepthStencilView*     g_pDepthStencilView;
	ID3D10Effect*               g_pEffect;
	//############## Input #########################
	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;
	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;

	float rotx;
	float rotz;
	float moveUD;
	float moveLR;

	D3DXMATRIX Rotationx;
	D3DXMATRIX Rotationz;

	void CleanupDevice();
	//LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
	virtual void Render();
	void InitWindow();
	void InitDevice();
	bool InitDirectInput(HINSTANCE hInstance);
	void DetectInput();
};
#endif

