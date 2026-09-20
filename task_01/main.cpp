#include <climits>
#include <cstddef>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>

class TextBuffer {
private:
    char* data_;
    std::size_t size_;
    std::size_t capacity_;

    void reserve(std::size_t capacity) {
        if (capacity <= capacity_) return;
        char* next = new char[capacity];
        for (std::size_t i = 0; i <= size_; ++i) next[i] = data_[i];
        delete[] data_;
        data_ = next;
        capacity_ = capacity;
    }

    void swap(TextBuffer& other) noexcept {
        char* data = data_; data_ = other.data_; other.data_ = data;
        std::size_t size = size_; size_ = other.size_; other.size_ = size;
        std::size_t capacity = capacity_; capacity_ = other.capacity_; other.capacity_ = capacity;
    }

public:
    TextBuffer() : data_(new char[16]), size_(0), capacity_(16) { data_[0] = '\0'; }

    TextBuffer(const TextBuffer& other) : data_(new char[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
        for (std::size_t i = 0; i <= size_; ++i) data_[i] = other.data_[i];
    }

    TextBuffer& operator=(const TextBuffer& other) {
        if (this != &other) { TextBuffer copy(other); swap(copy); }
        return *this;
    }

    ~TextBuffer() { delete[] data_; }

    void push(char value) {
        if (size_ + 1 >= capacity_) reserve(capacity_ * 2);
        data_[size_++] = value;
        data_[size_] = '\0';
    }

    void clear() { size_ = 0; data_[0] = '\0'; }
    const char* data() const { return data_; }
    std::size_t size() const { return size_; }
};

bool parse_integer(const char* text, int& result) {
    if (!text || !*text) return false;
    std::size_t position = 0;
    bool negative = false;
    if (text[position] == '+' || text[position] == '-') {
        negative = text[position] == '-';
        ++position;
    }
    if (!std::isdigit(static_cast<unsigned char>(text[position]))) return false;
    long long value = 0;
    while (text[position]) {
        const unsigned char symbol = static_cast<unsigned char>(text[position]);
        if (!std::isdigit(symbol)) return false;
        value = value * 10 + (text[position] - '0');
        const long long limit = negative ? -(static_cast<long long>(INT_MIN)) : INT_MAX;
        if (value > limit) return false;
        ++position;
    }
    result = negative ? static_cast<int>(-value) : static_cast<int>(value);
    return true;
}

class SkewHeap {
private:
    struct Node {
        int value;
        Node* left;
        Node* right;
        explicit Node(int number) : value(number), left(nullptr), right(nullptr) {}
    };

    Node* root_;
    bool minimum_first_;

    static bool comes_first(int left, int right, bool minimum_first) {
        return minimum_first ? left < right : left > right;
    }

    static Node* meld(Node* first, Node* second, bool minimum_first) {
        if (!first) return second;
        if (!second) return first;
        if (comes_first(second->value, first->value, minimum_first)) {
            Node* temporary = first;
            first = second;
            second = temporary;
        }
        Node* old_left = first->left;
        first->left = meld(first->right, second, minimum_first);
        first->right = old_left;
        return first;
    }

