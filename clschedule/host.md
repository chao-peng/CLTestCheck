# Example of applying schedule amplifier to a simple OpenCL program

Add the following lines to the host code where you configure the OpenCL program

```c
// Generate a value in the range of [0,4)
int target_group = randint(4);
// Pass the value as a macro to GPU code
sprintf(clOptions,"-DTARGET_GROUP=%d", target_group); 
```
