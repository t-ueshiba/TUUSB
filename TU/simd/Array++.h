/*!
  \file		Array++.h
  \author	Toshio UESHIBA
  \brief	SIMD命令を適用できる配列クラスの定義
*/
#if !defined(TU_SIMD_ARRAYPP_H)
#define TU_SIMD_ARRAYPP_H

#include "TU/simd/simd.h"
#include "TU/Array++.h"
#ifdef TU_DEBUG
#  include <boost/core/demangle.hpp>
#endif

#if defined(SIMD)
namespace TU
{
namespace simd
{
//! 反復子をラップして名前空間 TU::simd に取り込む
/*!
  \param iter	ラップする反復子
*/
template <bool ALIGNED=true, class ITER> inline iterator_wrapper<ITER, ALIGNED>
wrap_iterator(ITER iter)
{
    return {iter};
}
    
//! ラップされた反復子からもとの反復子を取り出す
/*!
  \param iter	ラップされた反復子
  \return	もとの反復子
*/
template <class ITER, bool ALIGNED> inline ITER
make_accessor(iterator_wrapper<ITER, ALIGNED> iter)
{
    return iter.base();
}

//! ラップされた定数ポインタからSIMDベクトルを読み込む反復子を生成する
/*!
  ラップされたポインタは sizeof(vec<T>) にalignされていなければならない．
  \param p	ラップされた定数ポインタ
  \return	SIMDベクトルを読み込む反復子
*/
template <class T, bool ALIGNED> inline load_iterator<T, ALIGNED>
make_accessor(iterator_wrapper<const T*, ALIGNED> p)
{
    return {p.base()};
}

//! ラップされたポインタからSIMDベクトルを書き込む反復子を生成する
/*!
  ラップされたポインタは sizeof(vec<T>) にalignされていなければならない．
  \param p	ラップされたポインタ
  \return	SIMDベクトルを書き込む反復子
*/
template <class T, bool ALIGNED> inline store_iterator<T, ALIGNED>
make_accessor(iterator_wrapper<T*, ALIGNED> p)
{
    return {p.base()};
}

//! zip_iterator中の各反復子からSIMDベクトルを読み書きする反復子を生成し，それを再度zip_iteratorにまとめる
/*!
  \param zip_iter	SIMDベクトルを読み込み元/書き込み先を指す反復子を束ねた
			zip_iterator
  \return		SIMDベクトルを読み書きする反復子を束ねたzip_iterator
*/
template <class ITER_TUPLE> inline auto
make_accessor(zip_iterator<ITER_TUPLE> zip_iter)
{
    return make_zip_iterator(tuple_transform([](auto iter)
					     { return make_accessor(iter); },
					     zip_iter.get_iterator_tuple()));
}

template <class T, class ITER, bool ALIGNED> inline auto
make_converter(iterator_wrapper<ITER, ALIGNED> iter)
{
    using accessor	= decltype(make_accessor(iter));
    using element_type	= typename iterator_value<accessor>::element_type;
    using converter	= std::conditional_t<
				is_vec<iterator_reference<accessor> >::value,
				std::conditional_t<
				    (vec<T>::size >= vec<element_type>::size),
				    cvtdown_iterator<T, accessor>, accessor>,
				std::conditional_t<
				    (vec<element_type>::size > vec<T>::size),
				    accessor, cvtup_iterator<accessor> > >;

    return converter(make_accessor(iter));
}
    
namespace detail
{
  template <class ITER>
  struct vsize		// ITER::value_type が vec<T> 型
  {
      constexpr static size_t	value = ITER::value_type::size;
  };
  template <class T>
  struct vsize<T*>
  {
      constexpr static size_t	value = vec<std::remove_cv_t<T> >::size;
  };
}	// namespace detail

//! ある要素型を指定された個数だけカバーするために必要なSIMDベクトルの個数を調べる
/*!
  T を iterator_value<ITER> がSIMDベクトル型となる場合はその要素型，ITER がポインタの
  場合はそれが指す要素の型としたとき，指定された個数のT型要素をカバーするために
  必要な vec<T> 型SIMDベクトルの最小個数を返す．
  \param n	要素の個数
  \return	nをカバーするために必要なSIMDベクトルの個数
*/
template <class ITER> constexpr inline size_t
make_terminator(size_t n)
{
    return (n ? (n - 1)/detail::vsize<ITER>::value + 1 : 0);
}
    
#ifdef TU_DEBUG
namespace detail
{
  template <class T> inline void
  print_types(std::ostream& out)
  {
      out << '\t' << boost::core::demangle(typeid(T).name()) << std::endl;
  }
  template <class S, class... T> inline std::enable_if_t<sizeof...(T) != 0>
  print_types(std::ostream& out)
  {
      out << '\t' << boost::core::demangle(typeid(S).name()) << std::endl;
      print_types<T...>(out);
  }
}	// namespace detail
#endif
    
template <class FUNC, class... ITER, bool... ALIGNED> inline auto
make_map_iterator(FUNC func, iterator_wrapper<ITER, ALIGNED>... iter)
{
#ifdef TU_DEBUG
    std::cout << "(simd)make_map_iterator:\n";
    detail::print_types<ITER...>(std::cout);
#endif		  
    return wrap_iterator(TU::make_map_iterator(func, make_accessor(iter)...));
}

/************************************************************************
*  algorithms overloaded for simd::iterator_wrapper<ITER, ALIGNED>	*
************************************************************************/
namespace detail
{
  template <class T>
  struct vec_element_t
  {
      using type	= T;
  };
  template <class T>
  struct vec_element_t<vec<T> >
  {
      using type	= T;
  };
}	// namespace detail
    
//! 指定された範囲の各要素に関数を適用する
/*!
  N != 0 の場合，Nで指定した要素数だけ適用し，nは無視．
  N = 0 の場合，要素数をnで指定，
  \param func	適用する関数
  \param n	適用要素数
  \param begin	適用範囲の先頭を指す反復子
*/
template <size_t N, class FUNC, class ITER, bool ALIGNED> inline FUNC
for_each(FUNC func, size_t n, iterator_wrapper<ITER, ALIGNED> iter)
{
    constexpr auto	M = make_terminator<ITER>(N);
    
    return TU::for_each<M>(func, make_terminator<ITER>(n),
			   make_accessor(iter));
}

//! 指定された2つの範囲の各要素に2変数関数を適用する
/*!
  N != 0 の場合，Nで指定した要素数だけ適用し，nは無視．
  N = 0 の場合，要素数をnで指定，
  \param begin0	第1の適用範囲の先頭を指す反復子
  \param n	適用要素数
  \param begin1	第2の適用範囲の先頭を指す反復子
  \param func	適用する関数
*/
template <size_t N, class FUNC,
	  class ITER0, bool ALIGNED0, class ITER1, bool ALIGNED1> inline FUNC
for_each(FUNC func, size_t n,
	 iterator_wrapper<ITER0, ALIGNED0> begin0, 
	 iterator_wrapper<ITER1, ALIGNED1> begin1)
{
#ifdef TU_DEBUG
    std::cout << "(simd)for_each<" << N << "> ["
	      << print_sizes(range<iterator_wrapper<ITER0, ALIGNED0>, N>(
				 begin0, n))
	      << ']' << std::endl;
#endif
    using T0	= typename detail::vec_element_t<iterator_value<ITER0> >::type;
    using T1	= typename detail::vec_element_t<iterator_value<ITER1> >::type;
    using CVTR  = std::conditional_t<(vec<T0>::size > vec<T1>::size),
				     decltype(make_converter<T1>(begin0)),
				     decltype(make_converter<T0>(begin1))>;
    
    constexpr auto	M = make_terminator<CVTR>(N);
    
    return TU::for_each<M>(func, make_terminator<CVTR>(n),
			   make_converter<T1>(begin0),
			   make_converter<T0>(begin1));
}
    
//! 指定された範囲の内積の値を返す
/*!
  N != 0 の場合，Nで指定した要素数の範囲の内積を求め，nは無視．
  N = 0 の場合，要素数をnで指定，
  \param begin0	適用範囲の第1変数の先頭を指す反復子
  \param n	要素数
  \param begin1	適用範囲の第2変数の先頭を指す反復子
  \param init	初期値
  \return	内積の値
*/
template <size_t N, class ITER0, bool ALIGNED0,
		    class ITER1, bool ALIGNED1, class T> inline T
inner_product(iterator_wrapper<ITER0, ALIGNED0> begin0, size_t n,
	      iterator_wrapper<ITER1, ALIGNED1> begin1, T init)
{
#ifdef TU_DEBUG
    std::cout << "(simd)inner_product<" << N << "> ["
	      << print_sizes(range<iterator_wrapper<ITER0, ALIGNED0>, N>(
				 begin0, n))
	      << "]" << std::endl;
#endif
    constexpr auto	M = make_terminator<ITER0>(N);
    
    return hadd(TU::inner_product<M>(make_accessor(begin0),
				     make_terminator<ITER0>(n),
				     make_accessor(begin1),
				     vec<T>(init)));
}
    
//! 指定された範囲にある要素の2乗和を返す
/*!
  N != 0 の場合，Nで指定した要素数の範囲の2乗和を求め，nは無視．
  N = 0 の場合，要素数をnで指定，
  \param begin	適用範囲の先頭を指す反復子
  \param n	要素数
  \return	2乗和の値
*/
template <size_t N, class ITER, bool ALIGNED> inline auto
square(iterator_wrapper<ITER, ALIGNED> iter, size_t n)
{
    constexpr auto	M = make_terminator<ITER>(N);
    
    return hadd(square<M>(make_accessor(iter), make_terminator<ITER>(n)));
}

}	// namespace simd
    
namespace detail
{
  template <class T, bool ALIGNED>
  struct const_iterator_t<simd::iterator_wrapper<T*, ALIGNED> >
  {
      using type = simd::iterator_wrapper<const T*, ALIGNED>;
  };
}	// namespace detail
    
/************************************************************************
*  traits for Buf<T, ALLOC, SIZE, SIZES...>				*
************************************************************************/
template <class T, class ALLOC>
class BufTraits<simd::vec<T>, ALLOC>
    : public std::allocator_traits<simd::allocator<simd::vec<T> > >
{
  private:
    using super			= std::allocator_traits<
				      simd::allocator<simd::vec<T> > >;

  public:
    using iterator		= simd::store_iterator<T, true>;
    using const_iterator	= simd::load_iterator<T, true>;
    
  protected:
    using			typename super::pointer;

    constexpr static size_t	Alignment = super::allocator_type::Alignment;
    
    static pointer	null()		{ return nullptr; }
    static auto		ptr(pointer p)	{ return p.base(); }
};

}	// namespace TU
#endif	// SIMD
#endif	// !TU_SIMD_ARRAYPP_H