    static void destroy(Node* node) noexcept {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    static Node* clone(const Node* node) {
        if (!node) return nullptr;
        Node* copy = new Node(node->value);
        try {
            copy->left = clone(node->left);
            copy->right = clone(node->right);
        } catch (...) {
            destroy(copy);
            throw;
        }
        return copy;
    }

    void swap(SkewHeap& other) noexcept {
        Node* root = root_; root_ = other.root_; other.root_ = root;
        bool order = minimum_first_; minimum_first_ = other.minimum_first_; other.minimum_first_ = order;
    }

public:
    explicit SkewHeap(bool minimum_first) : root_(nullptr), minimum_first_(minimum_first) {}
    SkewHeap(const SkewHeap& other) : root_(clone(other.root_)), minimum_first_(other.minimum_first_) {}
    SkewHeap& operator=(const SkewHeap& other) { if (this != &other) { SkewHeap copy(other); swap(copy); } return *this; }
    ~SkewHeap() { destroy(root_); }

    void push(int value) { root_ = meld(root_, new Node(value), minimum_first_); }

    int pop() {
        if (!root_) throw std::underflow_error("Heap is empty");
        Node* removed = root_;
        const int result = removed->value;
        root_ = meld(removed->left, removed->right, minimum_first_);
        delete removed;
        return result;
    }
};

class IntegerArray {
public:
    struct Statistics {
        std::size_t size;
        int minimum;
        std::size_t minimum_index;
        int maximum;
        std::size_t maximum_index;
        int most_frequent;
        double average;
        double maximum_deviation;
    };

private:
    int* data_;
    std::size_t size_;
    std::size_t capacity_;

    void reserve(std::size_t capacity) {
        if (capacity <= capacity_) return;
        int* next = new int[capacity];
        for (std::size_t i = 0; i < size_; ++i) next[i] = data_[i];
        delete[] data_;
        data_ = next;
        capacity_ = capacity;
    }

    void swap(IntegerArray& other) noexcept {
        int* data = data_; data_ = other.data_; other.data_ = data;
        std::size_t size = size_; size_ = other.size_; other.size_ = size;
        std::size_t capacity = capacity_; capacity_ = other.capacity_; other.capacity_ = capacity;
    }

public:
    IntegerArray() : data_(nullptr), size_(0), capacity_(0) {}

    IntegerArray(const IntegerArray& other) : data_(nullptr), size_(0), capacity_(0) {
        if (other.size_ != 0) {
            data_ = new int[other.size_];
            capacity_ = other.size_;
            size_ = other.size_;
            for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        }
    }

    IntegerArray& operator=(const IntegerArray& other) {
        if (this != &other) { IntegerArray copy(other); swap(copy); }
        return *this;
    }

    ~IntegerArray() { delete[] data_; }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    int at(std::size_t index) const {
        if (index >= size_) throw std::out_of_range("Array index is out of range");
        return data_[index];
    }

    void append(int value) {
        if (size_ == capacity_) reserve(capacity_ == 0 ? 8 : capacity_ * 2);
        data_[size_++] = value;
    }

    void clear() noexcept {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    void concat(const IntegerArray& other) {
        if (other.size_ > static_cast<std::size_t>(-1) - size_) throw std::overflow_error("Array is too large");
        reserve(size_ + other.size_);
        for (std::size_t i = 0; i < other.size_; ++i) data_[size_++] = other.data_[i];
    }

    void remove(std::size_t start, std::size_t count) {
        if (start > size_ || count > size_ - start) throw std::out_of_range("Remove range is out of bounds");
        for (std::size_t i = start + count; i < size_; ++i) data_[i - count] = data_[i];
        size_ -= count;
    }

    IntegerArray copy_range(std::size_t first, std::size_t last) const {
        if (first > last || last >= size_) throw std::out_of_range("Copy range is out of bounds");
        IntegerArray result;
        result.reserve(last - first + 1);
        for (std::size_t i = first; i <= last; ++i) result.append(data_[i]);
        return result;
    }

    void sort(bool ascending) {
        SkewHeap heap(ascending);
        for (std::size_t i = 0; i < size_; ++i) heap.push(data_[i]);
        for (std::size_t i = 0; i < size_; ++i) data_[i] = heap.pop();
    }

    template <typename Random>
    void shuffle(Random& random) {
        if (size_ < 2) return;
        for (std::size_t i = size_ - 1; i > 0; --i) {
            const std::size_t index = random.index(i + 1);
            const int temporary = data_[i];
            data_[i] = data_[index];
            data_[index] = temporary;
        }
    }

    Statistics statistics() const {
        if (empty()) throw std::domain_error("Statistics are undefined for an empty array");
        Statistics result;
        result.size = size_;
        result.minimum = result.maximum = data_[0];
        result.minimum_index = result.maximum_index = 0;
        long double sum = 0.0;
        for (std::size_t i = 0; i < size_; ++i) {
            if (data_[i] < result.minimum) { result.minimum = data_[i]; result.minimum_index = i; }
            if (data_[i] > result.maximum) { result.maximum = data_[i]; result.maximum_index = i; }
            sum += data_[i];
        }
        result.average = static_cast<double>(sum / size_);

        std::size_t best_count = 0;
        result.most_frequent = data_[0];
        for (std::size_t i = 0; i < size_; ++i) {
            std::size_t count = 0;
            for (std::size_t j = 0; j < size_; ++j) if (data_[j] == data_[i]) ++count;
            if (count > best_count || (count == best_count && data_[i] > result.most_frequent)) {
                best_count = count;
                result.most_frequent = data_[i];
            }
        }

        result.maximum_deviation = 0.0;
        for (std::size_t i = 0; i < size_; ++i) {
            double difference = data_[i] - result.average;
            if (difference < 0.0) difference = -difference;
            if (difference > result.maximum_deviation) result.maximum_deviation = difference;
        }
        return result;
    }
};

class RandomGenerator {
private:
    unsigned long long state_;
    unsigned long long next_raw() {
        state_ = state_ * 6364136223846793005ULL + 1442695040888963407ULL;
        return state_;
    }

public:
    RandomGenerator() : state_(static_cast<unsigned long long>(std::time(nullptr))) {}

