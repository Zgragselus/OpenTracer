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
						int2 dim,
						float4 dx,
						float4 dy)
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
	rayBuffer[4 * id + 0] = (float4)(origin.x, origin.y, origin.z, near);
	rayBuffer[4 * id + 1] = (float4)(dir.x, dir.y, dir.z, far);
	rayBuffer[4 * id + 2] = (float4)(dx.x, dx.y, dx.z, 0.0f);
	rayBuffer[4 * id + 3] = (float4)(dy.x, dy.y, dy.z, 0.0f);
}