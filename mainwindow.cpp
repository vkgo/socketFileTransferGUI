#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"

#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>

// winsocket用的头文件和动态库
#include <Ws2tcpip.h>
#include <winsock2.h>

#define BUFFERLIMIT 2048

#pragma comment(lib,"ws2_32.lib")



using namespace std;

QString SendedFileLoc; // 要发送的文件的路径

// 下面是socket用的
SOCKET server_socket;
sockaddr_in server_addr;
sockaddr_in client_addr;
int client_addr_len;

// ui的一个指针，用于自定义函数内调用ui指针
Ui::MainWindow *dis;

bool server_work_signal = false;


DWORD WINAPI Fun(LPVOID lpParamter)
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
    fin.open(SendedFileLoc.toStdString(),ios::binary);
    string filename = SendedFileLoc.toStdString().substr(SendedFileLoc.toStdString().find_last_of("/") + 1);
    send(client_socket, filename.c_str(), filename.size(), 0);
    char buffer[BUFFERLIMIT];
    int length;
    do
    {
        fin.read(buffer, BUFFERLIMIT*sizeof(char));
        length = fin.gcount();
        cout << length << endl;
        // dis->textEdit->append("<font color=\"#00000\">---->111</font> ");
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

    WSACleanup();
    server_work_signal = false;
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
    ui->textEdit->append("<font color=\"#000000\">*****发送文件*****</font> ");
    if (server_work_signal == true)
    {
        ui->textEdit->append("<font color=\"#813732\">---->已经打开文件服务器，请勿再次发送，等待文件传输完毕！</font> ");
        return;
    }
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

    server_work_signal = true; // 禁止线程正在工作时再按按钮打开线程

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

void MainWindow::on_pushButton_2_clicked()
{
    ui->textEdit->append("<font color=\"#000000\">*****接收文件*****</font> ");
    // 初始化WSA
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


    // 创建socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET)
    {
        //如果创建的socket无效，则结束程序
        ui->textEdit->append("<font color=\"#813732\">---->创建socket失败！</font> ");
        return;
    }else
    {
        ui->textEdit->append("<font color=\"#000000\">---->创建socket成功！</font> ");
    }

    // 创建服务器地址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2517);
    server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    // 连接，奖client_socket传给server_addr
    if (WSAAPI::connect(client_socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        ui->textEdit->append("<font color=\"#813732\">---->创建connect失败！</font> ");
        return;
    }else
    {
        ui->textEdit->append("<font color=\"#000000\">---->创建connect成功！</font> ");
    }


    char buffer[BUFFERLIMIT];
    memset(buffer,0 , BUFFERLIMIT);
    // 先接受文件名
    recv(client_socket, buffer, BUFFERLIMIT, 0);
    stringstream ss;
    ss << buffer;
    string filename;
    ss >> filename;

    cout << "filename: " << filename << endl;

    ofstream outfile;
    outfile.open("./" + filename, ios::binary);
    int len;
    do
    {
        len = recv(client_socket, buffer, BUFFERLIMIT, 0);
        // 奖buffer写入outfile
        outfile.write(buffer, len);
    } while (len > 0);

    outfile.close();
    closesocket(client_socket);
    WSACleanup();
}
