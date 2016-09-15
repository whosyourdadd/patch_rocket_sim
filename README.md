=====
User Guide
=====
1. git clone the **"patch_rocket_sim"** under your **rocket-sim/**  
2. Backup your original Makefile to different name.  
3. Create a new Makefile under **rocket-sim/**, and following content **" -include patch_rocket_sim/Makefile "** in your new Makefile.  
4. Modify the **rocket-sim/execution.cpp**. Comment the systrm pause.  
[Please follow this link in detail to modify your execution.cpp](https://gist.github.com/ldotrg/88a1b4c3142bd0cfef069f259e5f6e70)  
5. Choose the following one malloc lib  
- **make tm=ltalloc** 
-  **make tm=scalloc**
-  **make tm=glibc**
6. **./rocket-sim-exe**

## Note 
### Run simulation faster
- If you have enough memory, you can modify ** #define NUM_OF_CELL** number in ringbuffer.h for more lager memory
### Scalloc User
- Execute **./patch_rocket_sim/scalloc_config.sh** under the **rocket-sim/** before build the code.
- **make tm=scalloc**