    int integer(int lower, int upper) {
        if (lower > upper) throw std::invalid_argument("Random range is invalid");
        const unsigned long long span = static_cast<unsigned long long>(static_cast<long long>(upper) - lower) + 1ULL;
        return static_cast<int>(static_cast<long long>(lower) + static_cast<long long>(next_raw() % span));
    }

    std::size_t index(std::size_t upper_exclusive) {
        if (upper_exclusive == 0) throw std::invalid_argument("Random index range is empty");
        return static_cast<std::size_t>(next_raw() % upper_exclusive);
    }
};

class ArrayStore {
public:
    virtual ~ArrayStore() {}
    virtual IntegerArray* find(const char* name) = 0;
    virtual const IntegerArray* find(const char* name) const = 0;
    virtual IntegerArray& get_or_create(const char* name) = 0;
};

class TrieArrayStore : public ArrayStore {
private:
    struct Node {
        Node* children[62];
        IntegerArray* array;
        Node() : array(nullptr) { for (int i = 0; i < 62; ++i) children[i] = nullptr; }
    };

    Node* root_;

    static int symbol_index(char symbol) {
        if (symbol >= '0' && symbol <= '9') return symbol - '0';
        if (symbol >= 'A' && symbol <= 'Z') return 10 + symbol - 'A';
        if (symbol >= 'a' && symbol <= 'z') return 36 + symbol - 'a';
        return -1;
    }

    static bool valid_name(const char* name) {
        if (!name || !*name || (name[0] >= '0' && name[0] <= '9')) return false;
        std::size_t length = 0;
        while (name[length]) {
            if (symbol_index(name[length]) < 0 || length >= 16) return false;
            ++length;
        }
        return length <= 16;
    }

    static void destroy(Node* node) noexcept {
        if (!node) return;
        for (int i = 0; i < 62; ++i) destroy(node->children[i]);
        delete node->array;
        delete node;
    }

    static Node* clone(const Node* node) {
        if (!node) return nullptr;
        Node* copy = new Node();
        try {
            if (node->array) copy->array = new IntegerArray(*node->array);
            for (int i = 0; i < 62; ++i) copy->children[i] = clone(node->children[i]);
        } catch (...) {
            destroy(copy);
            throw;
        }
        return copy;
    }

    void swap(TrieArrayStore& other) noexcept { Node* root = root_; root_ = other.root_; other.root_ = root; }

