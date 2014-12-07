__kernel void TraceNaive(__global float4* triangles,
	__global float4* rays,
	__global float4* results,
	int trianglesCount,
	int raysCount)
{
	int i = get_global_id(0);
	if (i >= raysCount)
	{
		return;
	}

	float4 o = rays[i * 2 + 0];
	float4 d = rays[i * 2 + 1];

	bool hit = true;
	int id = -1;
	float bu = 0.0f, bv = 0.0f;
	float dist = d.w;
	
	for (int n = 0; n < trianglesCount; n++)
	{
		float4 v = triangles[n * 3];
		float4 p = triangles[n * 3 + 1];
		float4 q = triangles[n * 3 + 2];

		float o_z = v.w - o.x * v.x - o.y * v.y - o.z * v.z;
		float i_z = 1.0f / (d.x * v.x + d.y * v.y + d.z * v.z);
		float t = o_z * i_z;

		if (t > o.w && t < dist)
		{
			float o_x = p.w + o.x * p.x + o.y * p.y + o.z * p.z;
			float d_x = d.x * p.x + d.y * p.y + d.z * p.z;
			float u = o_x + t * d_x;

			if (u >= 0.0f && u <= 1.0f)
			{
				float o_y = q.w + o.x * q.x + o.y * q.y + o.z * q.z;
				float d_y = d.x * q.x + d.y * q.y + d.z * q.z;
				float v = o_y + t * d_y;

				if (v >= 0.0f && u + v <= 1.0f)
				{
					dist = t;
					bu = u;
					bv = v;
					id = n;
				}
			}
		}
	}

	results[i] = (float4)(bu, bv, dist, as_float(id));
}

struct KDNode
{
	union
	{
		float split;
		unsigned int prim_offset;
	};

	union
	{
		unsigned int flags;
		unsigned int prim_count;
		unsigned int above_child;
	};
};

#define SPATIAL_STACK_SIZE 32
struct KDStackNode
{
	unsigned int node;
	float near, far;
};

__kernel void TraceSpatial(__global float4* triangles,
	__global float4* rays,
	__global float4* results,
	__global struct KDNode* nodes,
	__global unsigned int* indices,
	float4 boundsMin,
	float4 boundsMax,
	int trianglesCount,
	int raysCount)
{
	int i = get_global_id(0);
	if (i >= raysCount)
	{
		return;
	}

	float4 o = rays[i * 2 + 0];
	float4 d = rays[i * 2 + 1];
	float4 inv = native_recip(d);

	struct KDStackNode stack[SPATIAL_STACK_SIZE];
	unsigned int stack_ptr = 0;

	{
		float4 v1 = (boundsMin - o) * inv;
		float4 v2 = (boundsMax - o) * inv;
		float4 near = min(v1, v2);
		float4 far = max(v1, v2);
		float enter = max(near.x, max(near.y, near.z));
		float exit = min(far.x, max(far.y, far.z));

		if (exit > 0.0f && enter < exit)
		{
			stack[stack_ptr].node = 0;
			stack[stack_ptr].near = enter;
			stack[stack_ptr].far = exit;
			stack_ptr++;
		}
	}

	int id = -1;
	float bu = 0.0f, bv = 0.0f;
	float dist = d.w;

	unsigned int node;
	while (stack_ptr != 0)
	{
		stack_ptr--;
		node = stack[stack_ptr].node;
		float near = stack[stack_ptr].near;
		float far = stack[stack_ptr].far;

		while ((nodes[node].flags & 3) != 3)
		{
			int axis = (nodes[node].flags & 3);
			float hitpos;
			int below_first;
			unsigned int first, second;

			switch (axis)
			{
			case 0:
				hitpos = (nodes[node].split - o.x) * inv.x;
				below_first = (o.x < nodes[node].split) || (o.x == nodes[node].split && d.x >= 0.0f);
				break;

			case 1:
				hitpos = (nodes[node].split - o.y) * inv.y;
				below_first = (o.y < nodes[node].split) || (o.y == nodes[node].split && d.y >= 0.0f);
				break;

			case 2:
				hitpos = (nodes[node].split - o.z) * inv.z;
				below_first = (o.z < nodes[node].split) || (o.z == nodes[node].split && d.z >= 0.0f);
				break;
			}

			if (below_first)
			{
				first = node + 1;
				second = nodes[node].above_child >> 2;
			}
			else
			{
				first = nodes[node].above_child >> 2;
				second = node + 1;
			}
			
			if (hitpos > far || hitpos < 0.0f)
			{
				node = first;
			}
			else if (hitpos < near)
			{
				node = second;
			}
			else
			{
				stack[stack_ptr].node = second;
				stack[stack_ptr].near = hitpos;
				stack[stack_ptr].far = far;
				stack_ptr++;

				node = first;
				far = hitpos;
			}
		}

		unsigned int prims_num = (nodes[node].prim_count >> 2);

		if (prims_num > 0)
		{
			__global unsigned int *prims_ids = &indices[nodes[node].prim_offset];

			for (unsigned int i = 0; i < prims_num; i++)
			{
				float4 v = triangles[prims_ids[i] * 3 + 0];
				float4 p = triangles[prims_ids[i] * 3 + 1];
				float4 q = triangles[prims_ids[i] * 3 + 2];

				float o_z = v.w - o.x * v.x - o.y * v.y - o.z * v.z;
				float i_z = 1.0f / (d.x * v.x + d.y * v.y + d.z * v.z);
				float t = o_z * i_z;

				if (t > o.w && t < dist)
				{
					float o_x = p.w + o.x * p.x + o.y * p.y + o.z * p.z;
					float d_x = d.x * p.x + d.y * p.y + d.z * p.z;
					float u = o_x + t * d_x;

					if (u >= 0.0f && u <= 1.0f)
					{
						float o_y = q.w + o.x * q.x + o.y * q.y + o.z * q.z;
						float d_y = d.x * q.x + d.y * q.y + d.z * q.z;
						float v = o_y + t * d_y;

						if (v >= 0.0f && u + v <= 1.0f)
						{
							dist = t;
							bu = u;
							bv = v;
							id = i;
						}
					}
				}
			}

			if (dist < far)
			{
				break;
			}
		}
	}

	results[i] = (float4)(bu, bv, dist, as_float(id));
}