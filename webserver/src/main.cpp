/*
 * @Author       : GameServer
 * @Date         : 2024-01-01
 * @Description  : WebServer主入口
 */ 

#include "../include/webserver.h"

int main() {
    /* 守护进程后台运行 */
    //daemon(1, 0); 

    WebServer server(
        1316, 3, 60000, false,             /* 端口 ET模式 timeoutMs 优雅退出  */
        6, true, 1, 1024);                 /* 线程数量 日志开关 日志等级 日志异步队列容量 */
    server.Start();
} 