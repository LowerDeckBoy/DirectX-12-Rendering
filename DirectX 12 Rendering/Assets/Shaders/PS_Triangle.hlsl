
struct PS_Input
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

float4 PS(PS_Input pin) : SV_TARGET
{
	return pin.Color;
}