cbuffer cbPerObject
{
	row_major float4x4 worldViewProjection;
};

struct VertexInput
{
	float3 Position : POSITION;
};

struct VertexOutput
{
	float4 WVPPosition : SV_POSITION;
	float3 LPosition : POSITION;
};

VertexOutput SkyVSMain(VertexInput input)
{
	VertexOutput output = (VertexOutput)0;

	/*
		We set z = w so both z and w is 1 because a skydome always is on far plane. 
	*/
	float4 HPos = mul(worldViewProjection, float4(input.Position, 1.0f));
	output.WVPPosition = HPos.xyww;
	output.LPosition = input.Position;

	return output;
}