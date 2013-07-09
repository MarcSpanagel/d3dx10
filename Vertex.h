#pragma once
#ifndef VERTEX_H
#define VERTEX_H

	// Vertexlayout
static D3D10_INPUT_ELEMENT_DESC layout[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
	D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20,
	D3D10_INPUT_PER_VERTEX_DATA, 0 },
};
static UINT numVertexElements = sizeof(layout) / sizeof(layout[0]);

struct Vertex
{
	Vertex(){}
	Vertex(float x, float y, float z,
		float u, float v, float nx, float ny, float nz) : 
			Pos(x,y,z), Tex(u,v), normal(nx, ny, nz){}
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Tex;
	D3DXVECTOR3 normal;
};


#endif