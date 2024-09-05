
# ReverseX : For reversing large files

 ## **Environment and setup**:
   - Make sure to run these programmes on a linux machine.
   - Directory Structure of the assignment is
     - **`ReverseX/`**: Root directory for the assignment.
       - **`reverseFile.cpp`**: Source file reversing files. 
       - **`checkReverse.cpp`**: Source file for checking if two files are reversal of each other or not.
       - **`README.md`**: Documentation file providing details about the project.
   - Open the terminal in the directory containing the files, `reverseFile.cpp` , `checkReverse.cpp`.

 ## Reverse File
   ## Description and execution
   - Along with the main `c++` files, a python script is also provided to generate a file of desired size. Check the file for more understanding.
   - This problem requires us to reverse the content of a text file with the help of a flag variable.
   - If value of flag variable is 0, reverse the whole file.
   - If value of flag variable is 1, two seperate indices should be provided and we have to ensure that these indices are valid. We need to reverse the contents of file from 0th index to si - 1 index, and from ei + 1 to eof.
   - Output of the program will be stored in the directory created as `OutputFolder` and the name would be `x_<input_file_name>.txt` where x is the flag provided.
   - Make sure that the file provided is absolute path of the file.
   - To run the programme, open the terminal in directory and type the following command:
     ```bash
     g++ reverseFile.cpp
   - Run the compiled file (flag 0).
      ```bash
      ./a.out ./<absolute_file_path>.txt 0
   - For flag 1
       ```bash
      ./a.out ./<absolute_file_path>.txt 1 x y
   - Make sure that x and y are valid indices and are non negative integers.
     
## Program Flow
 - Program first validates the number of arguments passed to it and ensures it's validity.
 - It tries to find the file based on the path provided.
 - This program can reverse the file greater than the size of the ram, as it takes the file in chunks of 1 MB.
 - This program is made to reverse large files swiftly as it works mostly on system calls.
 - To efficiently manage the resources, a class for `ResourceManager` is designed to keep track of all the resources that are opened.
 - For flag 0 we take the last chunk and reverse it and we start writing it to destination file.
 - For flag 1, we take last chunk from 0 to si -1 and write it to destination file, from si to ei we write the file as it is. Then for last part from ei + 1 to EOF, take the last chunk and write it on the destination file.

## Check for reversal
   ## Description and execution
   - This problem gives us the path of new file, old file and directory to ensure if the contents of new file is reverse of the old file or not
   - The program provided will check for the flag 0 of the previous problem.
   - This program will check if the directory is valid, files are reversed and file sizes are same or not.
   - This program will also check the permission of read,write and execute for owner, group and others for the new file, old file and the directory.
   - To run the programme, open the terminal in directory and type the following command:
        ```bash
        g++ checkreverse.cpp
   - Run the compiled file (flag 0).
      ```bash
      ./a.out ./<new_file_path>.txt ./<old_file_path>.txt ./<dir_path>
   - Output will be 30 lines which will indicate:
        - If the directory is created.
        - If the files are reversed.
        - If the file sizes are same or not.
        - Following will be displaye for both the 2 files and the directory.
        - Read, Write and Execute for owner.
        - Read, Write and Execute for group.
        - Read, Write and Execute for others.
      
     
   ## Program Flow 
   - The program will take input as command line argument for the path of new file, old file and the directory.
   - The program will display errors if path provided is wrong.
   - It will check from the last chunk of old file and starting chunk from new chunk and will also compare their sizes.

     ## General Assumptions
   - Path provided for both the files and the directory is valid.
   - If the path provided to the files are not valid, the execution will stop.
   - If the path for the directory is not valid, the program will indicate that the directory is not created and all the permission will be set to No.
   - If the path to directory points to some other directory which is not created from q1, it will display that the directory is created and will show its permissions.

