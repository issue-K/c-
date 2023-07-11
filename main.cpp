//
// Created by cl on 2023/7/11.
//
#include <iostream>
#include <thread>

class thread_guard {
    std::thread& t;
public:
    explicit thread_guard(std::thread& t_): t(t_) {}
    ~thread_guard() {
        if(t.joinable()) {
            t.join();
        }
    }
    thread_guard(thread_guard const& ) = delete;
    thread_guard& operator=(thread_guard const& ) = delete;
};

class func {
public:
    int& i;
    func(int& i_) : i(i_) {}
    static void do_something(int i) {
        std::cout << i << "\n";
    }
    void operator()() const {
        for (unsigned j = 0; j < 10; ++j) {
            do_something(i);           // 1. 潜在访问隐患：悬空引用
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

class background_task {
public:
    static void do_something() {
        std::cout << "do something......\n";
    }
    void operator()() const {
        do_something();
    }
};
/*
 * 本章将从基本开始：启动一个线程，等待这个线程结束，或放在后台运行。
 * 再看看怎么给已经启动的线程函数传递参数，以及怎么将一个线程的所有权从当前`std::thread`对象移交给另一个。
 * 最后，再来确定线程数，以及识别特殊线程。
 */

void start(int a, int b) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << a << " " << b << "启动一个线程....\n";
}

// 普通的启动一个线程
void test1() {
    // 普通的启动一个线程
    std::thread thread1(start, 1, 3);
    thread1.join();
}

// 用可调用(callable)类型构造，将带有函数调用符类型的实例传入`std::thread`类中即可
void test2() {
    background_task f;
    std::thread my_thread(f);
    my_thread.join();
}

/*
 * 启动了线程，你需要明确是要等待线程结束，还是让其自主运行。
 * 如果`std::thread`对象销毁之前还没有做出决定，程序就会终止(`std::thread`的析构函数会调用`std::terminate()`)。
 * 如果不等待线程，就必须保证线程结束之前，可访问的数据得有效性
 * 下面的清单(test3)中就展示了这样的一种情况。函数已经结束，线程依旧访问局部变量
 * 运行test3, 你会发现只有最开始的两个输出是99, 因为200ms后该对象会被销毁
 */
void oops() {
    int some_local_state=99;
    func my_func(some_local_state);
    std::thread my_thread(my_func);
    my_thread.detach();          // 2. 不等待线程结束
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void test3() {
    oops();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

/*
 * 如果需要等待线程，相关的`std::thread`实例需要使用join()
 * 在这种情况下，因为原始线程在其生命周期中并没有做什么事，使得用一个独立的线程去执行函数变得收益甚微
 * 但在实际编程中，原始线程要么有自己的工作要做；要么会启动多个子线程来做一些有用的工作，并等待这些线程结束。
 * join()是简单粗暴的等待线程完成或不等待。当你需要对等待中的线程有更灵活的控制时
 * 比如，看一下某个线程是否结束，或者只等待一段时间(超过时间就判定为超时)。想要做到这些，你需要使用其他机制来完成，比如条件变量和期待(futures)，相关的讨论将会在第4章继续。
 * 使用RAII等待线程完成。这样一定会在对象销毁前调用析构函数从而Join
 */
void raii_oops() {
    int some_local_state = 99;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}
void test4() {
    raii_oops();
}

int main() {
    test4();
}