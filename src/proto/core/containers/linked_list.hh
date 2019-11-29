#pragma once

namespace proto {

    template<typename T>
    struct linked_list {
        struct node {
            node * prev;
            node * next;
            T value;
        };

        size_t size = 0;
        node * first = nullptr;
        node * last =  nullptr;

        ~linked_list() {
            clear();
        }
        void clear() {
            node * next, * it = first;
            while(it != nullptr) {
                next = it->next;
                delete it;
                it = next;
            }
            first = last = nullptr;
            size = 0;
        }

        node * append(T val) {
            node * new_node = new node{.prev = last,
                                       .next = nullptr,
                                       .value = val};

            if(size == 0) {
                first = new_node;
                last = new_node;
            } else {
                last->next = new_node;
                last = new_node;
            }
            size++;
            return new_node;
        }

        node * prepend(T val) {
            node * new_node = new node{.prev = nullptr,
                                       .next = first,
                                       .value = val};
            if(size == 0) {
                first = new_node;
                last = new_node;
            } else {
                first->prev = new_node;
                first = new_node;
            }
            size++;
            return new_node;
        }

        node * insert(T val, node * at) {
            node * it = first;
            while(it != nullptr) {
                if(it == at) {
                    node * new_node = new node{.prev = it->prev,
                                               .next = it,
                                               .value = val};
                    if(it->prev == nullptr) {
                        assert(first == it);
                        first = new_node;
                    } else
                        it->prev->next = new_node;

                    it->prev = new_node;

                    size++;
                    return new_node;
                }
                it = it->next;
            }
            return nullptr;
        }
        void erase(node * at) {
            node * it = first;
            while(it != nullptr) {
                if(it == at) {
                    if(it->prev == nullptr) {
                        assert(first == it);
                        first = it->next;
                    } else
                        it->prev->next = it->next;

                    if(it->next == nullptr) {
                        assert(last == it);
                        last = it->prev;
                    } else
                        it->next->prev = it->prev;

                    delete it;
                    size--;
                    return;
                }
                it = it->next;
            }
        }

    };

}
