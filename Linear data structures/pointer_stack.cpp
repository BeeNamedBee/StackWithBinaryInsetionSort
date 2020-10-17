/*
    Copyright (c) 2020 Mikhail S. V. <@BeeNamedBee>
    under MIT License
    
    Desc:
      Stack based on linked list with implemented Binary Insertion Sort
*/

#include <iostream>
#include <chrono>
#include <stdexcept>

template<class T>
struct element {
	T value;
	element<T>* next;

	element() : element::element(NULL, nullptr) {}
	element(T&& val, element<T>* next) : value(val), next(next) {}
	element(const element& other) {
		#ifdef _DEBUG
		std::cout << "element copy constructor called\n";
		#endif // _DEBUG

		value = other.value;
		next = other.next;
	}
};

template<class T>
std::ostream& operator<<(std::ostream& stream, const element<T>& other) {
	stream << other.value;
	return stream;
}

template<class T>
class stack {
private:
	size_t _size;
	element<T>* _head;

	size_t get_pos(T&& val, bool(*cmp)(const T& val1, const T& val2)) {
		size_t l = 0, r = _size, piv;

		while (l < r) {
			piv = l + (r - l) / 2;

			if (cmp(this->at(piv), val)) {
				l = piv + 1;
			}
			else r = piv;
		}

		return r;
	}

public:
	void print() {
		stack<T> tmp(this);

		while (tmp.size()) {
			std::cout << tmp.pop() << ' ';
		}

		std::cout << '\n';
	}

	stack(size_t size, T&& val = NULL) : _size(size) {
		if (size > 0) {
			_head = new element<T>(std::move(val), nullptr);
			element<T>* ptr = _head;

			for (int i = 1; i < size; i++) {
				ptr->next = new element<T>(std::move(val), nullptr);
				ptr = ptr->next;
			}
		}
		else throw std::runtime_error("invalid size\nmust be greater than 0 and fit size_t");
	}

	stack() : _size(0), _head(nullptr) {}

	~stack() { clear(); }

	stack(const stack& other) {
		#ifdef _DEBUG
		std::cout << "stack copy constructor called\n";
		#endif // _DEBUG

		_size = other._size;
		if (_size == 0) return;
		
		_head = new element<T>(*other._head);
		element<T>* ptr = _head;
		element<T>* other_ptr = other._head;

		for (size_t i = 1; i < _size; i++) {
			ptr->next = new element<T>(*(other_ptr->next));
			ptr = ptr->next;
			other_ptr = other_ptr->next;
		}
	}

	stack(const stack* other) {
		#ifdef _DEBUG
		std::cout << "stack copy constructor called\n";
		#endif // _DEBUG

		_size = other->_size;
		if (_size == 0) return;

		_head = new element<T>(*other->_head);
		element<T>* ptr = _head;
		element<T>* other_ptr = other->_head;

		for (size_t i = 1; i < _size; i++) {
			ptr->next = new element<T>(*(other_ptr->next));
			ptr = ptr->next;
			other_ptr = other_ptr->next;
		}
	}

	void clear(size_t depth) {
		depth = (_size >= depth) * depth +
			(_size < depth) * _size;
		element<T>* ptr;

		for (size_t i = 0; i < depth; i++) {
			 ptr = _head;
			_head = _head->next;
			delete ptr;
		}

		_size -= depth;
	}

	void clear() {
		clear(_size);
	}

	void push(T&& val) {
		element<T> *elem = new element<T>(std::move(val), _head);
		_head = elem;
		_size++;
	}

	T pop() {
		if (_size > 0){
			T payload = _head->value;
			element<T>* ptr = _head;
			_head = _head->next;
			delete ptr;
			_size--;

			return payload;
		}else throw std::runtime_error("attempt to pop empty stack");
	}

	T peek() {
		if (_size > 0)
			return _head->value;
		else throw std::runtime_error("attempt to peek empty stack");
	}

	size_t size() {
		return _size;
	}

	T& at(size_t pos) {
		return operator[](pos);
	}

	T& operator[](size_t pos) {
		if (pos >= 0 && pos < _size) { // const
			stack<T> tmp; // const
			for (size_t i = 0; i < pos; i++) { // <sigma i = 0 -> pos> (const + const)
				tmp.push(this->pop()); // const + const
			}
			T& res = this->_head->value; //const
			this->consume(tmp); // pos * const

			return res;
		}
		else throw std::runtime_error("pos out of bounds");
	}
  
	void insert(T&& val, size_t pos) {
		if (pos<0 || pos>_size) throw std::runtime_error("out of bounds");
		stack<T> tmp;

		for (size_t i = 0; i < pos; i++) {
			tmp.push(this->pop());
		}

		this->push(std::move(val));

		this->consume(tmp);
	}

	stack<T> substack(size_t start, size_t end) {
		if (start > end) throw std::runtime_error("start > end");
		if (start < 0 && end > _size) throw std::runtime_error("out of bounds");

		stack<T> tmp(this);
		stack<T>* res = new stack<T>();
		tmp.clear(start);

		for (size_t i = 0; i < end-start; i++) {
			res->push(tmp.pop());
		}

		return *res;
	}

	void consume(stack<T>& stack) {
		while (stack.size()) {
			this->push(stack.pop());
		}
	}

	void sort(bool(*cmp)(const T& val1, const T& val2)) {
		if (_size == 0) return;
		stack<T> original(this);
		this->clear();

		this->push(original.pop());

		while(original.size()){
			size_t pos = get_pos(original.peek(), cmp);
			insert(original.pop(), pos);
		}
	}
};

template<typename T1, typename T2>
void timeit(T1&& preproc, T2&& func, int ntimes = 1) {
	ntimes = ntimes > 0 ? ntimes : 1;
	long long result = 0;

	for (int i = 0; i < ntimes; i++) {
		auto payload = preproc();
		auto startPoint = std::chrono::high_resolution_clock::now();

		func(payload);

		auto endPoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(startPoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();

		result += (end - start);
		delete payload;
	}

	std::cout << "per measurment on average\n";
	std::cout << result / ntimes << " microsecs.\n";
	std::cout << (long double)result * 0.001 / ntimes << " ms.\n";
	std::cout << (long double)result * 0.000001 / ntimes<< " s.\n";
	std::cout << "measurments taken: " << ntimes << '\n';
	
	
}

template<class T>
bool cmp(const T& val1, const T& val2) {
	return val1 < val2;
}

int main()
{
	timeit([]()->stack<int>* {
		stack<int> *s = new stack<int>();

		for (int i = 0; i < 3000; i++) {
					s->push((int)(rand()% 1000 - 500));
		}
		return s;
		},
		[&](stack<int> s) {
		s.sort(cmp);
		},
		20);
}
