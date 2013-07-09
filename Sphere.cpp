#include "Sphere.h"


Sphere::Sphere(void) : mDevice(0)
{
	meshTextures = 0;
	meshCount = 0;

}


Sphere::~Sphere(void)
{
	for(int i = 0; i < meshCount; i++)
	{
		ReleaseCOM(meshes[i]);
	}
}
void Sphere::init(ID3D10Device* device)
{
	mDevice = device;
	LoadMesh(L"sphere.dat");
}
bool Sphere::LoadMesh(wstring filename)
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

		hr = D3DX10CreateMesh(mDevice,
			layout, 
			numVertexElements, 
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

			D3DX10CreateShaderResourceViewFromFile(mDevice,
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

void Sphere::draw(UINT subsetID)
{

	meshes[0]->DrawSubset(subsetID);
}