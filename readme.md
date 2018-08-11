# ConwayScript

ConwayScript is an esoteric programming language based on the Conway's Game of Life.

## Specification

The interpreter has the usual automaton of Conways' Game of Life.
Also 19 oracle cells are fixed.
The 8 + 8 cells of them are to be read/written as data of input and output.
The 3 cells of them are for switchs of oracles, which are INPUT, OUTPUT and HALT.
For example, if a switch cell of INPUT is living, 1 byte is read from stdin and written to states of corresponding 8 cells.
