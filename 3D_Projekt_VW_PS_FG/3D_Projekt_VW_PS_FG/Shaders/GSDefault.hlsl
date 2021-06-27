struct GSOutput
{
	float4 WVPPosition : SV_POSITION;
	float4 WPosition : WPOSITION;
	float2 WTexCoord : TEXCOORD;
	float3 WNormal : NORMAL;
	float3 WTangent : TANGENT;
	float3 ViewDir : TEXCOORD1;
};

cbuffer cBufferCamera : register(b0)
{
	float3 cameraPosition;
	float padding;
};

cbuffer cBufferMaterial : register(b1)
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

/*
Definerar mängden verticer per polygon som Geometry shadern hanterar
*/
[maxvertexcount(3)]
void GSMain(triangle GSOutput input[3], inout TriangleStream< GSOutput > output)
{
	/*
	Define two of three edges on the polygon.
	
	Create a vector from the vertex on index 0 to the camera.

	Cross the two edges to get the normal of the polygon.
	*/
	float3 V = input[1].WPosition - input[0].WPosition;
	float3 U = input[2].WPosition - input[0].WPosition;
	float3 W = cameraPosition - input[0].WPosition;

	float3 normal = normalize(cross(V, U));

	/*
	Check if the object is an obj file or not.

	Check if the dot product is greater than or equal to 0, if so, return the vertices. If the object is an obj, flip the camera vector.

	If the object is an obj, return them in a backwards order since objs are right handed.
	Else, return them normally.
	*/
	if (isObj) {
		if (dot(normal, -W) >= 0) {
			output.Append(input[2]);
			output.Append(input[1]);
			output.Append(input[0]);
		}
	}
	else {
		if (dot(normal, W) >= 0) {
			output.Append(input[0]);
			output.Append(input[1]);
			output.Append(input[2]);
		}
	}
}