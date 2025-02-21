#pragma once

#include <array>
#include <stdexcept>

template <typename ElementType>
class Queue
{
    public:
        virtual void enqueue(const ElementType& element) = 0;

        virtual ElementType dequeue() = 0;

        virtual bool isEmpty() const = 0;

        virtual bool hasData() const = 0;

        virtual bool isFull() const = 0;

        virtual size_t size() const = 0;
};

template <typename ElementType, size_t BufferSize>
class StaticQueue : public Queue<ElementType>
{
    private:
        std::array<ElementType, BufferSize> buffer;
        size_t front = 0;
        size_t rear = 0;
        size_t count = 0;

    public:
        void enqueue(const ElementType& element);

        ElementType dequeue();

        bool isEmpty() const;

        bool hasData() const;

        bool isFull() const;

        size_t size() const;
};
#include "queue.tpp"