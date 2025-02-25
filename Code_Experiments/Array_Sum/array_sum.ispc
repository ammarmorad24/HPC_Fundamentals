export uniform float sum_array(uniform int N, uniform float x[])
{
	uniform float sum;
	float partial = 0.0f;
	foreach (i = 0 ... N)
	{
		partial += x[i];
	}
	// reduce_add() is part of ISPC's cross-instance standard library
    // it simulates MapReduce Framwork's reduce operation
	sum = reduce_add(partial);
	return sum;
}