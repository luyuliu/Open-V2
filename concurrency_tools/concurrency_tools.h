#pragma once
#include <stdint.h>
#include <atomic>
#include <string>
#include <type_traits>

template<typename E1, typename E2>
class string_sum_expression;

template<typename T>
class string_expression {
public:
	uint32_t length() const {
		return static_cast<const T*>(this)->length();
	}
	char operator[](uint32_t i) const {
		return static_cast<const T*>(this)->operator[](i);
	}
	template<typename O>
	string_sum_expression<string_expression<T>, string_expression<O>> operator+(const string_expression<O>& o) const {
		return string_sum_expression<string_expression<T>, string_expression<O>>(*this, o);
	}
	template<typename O, typename = std::enable_if_t<std::is_same_v<std::string, O>>>
	string_sum_expression<string_expression<T>, O> operator+(const O& o) const {
		return string_sum_expression<string_expression<T>, O>(*this, o);
	}
	template<size_t N>
	string_sum_expression<string_expression<T>, string_expression<const char[N]>> operator+(const string_expression<const char[N]>& o) const {
		return string_sum_expression<string_expression<T>, string_expression<const char[N]>>(*this, o);
	}
	template<size_t N>
	string_sum_expression<string_expression<T>, string_expression<char[N]>> operator+(const string_expression<char[N]>& o) const {
		return string_sum_expression<string_expression<T>, string_expression<const char[N]>>(*this, o);
	}
	string_sum_expression<string_expression<T>, string_expression<const char*>> operator+(const string_expression<const char*>& o) const {
		return string_sum_expression<string_expression<T>, string_expression<const char*>>(*this, o);
	}
	string_sum_expression<string_expression<T>, string_expression<char*>> operator+(const string_expression<char*>& o) const {
		return string_sum_expression<string_expression<T>, string_expression<char*>>(*this, o);
	}
	operator std::string() const;
};

template<>
class string_expression<const char*> {
private:
	const char* base;
	const uint32_t len = 0;
public:
	string_expression(const char* b) : base(b), len(strlen(b)) {}
	uint32_t length() const;
	char operator[](uint32_t i) const;
};

template<>
class string_expression<char*> : public string_expression<const char*>  {};

template<size_t N>
class string_expression<const char[N]> {
private:
	const char* base;
public:
	string_expression(const char(&b)[N]) : base(b) {}
	uint32_t length() const;
	char operator[](uint32_t i) const;
};

template<size_t N>
class string_expression<char[N]>  : public string_expression<const char[N]> {};

class empty_string_expression : public string_expression<empty_string_expression> {
public:
	empty_string_expression() {}
	empty_string_expression(const string_expression<empty_string_expression>&) {}
	uint32_t length() const { return 0; };
	char operator[](uint32_t i) const { return 0; };
};

template<typename E1, typename E2>
class string_sum_expression : public string_expression<string_sum_expression<E1, E2>> {
public:
	const E1& a;
	const E2& b;
	string_sum_expression(const E1& aa, const E2& bb) : a(aa), b(bb) {};
	uint32_t length() const {
		return a.length() + b.length();
	}
	char operator[](uint32_t i) const {
		const auto a_len = a.length();
		if (i < a_len)
			return a[i];
		else
			return b[i - a_len];
	}
};

template<typename E2>
class string_sum_expression<string_expression<empty_string_expression>, E2> : public string_expression<string_sum_expression<string_expression<empty_string_expression>, E2>> {
public:
	const E2& b;
	string_sum_expression(const string_expression<empty_string_expression>&, const E2& bb) : b(bb) {};
	uint32_t length() const {
		return b.length();
	}
	char operator[](uint32_t i) const {
		return b[i];
	}
};

class concurrent_string : public string_expression<concurrent_string> {
private:
	static constexpr uint32_t internal_concurrent_string_size = 16;

	union {
		char local_data[internal_concurrent_string_size];
		struct {
			char* data;
			uint32_t length;
		} remote_data;
	} _data;
public:
	concurrent_string();
	concurrent_string(const char* source);
	concurrent_string(const char* start, const char* end);
	concurrent_string(const char* start, uint32_t size);
	concurrent_string(const concurrent_string&);
	concurrent_string(concurrent_string&&) noexcept;
	template<typename T>
	concurrent_string(const string_expression<T>&);
	~concurrent_string();

	uint32_t length() const;
	const char* c_str() const;
	char operator[](uint32_t i) const;
	concurrent_string& operator=(const concurrent_string&);
	template<typename T>
	concurrent_string& operator=(const string_expression<T>&);
	concurrent_string& operator=(concurrent_string&&) noexcept;
	concurrent_string& operator+=(const concurrent_string&);
	concurrent_string& operator+=(const char*);
	template<typename T>
	concurrent_string& operator+=(const string_expression<T>&);
	bool operator==(const char*) const;
	bool operator==(const concurrent_string&) const;
	template<typename T>
	bool operator==(const string_expression<T>&) const;
	void clear();
};

