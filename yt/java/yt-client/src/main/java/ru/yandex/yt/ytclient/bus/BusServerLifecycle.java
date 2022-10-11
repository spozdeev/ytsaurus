package ru.yandex.yt.ytclient.bus;

public interface BusServerLifecycle {
    /**
     * Вызывается в случае успешного завершение listen
     */
    void listenSucceeded();

    /**
     * Вызывается в случае ошибки в вызове listen
     */
    void listenFailed(Throwable cause);
}