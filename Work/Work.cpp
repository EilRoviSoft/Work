#include <functional>
#include <iostream>
#include <tuple>

template <class _ValType>
struct list_cell_t {
	list_cell_t(_ValType value, list_cell_t <_ValType>* left, list_cell_t <_ValType>* right) : v(value), l(left), r(right) {}
	_ValType v;
	list_cell_t <_ValType>* l, * r;
};

template <class _Type>
using carray_t = std::tuple <_Type*, size_t>;

template <class _ValType>
class list {
	using value_t = _ValType;
	using value_ptr = _ValType*;
	using cell_t = list_cell_t <_ValType>;
	using cell_ptr = list_cell_t <_ValType>*;
	using container_t = list <_ValType>;

	class iterator {
		friend class list;
	public:
		iterator(const container_t& content, cell_ptr current) : m_container(content), m_current(current) {}

		value_t& operator*() {
			return this->m_current->v;
		}
		const value_t& value() const {
			return this->m_current->v;
		}

		iterator& operator++() {
			this->m_current = this->m_current->r;
			return *this;
		}
		iterator next() {
			this->m_current = this->m_current->r;
			return *this;
		}

		bool is_out_of_bound() const {
			return this->m_current == nullptr;
		}

		bool operator==(const iterator& another) const {
			return this->m_current == another.m_current;
		}
		bool operator!=(const iterator& another) const {
			return this->m_current != another.m_current;
		}

	protected:
		const container_t& m_container;
		cell_ptr m_current;

	private:
		iterator(const container_t& container) : m_container(container), m_current(nullptr) {}
	};
public:
	list() {}
	list(const list& another) {
		if (another.size() == 0)
			return;

		this->_resize_if_empty(another.size());
		auto it = this->m_head;
		for (auto jt : another) {
			it->v = jt;
			it = it->r;
		}
	}
	list(list&& another) noexcept {
		this->m_head = another.m_head;
		this->m_tail = another.m_tail;
		this->m_size = another.m_size;
	}
	explicit list(std::initializer_list <value_t> content) {
		this->_resize_if_empty(content.size());
		auto it = this->m_head;
		for (auto jt : content) {
			it->v = jt;
			it = it->r;
		}
	}
	~list() {
		this->_clear();
	}

	container_t& operator=(const container_t& another) {
		this->_clear();
		if (another.size() != 0) {
			this->_resize_if_empty(another.size());
			auto it = this->m_head;
			for (auto jt : another) {
				it->v = jt;
				it = it->r;
			}
		}
		return *this;
	}

	friend container_t operator+(const container_t& lhs, const container_t& rhs) {
		container_t result;
		result._resize_if_empty(lhs.size() + rhs.size());
		auto it = result.begin();
		for (auto jt = lhs.begin(); jt != lhs.end(); ++jt, ++it)
			*it = *jt;
		for (auto jt = rhs.begin(); jt != rhs.end(); ++jt, ++it)
			*it = *jt;
		return result;
	}
	container_t& operator+=(const container_t& another) {
		this->push_range_back(another);
		return *this;
	}

	void insert(value_t val, iterator place) {
		auto curr = place.m_current;
		auto prev = curr->l;
		auto next = curr;
		auto on_insert = new cell_t(val, prev, curr);
		if (prev)
			prev->r = on_insert;
		else
			this->m_head = on_insert;
		next->l = on_insert;
	}
	void insert(value_t val, size_t idx) {
		if (idx >= this->m_size)
			return;
		auto place = this->_index_to_iterator(idx);
		this->insert(val, place);
	}

	//add
	void push_back(value_t val) {
		this->m_tail->r = new cell_t(val, this->m_tail, nullptr);
		this->m_tail = this->m_tail->r;
		++this->m_size;
	}
	//add_range
	void push_range_back(const container_t& data) {
		for (auto it = data.begin(); it != data.end(); ++it) {
			this->m_tail->r = new cell_t(it.value(), this->m_tail, nullptr);
			this->m_tail = this->m_tail->r;
		}
		this->m_size += data.size();
	}
	//add_range
	void push_range_back(value_ptr range, size_t size) {
		for (size_t i = 0; i != size; ++i) {
			this->m_tail->r = new cell_t(range[i], this->m_tail, nullptr);
			this->m_tail = this->m_tail->r;
		}
		this->m_size += size;
	}

