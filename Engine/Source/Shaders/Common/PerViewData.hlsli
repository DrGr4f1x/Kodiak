#ifndef PER_VIEW_DATA
#define PER_VIEW_DATA

cbuffer PerViewConstants : register(b0)
{
	matrix viewProjection;
	matrix modelToShadow;
	float3 viewPosition;
};

#endif