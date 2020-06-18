#pragma once

template <class T1, class T2, int I>
class partial_human {
};

template <class T, int I>
class partial_human<T, T*, I> {
};

template <class T, class T2, int I>
class partial_human<T*, T2, I> {
};

template <class T>
class partial_human<int, T*, 5> {
};

template <class X, class T, int I>
class partial_human<X, T*, I> {
};
