///SimpleLinker
///author: Duy Nguyen <tapvanvn@gmail.com>

template <typename T>
class SimpleLinker
{
public:
    SimpleLinker():_next(0), _value(0)
    {

    }
    typedef SimpleLinker<T> Element;
    Element* _next;
    T* _value;
};
