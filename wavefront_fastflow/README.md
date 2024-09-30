## Fastflow Implementation

### wavefront_diagonal_ff
- Approach based on farm implementation of diagonal wavefront computation. In this implementation, **the emitter divides the diagonal computation** based on number of workers and it assigns the task at each of them. Before to send another diagonal task, it wait each workers terminate the computation

### wavefront_triangles__ff
- Approach based on farm implentation of triangular wavefront computation. In this implementation, **the matrix is divided in triangles** and each of them is sent at each worker by the emitter. Before to send triangles that is not at the same diagonal level of the others, the emitter waits that all workers terminate the computation.

### wavefront_triangles_map_ff
- Approach based on farm implentation of triangular wavefront computation. In this implementation, the matrix is divided in triangles and **each of them** is sent at each worker by the emitter. Each worker node is a map stage to parallelize the diagonal computation of each triangles. Before to send triangles that is not at the same diagonal level of the others, the emitter waits that all workers terminate the computation.

### wavefront_triangles_map_ff_comm
- Approach based on farm implentation of triangular wavefront computation. In this implementation, the matrix is divided in triangles and the **index of triangles** is sent at each worker by the emitter. Each worker node is a map stage to parallelize the diagonal computation of each triangles. Before to send triangles that is not at the same diagonal level of the others, the emitter waits that all workers terminate the computation.