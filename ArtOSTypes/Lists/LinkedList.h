// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

//
// Created by artypoole on 15/09/24.
//

#ifndef LINKEDLIST_H
#define LINKEDLIST_H


template <typename T>
class LinkedList
{
    struct LinkedListNode
    {
        T data;
        LinkedListNode* next = nullptr;
    };

    LinkedListNode* _head = nullptr;
    LinkedListNode* _tail = nullptr;

public:
    T* head()
    {
        return &_head->data;
    }

    T* tail()
    {
        return &_tail->data;
    }

    bool append(T data)
    {
        auto* newNode = new LinkedListNode{data, nullptr};
        if (newNode == nullptr) { return false; }
        if (_head == nullptr)
        {
            // empty list
            _head = newNode;
            _tail = newNode;
            return true;
        }
        // Not empty, just add a node to the end and update tail
        _tail->next = newNode;
        _tail = newNode;
        return true;
    };

    // TODO: tail not handled properly here.
    bool remove(T data)
    {
        if (_head == nullptr) return false;
        LinkedListNode* curr = _head;
        LinkedListNode* prev = nullptr;
        while (curr)
        {
            if (curr->data == data)
            {
                if (prev) // not found at first node
                {
                    prev->next = curr->next;
                }
                else // currently at the first node.
                {
                    _head = curr->next; // remake the link but the first node is replaced with the second
                }
                if (curr == _tail) // update tail
                {
                    _tail = prev;
                }
                delete curr; // free the memory
                return true; // head success
            }
            // Not yet found so go to the next node
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    // TODO: tail not handled properly here.
    bool remove(T* data)
    {
        if (_head == nullptr) return false;
        LinkedListNode* curr = _head;
        LinkedListNode* prev = nullptr;
        while (curr)
        {
            if (&curr->data == data)
            {
                if (prev) // not found at first node
                {
                    prev->next = curr->next;
                }
                else // currently at the first node.
                {
                    _head = curr->next; // remake the link but the first node is replaced with the second
                }
                if (curr == _tail) // update tail
                {
                    _tail = prev;
                }
                delete curr; // free the memory
                return true; // success
            }
            // Not yet found so go to the next node
            prev = curr;
            curr = curr->next;
        }
        return false;
    }


    template <typename predicate>
    void iterate(predicate pred) const
    {
        if (!_head) { return; }
        LinkedListNode* n = _head;
        while (n != nullptr)
        {
            pred(&n->data);
            n = n->next;
        }
    }

    template <typename result, typename predicate>
    result find_first(predicate pred)
    {
        LinkedListNode* curr = _head;
        while (curr != nullptr)
        {
            if (result res = pred(curr->data); res)
            {
                return res;
            }
            curr = curr->next;
        }
        return 0;
    }

    template <typename predicate>
    T* find_if(predicate pred)
    {
        LinkedListNode* curr = _head;
        while (curr != nullptr)
        {
            if (pred(curr->data))
            {
                return &curr->data;
            }
            curr = curr->next;
        }
        return nullptr;
    }
};

//TODO: doubly linked list
// template<typename T>
// class DoublyLinkedList
// {
//     struct DoublyLinkedListNode{
//         DoublyLinkedListNode *prev;
//         T data;
//         DoublyLinkedListNode *next;
//     }
// };


#endif //LINKEDLIST_H
