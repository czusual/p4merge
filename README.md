# p4merge
 Merge two or more P4 programs

## Get started
1. This project is an extension of the p4 compiler:p4c, so please first enable the p4c environment according to their instructions (https://github.com/p4lang/p4c)  *For my own issue, I cannot enable the latest version, therefore I use a virtual machine provided by p4 official tutorial(I will give the address later), so I recommend downloading the 2018.12.x version of p4c which is the same as the VM for now*
2. Create a folder named **"extensions"** in /p4c, which has the same level as "frontends""ir" etc.
3. Copy the folder "p4merge" here to /p4c/extensions
4. Copy the program files in "/p4c/extensions/p4merge/frontends" to "p4c/frontends/p4" , modify the CMakelists.txt in "p4c/frontends" to add these files ([P4_FRONTEND_SRCS][P4_FRONTEND_HDRS]). So as to files in "/p4c/extensions/p4merge/ir". Replace files in p4c which have the same names in our project.  *p4c provides CMake variable to add extended frontends source files, but I build fail,so this time we add code to the original folders*
5. Build: go to /p4c/build (If you don't have it , mkdir build)
```
cmake .. -DCMAKE_BUILD_TYPE=DEBUG
make -j4
sudo make install
```
  The number"4" in "make -j4" means to the number of CPU. 
  
  6. Now you can use `p4merge --help` in the shell, the command `p4merge` requests for 2 or more input files *For now, more than 2 is meaningless*

## List
1. Command line interface  √
2. Top4      √
3. Rename IR::nodes     *working on more complex conditions*
4.  Naive merge  √
5.  Optimize   *working on*


