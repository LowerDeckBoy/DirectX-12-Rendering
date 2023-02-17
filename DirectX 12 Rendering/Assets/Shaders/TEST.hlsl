
cbuffer cbPerObject : register(b0)
{
    float4x4 WVP;
    float4 padding[15];
}

struct VS_Input
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

VS_Output VS(VS_Input vin)
{
    VS_Output output = (VS_Output) 0;
	
    output.Position = mul(WVP, float4(vin.Position, 1.0f));
    //output.Position = vin.Position; -> "glues" object to the screen
    output.Color = vin.Color;
    
    return output;
}

struct PS_Input
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

float4 PS(PS_Input pin) : SV_TARGET
{
    return pin.Color;
}