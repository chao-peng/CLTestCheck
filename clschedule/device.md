# Example of applying schedule amplifier to a simple OpenCL program.

## Original program
```c
__kernel void gpuverify(__global int* A, __global int* scheduler_count){
    int buf, x, y, i, j;
    int lid = get_global_id(0);

    if (lid == 0) x = 4;
    else x = 1;
    if (lid == 0) y = 1;
    else y = 4;
	
    buf = i = 0;
    while (i < x) {
        j = 0;
        while (j < y) {
        barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
        //A[1-buf][lid] = A[buf][(lid+1)%4];
        A[(1-buf)*4+lid] = A[buf*4+(lid+1)%4];
        buf = 1 - buf;
        j++;
        }
        i++;
        }
}
```

## Instrumented program with schedule amplifier

```c
__kernel void gpuverify(__global int* A, __global int* scheduler_count){
    //init scheduler
    __local int scheduler_group_count;
    int scheduler_group_size = get_local_size(0);
    int scheduler_group_id = get_group_id(0);
    int scheduler_local_id = get_local_id(0);
    int scheduler_num_groups = get_num_groups(0);
    if (get_local_id(0)==0){
        scheduler_group_count = 0;
    }
	
    if (get_global_id(0) == 0) {
        *scheduler_count = 0;
    }
    
    barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
	
    int buf, x, y, i, j;
    int lid = get_global_id(0);

    if (lid == 0) x = 4;
    else x = 1;
    if (lid == 0) y = 1;
    else y = 4;
	
    buf = i = 0;

    if (scheduler_group_id == TARGET_GROUP || TARGET_GROUP >= scheduler_num_groups ){
        while (i < x) {
            j = 0;
            while (j < y) {
                barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
                //A[1-buf][lid] = A[buf][(lid+1)%4];
                A[(1-buf)*4+lid] = A[buf*4+(lid+1)%4];
                buf = 1 - buf;
                j++;
            }
            i++;
        }
	  atomic_add(&scheduler_group_count, 1);
        barrier(CLK_LOCAL_MEM_FENCE);
        if (scheduler_local_id == 0){
            while (scheduler_group_count != scheduler_group_size) continue;
            atomic_or(scheduler_count, 1);
            //printf("group count %d group size %d\n", group_count, group_size);
        }
    }	
    
    if (scheduler_group_id != TARGET_GROUP && TARGET_GROUP < scheduler_num_groups) {
        if (scheduler_local_id == 0){
            int proceed = atomic_min(scheduler_count, 1);
            while (proceed != 1) {/*
                int t = 0;
                for (int i = 0; i < group_size * 100; i++){
                    for (int j = 0; j < group_size; j++){
                        t += j;
                    }
                    t -= i;
                }*/
                proceed = atomic_min(scheduler_count, 1);
            }
            //printf("count: %d\n", *count);
            atomic_or(&scheduler_group_count,1);
        }
        while (atomic_min(&scheduler_group_count,1) != 1) continue;
        while (i < x) {
            j = 0;
            while (j < y) {
                barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
                //A[1-buf][lid] = A[buf][(lid+1)%4];
                A[(1-buf)*4+lid] = A[buf*4+(lid+1)%4];
                buf = 1 - buf;
                j++;
            }
            i++;
        }
    }
}
```
