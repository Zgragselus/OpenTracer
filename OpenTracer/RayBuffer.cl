__kernel void Primary(	__global float4* rayBuffer,
						float4 origin,
						float4 forward,
						float4 right,
						float4 up,
						float2 halfDim,
						float2 invHalfDim,
						float near,
						float far,
						float aspect,
						int2 dim)
{
	int i = get_global_id(0);
	int j = get_global_id(1);

	if (i >= dim.x || j >= dim.y)
	{
		return;
	}

	float x = ((float)i - halfDim.x);
	float y = ((float)j - halfDim.y) * aspect;

	float4 dir = normalize(forward + x * right + y * up);

	int id = j * dim.x + i;
	rayBuffer[2 * id + 0] = (float4)(origin.x, origin.y, origin.z, near);
	rayBuffer[2 * id + 1] = (float4)(dir.x, dir.y, dir.z, far);
}