#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include "resource.h"
#include "Base.h"
#include "Vertex.h"
#include "Light.h"


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

	ID3D10EffectVariable* fxLightVar;

	//ID3D10EffectVectorVariable* g_pLightDirVariable;
	//ID3D10EffectVectorVariable* g_pLightColorVariable;
	ID3D10EffectVectorVariable* g_pOutputColorVariable;
	D3DXMATRIX                  g_World;
	D3DXMATRIX                  g_View;
	D3DXMATRIX                  g_Projection;
	D3DXMATRIX                  g_WVP;
	D3DXMATRIX					l_Scale;
	D3DXMATRIX					l_RotX;
	D3DXMATRIX					l_RotY;
	D3DXMATRIX					l_Transform;

	UINT g_numIndices;
	UINT g_numVertices;

	ID3D10RasterizerState* WireFrame;

	ID3D10ShaderResourceView* g_DiffuseMapResourceView;
	ID3D10EffectShaderResourceVariable* fxDiffuseMapVar;

	Light light;
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
	D3DXMatrixIdentity(&l_RotX);
	D3DXMatrixIdentity(&l_RotY);
}

Application::~Application() 
{
	if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
	if( g_DiffuseMapResourceView ) g_DiffuseMapResourceView->Release();
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

	/*if ( FAILED(InitWireframe()) ) 
	{
		CleanupDevice();
		MessageBox(0, L"RasterizerStateCreation - Failed",
			L"Error", MB_OK);
		return 0;
	}*/

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
	// ShaderResourceView von der Texture erzeugen:
	D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, 
		L"braynzar.jpg", 0, 0, &g_DiffuseMapResourceView, 0 );

	g_pTechniqueRender = g_pEffect->GetTechniqueByName("Render");

	// Hier die Variablen aus dem FX-File holen:
	// ---------------------------
	fxDiffuseMapVar = g_pEffect->GetVariableByName("DiffuseMap")->AsShaderResource();
	g_pWorldVariable = g_pEffect->GetVariableByName( "World" )->AsMatrix();
    g_pViewVariable = g_pEffect->GetVariableByName( "View" )->AsMatrix();
    g_pProjectionVariable = g_pEffect->GetVariableByName( "Projection" )->AsMatrix();
	g_WVPVariable = g_pEffect->GetVariableByName("WVP")->AsMatrix();
	fxLightVar  = g_pEffect->GetVariableByName("light");
	// ---------------------------

	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
		D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20,
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
	//Licht initialisieren
	light.dir = D3DXVECTOR3(0.25f, 0.5f, -1.0f);
	light.ambient = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
	light.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	HRESULT hr = 0;
	// Vertexzahl und -seiten initialisieren
	g_numVertices = 24;
	g_numIndices = 36;
	// Dies ist der Kegel:
	//Vertex v[] =
 //   {
	//	//Vorne
	//	Vertex( 0.0f, 1.0f, 2.0f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f ),
	//	Vertex( 1.0f, -0.9f, 1.0f, 1.0f, 1.0f, 1.0f, -0.9f, 1.0f),
 //       Vertex( -1.0f, -0.9f, 1.0f, 0.0f, 1.0f, -1.0f, -0.9f, 1.0f),
 //       
	//	//Rechts
	//	Vertex( 0.0f, 1.0f, 2.0f, 0.5f, 0.0f , 0.0f, 1.0f, 2.0f),
	//	Vertex( 1.0f, -0.9f, -0.9f, 1.0f, 1.0f, 1.0f, -0.9f, -0.9f), //hinten
	//	Vertex( 1.0f, -0.9f, 1.0f, 0.0f, 1.0f, 1.0f, -0.9f, 1.0f),
	//	
	//	//Links
	//	Vertex( 0.0f, 1.0f, 2.0f, 0.5f, 0.0f, 0.0f, 1.0f, 2.0f ),
	//	Vertex( -1.0f, -0.9f, 1.0f, 1.0f, 1.0f, -1.0f, -0.9f, 1.0f),
	//	Vertex( 1.0f, -0.9f, -0.9f, 0.0f, 1.0f, 1.0f, -0.9f, -0.9f), //hinten
	//	//unten
	//	Vertex( 1.0f, -0.9f, -0.9f, 0.5f, 0.0f, 1.0f, -0.9f, -0.9f), //hinten
	//	Vertex( -1.0f, -0.9f, 1.0f, 1.0f, 1.0f, -1.0f, -0.9f, 1.0f), //Vlinks
	//	Vertex( 1.0f, -0.9f, 1.0f, 0.0f, 1.0f, 1.0f, -0.9f, 1.0f), //Vrechts
 //   };
	Vertex v[24];
	// Front Face
	v[0] = Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f,-1.0f, -1.0f, -1.0f);
	v[1] = Vertex(-1.0f,  1.0f, -1.0f, 0.0f, 0.0f,-1.0f,  1.0f, -1.0f);
	v[2] = Vertex( 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, -1.0f);
	v[3] = Vertex( 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f);

	// Back Face
	v[4] = Vertex(-1.0f, -1.0f, 1.0f, 1.0f, 1.0f,-1.0f, -1.0f, 1.0f);
	v[5] = Vertex( 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f);
	v[6] = Vertex( 1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f);
	v[7] = Vertex(-1.0f,  1.0f, 1.0f, 1.0f, 0.0f,-1.0f,  1.0f, 1.0f);

	// Top Face
	v[8]  = Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f,-1.0f, 1.0f, -1.0f);
	v[9]  = Vertex(-1.0f, 1.0f,  1.0f, 0.0f, 0.0f,-1.0f, 1.0f,  1.0f);
	v[10] = Vertex( 1.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f);
	v[11] = Vertex( 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f);

	// Bottom Face
	v[12] = Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f,-1.0f, -1.0f, -1.0f);
	v[13] = Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f);
	v[14] = Vertex( 1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, -1.0f,  1.0f);
	v[15] = Vertex(-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,-1.0f, -1.0f,  1.0f);

	// Left Face
	v[16] = Vertex(-1.0f, -1.0f,  1.0f, 0.0f, 1.0f,-1.0f, -1.0f,  1.0f);
	v[17] = Vertex(-1.0f,  1.0f,  1.0f, 0.0f, 0.0f,-1.0f,  1.0f,  1.0f);
	v[18] = Vertex(-1.0f,  1.0f, -1.0f, 1.0f, 0.0f,-1.0f,  1.0f, -1.0f);
	v[19] = Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f,-1.0f, -1.0f, -1.0f);

	// Right Face
	v[20] = Vertex( 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f);
	v[21] = Vertex( 1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  1.0f, -1.0f);
	v[22] = Vertex( 1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f);
	v[23] = Vertex( 1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 1.0f, -1.0f,  1.0f);

	//  Index für Kegel
	/*DWORD i[] =
    {
		0,1,2,
		3,4,5,
		6,7,8,
		9,10,11
    };*/
	DWORD i[36] =
	{
		0,1,2,
		0,2,3,
		4,5,6,
		4,6,7,
		8,9,10,
		8,10,11,
		12,13,14,
		12,14,15,
		16,17,18,
		16,18,19,
		20,21,22,
		20,22,23
	};

	//// Front Face
	//i[0] = 0; i[1] = 1; i[2] = 2;
	//i[3] = 0; i[4] = 2; i[5] = 3;

	//// Back Face
	//i[6] = 4; i[7]  = 5; i[8]  = 6;
	//i[9] = 4; i[10] = 6; i[11] = 7;

	//// Top Face
	//i[12] = 8; i[13] =  9; i[14] = 10;
	//i[15] = 8; i[16] = 10; i[17] = 11;

	//// Bottom Face
	//i[18] = 12; i[19] = 13; i[20] = 14;
	//i[21] = 12; i[22] = 14; i[23] = 15;

	//// Left Face
	//i[24] = 16; i[25] = 17; i[26] = 18;
	//i[27] = 16; i[28] = 18; i[29] = 19;

	//// Right Face
	//i[30] = 20; i[31] = 21; i[32] = 22;
	//i[33] = 20; i[34] = 22; i[35] = 23;

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_IMMUTABLE;//D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * g_numVertices;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = v;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr)) return hr;

	// VertexBuffer in den InputAssembler schmeissen
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pd3dDevice->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// IndexBuffer erzeugen

	bd.Usage = D3D10_USAGE_IMMUTABLE;//D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DWORD) * g_numIndices;
	bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	InitData.pSysMem = i;
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
	D3DXMatrixIdentity(&l_Scale);
	//D3DXMatrixScaling( &l_Scale, 0.6f, 0.6f,  0.6f); 
	D3DXMatrixRotationYawPitchRoll(&l_RotY, t , t, t);
	//D3DXMatrixRotationX(&l_RotX, t );
	D3DXMatrixMultiply(&l_Transform, &l_RotX, &l_RotY);

	g_WVP = g_World *l_Transform * g_View * g_Projection;
	
	// Update variables
    g_pWorldVariable->SetMatrix( ( float* )&g_World );
    g_pViewVariable->SetMatrix( ( float* )&g_View );
    g_pProjectionVariable->SetMatrix( ( float* )&g_Projection );
	g_WVPVariable->SetMatrix( (float*)&g_WVP );
	// Texturevariable in FX setzen
	fxDiffuseMapVar->SetResource(g_DiffuseMapResourceView);
	fxLightVar->SetRawValue(&light, 0, sizeof(Light));
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