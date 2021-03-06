#include "Base.h"
#include <iostream>
#include <sstream>
using namespace std;

Base::Base(HINSTANCE hInstance)
{
	g_hInst = hInstance;
	g_hWnd = NULL;
	g_driverType = D3D10_DRIVER_TYPE_NULL;
	g_pd3dDevice = NULL;
	g_pSwapChain = NULL;
	g_pRenderTargetView = NULL;
	g_pDepthStencil = NULL;
	g_pDepthStencilView = NULL;
	g_pEffect = NULL;
	DIKeyboard = NULL;
	DIMouse = NULL;
	DirectInput = NULL;

	Eye = D3DXVECTOR3( 0.0f, 30.0f, -70.0f );
    At = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    Up = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	FW = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	LR = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
	DefaultFW = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	DefaultLR = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );

	// Rotation
	rotx = 0;
	rotz = 0;
	moveUD = 0;
	moveLR = 0;
	moveZ = 0;
	//Camera
	lookBF = 0.0f;
	lookLR = 0.0f;
	yaw = 0.0f;
	pitch = 0.0f;
	

}

Base::~Base(void)
{
	CleanupDevice();
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

int Base::Run() {
	    // Main message loop
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
			DetectInput();
            Render();
        }
    }
    CleanupDevice();
    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
void Base::InitWindow()
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = g_hInst;
    wcex.hIcon = LoadIcon( g_hInst, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"MarcWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    HR(!RegisterClassEx( &wcex ) )

    // Create window
    RECT rc = { 0, 0, 640, 480 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"MarcWindowClass", L"Marcs Direct3D 10 Example", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, g_hInst,
                           NULL );

    ShowWindow( g_hWnd, SW_SHOW );
}

void Base::InitDevice() {

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;

    D3D10_DRIVER_TYPE driverTypes[] =
    {
        D3D10_DRIVER_TYPE_HARDWARE,
        D3D10_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
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

	HRESULT hr = S_OK;
    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D10CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags,
                                            D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice );
        if( SUCCEEDED( hr ) )
            break;
    }
	if (FAILED(hr)) 
	{
		CleanupDevice();
		return;
	}

    // Create a render target view
    ID3D10Texture2D* pBuffer;
    HR(g_pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBuffer ));

    HR(g_pd3dDevice->CreateRenderTargetView( pBuffer, NULL, &g_pRenderTargetView ));
    pBuffer->Release();

    // Create depth stencil texture
    D3D10_TEXTURE2D_DESC descDepth;
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D10_USAGE_DEFAULT;
    descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    HR(g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil ));

    // Create the depth stencil view
    D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    HR(g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView ));

    g_pd3dDevice->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D10_VIEWPORT vp;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pd3dDevice->RSSetViewports( 1, &vp );
}

bool Base::InitDirectInput(HINSTANCE hInstance)
{
	DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL); 

	DirectInput->CreateDevice(GUID_SysKeyboard,
		&DIKeyboard,
		NULL);

	DirectInput->CreateDevice(GUID_SysMouse,
		&DIMouse,
		NULL);

	DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	DIKeyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	DIMouse->SetDataFormat(&c_dfDIMouse);
	DIMouse->SetCooperativeLevel(g_hWnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;
}

void Base::DetectInput()
{
	float l_delta_mouse = 0.01f;
	float l_deltarot = 0.05f;
	
	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];

	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	DIKeyboard->GetDeviceState(sizeof(keyboardState),(LPVOID)&keyboardState);
	
	if(keyboardState[DIK_ESCAPE] & 0x80)
		PostMessage(g_hWnd, WM_DESTROY, 0, 0);

	if(keyboardState[DIK_LEFT] & 0x80)
	{
		rotz -= l_deltarot;
	}
	if(keyboardState[DIK_RIGHT] & 0x80)
	{
		rotz += l_deltarot;
	}
	if(keyboardState[DIK_UP] & 0x80)
	{
		rotx += l_deltarot;
	}
	if(keyboardState[DIK_DOWN] & 0x80)
	{
		rotx -= l_deltarot;
	}
	if(mouseCurrState.lX != mouseLastState.lX)
	{
		//moveLR -= (mouseCurrState.lX * l_delta_mouse);
		yaw += mouseCurrState.lX * l_delta_mouse;
	}
	if(mouseCurrState.lY != mouseLastState.lY)
	{
		//moveUD += (mouseCurrState.lY * l_delta_mouse);
		pitch += mouseCurrState.lY * l_delta_mouse;
	}
	if(mouseCurrState.lZ != mouseLastState.lZ)
	{
		moveZ += (mouseCurrState.lZ * l_delta_mouse);
	}

	// Look up and down
	if(keyboardState[DIK_A] & 0x80)
	{
		lookLR -= l_deltarot;
	}
	if(keyboardState[DIK_D] & 0x80)
	{
		lookLR += l_deltarot;
	}
	if(keyboardState[DIK_W] & 0x80)
	{
		lookBF += l_deltarot;
	}
	if(keyboardState[DIK_S] & 0x80)
	{
		lookBF -= l_deltarot;
	}
	
	LimitWithinTwoPi(rotx);
	LimitWithinTwoPi(rotz);

	mouseLastState = mouseCurrState;
	UpdateCamera();

	return;
}

void Base::UpdateCamera() 
{
	D3DXMatrixRotationYawPitchRoll(&cameraRotation, yaw, pitch, 0);
	// Rotate the Default-ForwardVector and put the result in At
	D3DXVec3TransformCoord(&At, &DefaultFW, &cameraRotation);
	D3DXVec3Normalize(&At, &At);// Normalize outofbounds

	D3DXMATRIX RotateY;
	// Rotate Tempvariable RotateY around the y-axis with the angle of yaw.
	D3DXMatrixRotationY(&RotateY, yaw);

	D3DXVec3TransformNormal(&LR, &DefaultLR, &RotateY);
	D3DXVec3TransformNormal(&Up, &Up, &RotateY);


	//_RPT1( 0, "%d, %d, %d\n", Up.x, Up.y, Up.z );
	D3DXVec3TransformNormal(&FW, &DefaultFW, &RotateY);

	Eye += lookLR * LR;
	Eye += lookBF * FW;

	lookLR = 0.0f;
	lookBF = 0.0f;

	At = Eye + At;
	D3DXMatrixLookAtLH( &g_View, &Eye, &At, &Up );
}

void Base::Render() 
{
}

void Base::LimitWithinTwoPi(float &r) 
{
	float c_2PI = 6.283185f;
	if ( r > c_2PI )
		r -=  c_2PI;
	else if ( r < 0 )
		r =  c_2PI + r;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void Base::CleanupDevice()
{
    if( g_pd3dDevice ) g_pd3dDevice->ClearState();
	ReleaseCOM(g_pRenderTargetView);
	ReleaseCOM(g_pSwapChain);
	ReleaseCOM(g_pRenderTargetView);
	ReleaseCOM(g_pd3dDevice);
	ReleaseCOM(g_pDepthStencil);
	ReleaseCOM(g_pDepthStencilView);
	ReleaseCOM(g_pEffect);
	ReleaseCOM(g_pd3dDevice);	
	DIKeyboard->Unacquire();
	DIMouse->Unacquire();
	ReleaseCOM(DirectInput);
}