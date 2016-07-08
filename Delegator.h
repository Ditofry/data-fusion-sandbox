#ifndef DELEGATOR_H
#define DELEGATOR_H


class Delegator
{
public:
    int PORT;
    Delegator();
    void spawn_tcp_listener();
    void stop_listener();
    void tcp_image_poll();
    void connection_check();
    bool is_listening();

private:
    bool listening;
    string thread_safe_file_name(std::string prefix, std::string extension);
    void error(const char *msg);
    static void *connection_handler(void *socket_desc);
    static void *tcp_listener(void *i);
};

#endif // DELEGATOR_H
