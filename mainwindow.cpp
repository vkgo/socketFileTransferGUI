#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"

#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>

// winsocket用的头文件和动态库
#include <Ws2tcpip.h>
#include <winsock2.h>

#define BUFFERLIMIT 2048

#pragma comment(lib,"ws2_32.lib")



using namespace std;

QString SendedFileLoc;
SOCKET server_socket;
sockaddr_in server_addr;
sockaddr_in client_addr;
int client_addr_len;
Ui::MainWindow *dis;


DWORD WINAPI Fun(LPVOID lpParamter)
{
    while (1)
    {
        //接收客户端的连接
        SOCKET client_socket = accept(server_socket, (LPSOCKADDR)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET)
        {
            dis->textEdit->append("<font color=\"#813732\">---->accept失败！</font> ");
        }else
        {
            dis->textEdit->append("<font color=\"#000000\">---->客户端连接！</font> ");
        }



        // 打开文件并读取文件数据
        ifstream fin;
        fin.open("data_batch_1.bin",ios::binary);
        char buffer[BUFFERLIMIT];
        int length;
        do
        {
            fin.read(buffer, BUFFERLIMIT*sizeof(char));
            length = fin.gcount();
            dis->textEdit->append("<font color=\"#00000\">---->111</font> ");
            if (send(client_socket, buffer, length, 0) < 0)
            {
                dis->textEdit->append("<font color=\"#813732\">---->文件发送失败！</font> ");
                break;
            }
            memset(buffer,0, BUFFERLIMIT);
        }
        while(fin);

        // 关闭与客户端的连接
        closesocket(client_socket);
        break;
    }
    WSACleanup();
    return 0L;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 这里是窗口初始化
    ui->setupUi(this);
    setWindowTitle("SocketFileTransfer by VK");
    this->setFixedSize(this->width(),this->height());
    dis = ui;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    SendedFileLoc = QFileDialog::getOpenFileName(
                this,
                "选择要发送的文件",
                "../"
                ,
                "*(*.*)");
    if(SendedFileLoc.isEmpty())
    {
        ui->textEdit->append("<font color=\"#813732\">---->打开失败：文件未选择！</font> ");
        return;
    }
    ui->textEdit->append("<font color=\"#000000\">---->导入图片:</font> <font color=\"#FF0000\">"+SendedFileLoc+"</font> ");

    /*
    初始化WSA，使得程序可以调用windows socket
    */
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        ui->textEdit->append("<font color=\"#813732\">---->初始化WSA失败！</font> ");
        return;
    }else
    {
        ui->textEdit->append("<font color=\"#000000\">---->创建WSA成功！</font> ");
    }

    /*
    创建监听用套接字，server_socket
    类型是TCP
    并检测是否创建成功
    */
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET)
    {
        //如果创建的socket无效，则结束程序
        ui->textEdit->append("<font color=\"#813732\">---->创建socket失败！</font> ");
        return;
    }else
    {
        ui->textEdit->append("<font color=\"#000000\">---->创建socket成功！</font> ");
    }


    /*
       创建地址，server_addr，并设置端口和IP
    */

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2517);
    //INADDR_ANY表示本机任意IP地址
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

    //将socket与地址server_addr绑定
    if (bind(server_socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        ui->textEdit->append("<font color=\"#813732\">---->socket与server_addr绑定失败！</font> ");
        return;
    }else
    {
        ui->textEdit->append("<font color=\"#000000\">---->socket与server_addr绑定成功！</font> ");
    }

    //监听socket
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        ui->textEdit->append("<font color=\"#813732\">---->监听失败！</font> ");
        return;
    }else
    {
        ui->textEdit->append("<font color=\"#000000\">---->监听成功！</font> ");
    }

    //接收客户端的连接
    client_addr_len = sizeof(client_addr);



    // 启动子线程
//    LPSECURITY_ATTRIBUTES   lpThreadAttributes, //线程安全相关的属性，常置为NULL
//    SIZE_T                  dwStackSize,        //新线程的初始化栈在大小，可设置为0
//    LPTHREAD_START_ROUTINE  lpStartAddress,     //被线程执行的回调函数，也称为线程函数
//    LPVOID                  lpParameter,        //传入线程函数的参数，不需传递参数时为NULL
//    DWORD                   dwCreationFlags,    //控制线程创建的标志
//    LPDWORD                 lpThreadId          //传出参数，用于获得线程ID，如果为NULL则不返回线程ID
    HANDLE hThread = CreateThread(NULL, 0, Fun, NULL, 0, NULL);
    CloseHandle(hThread);





}
