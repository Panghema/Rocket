#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/abstract_coder.h"
#include "rocket/net/abstract_protocol.h"

void test_connect() {
    // 调用connect连接server
    // write一个字符串
    // 等read返回结果

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        ERRORLOG("invalid listenfd %d", fd);
        exit(0);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    inet_aton("127.0.0.1", &server.sin_addr);
    server.sin_port = htons(12346);

    int rt = connect(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server));

    std::string msg = "hello rocket!";

    rt = write(fd, msg.c_str(), msg.size());

    DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

    char buf[128];
    rt = read(fd, buf, 128);

    DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());

}

void test_tcp_client() {
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12346);
    rocket::TcpClient client(addr);
    client.connect([addr, &client]() {
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        std::shared_ptr<rocket::StringProtocol> message = std::make_shared<rocket::StringProtocol>();
        message->info = "hello rocket\t";
        message->setReqId("123456");
        client.writeMessage(message, [](rocket::AbstractProtocol::s_ptr msg_ptr) {
            DEBUGLOG("send message success");
        });

        client.readMessage("123456", [](rocket::AbstractProtocol::s_ptr msg_ptr) {
            std::shared_ptr<rocket::StringProtocol> message = std::dynamic_pointer_cast<rocket::StringProtocol>(msg_ptr);
            DEBUGLOG("req_id[%s], get response %s", message->getReqId().c_str(), message->info.c_str());
        });

        client.writeMessage(message, [](rocket::AbstractProtocol::s_ptr msg_ptr) {
            DEBUGLOG("send message2222 success");
        });
    });
     
}

int main() {
    rocket::Config::SetGlobalConfig("./conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    // test_connect();
    test_tcp_client();

    return 0;
}