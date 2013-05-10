#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include "resource.h"
#include "Base.h"
#include "Vertex.h"


class Application : public Base
{
public:
	Application(HINSTANCE hInstance);
	~Application(void);
	void initApp();
	HRESULT SetupViewAndBuffer();
	void Render();
	HRESULT CreateFX();
	HRESULT CreateVertices();
	HRESULT InitWireframe();


private:
	ID3D10EffectTechnique*      g_pTechniqueRender;
	//ID3D10EffectTechnique*      g_pTechniqueRenderLight;
	ID3D10InputLayout*          g_pVertexLayout;
	ID3D10Buffer*               g_pVertexBuffer;
	ID3D10Buffer*               g_pIndexBuffer;
	ID3D10EffectMatrixVariable* g_pWorldVariable;
	ID3D10EffectMatrixVariable* g_pViewVariable;
	ID3D10EffectMatrixVariable* g_pProjectionVariable;
	ID3D10EffectMatrixVariable* g_WVPVariable;
	//ID3D10EffectVectorVariable* g_pLightDirVariable;
	//ID3D10EffectVectorVariable* g_pLightColorVariable;
	ID3D10EffectVectorVariable* g_pOutputColorVariable;
	D3DXMATRIX                  g_World;
	D3DXMATRIX                  g_View;
	D3DXMATRIX                  g_Projection;
	D3DXMATRIX                  g_WVP;
	D3DXMATRIX					l_Scale;
	D3DXMATRIX					l_Rot;
	D3DXMATRIX					l_Transform;

	UINT g_numIndices;
	UINT g_numVertices;

	ID3D10RasterizerState* WireFrame;
};

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
	Application app(hInstance);
	app.initApp();
	return app.Run();
}
Application::Application(HINSTANCE hInstance) : Base(hInstance) {
	g_pEffect = NULL;
	g_pTechniqueRender = NULL;
	//g_pTechniqueRenderLight = NULL;
	g_pVertexLayout = NULL;
	g_pVertexBuffer = NULL;
	g_pIndexBuffer = NULL;
	g_pWorldVariable = NULL;
	g_pViewVariable = NULL;
	g_pProjectionVariable = NULL;
	//g_pLightDirVariable = NULL;
	//g_pLightColorVariable = NULL;
	g_pOutputColorVariable = NULL;
	D3DXMatrixIdentity(&g_World);
	D3DXMatrixIdentity(&g_View);
	D3DXMatrixIdentity(&g_Projection);
	D3DXMatrixIdentity(&g_WVP);
}

Application::~Application() 
{
	if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
}

void Application::initApp() 
{
	Base::InitWindow();
	Base::InitDevice();
	SetupViewAndBuffer();
}

HRESULT Application::SetupViewAndBuffer() 
{
    if (FAILED(CreateFX()))
	{
		CleanupDevice();
		return 0;
	}
    
	// VertexBuffer und Indexbuffer initialisieren
	if (FAILED(CreateVertices()))
	{
		CleanupDevice();
		return 0;
	}

	if ( FAILED(InitWireframe()) ) 
	{
		CleanupDevice();
		MessageBox(0, L"RasterizerStateCreation - Failed",
			L"Error", MB_OK);
		return 0;
	}

	// Set the input layout
    g_pd3dDevice->IASetInputLayout( g_pVertexLayout );
    // Set primitive topology
    g_pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Initialize the world matrices
    D3DXMatrixIdentity( &g_World );
    // Initialize the view matrix

	D3DXVECTOR3 Eye( 0.0f, 5.0f,-5.0f );
    D3DXVECTOR3 At( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 Up( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &g_View, &Eye, &At, &Up );
    // Initialize the projection matrix
	RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    D3DXMatrixPerspectiveFovLH( &g_Projection, ( float )D3DX_PI * 0.25f, width / ( FLOAT )height, 0.0001f, 100.0f );
	

    return TRUE;
}

HRESULT Application::InitWireframe() 
{
	D3D10_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D10_RASTERIZER_DESC));
	wfdesc.FillMode = D3D10_FILL_WIREFRAME; //D3D10_FILL_SOLID->Default
	wfdesc.CullMode = D3D10_CULL_FRONT;
	wfdesc.FrontCounterClockwise = false;
	HRESULT hr = g_pd3dDevice->CreateRasterizerState(&wfdesc, &WireFrame);

	if(FAILED(hr))
	{
		return false;
	}
	g_pd3dDevice->RSSetState(WireFrame);
}

