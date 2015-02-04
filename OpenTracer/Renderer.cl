#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable

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

// Uncomment to render statistics
//#define RENDER_STATISTICS
//#define MEMORY_STATISTICS

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
	float pad;
};

__kernel void TraceSpatial(__global float4* triangles,
	__global float4* rays,
	__global float4* results,
	__global struct KDNode* nodes,
	__global unsigned int* indices,
	float4 boundsMin,
	float4 boundsMax,
	int trianglesCount,
	int raysCount,
	int2 dimensions)
{
	int i = get_global_id(0);
	int j = get_global_id(1);
	if (i >= dimensions.x || j >= dimensions.y)
	{
		return;
	}

	int k = i + j * dimensions.x;

	float4 o = rays[k * 4 + 0];
	float4 d = rays[k * 4 + 1];
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

#ifdef RENDER_STATISTICS
	int visited = 0;
	int interiors = 0;
	int leaves = 0;
#elif defined MEMORY_STATISTICS
	int globalop = 0;
	int privateop = 0;
#endif

	unsigned int node;
	while (stack_ptr != 0)
	{
		stack_ptr--;
		node = stack[stack_ptr].node;
		float near = stack[stack_ptr].near;
		float far = stack[stack_ptr].far;

		uint axis = (nodes[node].flags & 3);
		uint above_child = (nodes[node].above_child >> 2);
		float split = nodes[node].split;

#ifdef MEMORY_STATISTICS
		globalop += 3;
		privateop += 3;
#endif

		while (axis != 3)
		{
#ifdef RENDER_STATISTICS
			visited++;
			interiors++;
#endif

			float hitpos;	 
			int below_first;
			unsigned int first, second;

			float orig_tmp;
			float idir_tmp;

			switch (axis)
			{
			case 0:
				orig_tmp = o.x;
				idir_tmp = inv.x;
				break;

			case 1:
				orig_tmp = o.y;
				idir_tmp = inv.y;
				break;

			case 2:
				orig_tmp = o.z;
				idir_tmp = inv.z;
				break;
			}

			hitpos = (split - orig_tmp) * idir_tmp;
			below_first = (orig_tmp < split) || (orig_tmp == split && idir_tmp >= 0.0f);
			
			if (below_first)
			{
				first = node + 1;
				second = above_child;
			}
			else
			{
				first = above_child;
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
#ifdef MEMORY_STATISTICS
				privateop += 3;
#endif

				node = first;
				far = hitpos;
			}

			axis = (nodes[node].flags & 3);
			above_child = (nodes[node].above_child >> 2);
			split = nodes[node].split;
#ifdef MEMORY_STATISTICS
			globalop += 3;
#endif
		}

		unsigned int prim_offset = nodes[node].prim_offset;
#ifdef MEMORY_STATISTICS
		globalop += 1;
#endif
		unsigned int prims_num = above_child;

#ifdef RENDER_STATISTICS
		visited++;
		leaves += prims_num;
#endif

		if (prims_num > 0)
		{
			__global unsigned int *prims_ids = &indices[prim_offset];

			for (unsigned int n = 0; n < prims_num; n++)
			{
				// Don't trash cache by reading index through it
				int tri_idx = prims_ids[n] * 3;

				float4 r = triangles[tri_idx + 0];
				float4 p = triangles[tri_idx + 1];
				float4 q = triangles[tri_idx + 2];
#ifdef MEMORY_STATISTICS
				globalop += 4;
#endif
				float o_z = r.w - o.x * r.x - o.y * r.y - o.z * r.z;
				float i_z = 1.0f / (d.x * r.x + d.y * r.y + d.z * r.z);
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

#ifdef RENDER_STATISTICS
	results[k] = (float4)((float)visited * 0.01f, (float)interiors * 0.01f, (float)leaves * 0.01f, 1.0f);
#elif defined MEMORY_STATISTICS
	results[k] = (float4)((float)globalop * 0.004f, (float)privateop * 0.004f, 0.0f, 1.0f);
#else
	results[k] = (float4)(bu, bv, dist, as_float(id));
#endif

/*	int i = get_global_id(0);
	int j = get_global_id(1);
	if (i >= dimensions.x || j >= dimensions.y)
	{
		return;
	}
	
	int k = i + j * dimensions.x;

	float4 o = rays[k * 2 + 0];
	float4 d = rays[k * 2 + 1];
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
	
#ifdef RENDER_STATISTICS
	int visited = 0;
	int interiors = 0;
	int leaves = 0;
#endif

	unsigned int node;
	while (stack_ptr != 0)
	{
		stack_ptr--;
		node = stack[stack_ptr].node;
		float near = stack[stack_ptr].near;
		float far = stack[stack_ptr].far;
		
		uint axis = (nodes[node].flags & 3);
		uint above_child = (nodes[node].above_child >> 2);
		float split = nodes[node].split;
		unsigned int prim_offset = nodes[node].prim_offset;

		while (axis != 3)
		{
#ifdef RENDER_STATISTICS
			visited++;
			interiors++;
#endif

			float hitpos;
			int below_first;
			unsigned int first, second;

			switch (axis)
			{
			case 0:
				hitpos = (split - o.x) * inv.x;
				below_first = (o.x < split) || (o.x == split && d.x >= 0.0f);
				break;

			case 1:
				hitpos = (split - o.y) * inv.y;
				below_first = (o.y < split) || (o.y == split && d.y >= 0.0f);
				break;

			case 2:
				hitpos = (split - o.z) * inv.z;
				below_first = (o.z < split) || (o.z == split && d.z >= 0.0f);
				break;
			}

			if (below_first)
			{
				first = node + 1;
				second = above_child;
			}
			else
			{
				first = above_child;
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

			axis = (nodes[node].flags & 3);
			above_child = (nodes[node].above_child >> 2);
			split = nodes[node].split;
			prim_offset = nodes[node].prim_offset;
		}

		unsigned int prims_num = above_child;

#ifdef RENDER_STATISTICS
		visited++;
		leaves += prims_num;
#endif

		if (prims_num > 0)
		{
			__global unsigned int *prims_ids = &indices[prim_offset];

			for (unsigned int n = 0; n < prims_num; n++)
			{
				float4 v = triangles[prims_ids[n] * 3 + 0];
				float4 p = triangles[prims_ids[n] * 3 + 1];
				float4 q = triangles[prims_ids[n] * 3 + 2];

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

#ifdef RENDER_STATISTICS
	results[k] = (float4)((float)visited * 0.01f, (float)interiors * 0.01f, (float)leaves * 0.01f, 1.0f);
#else
	results[k] = (float4)(bu, bv, dist, as_float(id));
#endif*/
}