#ifndef Registrar_h
#define Registrar_h

#include <iterator>

/**
 * This provides to any derived class a static array of pointers to all instantiated
 * objects of the class.
 *
 * This follows the CRTP pattern.
 *
 * @see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
 */
template <typename T>
class Registrar
{
public:
    static void registerEntity(T *entity)
    {
        T **newArr = new T *[count + 1];
        std::copy(entities, entities + std::min(count, count + 1), newArr);
        delete[] entities;
        entities = newArr;

        entities[count] = entity;

        count++;
    }

    Registrar()
    {
        registerEntity((T *)this);
    }

    static T **getAll()
    {
        return entities;
    }

    static T *get(unsigned int index)
    {
        return entities[index];
    }

    static unsigned int getCount()
    {
        return count;
    }

    //  private: // TODO: HOW TO MAKE IT PRIVATE
    static unsigned int count;
    static T **entities;
};

template <typename T>
unsigned int Registrar<T>::count = 0;

template <typename T>
T **Registrar<T>::entities = new T *[0];

// TODO: SEE HOW TO LIMIT THESE TEMPLATES TO REGISTRAR TYPE
template <typename _Tp>
_Tp *begin(_Tp *&t)
{
    return (&(*t)->entities[0]);
}

template <typename _Tp>
_Tp *end(_Tp *&t)
{
    return (&(*t)->entities[(*t)->count]);
}
#endif
