#pragma once
#include "D3DUtil.h"
#include "Vertex.h"
#include <fstream>
#include <istream>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

class Sphere
{
public:
	Sphere(void);
	~Sphere(void);
	void init(ID3D10Device* device);
	void draw(UINT subsetID);
	vector<UINT> meshSubsets;
	vector<ID3D10ShaderResourceView*> TextureResourceViews;
private:
	ID3D10Device*  mDevice;
	bool LoadMesh(wstring filename);
	vector<ID3DX10Mesh*> meshes;
	int meshCount;
	
	int meshTextures;
	
};

