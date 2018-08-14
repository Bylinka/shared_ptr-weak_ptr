// ptr.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

namespace smart {
	template<typename element_type>
	struct counter_type {
		element_type* ptr = nullptr;
		size_t strong = 0;
		size_t week = 0;
	};
	template<typename element_type>
	class ptr_base {
	protected:
		template<class other_element_type> friend class ptr_base;
		element_type * pointer = nullptr;
		counter_type<element_type> *counter = nullptr;

		void constructor(const element_type* current_ptr) {
			this->counter = new counter_type<element_type>;
			this->pointer = const_cast<element_type*>(current_ptr);
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
		virtual void swap(ptr_base& smart_ptr) = 0;
		virtual ~ptr_base() {
			if (!this->counter)return;

			if (!this->counter->strong)
				if (!expired()) {
					delete this->counter->ptr;
					this->counter->ptr = nullptr;
				}
			if (!this->counter->week)
				delete this->counter;
		}
	};

	template<typename element_type>
	class shared_ptr : public ptr_base<element_type> {
	public:
		explicit shared_ptr(const element_type* current_ptr)
		{
			this->constructor(current_ptr);
			++this->counter->strong;
			++this->counter->week;
		}
		shared_ptr(const ptr_base<element_type>& smart_ptr)
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
		shared_ptr& operator=(const ptr_base<element_type>& smart_ptr) {
			reset();
			if (!smart_ptr.counter)return;
			this->counter = smart_ptr.counter;
			this->pointer = smart_ptr.pointer;
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
			if (this->counter) {
				if (this->counter->strong)
					--this->counter->strong;
				if (this->counter->week)
					--this->counter->week;
			}
		}
		void reset() {
			this->~shared_ptr();
			this->counter = nullptr;
			this->pointer = nullptr;
		}
		void swap(ptr_base<element_type>& ptr)
		{
			shared_ptr temp = ptr;
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
	template<typename element_type>
	class weak_ptr : public ptr_base<element_type> {
	public:

		weak_ptr(const ptr_base<element_type>& smart_ptr)
		{
			this->constructor_copy(smart_ptr);
			if (!this->counter)return;
			++this->counter->week;
		}
		weak_ptr& operator=(const ptr_base<element_type>& smart_ptr) {
			reset();
			this->counter = smart_ptr.counter;
			this->pointer = smart_ptr.pointer;
			if (!smart_ptr.counter)return *this;
			++this->counter->week;
			return *this;
		}

		~weak_ptr() {
			if (this->counter) {
				if (this->counter->week)
					--this->counter->week;
			}
		}
		shared_ptr<element_type> lock() const {
			return *this;
		}
		void reset() {
			this->~weak_ptr();
			this->counter = nullptr;
			this->pointer = nullptr;
		}
		void swap(ptr_base<element_type>& ptr)
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
int main()
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
	if(tiw.expired()) std::cout << "object has expired!! " << std::endl;
	else std::cout << *tiw.lock() << std::endl;
	
	shared_ptr<int> test(new int(45));
	shared_ptr<int> test3(new int(44));
	std::shared_ptr<int> test1(new int(45));
	std::shared_ptr<int> test4(new int(44));
	weak_ptr<int> t1 = test;
	std::weak_ptr<int> t2 = test1;
	t2.lock();
	*t1.lock() += 5;
	*test1 += 5;
	test.swap(test3);
	test3 = test;
	std::cout<<"unique: "<< std::boolalpha << test.unique() << std::endl;
	if(!t1.expired())
	std::cout << *test << " " << *test1 << std::endl;
	test.reset();
	std::cout << t1.use_count() << " " << t2.use_count() << std::endl;
	system("pause");
    return 0;
}

