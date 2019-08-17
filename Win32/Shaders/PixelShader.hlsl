struct PixelIn
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

float4 PS(PixelIn pin) : SV_Target
{
    return pin.Color;
}