    Node* find_node(const char* name) {
        if (!valid_name(name)) throw std::invalid_argument("Invalid array name");
        Node* node = root_;
        for (std::size_t i = 0; name[i] && node; ++i) node = node->children[symbol_index(name[i])];
        return node;
    }

    const Node* find_node(const char* name) const {
        if (!valid_name(name)) throw std::invalid_argument("Invalid array name");
        const Node* node = root_;
        for (std::size_t i = 0; name[i] && node; ++i) node = node->children[symbol_index(name[i])];
        return node;
    }

public:
    TrieArrayStore() : root_(new Node()) {}
    TrieArrayStore(const TrieArrayStore& other) : root_(clone(other.root_)) {}
    TrieArrayStore& operator=(const TrieArrayStore& other) { if (this != &other) { TrieArrayStore copy(other); swap(copy); } return *this; }
    ~TrieArrayStore() override { destroy(root_); }

    IntegerArray* find(const char* name) override {
        Node* node = find_node(name);
        return node ? node->array : nullptr;
    }

    const IntegerArray* find(const char* name) const override {
        const Node* node = find_node(name);
        return node ? node->array : nullptr;
    }

    IntegerArray& get_or_create(const char* name) override {
        if (!valid_name(name)) throw std::invalid_argument("Invalid array name");
        Node* node = root_;
        for (std::size_t i = 0; name[i]; ++i) {
            const int index = symbol_index(name[i]);
            if (!node->children[index]) node->children[index] = new Node();
            node = node->children[index];
        }
        if (!node->array) node->array = new IntegerArray();
        return *node->array;
    }
};

class CommandParser {
private:
    const char* text_;
    std::size_t position_;

public:
    explicit CommandParser(const char* text) : text_(text), position_(0) {}

    void spaces() {
        while (std::isspace(static_cast<unsigned char>(text_[position_]))) ++position_;
    }

    void word(char* result, std::size_t capacity) {
        spaces();
        std::size_t size = 0;
        while (std::isalpha(static_cast<unsigned char>(text_[position_]))) {
            if (size + 1 >= capacity) throw std::invalid_argument("Word is too long");
            result[size++] = text_[position_++];
        }
        if (size == 0) throw std::invalid_argument("Word expected");
        result[size] = '\0';
    }

    void identifier(char result[17]) {
        spaces();
        if (!std::isalpha(static_cast<unsigned char>(text_[position_]))) throw std::invalid_argument("Array name must start with a letter");
        std::size_t size = 0;
        while (std::isalnum(static_cast<unsigned char>(text_[position_]))) {
            if (size >= 16) throw std::invalid_argument("Array name is longer than 16 characters");
            result[size++] = text_[position_++];
        }
        result[size] = '\0';
    }

    void expect(char symbol) {
        spaces();
        if (text_[position_] != symbol) throw std::invalid_argument("Unexpected command syntax");
        ++position_;
    }

    int integer() {
        spaces();
        char buffer[32];
        std::size_t size = 0;
        if (text_[position_] == '+' || text_[position_] == '-') buffer[size++] = text_[position_++];
        while (std::isdigit(static_cast<unsigned char>(text_[position_]))) {
            if (size + 1 >= sizeof(buffer)) throw std::invalid_argument("Integer is too long");
            buffer[size++] = text_[position_++];
        }
        buffer[size] = '\0';
        int result = 0;
        if (!parse_integer(buffer, result)) throw std::invalid_argument("Valid integer expected");
        return result;
    }

    std::size_t index() {
        const int value = integer();
        if (value < 0) throw std::invalid_argument("Index must not be negative");
        return static_cast<std::size_t>(value);
    }

    void remaining(char* result, std::size_t capacity) {
        spaces();
        const std::size_t start = position_;
        std::size_t end = start;
        while (text_[end]) ++end;
        while (end > start && std::isspace(static_cast<unsigned char>(text_[end - 1]))) --end;
        if (end == start) throw std::invalid_argument("Non-empty argument expected");
        if (end - start + 1 > capacity) throw std::invalid_argument("Argument is too long");
        std::size_t size = 0;
        while (position_ < end) result[size++] = text_[position_++];
        result[size] = '\0';
    }

