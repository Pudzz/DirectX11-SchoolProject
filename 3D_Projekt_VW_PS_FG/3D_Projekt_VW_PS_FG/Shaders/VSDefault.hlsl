
cbuffer cbPerObject : register(b0)
{
	row_major matrix worldViewProjection;
	row_major matrix worldspace;
	row_major matrix InverseTransposeWorldMatrix;

	// Joint matrices for the current keyrframe in an animation
	row_major matrix boneTransforms[50];
};

cbuffer cBufferCamera : register(b1)
{
	float3 cameraPosition;
	float padding;
};

struct VertexInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;

	// Weight for the bones, used as a multiplier with the bones matrices
	float weight : BONEWEIGHTS;
	// Index of which matrix this vertex should be usedwith
	int ID : BONEINDICES;
};

struct VertexOutput
{
	float4 WVPPosition : SV_POSITION;
	float4 WPosition : WPOSITION;	
	float2 WTexCoord : TEXCOORD;
	float3 WNormal : NORMAL;
	float3 WTangent : TANGENT;
	float3 ViewDir : TEXCOORD1;
};

// Animation function
VertexOutput animation(VertexInput input)
{
	float4 positions = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 normals = float3(0.0f, 0.0f, 0.0f);
	float3 tangent = float3(0.0f, 0.0f, 0.0f);

	// Multiply the vertex position with the bone transform, depending on its ID and weight
	positions += mul(boneTransforms[input.ID], float4(input.Position, 1.0f)) * input.weight;
	// Multiply the vertex normal with the bone transform, depending on its ID and weight
	normals +=  mul((float3x3)boneTransforms[input.ID], input.Normal).xyz * input.weight;
	// Multiply the tangents with the bone transforms
	tangent += mul((float3x3)boneTransforms[input.ID], input.Tangent).xyz * input.weight;

	VertexOutput output;

	output.WPosition = positions;
	output.WNormal = normals;
	output.WTangent = normalize(tangent);

	return output;
}

VertexOutput VSMain(VertexInput input) {

	VertexOutput output = (VertexOutput)0;

	// If the vertex ID isnt -1, there is a valid skeleton applied to the model with an animation
	if (input.ID != -1)
	{
		// Calculate the animation
		VertexOutput anim = animation(input);

		output.WNormal = mul((float3x3)InverseTransposeWorldMatrix, anim.WNormal);
		output.WPosition = mul(worldspace, anim.WPosition);
		output.WTexCoord = input.TexCoord;
		output.WVPPosition = mul(worldViewProjection, anim.WPosition);
		output.WTangent = mul((float3x3)InverseTransposeWorldMatrix, anim.WTangent);
	}
	// If there are not skeleton/animation
	else
	{
		output.WVPPosition = mul(worldViewProjection, float4(input.Position, 1.0f));
		output.WPosition = mul(worldspace, float4(input.Position, 1.0f));
		output.WTexCoord = input.TexCoord;
		output.WNormal = mul((float3x3)InverseTransposeWorldMatrix, input.Normal);
		output.WTangent = mul((float3x3)InverseTransposeWorldMatrix, input.Tangent);
	}

	// Determine view direction based onm cam position and position vertex
	output.ViewDir = cameraPosition.xyz - output.WPosition.xyz;
	output.ViewDir = normalize(output.ViewDir);

	return output;
}