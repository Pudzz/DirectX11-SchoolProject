// Nonnumeric values cannot be added to a cbuffer.
TextureCube cubeMap : register(t0);
SamplerState samplerType : register(s0);

cbuffer cBufferMaterial : register(b0)
{
	float4 diffuseColor;
	float4 ambientColor;
	float4 specularColor;
	float4 reflectionColor;

	float shine;
	bool hasTexture;
	bool hasReflection;
	bool isTerrain;

	bool canMove;
	float3 padding2;

	float4 translation;

	bool hasNormMap;
	float3 padding3;

	bool isObj;
	float3 padding4;
};

struct PixelShaderInput
{
	float4 WVPPosition : SV_POSITION;
	float3 LPosition : POSITION;
};

float4 SkyPSMain(PixelShaderInput input) : SV_Target
{
	float4 difMaterial = diffuseColor;

	if (hasTexture == true) {
		difMaterial = cubeMap.Sample(samplerType, input.LPosition);
	}

	return difMaterial;
}