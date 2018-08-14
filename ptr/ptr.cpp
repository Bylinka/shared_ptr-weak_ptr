// ptr.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"


template<typename T>
struct ptr_counter {
	T* ptr = nullptr;
	size_t strong = 0;
	size_t week = 0;
};
template<typename T>
class ptr_base {
protected:
	template<class U> friend class ptr_base;
	T * pointer = nullptr;
	ptr_counter<T> *counter = nullptr;

	void constructor(const T* current_ptr) {
		this->counter = new ptr_counter<T>;
		this->pointer = const_cast<T*>(current_ptr);
		this->counter->ptr = this->pointer;
	}
	template<class U>
	void constructor_copy(const ptr_base<U>& smart_ptr, const T* current_ptr) {
		this->counter = reinterpret_cast<ptr_counter<T>*>(smart_ptr.counter);
		this->pointer = const_cast<T*>(current_ptr);
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
	virtual	explicit operator bool() const{
		return expired();
	}
	virtual void reset() = 0;
	virtual bool is_same_counter(const ptr_base& smart_ptr) {
		return smart_ptr.counter == this->counter;
	}
	virtual void swap(ptr_base& smart_ptr)  = 0;
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

template<typename T>
class shared_ptr : public ptr_base<T>{
public:
	explicit shared_ptr(const T* current_ptr)
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
	template<class U>
	shared_ptr(const ptr_base<U>& smart_ptr, const T* current_ptr)
	{
		this->constructor_copy(smart_ptr, current_ptr);
		if (!this->counter)return;
		++this->counter->strong;
		++this->counter->week;
	}
	shared_ptr& operator=(const ptr_base<T>& smart_ptr) {
		reset();
		if (!smart_ptr.counter)return;
		this->counter = smart_ptr.counter;
		this->pointer = smart_ptr.pointer;
		++this->counter->strong;
		++this->counter->week;
		return *this;
	}

	T& operator*() const{
		return *this->pointer;
	}
	T* operator->() const{
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
	void swap(ptr_base<T>& ptr)
	{
		shared_ptr temp = ptr;
		ptr = *this;
		*this = temp;

	}
	T* get() const {
		return this->pointer ? this->pointer : nullptr;
	}
	bool unique() const{
		return this->use_count() == 1;
	}
};
template<typename T>
class weak_ptr : public ptr_base<T> {
public:

	weak_ptr(const ptr_base<T>& smart_ptr)
	{
		this->constructor_copy(smart_ptr);
		if (!this->counter)return;
		++this->counter->week;
	}
	weak_ptr& operator=(const ptr_base<T>& smart_ptr) {
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
	shared_ptr<T> lock() const{
		return *this;
	}
	void reset() {
		this->~weak_ptr();
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


#include <iostream>
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

