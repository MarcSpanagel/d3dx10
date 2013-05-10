#pragma once

struct Vertex
{
	Vertex(){}
	Vertex(float x, float y, float z,
		float u, float v) : Pos(x,y,z), Tex(u,v){}
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Tex;
};