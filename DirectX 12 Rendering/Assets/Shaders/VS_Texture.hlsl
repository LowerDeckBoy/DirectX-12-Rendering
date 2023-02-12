
struct VS_Input
{
    float4 Position : POSITION;
    float2 UV : TexCoord;
};

struct VS_Output
{
    float4 Position : SV_POSITION;
    float2 UV : TexCoord;
};

VS_Output VS(VS_Input vin)
{
    VS_Output output = (VS_Output) 0;
	
    //output.Position = float4(vin.Position, 1.0f);
    output.Position = vin.Position;
    output.UV = vin.UV;
    
    return output;
}