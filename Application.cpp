#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include "resource.h"
#include "Base.h"
#include "Vertex.h"
#include "Light.h"
#include <fstream>
#include <istream>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

D3D10_INPUT_ELEMENT_DESC layout[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
	D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20,
	D3D10_INPUT_PER_VERTEX_DATA, 0 },
};

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
	bool LoadMesh(wstring filename);


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
	D3DXMATRIX					l_Translation;
	D3DXMATRIX					l_Rotation;
	D3DXMATRIX					l_Transform;
	float rot;

	UINT g_numIndices;
	UINT g_numVertices;

	ID3D10RasterizerState* WireFrame;

	ID3D10ShaderResourceView* g_DiffuseMapResourceView;
	ID3D10EffectShaderResourceVariable* fxDiffuseMapVar;

	Light light;

	vector<ID3DX10Mesh*> meshes;
	int meshCount;
	vector<UINT> meshSubsets;
	int meshTextures;
	vector<ID3D10ShaderResourceView*> TextureResourceViews;
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
	D3DXMatrixIdentity(&l_Rotation);
	D3DXMatrixIdentity(&l_Transform);
	rot = 0.01f;
	int meshTextures = 0;
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
	if(!Base::InitDirectInput(g_hInst))
	{
		MessageBox(0, L"Direct Input Initialization - Failed",
			L"Error", MB_OK);
		return;
	}
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

	D3DXVECTOR3 Eye( 0.0f, 4.0f,-11.0f );
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

	// Hier die Variablen aus dem FX-File holen:
	// ---------------------------
	fxDiffuseMapVar = g_pEffect->GetVariableByName("DiffuseMap")->AsShaderResource();
	g_pWorldVariable = g_pEffect->GetVariableByName( "World" )->AsMatrix();
    g_pViewVariable = g_pEffect->GetVariableByName( "View" )->AsMatrix();
    g_pProjectionVariable = g_pEffect->GetVariableByName( "Projection" )->AsMatrix();
	g_WVPVariable = g_pEffect->GetVariableByName("WVP")->AsMatrix();
	fxLightVar  = g_pEffect->GetVariableByName("light");
	// ---------------------------

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

bool Application::LoadMesh(wstring filename)
{
	HRESULT hr = 0;

	ID3DX10Mesh* tempMesh; 

	wifstream fileIn (filename.c_str());
	wstring skipString;

	UINT meshVertices  = 0;
	UINT meshTriangles = 0;
    UINT tempMeshSubsets = 0;

	if (fileIn)
	{
		fileIn >> skipString; // #Subsets
		fileIn >> tempMeshSubsets;
		fileIn >> skipString; // #Vertices
		fileIn >> meshVertices;
		fileIn >> skipString; // #Faces (Triangles)
		fileIn >> meshTriangles;
        
		meshSubsets.push_back(tempMeshSubsets);

		hr = D3DX10CreateMesh(g_pd3dDevice,
			layout, 
			3, 
			layout[0].SemanticName, 
			meshVertices, 
			meshTriangles, 
			D3DX10_MESH_32_BIT, 
			&tempMesh);

		if(FAILED(hr))
		{
			MessageBox(0, L"Mesh Creation - Failed",
				L"Error", MB_OK);
			return false;
		}

		fileIn >> skipString;	//#Subset_info
		for(UINT i = 0; i < tempMeshSubsets; ++i)
		{
			std::wstring diffuseMapFilename;

			fileIn >> diffuseMapFilename;

			ID3D10ShaderResourceView* DiffuseMapResourceView;

			D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice,
				diffuseMapFilename.c_str(), 0, 0, &DiffuseMapResourceView, 0 );

			TextureResourceViews.push_back(DiffuseMapResourceView);

			meshTextures++;
		}

		Vertex* verts = new Vertex[meshVertices];
		fileIn >> skipString;	//#Vertex_info
		for(UINT i = 0; i < meshVertices; ++i)
		{
			fileIn >> skipString;	//Vertex Position
			fileIn >> verts[i].Pos.x;
			fileIn >> verts[i].Pos.y;
			fileIn >> verts[i].Pos.z;

			fileIn >> skipString;	//Vertex Normal
			fileIn >> verts[i].normal.x;
			fileIn >> verts[i].normal.y;
			fileIn >> verts[i].normal.z;

			fileIn >> skipString;	//Vertex Texture Coordinates
			fileIn >> verts[i].Tex.x;
			fileIn >> verts[i].Tex.y;
		}
		tempMesh->SetVertexData(0, verts);

		delete[] verts;

		DWORD* indices = new DWORD[meshTriangles*3];
		UINT* attributeIndex = new UINT[meshTriangles];
		fileIn >> skipString;	//#Face_Index
		for(UINT i = 0; i < meshTriangles; ++i)
		{
			fileIn >> indices[i*3+0];
			fileIn >> indices[i*3+1];
			fileIn >> indices[i*3+2];
			fileIn >> attributeIndex[i];	//Current Subset
		}
		tempMesh->SetIndexData(indices, meshTriangles*3);
		tempMesh->SetAttributeData(attributeIndex);

		delete[] indices;
		delete[] attributeIndex;

		tempMesh->GenerateAdjacencyAndPointReps(0.001f);
		tempMesh->Optimize(D3DX10_MESHOPT_ATTR_SORT|D3DX10_MESHOPT_VERTEX_CACHE,0,0);
		tempMesh->CommitToDevice();

		meshCount++;
		meshes.push_back(tempMesh);
	}
	else
	{
		MessageBox(0, L"Load Mesh File - Failed",
			L"Error", MB_OK);
		return false;
	}


	return true;
}

HRESULT Application::CreateVertices()
{
	LoadMesh(L"sphere.dat");
	//Licht initialisieren
	light.dir = D3DXVECTOR3(0.25f, 0.5f, -1.0f);
	light.ambient = D3DXCOLOR(0.2f, 0.0f, 0.0f, 1.0f);
	light.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	return TRUE;
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

	////////////////////Rotation//////////////////////////////////////////////////////////////////////
	D3DXVECTOR3 rotyaxis(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 rotzaxis(0.0f, 0.0f, 1.0f);
	D3DXVECTOR3 rotxaxis(1.0f, 0.0f, 0.0f);

	//D3DXMatrixRotationAxis(&l_Rotation, &rotyaxis, rot);
	D3DXMatrixRotationAxis(&Rotationx, &rotxaxis, rotx);
	D3DXMatrixRotationAxis(&Rotationz, &rotzaxis, rotz);
	D3DXMatrixTranslation( &l_Translation, 0.0f, 0.0f, 4.0f );
	l_Transform = l_Translation * l_Rotation * Rotationx * Rotationz;

	rot += .0005f;

	if ( rot > (float)6.283185 )
		rot -=  (float)6.283185;
	else if ( rot < 0 )
		rot =  (float)6.283185 + rot;

	g_WVP = g_World *l_Transform * g_View * g_Projection;
	
	// Update variables
    g_pWorldVariable->SetMatrix( ( float* )&g_World );
    g_pViewVariable->SetMatrix( ( float* )&g_View );
    g_pProjectionVariable->SetMatrix( ( float* )&g_Projection );
	g_WVPVariable->SetMatrix( (float*)&g_WVP );

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
		for(UINT subsetID = 0; subsetID < meshSubsets[0]; ++subsetID)
		{
			// Texturevariable in FX setzen
			fxDiffuseMapVar->SetResource(TextureResourceViews[subsetID]);
			g_pTechniqueRender->GetPassByIndex( p )->Apply( 0 );
			meshes[0]->DrawSubset(subsetID);
		}
    }

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
}