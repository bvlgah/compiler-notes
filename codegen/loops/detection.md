# Detection of Natural Loops

A Loop is usually a hot spot of a program, so there are many optimizations about
loops.

## Definition of Natural Loops

Given a CFG of a function, a natural loop is defined as follows according to the
chapter 18 of [Appel's Modern Compiler Implementation in Java second edition][
tiger_book](a.k.a. tiger book):

- A loop is a set of CFG nodes including a header.

- From any node of the loop, there is a path of directed edges leading to the
header.

- There is a path of directed edges from the header to any node of the loop.

- There is no edge from any node outside the loop to any node inside the loop
except the header.

It is implied by the definition that the header of a loop dominates every node
of the loop. And there are directed edges from nodes in the loop to the header.
Such edges are named back edges. In [LLVM's terminology][llvm_loop_terminology],
a latch is one of a loop's nodes if it has a directed edge leading to the header
of the loop. An exiting node of a loop is the one that has a directed edge to
nodes outside the loop. Here is an illustration of a loop borrowed from LLVM:

![a loop from LLVM](https://llvm.org/docs/_images/loop-terminology.svg)

A loop can be composed of only one node which is the header as well as the latch
and the exiting node.

![a loop of single node](https://llvm.org/docs/_images/loop-single.svg)

## Detection

The first step of detecting a loop is to find out the back edge. If there is a
node `l` with an edge `l -> h` and `h` dominating `l`, then the edge `l -> h` is
a back edge. `h` is the header, and `l` is a latch. It is a common practice to
merge all natural loops sharing the same loop header as LLVM does. Given a CFG
(connected basic blocks) of a function in LLVM IR and the corresponding
dominator tree, loop headers and latches can be found by iterating through all
basic blocks.

Next, we need to identify all nodes of a loop given the header and latches of
the loop.

**Theorem 1**: Successors of all nodes of a loop except the header are inside
the loop.

**Proof**. By the definition of a natural loop, the header is the only entry of
the loop. Therefore, for nodes other than the header, there are no directed
edges from nodes outside the loop to nodes in the loop.

By theorem 1, any nodes on paths starting from latches and ending at the header
(with all directed edges reversed) are inside a loop. And these nodes are the
whole set of all nodes, `S`, for a loop. Since for every node of a loop, it must
be on a path from the header, through one of latches, back to the header. As for
exiting nodes, they can be discovered by checking if they have an edge to nodes
outside the loop.

**Theorem 2**: During a depth-first search starting from the entry node of a
function, the header of an outter loop is visited before the header of an inner
loop.

**Proof**. Because nodes of an inner loop are part of an outter loop, the header
of the outter loop, `h1`, dominates the inner loop's header, `h2`. So for every
path from the entry of the function to `h2`, it must go through `h1`.

It is sometimes helpful to know inner loops or outter loops. A tree can be
constructed with a node representing a loop and having inner loops as its
children. During the depth-first search memtioned in theorem 2, a stack is
maintained to keep nested loops. Once a loop is detected, we pop off from the
stack all loops which the just detected loop is not part of.

Overall, here is a sketch of the algorithm for loop detection:

1. Do a depth-first search from the entry node of a function, and check if a
node dominates one its predecessors to identify the header and latches of a
loop. With a loop's header and latches, do a depth-first search from latches to
the header with directed edges reversed to spot all nodes of the loop. Next,
exiting nodes are recognized by testing the existence of any edge to a node not
belonging to the loop.

2. Obtain nested loops by maintaining a stack. A newly detected loop will be
pushed into the stack after all non-containing loops are popped.

An implementation can be found in [/llvm_code_example/src/Loop.cpp](
/llvm_code_example/src/Loop.cpp).

Nested loops can be printed with my implementation:

```bash
$ cmake -G Ninja -B out -S .
$ clang++ -S -emit-llvm -fno-discard-value-names -O0 \
    -o llvm_code_example/test/loop.ll llvm_code_example/test/loop.cpp
$ opt -load-pass-plugin out/llvm_code_example/LLVMHello.so \
    -passes=my-loop-printer -disable-output llvm_code_example/test/loop.ll
$ opt -load-pass-plugin out/llvm_code_example/LLVMHello.so \
    -passes=my-loop-printer -disable-output \
    llvm_code_example/test/zlib_inffast.ll
```

[tiger_book]: https://dl.acm.org/doi/book/10.5555/599718

[llvm_loop_terminology]: https://llvm.org/docs/LoopTerminology.html
