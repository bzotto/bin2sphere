# bin2sphere

A utility for producing vintage Sphere cassette tape data blocks. The Sphere computer systems could read and write data cassettes in a format that was specific to those machines. Documentation on the format is provided in the **SPHERE_FORMAT.md** file in this repo. 

The sole program file builds into a `bin2sphere` tool. No makefile is provided, you can just `cc` up the file directly:

     cc main.c -o bin2sphere

You use the utility by giving it the name of the input binary data file, the desired output file, and the two-character block name. 

