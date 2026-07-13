#pragma once

template <
  typename S,
  auto Op,
  auto E
>
concept MonoidRequirements =
  requires(S a, S b) {
    { Op(a, b) } -> std::convertible_to<S>;
    { E() } -> std::convertible_to<S>;
  };

template <typename M>
concept IsMonoid =
  requires { typename M::value_type; } &&
  requires(typename M::value_type a, typename M::value_type b) {
    { M::op(a, b) } -> std::convertible_to<typename M::value_type>;
    { M::e() } -> std::convertible_to<typename M::value_type>;
  };

template <
  typename S,
  auto Op,
  auto E
> requires MonoidRequirements<S, Op, E>
struct Monoid {
  using value_type = S;
  static S op(const S& a, const S& b) { return Op(a, b); }
  static S e() { return E(); }
};

template <
  typename S,
  typename F,
  auto Mapping,
  auto Composition,
  auto Id
>
concept LazyRequirements = requires(S x, F f, F g) {
  { Mapping(f, x) } -> std::convertible_to<S>;
  { Composition(g, f) } -> std::convertible_to<F>;
  { Id() } -> std::convertible_to<F>;
};

template <typename M>
concept IsActedMonoid =
  requires {
    typename M::value_type;
    typename M::operator_type;
  } &&
  requires(typename M::value_type x,
           typename M::value_type y,
           typename M::operator_type f,
           typename M::operator_type g) {
    { M::op(x, y) } -> std::convertible_to<typename M::value_type>;
    { M::e() } -> std::convertible_to<typename M::value_type>;
    { M::mapping(f, x) } -> std::convertible_to<typename M::value_type>;
    { M::composition(g, f) } -> std::convertible_to<typename M::operator_type>;
    { M::id() } -> std::convertible_to<typename M::operator_type>;
  };

template <
  typename S,
  auto Op,
  auto E,
  typename F,
  auto Mapping,
  auto Composition,
  auto Id
>
requires MonoidRequirements<S, Op, E> && LazyRequirements<S, F, Mapping, Composition, Id>
struct ActedMonoid {
  using value_type = S;
  using operator_type = F;
  static S op(const S& a, const S& b) { return Op(a, b); }
  static S e() { return E(); }
  static S mapping(const F& f, const S& x) { return Mapping(f, x); }
  static F composition(const F& g, const F& f) { return Composition(g, f); }
  static F id() { return Id(); }
};

template <
  typename S,
  auto Rev
>
concept ReversibleRequirements = requires(S x) {
  { Rev(x) } -> std::convertible_to<S>;
};

template <typename M>
concept IsReversibleActedMonoid =
  requires {
    typename M::value_type;
    typename M::operator_type;
  } &&
  requires(typename M::value_type x,
                typename M::value_type y,
                typename M::operator_type f,
                typename M::operator_type g) {
    { M::op(x, y) } -> std::convertible_to<typename M::value_type>;
    { M::e() } -> std::convertible_to<typename M::value_type>;
    { M::rev(x) } -> std::convertible_to<typename M::value_type>;
    { M::mapping(f, x) } -> std::convertible_to<typename M::value_type>;
    { M::composition(g, f) } -> std::convertible_to<typename M::operator_type>;
    { M::id() } -> std::convertible_to<typename M::operator_type>;
  };

template <
  typename S,
  auto Op,
  auto E,
  auto Rev,
  typename F,
  auto Mapping,
  auto Composition,
  auto Id
>
requires
  MonoidRequirements<S, Op, E> &&
  LazyRequirements<S, F, Mapping, Composition, Id> &&
  ReversibleRequirements<S, Rev>
struct ReversibleActedMonoid {
  using value_type = S;
  using operator_type = F;
  static S op(const S& a, const S& b) { return Op(a, b); }
  static S e() { return E(); }
  static S rev(const S& x) { return Rev(x); }
  static S mapping(const F& f, const S& x) { return Mapping(f, x); }
  static F composition(const F& g, const F& f) { return Composition(g, f); }
  static F id() { return Id(); }
};

template <
  typename S,
  auto Inv
>
concept InvertibleRequirements = requires(S x) { { Inv(x) } -> std::convertible_to<S>; };

template <typename G>
concept IsGroup =
  requires {
    typename G::value_type;
  } &&
  requires(typename G::value_type x, typename G::value_type y) {
    { G::op(x, y) } -> std::convertible_to<typename G::value_type>;
    { G::e() } -> std::convertible_to<typename G::value_type>;
    { G::inv(x) } -> std::convertible_to<typename G::value_type>;
  };

template <
  typename S,
  auto Op,
  auto E,
  auto Inv
>
requires MonoidRequirements<S, Op, E> && InvertibleRequirements<S, Inv>
struct Group {
  using value_type = S;
  static S op(const S& a, const S& b) { return Op(a, b); }
  static S e() { return E(); }
  static S inv(const S& x) { return Inv(x); }
};

template <
  typename F,
  typename S,
  auto Eval,
  auto E
>
concept LiChaoRequirements =
  std::is_integral_v<S> &&
  requires(F f, S x) {
    { Eval(f, x) } -> std::convertible_to<S>;
    { E() } -> std::convertible_to<F>;
  };

template <typename L>
concept IsLiChao =
  requires {
    typename L::function_type;
    typename L::value_type;
  } &&
  std::is_integral_v<typename L::value_type> &&
  requires(typename L::function_type f, typename L::value_type x) {
    { L::eval(f, x) } -> std::convertible_to<typename L::value_type>;
    { L::e() } -> std::convertible_to<typename L::function_type>;
  };

template <
  typename F,
  typename S,
  auto Eval,
  auto E
>
requires LiChaoRequirements<F, S, Eval, E>
struct LiChao {
  using function_type = F;
  using value_type = S;
  static S eval(const F& f, const S& x) { return Eval(f, x); }
  static F e() { return E(); }
};