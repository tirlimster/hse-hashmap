#include <cstddef>
#include <vector>
#include <initializer_list>
#include <stdexcept>
#include <memory>


template<class ValueType>
class Optional {
    std::unique_ptr<ValueType> data;

   public:
    Optional() : data(nullptr) {}

    Optional(ValueType dt) : data(new ValueType(dt)) {}

    Optional& operator=(const ValueType& dt) {
        data.reset(new ValueType(dt));
        return *this;
    }

    Optional(const Optional& other) : Optional() {
        if (other) {
            data.reset(new ValueType(*other));
        }
    }

    Optional& operator=(const Optional& other) {
        if (this == &other) {
            return *this;
        }
        data.reset(new ValueType(*other));
        return *this;
    }

    ValueType& operator*() {
        return *data;
    }

    const ValueType& operator*() const {
        return *data;
    }

    ValueType* operator->() {
        return data.get();
    }

    const ValueType* operator->() const {
        return data.get();
    }

    void reset() {
        data.reset(nullptr);
    }

    operator bool() const {
        return bool(data);
    }
};


template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
    using PairType = std::pair<KeyType, ValueType>;
    using OutType = std::pair<const KeyType, ValueType>;
    using VertType = Optional<PairType>;

    size_t sz;
    std::vector<VertType> data;
    Hash hasher;

    void recap(size_t new_cap) {
        std::vector<VertType> tmp_data(new_cap);
        swap(data, tmp_data);
        sz = 0;
        for (const auto& ver : tmp_data) {
            if (ver) {
                insert(*ver);
            }
        }
    }

    size_t locate(const KeyType& key) const {
        return static_cast<size_t>(hasher(key)) % data.size();
    }

    size_t next_index(size_t index) const {
        return (index + 1) % data.size();
    }

   public:
    class iterator {
        size_t index;
        HashMap* owner;

       public:
        iterator() : index(0), owner(nullptr) {}

        iterator(size_t ind, HashMap* own) : index(ind), owner(own) {}

        OutType& operator*() {
            return reinterpret_cast<OutType&>(*owner->data[index]);
        }

        OutType* operator->() {
            return reinterpret_cast<OutType*>(&(*owner->data[index]));
        }

        iterator operator++(int) {
            auto copy = *this;
            do {
                index++;
            } while (index < owner->data.size() && !owner->data[index]);
            return copy;
        }

        iterator operator++() {
            do {
                index++;
            } while (index < owner->data.size() && !owner->data[index]);
            return *this;
        }

        template<class OtherIt>
        bool operator==(const OtherIt& other) const {
            return owner == other.owner && index == other.index;
        }

        template<class OtherIt>
        bool operator!=(const OtherIt& other) const {
            return !(*this == other);
        }
    };

    class const_iterator {
        size_t index;
        const HashMap* owner;

       public:
        const_iterator() : index(0), owner(nullptr) {}

        const_iterator(size_t ind, const HashMap* own) : index(ind),
                                                   owner(own) {}

        const OutType& operator*() const {
            return reinterpret_cast<const OutType&>(*owner->data[index]);
        }

        const OutType* operator->() const {
            return reinterpret_cast<const OutType*>(&(*owner->data[index]));
        }

        const_iterator operator++(int) {
            auto copy = *this;
            do {
                index++;
            } while (index < owner->data.size() && !owner->data[index]);
            return copy;
        }

        const_iterator operator++() {
            do {
                index++;
            } while (index < owner->data.size() && !owner->data[index]);
            return *this;
        }

        template<class OtherIt>
        bool operator==(const OtherIt& other) const {
            return owner == other.owner && index == other.index;
        }

        template<class OtherIt>
        bool operator!=(const OtherIt& other) const {
            return !(*this == other);
        }
    };

    HashMap(Hash hsr = Hash()) : sz(0), hasher(hsr) {}

    template<class Iter>
    HashMap(Iter beg, Iter end, Hash hsr = Hash()) : HashMap(hsr) {
        while (beg != end) {
            insert(*(beg++));
        }
    }

    HashMap(std::initializer_list<PairType> lst, Hash hsr = Hash()) :
                    HashMap(lst.begin(), lst.end(), hsr) {}

    bool empty() const {
        return sz == 0;
    }

    size_t size() const {
        return sz;
    }

    Hash hash_function() const {
        return hasher;
    }

    void insert(const PairType& pr) {
        if (4 * (sz + 1) >= data.size()) {
            recap(8 * (sz + 1));
        }
        size_t index = locate(pr.first);
        while (data[index]) {
            if (data[index]->first == pr.first) {
                return;
            }
            index = next_index(index);
        }
        sz += 1;
        data[index] = pr;
    }

    void erase(const KeyType& key) {
        if (data.empty()) {
            return;
        }
        size_t index = locate(key);
        std::vector<PairType> need_to_insert;
        bool found = false;
        while (data[index]) {
            if (found) {
                need_to_insert.emplace_back(*data[index]);
                data[index].reset();
                sz -= 1;
            }
            if (!found && data[index]->first == key) {
                found = true;
                data[index].reset();
                sz -= 1;
            }
            index = next_index(index);
        }
        for (const auto& ver : need_to_insert) {
            insert(ver);
        }
    }

    iterator begin() {
        size_t index = 0;
        while (index < data.size() && !data[index]) {
            index++;
        }
        return iterator{index, this};
    }

    iterator end() {
        return iterator{data.size(), this};
    }

    const_iterator begin() const {
        size_t index = 0;
        while (index < data.size() && !data[index]) {
            index++;
        }
        return const_iterator{index, this};
    }

    const_iterator end() const {
        return const_iterator{data.size(), this};
    }

    const_iterator find(const KeyType& key) const {
        if (data.empty()) {
            return end();
        }
        size_t index = locate(key);
        while (data[index]) {
            if (data[index]->first == key) {
                return const_iterator{index, this};
            }
            index = next_index(index);
        }
        return end();
    }

    iterator find(const KeyType& key) {
        if (data.empty()) {
            return end();
        }
        size_t index = locate(key);
        while (data[index]) {
            if (data[index]->first == key) {
                return iterator{index, this};
            }
            index = next_index(index);
        }
        return end();
    }

    ValueType& operator[](const KeyType& key) {
        if (find(key) == end()) {
            insert({key, ValueType()});
        }
        return find(key)->second;
    }

    const ValueType& at(const KeyType& key) const {
        if (find(key) == end()) {
            throw std::out_of_range("this key does not exist");
        }
        return find(key)->second;
    }

    void clear() {
        sz = 0;
        for (auto& vertex : data) {
            vertex.reset();
        }
    }
};
