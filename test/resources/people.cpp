#include <iostream>
#include <string>
#include <string_view>

static constexpr auto HELLO = "Hello";

namespace klang {

template <class T1, class T2, int I>
class partial_human {
};

template <class T, int I>
class partial_human<T, T*, I> {
};

template <class T, class T2, int I>
class partial_human<T*, T2, I> {
};

template <class T>
class partial_human<int, T*, 5> {
};

template <class X, class T, int I>
class partial_human<X, T*, I> {
};

struct person_info {
    std::string name;
    unsigned age;
};

class person {
public:
    explicit person() = default;

    explicit person(std::string_view name, const unsigned age)
        : _info { std::string(name), age }
    {
    }

    auto get_info() const noexcept -> const auto&
    {
        return _info;
    }

    void talk() const
    {
        constexpr std::string_view str = "my name is";
        std::cout << HELLO << ", " << str << " " << _info.name << " and I'm " << _info.age << " old." << std::endl;
    }

    ~person() = default;

private:
    person_info _info;
};

template <typename T>
struct collection {
};

class people : public collection<person> {
};

constexpr auto add(const int a, const int b)
{
    return a + b;
}

template <typename T>
auto multiply(T a, T b) noexcept
{
    return a * b;
}

void print(std::string_view message)
{
    std::cout << message << std::endl;
}

}

int main()
{
    using namespace klang;

    print("Program has started.");
    person john("John Doe", 32u);
    person ray("Ray Sin", 56u);

    std::cout << "John plus Ray equals " << add(john.get_info().age, ray.get_info().age) << std::endl;
    std::cout << "John times Ray equals " << multiply(john.get_info().age, ray.get_info().age) << std::endl;
    john.talk();

    return 0;
}
