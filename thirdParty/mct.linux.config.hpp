#ifndef MCT_CONFIG_HPP
#define MCT_CONFIG_HPP

#define MCT_MAJOR_VERSION               1
#define MCT_MINOR_VERSION               1
#define MCT_PATCH_VERSION               4
#define MCT_VERSION_STRING              "1.1.4"

#if !defined (MCT_HASH_HEADER) || !defined (MCT_HASH_NAMESPACE)
#  define MCT_HASH_HEADER               <tr1/functional>
#  define MCT_HASH_NAMESPACE            std::tr1
#endif

#if !defined (MCT_TYPE_TRAITS_HEADER) || !defined (MCT_TYPE_TRAITS_NAMESPACE)
#  define MCT_HAVE_TYPE_TRAITS          1
#  define MCT_TYPE_TRAITS_HEADER        <tr1/type_traits>
#  define MCT_TYPE_TRAITS_NAMESPACE     std::tr1
#else
#  define MCT_HAVE_TYPE_TRAITS          1
#endif

#if !defined (MCT_CXX0X_SUPPORTED)
#  define MCT_CXX0X_SUPPORTED           0
#endif

#define MCT_SIZEOF_STD_SIZE_T           4
#define MCT_SIZEOF_CHAR                 1
#define MCT_SIZEOF_SHORT                2
#define MCT_SIZEOF_INT                  4
#define MCT_SIZEOF_LONG                 4
#define MCT_HAVE_LONG_LONG              1
#define MCT_SIZEOF_LONG_LONG            8
#define MCT_SIZEOF_POINTER              4

#endif  // Multi-inclusion guard.


// Local variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// fill-column: 90
// End:
