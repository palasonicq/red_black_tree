#include <algorithm>
#include <memory>

template <class ValueType>
class Set {
 public:
    ~Set() {
        if (!empty()) {
            clear(root);
        }
        delete leaf;
    }

    Set(const Set& other) {
        _size = other._size;
        if (root != leaf) {
            clear(root);
        }
        if (other.root == other.leaf) {
            root = leaf;
            return;
        }
        root = new Node(*(other.root));
        root->parent = leaf;
        root->l = leaf;
        root->r = leaf;
        copy_nodes(other.root, root, other.leaf);
    }

    Set& operator=(const Set& other) {
        Set temp(other);
        if (this != &other) {
            temp.swap(*this);
        }
        return *this;
    }

    void swap(Set& other) {
        std::swap(_size, other._size);
        std::swap(leaf, other.leaf);
        std::swap(root, other.root);
    }

    Set() : _size(0), root(leaf) {}

    template <class _Iter>
    Set(_Iter First, _Iter Last) {
        _size = 0;
        root = leaf;
        while (First != Last) {
            InsertItem(*First);
            ++First;
        }
    }

    Set(std::initializer_list<const ValueType> list) {
        _size = 0;
        root = leaf;
        for (auto& x : list) {
            InsertItem(x);
        }
    }

    size_t size() const { return _size; }

    bool empty() const { return size() == 0; }

    void insert(ValueType key) { InsertItem(key); }

    void erase(const ValueType& key) {
        auto it = find(key);
        if (it == end()) {
            return;
        }
        --_size;
        Node* cur = root;
        while (cur->key != key) {
            if (cur->key < key) {
                cur = cur->r;
            } else {
                cur = cur->l;
            }
        }
        DeleteItem(cur);
    }

    class iterator;

    iterator begin() const {
        Node* cur_el = root;
        for (; cur_el != leaf && cur_el->l != leaf; cur_el = cur_el->l) {
        }
        return iterator(this, cur_el);
    }
    iterator end() const { return iterator(this, leaf); }

    iterator find(const ValueType& key) const {
        Node* cur = root;
        while (cur != leaf) {
            if (key < cur->key) {
                cur = cur->l;
            } else {
                if (cur->key < key) {
                    cur = cur->r;
                } else {
                    return iterator(this, cur);
                }
            }
        }
        return iterator(this, leaf);
    }

    iterator lower_bound(const ValueType& key) const {
        Node *cur = root, *result = leaf;
        while (cur != leaf) {
            if (key < cur->key) {
                result = cur;
                cur = cur->l;
            } else {
                if (cur->key < key) {
                    cur = cur->r;
                } else {
                    return iterator(this, cur);
                }
            }
        }
        return iterator(this, result);
    }

 private:
    enum node_color { BLACK, RED };
    struct Node {
        Node* l;
        Node* r;
        Node* parent;
        node_color color;
        ValueType key;
        Node()
            : l(nullptr),
              r(nullptr),
              parent(nullptr),
              color(BLACK),
              key(ValueType()) {}
        Node(ValueType _key, node_color _color, Node* _l, Node* _r,
             Node* _parent)
            : l(_l), r(_r), parent(_parent), color(_color), key(_key) {}
        Node(const Node& other) {
            l = other.l;
            r = other.r;
            parent = other.parent;
            color = other.color;
            key = other.key;
        }
    };

    void clear(Node* node) {
        if (node->l != leaf) {
            clear(node->l);
        }
        if (node->r != leaf) {
            clear(node->r);
        }
        delete node;
    }

    void copy_nodes(Node* first, Node*& second, Node* leaf_first) {
        if (first == leaf_first) {
            second = leaf;
            return;
        }
        if (first->l != leaf_first) {
            second->l = new Node(*(first->l));
            second->l->parent = second;
            copy_nodes(first->l, second->l, leaf_first);
        } else {
            second->l = leaf;
        }
        if (first->r != leaf_first) {
            second->r = new Node(*(first->r));
            second->r->parent = second;
            copy_nodes(first->r, second->r, leaf_first);
        } else {
            second->r = leaf;
        }
    }

    Node* leaf = new Node;
    size_t _size = 0;

    Node* root = leaf;

    void RotateLeft(Node* node) {
        Node* child = node->r;
        node->r = child->l;
        if (child != leaf) {
            child->parent = node->parent;
        }
        if (child->l != leaf) {
            child->l->parent = node;
        }
        if (node->parent == leaf) {
            root = child;
        } else {
            if (node != node->parent->l) {
                node->parent->r = child;
            } else {
                node->parent->l = child;
            }
        }
        child->l = node;
        if (node != leaf) {
            node->parent = child;
        }
    }

    void RotateRight(Node* node) {
        Node* child = node->l;
        node->l = child->r;
        if (child != leaf) {
            child->parent = node->parent;
        }
        if (child->r != leaf) {
            child->r->parent = node;
        }
        if (node->parent == leaf) {
            root = child;

        } else {
            if (node != node->parent->r) {
                node->parent->l = child;
            } else {
                node->parent->r = child;
            }
        }
        child->r = node;
        if (node != leaf) {
            node->parent = child;
        }
    }

