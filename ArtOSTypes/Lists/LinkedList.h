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

    LinkedListNode* head = nullptr;
    LinkedListNode* tail = nullptr;

public:
    bool append(T data)
    {
        auto* newNode = new LinkedListNode{data, nullptr};
        if (newNode == nullptr) { return false; }
        if (head == nullptr)
        {
            // empty list
            head = newNode;
            tail = newNode;
            return true;
        }
        // Not empty, just add a node to the end and update tail
        tail->next = newNode;
        tail = newNode;
        return true;
    };


    bool remove(T data)
    {
        if (head == nullptr) return false;
        LinkedListNode* curr = head;
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
                    head = curr->next; // remake the link but the first node is replaced with the second
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

    bool remove(T* data)
    {
        if (head == nullptr) return false;
        LinkedListNode* curr = head;
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
                    head = curr->next; // remake the link but the first node is replaced with the second
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
        if (!head) { return; }
        LinkedListNode* n = head;
        while (n != nullptr)
        {
            pred(&n->data);
            n = n->next;
        }
    }

    template <typename result, typename predicate>
    result find_first(predicate pred)
    {
        LinkedListNode* curr = head;
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
        LinkedListNode* curr = head;
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
