#include <iostream>

using std::cout;


class MoveDemo {
    char* data;
    size_t size;

public:
    explicit MoveDemo(char d[], size_t size) {
        this->size = size;
        data = new char[size];
        for (size_t i(0); i< size; ++i)
            data[i] = d[i];

    };

    MoveDemo(const MoveDemo& source) : MoveDemo(source.data, source.size) {
        cout << "CopyCtor\n";
    }

    MoveDemo(MoveDemo&& source) noexcept {
        data = source.data;
        source.data = nullptr;
        size = source.size;
        cout << "MoveCtor\n";
    }

    void setData(char ch) {
        *data = ch;
    }

    int getData(size_t i=0) const {
        return *data;
    }

    ~MoveDemo() {
        delete data;
    }
};

int main() {
    char buf[10]{1};
    MoveDemo  m1 = MoveDemo{ buf, 10 };

    MoveDemo  m0(m1);
    cout << "m0.data = " << m0.getData()  << "\n\n";

    MoveDemo  m2(std::move(m0));

    m2.setData(2);

    cout << "m2.data = " << m2.getData() << "\n";

    cout << "m1.data = " << m1.getData() << "\n\n";

  //cout << "m0.data = " << m0.getData()  << "\n\n";
    return 0;
}