template <typename T>
struct concurrent_allocator {
	using value_type = T;
	concurrent_allocator() noexcept {};
	template <typename U>
	concurrent_allocator(const concurrent_allocator<U>&) noexcept {};
	T* allocate(size_t n);
	void deallocate(T* p, size_t n);
};

template <typename T, typename U>
constexpr bool operator== (const concurrent_allocator<T>&, const concurrent_allocator<U>&) noexcept {
	return true;
}

template <class T, class U>
constexpr bool operator!= (const concurrent_allocator<T>&, const concurrent_allocator<U>&) noexcept {
	return false;
}

template<typename T, uint32_t block, uint32_t index_sz>
class fixed_sz_deque_iterator;

template<typename T, uint32_t block, uint32_t index_sz>
class fixed_sz_deque {
private:
	std::atomic<T*> index_array[index_sz] = { nullptr };
	std::atomic<uint32_t> first_free = 0;
	std::atomic<uint32_t> first_free_index = 1;
public:
	fixed_sz_deque();
	~fixed_sz_deque();

	T& at(uint32_t index) const;
	T* safe_at(uint32_t index) const;
	void free(uint32_t index);
	uint32_t past_end() const;

	template<typename ...P>
	uint32_t emplace_back(P&& ... params);
	template<typename F>
	void visit(uint32_t index, const F& f) const;

	fixed_sz_deque_iterator<T, block, index_sz> begin() const;
	fixed_sz_deque_iterator<T, block, index_sz> end() const;
};

template<typename T, uint32_t block, uint32_t index_sz>
class fixed_sz_list {
private:
	std::atomic<T*> index_array[index_sz] = { nullptr };
	std::atomic<uint32_t> first_free = (uint32_t)-1;
	std::atomic<uint32_t> first_in_list = (uint32_t)-1;
	std::atomic<uint32_t> first_free_index = 1;
public:
	fixed_sz_list();
	~fixed_sz_list();

	template<typename ...P>
	void emplace(P&& ... params);
	template<typename F>
	void try_pop(const F& f);
	template<typename F>
	void flush(const F& f);
};

template<typename T, uint32_t block, uint32_t index_sz>
class fixed_sz_deque_iterator {
private:
	const fixed_sz_deque<T, block, index_sz>* parent;
	int32_t position;
public:
	fixed_sz_deque_iterator() : parent(nullptr), position(0) {};
	fixed_sz_deque_iterator(const fixed_sz_deque<T, block, index_sz>& p) : parent(&p), position(0) {};
	fixed_sz_deque_iterator(const fixed_sz_deque<T, block, index_sz>& p, uint32_t o) : parent(&p), position(o) {};

	bool operator==(const fixed_sz_deque_iterator& o) const {
		return position == o.position;
	}
	bool operator!=(const fixed_sz_deque_iterator& o) const {
		return position != o.position;
	}
	T* operator*() const {
		return parent->safe_at(position);
	}
	T* operator[](int32_t offset) const {
		return parent->safe_at(position + offset);
	}
	T* operator->() const {
		return parent->safe_at(position);
	}
	fixed_sz_deque_iterator& operator++() {
		++position;
		return *this;
	}
	fixed_sz_deque_iterator& operator--() {
		++position;
		return *this;
	}
	fixed_sz_deque_iterator operator++(int) {
		++position;
		return fixed_sz_deque_iterator(parent, position - 1);
	}
	fixed_sz_deque_iterator operator--(int) {
		++position;
		return fixed_sz_deque_iterator(parent, position + 1);
	}
	fixed_sz_deque_iterator& operator+=(int32_t v) {
		position += v;
		return *this;
	}
	fixed_sz_deque_iterator& operator-=(int32_t v) {
		position -= v;
		return *this;
	}
	fixed_sz_deque_iterator operator+(int32_t v) {
		return fixed_sz_deque_iterator(parent, position + v);
	}
	fixed_sz_deque_iterator operator-(int32_t v) {
		return fixed_sz_deque_iterator(parent, position - v);
	}
	int32_t operator-(const fixed_sz_deque_iterator& o) {
		return position - o.position;
	}
	bool operator<(const fixed_sz_deque_iterator& o) const {
		return position < o.position;
	}
	bool operator<=(const fixed_sz_deque_iterator& o) const {
		return position <= o.position;
	}
	bool operator>(const fixed_sz_deque_iterator& o) const {
		return position > o.position;
	}
	bool operator>=(const fixed_sz_deque_iterator& o) const {
		return position >= o.position;
	}
};

template<typename T, uint32_t block, uint32_t index_sz>
class std::iterator_traits<fixed_sz_deque_iterator<T, block, index_sz>> {
public:
	using difference_type = int32_t;
	using value_type = T;
	using pointer = T*;
	using reference = T*&;
	using iterator_category = std::random_access_iterator_tag;
};