HRESULT Application::CreateFX()
{
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
	HRESULT hr = 0;
	ID3D10Blob* compilationErrors = 0;
	hr = D3DX10CreateEffectFromFile( L"marc02.fx", NULL, NULL,
		"fx_4_0", dwShaderFlags, 0, g_pd3dDevice,
		NULL, NULL, &g_pEffect,
		&compilationErrors, NULL);
	if( FAILED( hr ) ) 
	{
		if( compilationErrors )
		{
			MessageBoxA(0, (char*)compilationErrors->GetBufferPointer(), 0, 0);
			ReleaseCOM(compilationErrors);
		}
		return hr;
	}

	g_pTechniqueRender = g_pEffect->GetTechniqueByName("Render");

	// TODO Hier die Variablen aus dem FX-File holen:
	// ---------------------------
	
	g_pWorldVariable = g_pEffect->GetVariableByName( "World" )->AsMatrix();
    g_pViewVariable = g_pEffect->GetVariableByName( "View" )->AsMatrix();
    g_pProjectionVariable = g_pEffect->GetVariableByName( "Projection" )->AsMatrix();
	g_WVPVariable = g_pEffect->GetVariableByName("WVP")->AsMatrix();
	
	// ---------------------------

	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// -------------VertexLayout initialisieren----------------
	D3D10_PASS_DESC PassDesc;
	g_pTechniqueRender->GetPassByIndex(0)->GetDesc(&PassDesc);
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements,
		PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
		&g_pVertexLayout);

	if (FAILED(hr)) 
	{ return hr; }

	// -------------------------------------------------------
	return hr;
}

HRESULT Application::CreateVertices()
{
	HRESULT hr = 0;
	// Vertexzahl und -seiten initialisieren
	g_numVertices = 5;
	g_numIndices = 18;

	/*Vertex vertices[] = 
	{
		{D3DXVECTOR3(-1.0f, 1.0f, -1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },
		{D3DXVECTOR3(-1.0f, 1.0f,  1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },
		{D3DXVECTOR3(-1.0f,-1.0f, -1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },
		{D3DXVECTOR3(-1.0f,-1.0f,  1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },

		//{D3DXVECTOR3( 1.0f, 1.0f, -1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },
		//{D3DXVECTOR3( 1.0f, 1.0f,  1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },
		//{D3DXVECTOR3( 1.0f,-1.0f, -1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },
		//{D3DXVECTOR3( 1.0f,-1.0f,  1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },
	};*/
	Vertex vertices[] =
    {
		{D3DXVECTOR3( 0.0f, 1.0f, 5.0f ),D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f)},
        {D3DXVECTOR3( -1.0f, -0.9f, 1.0f ),D3DXVECTOR4(0.0f, 1.0f, 1.0f, 1.0f)},
        {D3DXVECTOR3( 1.0f, -0.9f, 1.0f),D3DXVECTOR4(1.0f, 0.0f, 1.0f, 1.0f)},
		{D3DXVECTOR3( 1.0f, -0.9f, -0.5f),D3DXVECTOR4(1.0f, 0.0f, 1.0f, 1.0f)},
		{D3DXVECTOR3( -1.0f, -0.9f, -0.5f),D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f)},
    };

	 DWORD indices[] =
    {
		0,2,1,
		0,3,2,
		0,4,3,
		0,1,4,
		4,1,2,
		4,2,3
        /*3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,*/
    };

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * g_numVertices;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr)) return hr;

	// VertexBuffer in den InputAssembler schmeissen
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pd3dDevice->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// IndexBuffer erzeugen

	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DWORD) * g_numIndices;
	bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	InitData.pSysMem = indices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr)) return hr;

	// IndexBuffer auch in den IA schmeissen
	g_pd3dDevice->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	return hr;
}

void Application::Render()
{
	// Update our time
    static float t = 0.0f;
    if( g_driverType == D3D10_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )D3DX_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 0.5f }; //red, green, blue, alpha
    g_pd3dDevice->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	
	//
    
	D3DXMatrixScaling( &l_Scale, 0.3f, 0.3f,  0.3f); 
	D3DXMatrixRotationY(&l_Rot, t );
	D3DXMatrixMultiply(&l_Transform, &l_Scale, &l_Rot);
	
	g_WVP = g_World *l_Transform * g_View * g_Projection;
	
	// Update variables
    g_pWorldVariable->SetMatrix( ( float* )&g_World );
    g_pViewVariable->SetMatrix( ( float* )&g_View );
    g_pProjectionVariable->SetMatrix( ( float* )&g_Projection );
	g_WVPVariable->SetMatrix( (float*)&g_WVP );
	
	//
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pd3dDevice->ClearDepthStencilView( g_pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );
	//
    // Render the first cube
    
    D3D10_TECHNIQUE_DESC techDesc;
    g_pTechniqueRender->GetDesc( &techDesc );
    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        g_pTechniqueRender->GetPassByIndex( p )->Apply( 0 );
        g_pd3dDevice->DrawIndexed( g_numIndices, 0, 0 );
    }

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
}