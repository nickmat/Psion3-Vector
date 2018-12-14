# Psion3-Vector
Vector is a CAD package for the Psion 3a, 3c and 3mx.
A version was also created for Psion Siena, but that is not included here.
It is intended to be used to create diagrams, schematics
and other types of drawings.
It has been designed from scratch to be used on the Psion
and has a number of features designed to make the most of these machines.
See the website http://www.kizzyco.com/vector
for a full description of the program.

## Notes on the program design
One of the design goals was that the user could create and modify the drawing
(and see the changes they make) in real time.
One way to achieve this was to hold all drawing data as 16 bit integers
and do all calculations (as far as possible) using integer arithmetic.
This in turn meant writing the drawing primitives,
which was mostly done using assembler.

One oddity is that angles are held as two 16 bit integers,
representing the scaled sin and cosine of the angle.
This is sufficient to do all the internal calculations with integer maths.
They only need to be converted to floating point when inputting from,
or outputting to, the user.

If there is interest, I can expand on this and make available
notes on the mathematics used in the program.

## The build process
The code in this repository currently represents the unfinished version P1.20
but, to the best of my knowledge, it should compile and run. 

The build process is fully controlled by the Vector.mak makefile,
designed to be run with the Borland make program.
An examination of the makefile will show the programs required
to compile the program.
Most of these are part of TopSpeed C and Psion's SIBOSDK development kit,
both of which are required. There may be some other freeware programs required.
