#pragma once

// Trio
//
// std::pair を 3 要素にしたもの
template <typename T1, typename T2, typename T3>
struct Trio {
  public:
  using first_type = T1;
  using second_type = T2;
  using third_type = T3;

  T1 first;
  T2 second;
  T3 third;

  constexpr Trio() noexcept(
    std::is_nothrow_default_constructible_v<T1> &&
    std::is_nothrow_default_constructible_v<T2> &&
    std::is_nothrow_default_constructible_v<T3>
  ) = default;

  constexpr Trio(const T1& a, const T2& b, const T3& c) : first(a), second(b), third(c) {}

  template <typename U1, typename U2, typename U3>
  explicit(!std::is_convertible_v<U1, T1> || !std::is_convertible_v<U2, T2> || !std::is_convertible_v<U3, T3>)
  constexpr Trio(U1&& a, U2&& b, U3&& c) noexcept(
    std::is_nothrow_constructible_v<T1, U1> &&
    std::is_nothrow_constructible_v<T2, U2> &&
    std::is_nothrow_constructible_v<T3, U3>
  ) : first(std::forward<U1>(a)),
      second(std::forward<U2>(b)),
      third(std::forward<U3>(c)) {}

  constexpr void swap(Trio& other) noexcept(
    std::is_nothrow_swappable_v<T1> &&
    std::is_nothrow_swappable_v<T2> &&
    std::is_nothrow_swappable_v<T3>
  ) {
    using std::swap;
    swap(first, other.first);
    swap(second, other.second);
    swap(third, other.third);
  }

  friend constexpr void swap(Trio& lhs, Trio& rhs) noexcept(noexcept(lhs.swap(rhs))) { lhs.swap(rhs); }

  [[nodiscard]] constexpr auto operator<=>(const Trio&) const = default;

  [[nodiscard]] constexpr bool operator==(const Trio&) const = default;
};

template <typename T1, typename T2, typename T3>
[[nodiscard]] constexpr auto make_trio(T1&& a, T2&& b, T3&& c) {
  return Trio<std::unwrap_ref_decay_t<T1>, std::unwrap_ref_decay_t<T2>, std::unwrap_ref_decay_t<T3>>(
    std::forward<T1>(a), std::forward<T2>(b), std::forward<T3>(c)
  );
}

template <std::size_t I, typename T1, typename T2, typename T3>
[[nodiscard]] constexpr decltype(auto) get(Trio<T1, T2, T3>& t) noexcept {
  if constexpr (I == 0) return (t.first);
  else if constexpr (I == 1) return (t.second);
  else if constexpr (I == 2) return (t.third);
}

template <std::size_t I, typename T1, typename T2, typename T3>
[[nodiscard]] constexpr decltype(auto) get(const Trio<T1, T2, T3>& t) noexcept {
  if constexpr (I == 0) return (t.first);
  else if constexpr (I == 1) return (t.second);
  else if constexpr (I == 2) return (t.third);
}

template <std::size_t I, typename T1, typename T2, typename T3>
[[nodiscard]] constexpr decltype(auto) get(Trio<T1, T2, T3>&& t) noexcept {
  using ElemType = std::tuple_element_t<I, Trio<T1, T2, T3>>;
  if constexpr (I == 0) return static_cast<ElemType&&>(t.first);
  else if constexpr (I == 1) return static_cast<ElemType&&>(t.second);
  else if constexpr (I == 2) return static_cast<ElemType&&>(t.third);
}

template <std::size_t I, typename T1, typename T2, typename T3>
[[nodiscard]] constexpr decltype(auto) get(const Trio<T1, T2, T3>&& t) noexcept {
  using ElemType = std::tuple_element_t<I, Trio<T1, T2, T3>>;
  if constexpr (I == 0) return static_cast<const ElemType&&>(t.first);
  else if constexpr (I == 1) return static_cast<const ElemType&&>(t.second);
  else if constexpr (I == 2) return static_cast<const ElemType&&>(t.third);
}

namespace std {
  template <typename T1, typename T2, typename T3>
  struct tuple_size<Trio<T1, T2, T3>> : std::integral_constant<std::size_t, 3> {};

  template <std::size_t I, typename T1, typename T2, typename T3>
  struct tuple_element<I, Trio<T1, T2, T3>>;

  template <typename T1, typename T2, typename T3>
  struct tuple_element<0, Trio<T1, T2, T3>> { using type = T1; };

  template <typename T1, typename T2, typename T3>
  struct tuple_element<1, Trio<T1, T2, T3>> { using type = T2; };

  template <typename T1, typename T2, typename T3>
  struct tuple_element<2, Trio<T1, T2, T3>> { using type = T3; };

  template <typename T1, typename T2, typename T3>
  struct hash<Trio<T1, T2, T3>> {
    std::size_t operator()(const Trio<T1, T2, T3>& t) const noexcept(
      noexcept(std::hash<T1>{}(t.first)) &&
      noexcept(std::hash<T2>{}(t.second)) &&
      noexcept(std::hash<T3>{}(t.third))
    ) {
      std::size_t h1 = std::hash<T1>{}(t.first);
      std::size_t h2 = std::hash<T2>{}(t.second);
      std::size_t h3 = std::hash<T3>{}(t.third);
      h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
      h1 ^= h3 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
      return h1;
    }
  };
}

template <typename T1, typename T2, typename T3>
Trio(T1, T2, T3) -> Trio<T1, T2, T3>;