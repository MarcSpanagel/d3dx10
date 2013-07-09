#pragma once
#ifndef MOUNTAINS_H
#define MOUNTAINS_H

#include "D3DUtil.h"
class Mountains
{
public:
	Mountains(void);
	~Mountains(void);
	ID3D10ShaderResourceView* mGrassMapRV;
	void init(ID3D10Device* device, DWORD m, DWORD n, float dx);
	void draw();
private:
	float getHeight(float x, float z) const;
	ID3D10Device* mDevice;
	ID3D10Buffer* mVertexBuffer;
	ID3D10Buffer* mIndexBuffer;

	DWORD mNumRows;
	DWORD mNumCols;

	DWORD mNumVertices;
	DWORD mNumFaces;
};

#endif