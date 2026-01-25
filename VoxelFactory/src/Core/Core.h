#pragma once
#include <memory>

// I don't know where to put these...
#define PI     3.1415926535f
#define PI2    6.283185307f
#define PIHalf 1.5707963268f

template <typename T> using Ref    = std::shared_ptr<T>; 
template <typename T> using Unique = std::unique_ptr<T>;

template<typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename T, typename... Args>
constexpr Unique<T> CreateUnique(Args&&... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}