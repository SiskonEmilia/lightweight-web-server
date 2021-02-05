#ifndef UNCOPYABLE_H
#define UNCOPYABLE_H

/**
 * @brief 避免派生类被拷贝或拷贝构造
*/
class Uncopyable
{
public:
    Uncopyable() {}
    ~Uncopyable() {}

private:
    Uncopyable(const Uncopyable &);
    Uncopyable &operator=(const Uncopyable &);
};

#endif