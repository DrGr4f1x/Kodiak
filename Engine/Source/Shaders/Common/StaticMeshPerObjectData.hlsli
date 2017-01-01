#ifndef PER_OBJECT_DATA
#define PER_OBJECT_DATA

cbuffer PerObjectConstants : register(b1)
{
	matrix model;
};

#endif