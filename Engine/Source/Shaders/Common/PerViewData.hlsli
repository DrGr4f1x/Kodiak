#ifndef PER_VIEW_DATA
#define PER_VIEW_DATA

cbuffer PerViewConstants : register(b0)
{
	matrix view;
	matrix projection;
	float3 viewPosition;
};

#endif