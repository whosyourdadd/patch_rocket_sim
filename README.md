User Guide
================
- 1. git clone the **"patch_rocket_sim"** under your **rocket-sim/**
- 2. Backup your original Makefile to different name.
- 3. Create a new Makefile under **rocket-sim/**, and add **" -include patch_rocket_sim/Makefile "** in your new Makefile.
- 4. Modify the **rocket-sim/execution.cpp**. This step purpose is creating a **rocket_sim_main thread** and **reader thread**.
+ [Please follow this link in detail to modify your execution.cpp](https://gist.github.com/ldotrg/88a1b4c3142bd0cfef069f259e5f6e70)
- 5. **make run**
-Now, The highest record is about 500 seconds on MacBook Air (13-inch, Early 2015) 1.6G Core i5