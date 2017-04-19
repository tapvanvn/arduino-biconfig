///SimpleLinker
///author: Duy Nguyen <tapvanvn@gmail.com>

template <typename T>
class SimpleLinker
{
public:
    typedef SimpleLinker<T> Element;
    Element* _next;
    T* _value;
};
