#include "queue.hpp"

template <typename ElementType, size_t BufferSize>
void StaticQueue<ElementType, BufferSize>::enqueue(const ElementType& element)
{
    if (isFull())
    {
        throw std::overflow_error("Queue is full");
    }
    buffer[rear] = element;
    rear = (rear + 1) % BufferSize;
    ++count;
}

template <typename ElementType, size_t BufferSize>
ElementType StaticQueue<ElementType, BufferSize>::dequeue()
{
    if (isEmpty())
    {
        throw std::underflow_error("Queue is empty");
    }
    ElementType element = buffer[front];
    front = (front + 1) % BufferSize;
    --count;
    return element;
}

template <typename ElementType, size_t BufferSize>
bool StaticQueue<ElementType, BufferSize>::isEmpty() const
{
    return count == 0;
}

template <typename ElementType, size_t BufferSize>
bool StaticQueue<ElementType, BufferSize>::hasData() const
{
    return count > 0;
}

template <typename ElementType, size_t BufferSize>
bool StaticQueue<ElementType, BufferSize>::isFull() const
{
    return count == BufferSize;
}

template <typename ElementType, size_t BufferSize>
size_t StaticQueue<ElementType, BufferSize>::size() const
{
    return count;
}