    void InsertUpd(Node* node) {
        while (node != root && node->parent->color == RED) {
            bool isLeft = node->parent == node->parent->parent->l;
            Node* uncle =
                isLeft ? node->parent->parent->r : node->parent->parent->l;
            if (uncle->color == BLACK) {
                if (isLeft && node == node->parent->r) {
                    node = node->parent;
                    RotateLeft(node);
                } else {
                    if (!isLeft && node == node->parent->l) {
                        node = node->parent;
                        RotateRight(node);
                    }
                }
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                isLeft ? RotateRight(node->parent->parent)
                       : RotateLeft(node->parent->parent);
            } else {
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            }
        }
        root->color = BLACK;
    }

    void DeleteUpd(Node* node) {
        while (node != root && node->color == BLACK) {
            bool isLeft = node == node->parent->l;
            Node* brother = isLeft ? node->parent->r : node->parent->l;
            if (brother->color == RED) {
                brother->color = BLACK;
                node->parent->color = RED;
                if (isLeft) {
                    RotateLeft(node->parent);
                    brother = node->parent->r;
                } else {
                    RotateRight(node->parent);
                    brother = node->parent->l;
                }
            }
            if (brother->l->color == BLACK && brother->r->color == BLACK) {
                brother->color = RED;
                node = node->parent;
            } else {
                Node* child1 = isLeft ? brother->r : brother->l;
                Node* child2 = isLeft ? brother->l : brother->r;
                if (child1->color == BLACK) {
                    child2->color = BLACK;
                    brother->color = RED;
                    if (isLeft) {
                        RotateRight(brother);
                        brother = node->parent->r;
                    } else {
                        RotateLeft(brother);
                        brother = node->parent->l;
                    }
                }
                brother->color = node->parent->color;
                node->parent->color = BLACK;
                if (isLeft) {
                    brother->r->color = BLACK;
                    RotateLeft(node->parent);
                } else {
                    brother->l->color = BLACK;
                    RotateRight(node->parent);
                }
                node = root;
            }
        }
        node->color = BLACK;
    }

    Node* InsertItem(const ValueType key) {
        Node *cur, *parent, *new_node;
        cur = root;
        parent = leaf;
        while (cur != leaf) {
            if (!(cur->key < key) && !(key < cur->key)) {
                return cur;
            }
            parent = cur;
            if (key < cur->key) {
                cur = cur->l;
            } else {
                cur = cur->r;
            }
        }
        new_node = new Node(key, RED, leaf, leaf, parent);
        if (parent != leaf) {
            if (key < parent->key) {
                parent->l = new_node;
            } else {
                parent->r = new_node;
            }
        } else {
            root = new_node;
        }
        ++_size;
        InsertUpd(new_node);
        return new_node;
    }

    void DeleteItem(Node* node) {
        Node *child, *del_node;
        if (!node || node == leaf) {
            return;
        }
        if (node->l == leaf || node->r == leaf) {
            del_node = node;
        } else {
            del_node = node->r;
            while (del_node->l != leaf) {
                del_node = del_node->l;
            }
        }
        if (del_node->l != leaf) {
            child = del_node->l;
        } else {
            child = del_node->r;
        }
        child->parent = del_node->parent;
        if (del_node->parent != leaf) {
            if (del_node == del_node->parent->l) {
                del_node->parent->l = child;
            } else {
                del_node->parent->r = child;
            }
        } else {
            root = child;
        }
        if (del_node != node) {
            node->key = del_node->key;
        }
        if (del_node->color == BLACK) {
            DeleteUpd(child);
        }
        delete del_node;
    }
};

template <class ValueType>
class Set<ValueType>::iterator
    : public std::iterator<std::bidirectional_iterator_tag, const ValueType> {
 public:
    iterator(const Set<ValueType>* _owner = nullptr, Node* _cur_el = nullptr)
        : cur_el(_cur_el), owner(_owner) {}

    const ValueType& operator*() { return cur_el->key; }
    const ValueType* operator->() { return &**this; }

    iterator& operator++() {
        if (cur_el == owner->leaf) {
            return *this;
        }
        if (cur_el->r == owner->leaf) {
            for (; cur_el->parent != owner->leaf && cur_el->parent->r == cur_el;
                 cur_el = cur_el->parent) {
            }
            cur_el = cur_el->parent;
            return *this;
        }
        cur_el = cur_el->r;
        for (; cur_el->l != owner->leaf; cur_el = cur_el->l) {
        }
        return *this;
    }

    const iterator operator++(int) {
        iterator temp = *this;
        ++(*this);
        return temp;
    }

    iterator& operator--() {
        if (cur_el == owner->leaf) {
            cur_el = owner->root;
            if (cur_el == owner->leaf) {
                return *this;
            }
            for (; cur_el->r != owner->leaf; cur_el = cur_el->r) {
            }
            return *this;
        }
        if (cur_el->l == owner->leaf) {
            for (; cur_el->parent != owner->leaf && cur_el->parent->l == cur_el;
                 cur_el = cur_el->parent) {
            }
            cur_el = cur_el->parent;
            return *this;
        }
        cur_el = cur_el->l;
        for (; cur_el->r != owner->leaf; cur_el = cur_el->r) {
        }
        return *this;
    }

    const iterator operator--(int) {
        iterator temp = *this;
        --(*this);
        return temp;
    }

    bool operator==(const iterator& other) const {
        return (cur_el == other.cur_el && owner == other.owner);
    }
    bool operator!=(const iterator& other) const { return !(*this == other); }

 private:
    Node* cur_el;
    const Set<ValueType>* owner;
};
