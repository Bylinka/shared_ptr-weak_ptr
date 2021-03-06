// ptr.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <malloc.h>
namespace smart {
	template<class T>
	struct remove_extent { typedef T type; };
	template<class T>
	struct remove_extent<T[]> { typedef T type; };
	template<class T, unsigned long N>
	struct remove_extent<T[N]> { typedef T type; };

	template<class T, T v>
	struct integral_constant {
		static constexpr T value = v;
	};
	using true_type = integral_constant<bool, true>;
	using false_type = integral_constant<bool, false>;

	template<bool B, class T, class F>
	struct conditional { typedef T type; };
	template<class T, class F>
	struct conditional<false, T, F> { typedef F type; };
	template<bool B, class T, class F>
	using conditional_t = typename conditional<B,T,F>::type;

	//template<class...> struct conjunction : true_type { };
	//template<class B1> struct conjunction<B1> : B1 { };
	//template<class B1, class... Bn>
	//struct conjunction<B1, Bn...>
	//	: conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};
	//template<class...T>
	//constexpr bool conjunction_v = conjunction<T...>::value;

	template<bool C, class T = void>
	struct enable_if {};
	template<class T>
	struct enable_if<true,T> { typedef T  type; };
	template<bool C, class T>
	using enable_if_t = typename enable_if<C,T>::type;

	template<class T>
	struct is_array : false_type {};
	template<class T>
	struct is_array<T[]> : true_type {};
	template<class T, long N>
	struct is_array<T[N]> : true_type {};
	template<class T>
	constexpr bool is_array_v = is_array<T>::value;

	template<typename element_type>
	struct counter_type {
		element_type* ptr = nullptr;
		size_t strong = 0;
		size_t week = 0;
		virtual void destroy() {
			delete ptr;
			ptr = nullptr;
//			std::cout << "destroy pointer" << std::endl;

		}
	};
	template<typename element_type>
	struct array_counter_type : counter_type<element_type> {
		void destroy() {
			delete[] this->ptr;
			this->ptr = nullptr;
//			std::cout << "destroy array" << std::endl;

		}
	};
	template<typename T>
	class ptr_base {
	protected:
		template<class other_element_type> friend class ptr_base;
		using element_type = typename remove_extent<T>::type;
		element_type * pointer = nullptr;
		counter_type<element_type> *counter = nullptr;

		virtual void create(const element_type* current_ptr,true_type) {
			this->counter = new array_counter_type<element_type>;
			this->pointer = const_cast<element_type*>(current_ptr);
//			std::cout << "create array" << std::endl;
		}
		virtual void create(const element_type* current_ptr,false_type) {
			this->counter = new counter_type<element_type>;
			this->pointer = const_cast<element_type*>(current_ptr);
//			std::cout << "create pointer" << std::endl;
		}


		void constructor(const element_type* current_ptr) {
			create(current_ptr,is_array<T>{});
			this->counter->ptr = this->pointer;
		}


		template<class other_element_type>
		void constructor_copy(const ptr_base<other_element_type>& smart_ptr, const element_type* current_ptr) {
			this->counter = reinterpret_cast<counter_type<element_type>*>(smart_ptr.counter);
			this->pointer = const_cast<element_type*>(current_ptr);
		}
		void constructor_copy(const ptr_base& smart_ptr) {
			this->counter = smart_ptr.counter;
			this->pointer = smart_ptr.pointer;
		}

	public:
		virtual bool expired() const {
			return this->counter ? !this->counter->strong : true;
		}
		virtual long use_count() const {
			return this->counter ? this->counter->strong : 0;
		}
		virtual	explicit operator bool() const {
			return expired();
		}
		virtual void reset() = 0;
		virtual bool is_same_counter(const ptr_base& smart_ptr) {
			return smart_ptr.counter == this->counter;
		}


		virtual ~ptr_base() {
		}
	};
	struct shared_array_t {};
	struct shared_pointer_t {};

	template<typename T>

	struct shared_array : public ptr_base<T>{
		using typename ptr_base<T>::element_type;

	public:
		element_type& operator[](unsigned long index) {
			return this->pointer[index];
		}

