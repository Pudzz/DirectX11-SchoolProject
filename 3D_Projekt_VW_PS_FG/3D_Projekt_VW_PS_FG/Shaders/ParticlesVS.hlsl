
cbuffer MatrixBuffer : register(b0)
{
	row_major matrix worldViewProjection;
	row_major matrix worldspace;
};

struct VertexInput
{
	float4 Position : POSITION;
	float2 Texcoord : TEXCOORD;
	float4 Color : COLOR;
};

struct VertexOutput
{
	float4 WVPPosition : SV_POSITION;
	float4 WPosition : WPOSITION;
	float2 WTexcoord : TEXCOORD;
	float4 Color : COLOR;
};

VertexOutput VSMain(VertexInput input)
{
	VertexOutput output = (VertexOutput)0;

	input.Position.w = 1.0f;
	output.WVPPosition = mul(worldViewProjection, float4(input.Position));
	output.WPosition = mul(worldspace, float4(input.Position));
	output.WTexcoord = input.Texcoord;

	// Store the particle color for the pixel shader. // We dont use it but we can if we want to.
	output.Color = input.Color;
		
	return output;
}