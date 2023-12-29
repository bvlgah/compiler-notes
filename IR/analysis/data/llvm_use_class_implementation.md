# Analysis of `llvm::Use`'s Implementation

## `llvm::Use`

[`llvm::Use`][1] represents use of a value and the user with the the following
defition (as of LLVM 15.0.7):

```
// With irrelavant information omitted
class Use {
public:
  void swap(Use &RHS);

private:
  Value *Val = nullptr;
  Use *Next = nullptr;
  Use **Prev = nullptr;
  User *parent = nullptr;

  void addToList(Use **List);
  void removeFromList();
}
```

It says in [llvm/IR/Use.h][1]

> A Use represents the edge between a Value definition and its users.
>
> This is notionally a two-dimensional linked list. It supports traversing
> all of the uses for a particular value definition. It also supports jumping
> directly to the used value when we arrive from the User's operands, and
> jumping directly to the User when we arrive from the Value's uses.

At first glance, I thought `Use` can be used to construct a doubly linked list
of which an instance of `Use` represents a node while `Prev` refers to the
previous node and `Next` points to the next, assuming the both pointers store
addresses to intances of `Use`. However, `Prev` is of type `Use **` instead of
`Use *`.

[1]: https://github.com/llvm/llvm-project/blob/llvmorg-15.0.7/llvm/include/llvm/IR/Use.h
