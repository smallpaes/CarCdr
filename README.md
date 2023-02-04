
# ADT: Lisp/CarCdr

When storing real-world data, lists containing other lists are ubiquitous e.g. : (0 (1 2) 3 4 5).
In fact many functional (or at least partly functional) languages are based around the idea that all
data are, conceptually, nested lists. Examples of such languages include Haskell and, of interest
here, [Lisp](https://katherineoelsner.com/)


One of the main operations of these languages is the ability to extract the front (head) or
remainder (tail) of the list very efficiently. A traditional linked list is not good for this, since it
would require work to untangle the head (it has a pointer to the rest of the list) and it’s unclear
how a list-within-a-list would be stored.
For Lisp, a better structure was developed, which is slightly more complex than a traditional
linked list, but which makes the extraction of the head (and remainder) simple and fast.

The atomic elements (the integers) are stored as ‘leaf’ nodes with both car and cdr pointers set
to NULL. The data structure used to store the atom and also car and cdr pointers is known as a
cons (short for constructor), due the to process by which we build lists.
So, here, we’re interested in recreating this data structure, allowing the user to build lists (the
cons operation), extract the head and remainder of lists (the car and cdr operations) and other
associated functions such as copying a list or counting the number of elements in it.




## Used By

This project is used by the following projects:

- [NUCLEI](https://github.com/)


## Installation

The following instructions will get you a copy of the project and all the setting needed to test and compile on your local machine.


### Prerequisites

- [make (Windows)](https://community.chocolatey.org/packages/make)
- [Valgrind (Not Supported on certain operatiing systems or versions)](https://valgrind.org/downloads/?src=www.discoversdk.com)
- GCC / Clang (Linux and Windows)


### Compile & Test

**Clone and navigate to the project**
```bash
  git clone https://github.com/smallpaes/CarCdr.git
  cd CarCdr
```

**Build and test**

In this C project, we are using `make` to compile from several sources files into one executable. Please refer to `Makefile` for all the predefined rules and configuration.

- Compile with essestial flags and sanitizers.

```bash
  make testlinked_s
```

- Compile with essestial flags and sanitizers while producing useful debugging information.

```bash
  make testlinked_v
```

- Compile into production-ready code.

```bash
  make testlinked
```

- Compile for all the stages mentioned above: This will generate three different executables.

```bash
  make all
```

- Run the test (If Valgrind is supported): This will compile the code, run all the test cases, and run Valgrind to detect memory-related errors.

```bash
  make run
```

- Run the test (If Valgrind is not supported): This will compile the code, run all the test cases, and skip the  Valgrind check.

```bash
  make run_no_val
```

- Clean up all the executables generated.

```bash
  make clean
```
## ADT Interface

Please refer to `lisp.h` for more detail.

| Function name  | Meaning                                         |
|-----------------|-------------------------------------------------|
| lisp_atom            | Appends an element to the end of the list       |
| lisp_cons             | Returns a new list containing the car as one list input and the cdr as another list input          |
| lisp_car           | Returns the car (1st) component of the input list         |
| lisp_cdr         | Returns the cdr (all but the 1st) component of the input list |
| lisp_getval         | Returns the data/value stored in the cons input list |
| lisp_copy           | Returns a deep copy of the input list         |
| lisp_tostring         | Returns stringified version of list |
| lisp_fromstring         | Builds a new list based on the input string  |
| lisp_isatomic         | Returns a boolean depending on whether the input list points to an atom  |
| lisp_length         | Returns number of components in the list  |
| lisp_list         | Returns a new list from a set of input  lists  |
| lisp_reduce         | Allows a user defined function input to be applied to each atom in the input list  |
| lisp_free         | Clears up all space used |


### Available data structures
 Name            | Header          | Storage type	         | Requires malloc/free |
|-----------------|-----------------|---------------------|----------------------|
| lisp       | specific.h       | Objects (void*)	     | Yes                  |

> Data type of the value being stored in lisp could be customized(Default: `int`):
#### **`lisp.h`**
``` c
typedef <customized_data_type> atomtype;

```


### Examples

``` c
lisp *l1 = lisp_fromstring("(1 2 3 4)");
lisp *l2 = lisp_copy(l1);
lisp *l3 = lisp_cons(l1, l2);
lisp_free(l1);
lisp_free(l2);
lisp_free(l3);
```

## Acknowledgements

 - This exercise topic was given by the instructor at the University of Bristol: [Neill Campbell](https://github.com/csnwc) from the course of Programming in C.
 - [Toolkits: General](https://github.com/csnwc/ADTs/tree/main/General)