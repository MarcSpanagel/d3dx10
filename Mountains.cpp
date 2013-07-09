#include "Mountains.h"

#include "Vertex.h"
#include <vector>


Mountains::Mountains(void)
	: mNumRows(0), mNumCols(0), mNumVertices(0), mNumFaces(0),
	mDevice(0), mVertexBuffer(0), mIndexBuffer(0)
{
}


Mountains::~Mountains(void)
{
	ReleaseCOM(mVertexBuffer);
	ReleaseCOM(mIndexBuffer);
}


float Mountains::getHeight(float x, float z) const
{
	return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
}

void Mountains::init(ID3D10Device* device, DWORD m, DWORD n, float dx)
{
	mDevice = device;

	mNumRows  = m;
	mNumCols  = n;

	mNumVertices = m*n;
	mNumFaces    = (m-1)*(n-1)*2;

	std::vector<Vertex> vertices(mNumVertices);
	float halfWidth = (n-1)*dx*0.5f;
	float halfDepth = (m-1)*dx*0.5f;

	float du = 1.0f / (n-1);
	float dv = 1.0f / (m-1);
	for(DWORD i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dx;
		for(DWORD j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx;
			float y = getHeight(x,z);
			vertices[i*n+j].Pos = D3DXVECTOR3(x, y, z);

			// Stretch texture over grid.
			vertices[i*n+j].Tex.x = j*du;
			vertices[i*n+j].Tex.y = i*dv;

			// Compute the normals for the texture
			D3DXVECTOR3 normal;
			normal.x = -0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z);
			normal.y = 1.0f;
			normal.z = -0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z);
			D3DXVec3Normalize(&vertices[i*n+j].normal, &normal);
		}
	}
 
    D3D10_BUFFER_DESC vbd;
    vbd.Usage = D3D10_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * mNumVertices;
    vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(mDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer));

	std::vector<DWORD> indices(mNumFaces*3); // 3 indices per face

	// Iterate over each quad and compute indices.
	int k = 0;
	for(DWORD i = 0; i < m-1; ++i)
	{
		for(DWORD j = 0; j < n-1; ++j)
		{
			indices[k]   = i*n+j;
			indices[k+1] = i*n+j+1;
			indices[k+2] = (i+1)*n+j;

			indices[k+3] = (i+1)*n+j;
			indices[k+4] = i*n+j+1;
			indices[k+5] = (i+1)*n+j+1;
			k += 6; // next quad
		}
	}

	D3D10_BUFFER_DESC ibd;
    ibd.Usage = D3D10_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(DWORD) * mNumFaces*3;
    ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(mDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer));

	HR(D3DX10CreateShaderResourceViewFromFile(mDevice, 
		L"grass.dds", 0, 0, &mGrassMapRV, 0 ));
}


void Mountains::draw()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	mDevice->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mDevice->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    mDevice->DrawIndexed(mNumFaces*3, 0, 0);
}