	//remove_elem
	void remove(size_t idx) {
		auto curr = this->_index_to_iterator(idx).m_current;
		auto prev = curr->l;
		auto next = curr->r;

		prev->r = next;
		next->l = prev;
		delete curr;
		--this->m_size;
	}

	//set_elem
	void set(const value_t& val, size_t idx) {
		auto it = this->_index_to_iterator(idx);
		*it = val;
	}
	//get_elem
	value_t& get(size_t idx) {
		auto it = this->_index_to_iterator(idx);
		return *it;
	}

	value_t& operator[](size_t idx) {
		auto it = this->_index_to_iterator(idx);
		return *it;
	}

	//bubble_sort
	void sort(const std::function <int(const value_t& l, const value_t& r)>& comp) {
		bool swapped = true;
		for (size_t i = 0; i != this->m_size - 1; ++i) {
			swapped = false;
			auto curr = this->m_head;
			auto next = curr->r;
			for (size_t j = 0; j != this->m_size - i - 1; ++j) {
				if (comp(curr->v, next->v)) {
					value_t t = curr->v;
					curr->v = next->v;
					next->v = t;
					swapped = true;
				}
				curr = next;
				next = next->r;
			}
			if (!swapped)
				break;
		}
	}

	//вообще, лучше бы выдавать uint64_t.max(), а не -1
	int64_t get_index(const value_t& what) const {
		size_t result = 0;
		for (auto it = this->begin(); it != this->end(); ++it, ++result) {
			if (*it == what)
				return result;
		}
		return -1;
	}

	carray_t <value_t> to_array() const {
		value_ptr result = new value_t[this->m_size];
		size_t i = 0;
		for (auto it = this->begin(); it != this->end(); ++it, ++i)
			result[i] = *it;
		return { result, this->m_size };
	}
	void reverse() {
		cell_ptr it = this->m_head, jt = this->m_tail;
		for (size_t i = 0; i != this->m_size / 2; i++) {
			value_t t = it->v;
			it->v = jt->v;
			jt->v = t;

			it = it->r;
			jt = jt->l;
		}
	}

	iterator begin() const {
		return iterator(*this, this->m_head);
	}
	iterator end() const {
		return iterator(*this, nullptr);
	}

	size_t size() const {
		return this->m_size;
	}

protected:
	cell_ptr m_head = nullptr, m_tail = nullptr;
	size_t m_size = 0;

private:
	static void _swap_iterators(iterator l, iterator r) {
		value_t t = l.m_current.v;
		l.m_current.v = r.m_current.v;
		r.m_current.v = t;
	}

	void _clear() {
		auto it = this->m_head;
		while (this->m_size) {
			auto temp = it;
			it = it->r;
			delete temp;
			--this->m_size;
		}
	}
	void _resize_if_empty(size_t nsize) {
		if (nsize == 0)
			return;

		this->m_size = nsize;
		this->m_head = new cell_t(0, nullptr, nullptr);
		this->m_tail = this->m_head;
		for (size_t i = 1; i != nsize; ++i) {
			this->m_tail->r = new cell_t(0, this->m_tail, nullptr);
			this->m_tail = this->m_tail->r;
		}
	}
	iterator _index_to_iterator(size_t idx) const {
		if (idx >= this->m_size)
			return this->end();
		iterator result = this->begin();
		while (idx) {
			--idx;
			++result;
		}
		return result;
	}
};

template <class _Type>
void print(list <_Type>& arr) {
	for (auto it = arr.begin(); it != arr.end(); ++it)
		std::cout << *it << ' ';
}

int fcmp(const float& l, const float& r) { return l < r; }

void test1() {
	list <float> l({ 3, 1, 8, 4, 9, 12, 5, 7 });
	l.sort(&fcmp);
	l.push_back(6);

	print(l);
	std::cout << '\n';

	{
		auto [arr, size] = l.to_array();
		for (size_t i = 0; i != size; i++)
			std::cout << arr[i] * arr[i] << ' ';
		std::cout << '\n';
		delete[] arr;
	}
	l.reverse();
	{
		auto [arr, size] = l.to_array();
		for (size_t i = 0; i != size; i++)
			std::cout << arr[i] * arr[i] << ' ';
		delete[] arr;
	}
}
void test2() {
	list <float> l1{ 1, 2, 3 }, l2{ 1, 2, 3 };
	l2 += l1;
	auto [arr, size] = l2.to_array();
	for (size_t i = 0; i != size; i++)
		std::cout << arr[i] << ' ';
	delete[] arr;
}

int main() {
	test1();
}