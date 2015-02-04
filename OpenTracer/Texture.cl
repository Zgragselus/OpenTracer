__kernel void ClearColor(__global float4* mTexture, float4 mColor, int mItems)
{
	int id = get_global_id(0);
	if (id >= mItems)
	{
		return;
	}
	mTexture[id] = mColor;
}