    bool read_literal(const char* literal) {
        spaces();
        const std::size_t saved = position_;
        std::size_t index = 0;
        while (literal[index] && text_[position_] == literal[index]) { ++position_; ++index; }
        if (literal[index]) { position_ = saved; return false; }
        return true;
    }

    bool take(char symbol) {
        spaces();
        if (text_[position_] != symbol) return false;
        ++position_;
        return true;
    }

    char character() { spaces(); return text_[position_] ? text_[position_++] : '\0'; }

    void end() {
        spaces();
        if (text_[position_] != '\0') throw std::invalid_argument("Unexpected characters at the end of command");
    }
};

bool same_text(const char* first, const char* second) {
    std::size_t index = 0;
    while (first[index] && second[index] && first[index] == second[index]) ++index;
    return first[index] == second[index];
}

void process_loaded_token(const TextBuffer& token, IntegerArray& array) {
    if (token.size() == 0) return;
    int value = 0;
    if (parse_integer(token.data(), value)) array.append(value);
}

IntegerArray load_array(const char* path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open input data file");
    IntegerArray result;
    TextBuffer token;
    char symbol = 0;
    while (input.get(symbol)) {
        if (std::isspace(static_cast<unsigned char>(symbol))) {
            process_loaded_token(token, result);
            token.clear();
        } else {
            token.push(symbol);
        }
    }
    if (!input.eof()) throw std::runtime_error("Cannot read input data file");
    process_loaded_token(token, result);
    return result;
}

void save_array(const char* path, const IntegerArray& array) {
    std::ofstream output(path);
    if (!output) throw std::runtime_error("Cannot open output data file");
    for (std::size_t i = 0; i < array.size(); ++i) {
        if (i != 0) output << ' ';
        output << array.at(i);
    }
    output << '\n';
    if (!output) throw std::runtime_error("Cannot write output data file");
}

class Interpreter {
private:
    ArrayStore& store_;
    RandomGenerator random_;

    IntegerArray& require(const char* name) {
        IntegerArray* array = store_.find(name);
        if (!array) throw std::runtime_error("Array does not exist");
        return *array;
    }

    const IntegerArray& require(const char* name) const {
        const IntegerArray* array = store_.find(name);
        if (!array) throw std::runtime_error("Array does not exist");
        return *array;
    }

    static void print_range(const IntegerArray& array, std::size_t first, std::size_t last) {
        if (first > last || last >= array.size()) throw std::out_of_range("Print range is out of bounds");
        for (std::size_t i = first; i <= last; ++i) {
            if (i != first) std::cout << ' ';
            std::cout << array.at(i);
        }
        std::cout << '\n';
    }

public:
    explicit Interpreter(ArrayStore& store) : store_(store), random_() {}

