Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct PixelInput
{
    float4 WVPPosition : SV_POSITION;
    float4 WPosition : WPOSITION;
    float2 WTexcoord : TEXCOORD;
    float4 Color : COLOR;
};

float4 PSMain(PixelInput input) : SV_TARGET
{
    float4 textureColor = float4(0.0f,0.0f,0.0f,0.0f);
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, input.WTexcoord);

    // Combine the texture color and the particle color to get the final color result.
    finalColor = textureColor * input.Color;

    return finalColor;
}