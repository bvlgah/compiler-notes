# Managed Static Objects

I am reading the source code of LLVM 14. Since LLVM is a very popular and
involving project, the following content is subject to change.

LLVM uses [`ManagedStatic`][1] to create static objects on demand in order to
reduce startup time. These objects are destroyed by calling [`llvm_shutdown`][2]
or using [a helper class][3].

## ManagedStatic

An onject of class `ManagedStatic` is somewhat like a node of a linked list, and
it holds a pointer to the next node. LLVM uses one mutex and [double-checked
locking][4] to assure the thread safety of [initializing an static object][5].
The same mutex is used to maintain the list of managed objects. In my opinion,
an individual metux for each managed static object can reduce lock contention at
the cost of more memory space. And I think it can further reduce the duration
of the lock being hold by using compare-and-swap to insert a node into the list.

## llvm\_shutdown

I considered the function `llvm_shutdown` not thread-safe at first glance. If
this function is called in multiple threads, it may free the same memory more
than once (double free) or leave the nodes from some point in the middle to the
end of the list undestroyed (memory leak). I think it helps to use an atomic
pointer to refer to the head of the list. After it is successfully exchanged
with `nullptr` by one thread, all nodes will be freed in that thread. However,
if one thread is using other LLVM APIs, and another one is calling
`llvm_shutdown`, there will still be some of so-called managed static objects
leaked. As said in the [comment][6], `llvm_shutdown` should be the last used LLVM
API. Therefore, it is not as necessary to make `llvm_shutdown` thread-safe alone
as I considered at first.

[1]: https://github.com/llvm/llvm-project/blob/f28c006a5895fc0e329fe15fead81e37457cb1d1/llvm/include/llvm/Support/ManagedStatic.h#L83

[2]: https://github.com/llvm/llvm-project/blob/f28c006a5895fc0e329fe15fead81e37457cb1d1/llvm/include/llvm/Support/ManagedStatic.h#L114

[3]: https://github.com/llvm/llvm-project/blob/f28c006a5895fc0e329fe15fead81e37457cb1d1/llvm/include/llvm/Support/ManagedStatic.h#L118

[4]: https://en.wikipedia.org/wiki/Double-checked_locking

[5]: https://github.com/llvm/llvm-project/blob/f28c006a5895fc0e329fe15fead81e37457cb1d1/llvm/lib/Support/ManagedStatic.cpp#L27

[6]: https://github.com/llvm/llvm-project/blob/f28c006a5895fc0e329fe15fead81e37457cb1d1/llvm/lib/Support/ManagedStatic.cpp#L74