		unsigned long size() const {
			if (std::is_base_of<shared_array_t, element_type>::value
				|| std::is_base_of<shared_pointer_t, element_type>::value)
				return ((unsigned*)this->pointer)[-1];
			return _msize(this->pointer) / sizeof(element_type);
		}
		void *operator new[](unsigned size) {
			unsigned* r = (unsigned*)malloc(size+sizeof(unsigned));
			*r = size / sizeof(element_type);
			return (++r);
		}
		void operator delete[](void *p) {
			unsigned* r = (unsigned*)p;
			free(r-1);
		}

	};

	template<typename T>
		class shared_ptr 
			: public conditional_t<is_array<T>::value, shared_array<T>, ptr_base<T>>
			, public conditional_t<is_array<T>::value, shared_array_t, shared_pointer_t>{
	public:
		using typename ptr_base<T>::element_type;

		shared_ptr() {
		}

		explicit shared_ptr(const element_type* current_ptr)
		{
			this->constructor(current_ptr);
			++this->counter->strong;
			++this->counter->week;
		}
		shared_ptr(const ptr_base<T>& smart_ptr)
		{
			this->constructor_copy(smart_ptr);
			if (!this->counter)return;
			++this->counter->strong;
			++this->counter->week;
		}
		shared_ptr(const shared_ptr& smart_ptr)
		{
			this->constructor_copy(smart_ptr);
			if (!this->counter)return;
			++this->counter->strong;
			++this->counter->week;
		}
		template<class other_element_type>
		shared_ptr(const ptr_base<other_element_type>& smart_ptr, const element_type* current_ptr)
		{
			this->constructor_copy(smart_ptr, current_ptr);
			if (!this->counter)return;
			++this->counter->strong;
			++this->counter->week;
		}
		shared_ptr& operator=(const shared_ptr& smart_ptr) {
			this->reset();
			if (!smart_ptr.counter)return *this;
			this->constructor_copy(smart_ptr);
			++this->counter->strong;
			++this->counter->week;
			return *this;
		}

		element_type& operator*() const {
			return *this->pointer;
		}
		element_type* operator->() const {
			return this->pointer;
		}
		~shared_ptr() {
			reset();
		}
		void reset() {
			if (this->counter) {
				if (this->counter->strong)
					--this->counter->strong;
				if (this->counter->week)
					--this->counter->week;
			}
			if (!this->counter)return;

			if (!this->counter->strong)
				if (this->counter->ptr) {
					this->counter->destroy();
				}
			if (!this->counter->week)
				delete this->counter;
			this->counter = nullptr;
			this->pointer = nullptr;
		}
		
		void swap(shared_ptr& ptr)
		{
			shared_ptr temp(ptr);
			ptr = *this;
			*this = temp;

		}
		element_type* get() const {
			return this->pointer ? this->pointer : nullptr;
		}
		bool unique() const {
			return this->use_count() == 1;
		}
	};
	template<typename T>
	class weak_ptr : public ptr_base<T> , conditional_t<is_array<T>::value, shared_array_t, shared_pointer_t>{
	public:
		using typename ptr_base<T>::element_type;
		
//		weak_ptr() {}
		weak_ptr(const ptr_base<T>& smart_ptr)
		{
			this->constructor_copy(smart_ptr);
			if (!this->counter)return;
			++this->counter->week;
		}
		weak_ptr(const weak_ptr& smart_ptr)
		{
			this->constructor_copy(smart_ptr);
			if (!this->counter)return;
			++this->counter->week;
		}

		weak_ptr& operator=(const ptr_base<T>& smart_ptr) {
			reset();
			this->constructor_copy(smart_ptr);
			if (!this->counter)return *this;
			++this->counter->week;
			return *this;
		}
		weak_ptr& operator=(const weak_ptr& smart_ptr) {
			reset();
			this->constructor_copy(smart_ptr);
			if (!this->counter)return *this;
			++this->counter->week;
			return *this;
		}

		~weak_ptr() {
			reset();
		}
		shared_ptr<element_type> lock() const {
			return *this;
		}
		void reset() {
			if (this->counter) {
				if (this->counter->week)
					--this->counter->week;
			}
			if (!this->counter)return;

			if (!this->counter->strong)
				if (this->counter->ptr) {
					this->counter->destroy();
				}
			if (!this->counter->week)
				delete this->counter;
			this->counter = nullptr;
			this->pointer = nullptr;
		}
		
