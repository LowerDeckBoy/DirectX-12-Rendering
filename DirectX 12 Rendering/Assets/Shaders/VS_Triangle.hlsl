
struct VS_Input
{
    float4 Position : POSITION;
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
	
    //output.Position = float4(vin.Position, 1.0f);
    output.Position = vin.Position;
    output.Color = vin.Color;
    
    return output;
}