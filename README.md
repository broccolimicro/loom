# Haystack

Haystack is a collection of tools for the design and verification of
asynchronous circuits. Not all of the tools are complete. The documentation for
each tool may be found in that tool's git repository. Below lists the state of
each tool:

 - [  2%] [**chpsim**](https://github.com/nbingham1/chpsim/) is a simulator for Communicating Hardware Processes.
 - [ 50%] [**hseenc**](https://github.com/nbingham1/hseenc/) finds state space conflicts and helps to fix them with state variable assignments.
 - [100%] [**hseplot**](https://github.com/nbingham1/hseplot/) renders state graphs, petri nets, and signal transition graphs.
 - [100%] [**hsesim**](https://github.com/nbingham1/hsesim/) is a simulator for Handshaking Expansions which can efficiently elaborate the whole state space of a circuit for input to hseenc.
 - [100%] [**prsim**](https://github.com/nbingham1/prsim/) is a simulator for Production Rules.
 - [100%] [**bubble**](https://github.com/nbingham1/bubble/) for bubble reshuffling Production Rules.
 - [ 80%] [**gated**](https://github.com/nbingham1/gated/) a simple logic synthesis tool to convert arithmetic expressions to boolean logic.
 - [ 20%] [**prsize**](https://github.com/nbingham1/prsize/) is an automatic sizing program for Production Rules.

## Build

Haystack is built in two phases: libraries then binaries.

```
git submodule update --init --recursive
cd lib
make
cd ../bin
make
```

## Example

**stream_complete.hse**
```
(_Lr+,L.r-; [L.e];
*[_Lr-; L.r+; [~L.e]; _Lr+; L.r-; [L.e]])'1 ||

_Rr+,R.r-,L.e+,v0-,v1+,v2+; [R.e&~L.r];
*[[L.e & R.e & L.r]; _Rr-; R.r+; v2-; [~R.e]; _Rr+; R.r-; v0+; v1-; v2+;
  [L.e & R.e & L.r]; _Rr-; R.r+; L.e-; [~R.e]; _Rr+; R.r-; v1+; v0-;
  [~L.r]; L.e+
 ]||

(R.e+; [~R.r];
*[[R.r]; R.e-; [~R.r]; R.e+])'1
```

![stream](https://github.com/broccolimicro/haystack/assets/8902287/55b7a2dd-d651-4169-a9f9-57c9241a6687)

## License

Licensed by Cornell University under the GNU GPL Version 3 License.

Written by Ned Bingham.
Copyright © 2020 Cornell University.

A copy of the GNU GPL Version 3 License may be found in LICENSE.
