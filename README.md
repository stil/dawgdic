C++ Header Library of DAWG Dictionary Algorithm
==========

This project provides a library **dawgdic** for building and accessing dictionaries implemented with directed acyclic word graphs (DAWG).

A dawg is constructed by minimizing a trie as a deterministic finite automaton (DFA), and thus the dawg has an advantage in memory usage. In addition, **dawgdic** uses a double-array as a base data structure, so its retrieval speed is as fast as that of **Darts**, a library for building and accessing double-array tries.

* Documentation
  * Readme: Please read this document first.
    * HowToBuild: Examples to build a dictionary from a sorted lexicon.
    * HowToLookup: Examples to lookup keys in a dictionary.
    * HowToComplete: Examples to complete keys from a given prefix. 
  * Interface: This document describes class interfaces.
  * Algorithm: This document describes algorightms (to be written). 
* Related projects
  * A Python wrapper. (https://github.com/kmike/DAWG) 
