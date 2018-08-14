// ptr.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

template<typename T>
class ptr_base {
protected:
	struct ptr_counter {
		T* ptr = nullptr;
		size_t strong = 0;
		size_t week = 0;
	}* ptr = nullptr;
public:
	explicit ptr_base(const T* ptr) {
		this->ptr = new ptr_counter;
		this->ptr->ptr = const_cast<T*>(ptr);
	}
	ptr_base(const ptr_base& ptr)  {
		this->ptr = ptr.ptr;
	}
	virtual bool expired() const {
		return ptr ? !ptr->ptr : true;
	}
	virtual long use_count() const {
		return ptr ? ptr->strong : 0;
	}
	virtual	explicit operator bool() const{
		return expired();
	}
	virtual void reset() = 0;
	virtual bool is_same(const ptr_base& ptr) {
		return ptr.ptr == this->ptr;
	}
	virtual void swap(ptr_base& ptr)  = 0;
	virtual ~ptr_base() {
		if (!ptr)return;

		if (!ptr->strong)
			if (!expired()) {
				delete ptr->ptr;
				ptr->ptr = nullptr;
			}
		if (!ptr->week)
			delete ptr;
	}
};

template<typename T>
class shared_ptr : public ptr_base<T>{
public:
	explicit shared_ptr(const T* ptr) 
		: ptr_base<T>(ptr)
	{
		++this->ptr->strong;
		++this->ptr->week;
	}
	shared_ptr(const ptr_base<T>& ptr) 
		: ptr_base<T>(ptr)
	{
		if (!this->ptr)return;
		++this->ptr->strong;
		++this->ptr->week;
	}
	shared_ptr& operator=(const ptr_base<T>& ptr) {
		reset();
		if (!ptr.ptr)return;
		this->ptr = ptr.ptr;
		++this->ptr->strong;
		++this->ptr->week;
		return *this;
	}

	T& operator*() const{
		return *this->ptr->ptr;
	}
	T* operator->() const{
		return this->ptr->ptr;
	}
	~shared_ptr() {
		if (this->ptr) {
			if (this->ptr->strong)
				--this->ptr->strong;
			if (this->ptr->week)
				--this->ptr->week;
		}
	}
	void reset() {
		this->~shared_ptr();
		this->ptr = nullptr;
	}
	void swap(ptr_base<T>& ptr)
	{
		shared_ptr temp = ptr;
		ptr = *this;
		*this = temp;

	}
	T* get() const {
		return this->ptr ? this->ptr->ptr : nullptr;
	}
	bool unique() const{
		return this->use_count() == 1;
	}
};
template<typename T>
class weak_ptr : public ptr_base<T> {
public:

	weak_ptr(const ptr_base<T>& ptr)
		: ptr_base<T>(ptr)
	{
		if (!this->ptr)return;
		++this->ptr->week;
	}
	weak_ptr& operator=(const ptr_base<T>& ptr) {
		reset();
		this->ptr = ptr.ptr;
		if (!ptr.ptr)return *this;
		++this->ptr->week;
		return *this;
	}

	~weak_ptr() {
		if (this->ptr) {
			if (this->ptr->week)
				--this->ptr->week;
		}
	}
	shared_ptr<T> lock() const{
		return *this;
	}
	void reset() {
		this->~weak_ptr();
		this->ptr = nullptr;
	}
	void swap(ptr_base<T>& ptr)
	{
		weak_ptr temp = ptr;
		ptr = *this;
		*this = temp;
	}

};


#include <iostream>

int main()
{
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