    void execute(const char* command_text) {
        CommandParser parser(command_text);
        char command[16];
        char first[17];
        char second[17];
        parser.word(command, sizeof(command));

        if (same_text(command, "Load")) {
            parser.identifier(first); parser.expect(',');
            char path[512]; parser.remaining(path, sizeof(path)); parser.end();
            store_.get_or_create(first) = load_array(path);
        } else if (same_text(command, "Save")) {
            parser.identifier(first); parser.expect(',');
            char path[512]; parser.remaining(path, sizeof(path)); parser.end();
            save_array(path, require(first));
        } else if (same_text(command, "Rand")) {
            parser.identifier(first); parser.expect(',');
            const std::size_t count = parser.index(); parser.expect(',');
            const int lower = parser.integer(); parser.expect(',');
            const int upper = parser.integer(); parser.end();
            if (lower > upper) throw std::invalid_argument("Lower bound is greater than upper bound");
            IntegerArray generated;
            for (std::size_t i = 0; i < count; ++i) generated.append(random_.integer(lower, upper));
            store_.get_or_create(first) = generated;
        } else if (same_text(command, "Concat")) {
            parser.identifier(first); parser.expect(','); parser.identifier(second); parser.end();
            IntegerArray& destination = require(first);
            const IntegerArray& source = require(second);
            if (&destination == &source) { IntegerArray copy(source); destination.concat(copy); }
            else destination.concat(source);
        } else if (same_text(command, "Free")) {
            parser.expect('('); parser.identifier(first); parser.expect(')'); parser.end();
            store_.get_or_create(first).clear();
        } else if (same_text(command, "Remove")) {
            parser.identifier(first); parser.expect(',');
            const std::size_t start = parser.index(); parser.expect(',');
            const std::size_t count = parser.index(); parser.end();
            require(first).remove(start, count);
        } else if (same_text(command, "Copy")) {
            parser.identifier(first); parser.expect(',');
            const std::size_t start = parser.index(); parser.expect(',');
            const std::size_t finish = parser.index(); parser.expect(',');
            parser.identifier(second); parser.end();
            IntegerArray copied = require(first).copy_range(start, finish);
            store_.get_or_create(second) = copied;
        } else if (same_text(command, "Sort")) {
            parser.identifier(first);
            const char order = parser.character(); parser.end();
            if (order != '+' && order != '-') throw std::invalid_argument("Sort order must be + or -");
            require(first).sort(order == '+');
        } else if (same_text(command, "Shuffle")) {
            parser.identifier(first); parser.end();
            require(first).shuffle(random_);
        } else if (same_text(command, "Stats")) {
            parser.identifier(first); parser.end();
            const IntegerArray::Statistics stats = require(first).statistics();
            std::cout << "Stats " << first << ":\n"
                      << "  size: " << stats.size << '\n'
                      << "  min: " << stats.minimum << " at " << stats.minimum_index << '\n'
                      << "  max: " << stats.maximum << " at " << stats.maximum_index << '\n'
                      << "  most frequent: " << stats.most_frequent << '\n'
                      << "  average: " << stats.average << '\n'
                      << "  max deviation: " << stats.maximum_deviation << '\n';
        } else if (same_text(command, "Print")) {
            parser.identifier(first); parser.expect(',');
            const IntegerArray& array = require(first);
            if (parser.read_literal("all")) {
                parser.end();
                if (array.empty()) std::cout << '\n';
                else print_range(array, 0, array.size() - 1);
            } else {
                const std::size_t start = parser.index();
                if (parser.take(',')) {
                    const std::size_t finish = parser.index(); parser.end();
                    print_range(array, start, finish);
                } else {
                    parser.end();
                    std::cout << array.at(start) << '\n';
                }
            }
        } else {
            throw std::invalid_argument("Unknown command");
        }
    }

    void run_file(const char* path) {
        std::ifstream input(path);
        if (!input) throw std::runtime_error("Cannot open command file");
        TextBuffer command;
        std::size_t command_number = 0;
        char symbol = 0;
        while (input.get(symbol)) {
            if (symbol == ';') {
                ++command_number;
                try {
                    execute(command.data());
                } catch (const std::exception& error) {
                    std::cerr << "Command " << command_number << " error: " << error.what() << '\n';
                }
                command.clear();
            } else {
                command.push(symbol);
            }
        }
        if (!input.eof()) throw std::runtime_error("Cannot read command file");
        bool only_spaces = true;
        for (std::size_t i = 0; i < command.size(); ++i)
            if (!std::isspace(static_cast<unsigned char>(command.data()[i]))) only_spaces = false;
        if (!only_spaces) std::cerr << "Error: final command has no semicolon\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: task_01 [commands_file]\n";
        return 1;
    }
    const char* command_file = argc == 2 ? argv[1] : "task_01/commands.txt";
    try {
        TrieArrayStore arrays;
        Interpreter interpreter(arrays);
        interpreter.run_file(command_file);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
