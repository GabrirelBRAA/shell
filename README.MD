# Shell made with C
It has basic functions:
- Its able to access /bin and usr/bin paths by default
- commands typed will trigger a search for respective binaries and the execution of these as a separate process
- You can redirect the output by using `>` Ex: `$ls > output.txt`
- You can add more paths by using `$path <new_path1> <new_path2> ...`
- You can change the current directory with `cd`.
- `exit` exits the shell.

# Build
You should be able to build it with any compiler on a Unix environment. Ex: `$gcc galsh.c`: