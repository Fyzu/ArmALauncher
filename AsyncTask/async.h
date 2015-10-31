#ifndef ASYNC_H
#define ASYNC_H

#include <QVector>
#include <QThread>
#include <QApplication>
#include <functional>

template<typename T> struct AsyncInfo {
    std::function<T()> worker;
    std::function<void(T, QThread* thread)> callback;
};

template<typename T> class AsyncThread : public QThread {
    public:
        AsyncThread(AsyncInfo<T> info);
        void run();
        void work();
    public slots:
        void onFinished();
    private:
        AsyncInfo<T> thread_info;
        T result;
};

template<typename T> AsyncThread<T>::AsyncThread(AsyncInfo<T> info) : QThread()
{
    thread_info = info;
}

template<typename T> void AsyncThread<T>::work()
{
    start();
    while(isRunning())
    {
        QApplication::processEvents();
    }
    onFinished();
}

template<typename T> void AsyncThread<T>::run()
{
    result = thread_info.worker();
}

template<typename T> void AsyncThread<T>::onFinished()
{
    thread_info.callback(result,this);
}

template<typename T> class Async
{
public:
    Async(std::function<T()> worker, std::function<void(T)> callback);
    static void remove(AsyncThread<T>* thread);
private:
    static QVector<AsyncThread<T>*> threads;
};

template<typename T> QVector<AsyncThread<T>*> Async<T>::threads;

template<typename T> Async<T>::Async(std::function<T()> worker, std::function<void(T)> callback)
{
    AsyncInfo<T> info;
    info.worker = worker;
    info.callback = [callback](T info, QThread* thread) {
        callback(info);
        remove( (AsyncThread<T>*)thread );
    };
    AsyncThread<T>* thread = new AsyncThread<T>(info);
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    threads.append(thread);
    thread->work();
}

template<typename T> void Async<T>::remove(AsyncThread<T>* thread)
{
    int index = threads.indexOf(thread);
    threads.removeAt(index);
}

#endif // ASYNC_H
