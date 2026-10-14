// The heap functions of VC6's <algorithm>, with a predicate, under their parameter names, which decide the order of
// their stack slots where they are expanded.
#ifndef HEAP_H
#define HEAP_H

template <class _Ty, class _Pd, class _Pr>
inline void push_heap_hole(_Ty* _F, _Pd _H, _Pd _J, _Ty _V, _Pr _P)
{
    for (_Pd _I = (_H - 1) / 2; _J < _H && _P(*(_F + _I), _V); _I = (_H - 1) / 2)
        *(_F + _H) = *(_F + _I), _H = _I;
    *(_F + _H) = _V;
}

template <class _Ty, class _Pd, class _Pr>
inline void push_heap_last(_Ty* _F, _Ty* _L, _Pr _P, _Pd*)
{
    push_heap_hole(_F, _Pd(_L - _F - 1), _Pd(0), _Ty(*(_L - 1)), _P);
}

template <class _Ty, class _Pr>
inline void push_heap(_Ty* _F, _Ty* _L, _Pr _P)
{
    push_heap_last(_F, _L, _P, (int*)0);
}

template <class _Ty, class _Pd, class _Pr>
inline void adjust_heap(_Ty* _F, _Pd _H, _Pd _N, _Ty _V, _Pr _P)
{
    _Pd _J = _H;
    _Pd _K = 2 * _H + 2;
    for (; _K < _N; _K = 2 * _K + 2) {
        if (_P(*(_F + _K), *(_F + (_K - 1))))
            --_K;
        *(_F + _H) = *(_F + _K), _H = _K;
    }
    if (_K == _N)
        *(_F + _H) = *(_F + (_K - 1)), _H = _K - 1;
    push_heap_hole(_F, _H, _J, _V, _P);
}

template <class _Ty, class _Pd, class _Pr>
inline void pop_heap_into(_Ty* _F, _Ty* _L, _Ty* _X, _Ty _V, _Pr _P, _Pd*)
{
    *_X = *_F;
    adjust_heap(_F, _Pd(0), _Pd(_L - _F), _V, _P);
}

template <class _Ty, class _Pr>
inline void pop_heap(_Ty* _F, _Ty* _L, _Pr _P)
{
    pop_heap_into(_F, _L - 1, _L - 1, _Ty(*(_L - 1)), _P, (int*)0);
}

#endif