		void swap(ptr_base<T>& ptr)
		{
			weak_ptr temp = ptr;
			ptr = *this;
			*this = temp;

		}

	};
}

#include <iostream>
using namespace smart;
struct test {
	int a, b;
};
void foo(weak_ptr<int> w) {
	if (!w.expired())
		++*w.lock();
}
void foo2(shared_ptr<int> s) {
	if(!s.expired())
		--*s;
}
template<class T,long N>
int size(T (&arr)[N]) {
	return N;
}



int main()
{

	{

		shared_ptr<shared_ptr<int[]>[]> aaa(new shared_ptr<int[]>[10]);

		for (int i = 0; i < aaa.size(); ++i) {
			aaa[i] = shared_ptr<int[]>(new int[10]);
			for (int j = 0; j < aaa[i].size(); ++j) aaa[i][j] = i * 10 + j;
		}

		for (int i = 0; i < aaa.size(); ++i) {
			for (int j = 0; j < aaa[i].size(); ++j) std::cout << aaa[i][j] << "\t";
			std::cout << std::endl;
		}


	}
	system("pause");
	{
		shared_ptr<shared_ptr<shared_ptr<int>[]>[]> arr;
		{
			shared_ptr<int[]> temp(new int[100]);		

			for (int i = 0; i < temp.size(); ++i)temp[i] = i;
			arr = shared_ptr<shared_ptr<shared_ptr<int>[]>[]>(new shared_ptr<shared_ptr<int>[]>[10]);

			for (int i = 0; i < arr.size(); ++i) {
				arr[i] = shared_ptr<shared_ptr<int>[]>(new shared_ptr<int>[10]);
				for (int j = 0; j < arr[i].size(); ++j) arr[i][j] = shared_ptr<int>(temp,&temp[i * 10 + j]);
			}
		}
		for (int i = 0; i < arr.size(); ++i) {
			for (int j = 0; j < arr[i].size(); ++j) std::cout << *arr[i][j] << "\t";
			std::cout << std::endl;
		}


	}
	system("pause");

	{
		shared_ptr<test[]> t(new test[6]);
		t[2] = { 3,4 };
		shared_ptr<test[]> tt(new test[3]);
		std::cout << t.size() << " " << tt.size() << std::endl;
		t.swap(tt);
		std::cout << t.size() << " " << tt.size() << std::endl;
		std::cout << tt[2].a << std::endl;
		
	}
	{
		shared_ptr<test> t(new test{ 3,4 });
		foo(shared_ptr<int>(t, &t->a));
		std::cout << t->a << std::endl;
		foo(shared_ptr<int>(t, &t->a));
		std::cout << t->a << std::endl;
		weak_ptr<int> w = shared_ptr<int>(t, &t->a);
		foo2(shared_ptr<int>(t, &t->a));
		std::cout << t->a << std::endl;
		foo2(w);
		std::cout << *w.lock() << std::endl;
		foo(w);
		std::cout << *w.lock() << std::endl;
		w = shared_ptr<int>(t, &t->b);
		foo(w);
		std::cout << *w.lock() << std::endl;
	}
	{
		shared_ptr<test> t(new test);
		t->a = 5;
		t->b = 10;
		
		shared_ptr<int> ti(t, &t->a);
		*ti += 10;
		std::cout << t->a << std::endl;
		t.reset();
		std::cout << *ti << std::endl;
		weak_ptr<int> tiw = ti;
		if (tiw.expired()) std::cout << "object has expired!! " << std::endl;
		else std::cout << *tiw.lock() << std::endl;
		ti.reset();
		if (tiw.expired()) std::cout << "object has expired!! " << std::endl;
		else std::cout << *tiw.lock() << std::endl;

		shared_ptr<int> test(new int(45));
		shared_ptr<int> test3(new int(44));
		weak_ptr<int> t1 = test;
		*t1.lock() += 5;
		test.swap(test3);
		test3 = test;
		std::cout << "unique: " << std::boolalpha << test.unique() << std::endl;
		if (!t1.expired())
			std::cout << *test << std::endl;
		test.reset();
		std::cout << t1.use_count() << " " << std::endl;
	}
	system("pause");
    return 0;
}

