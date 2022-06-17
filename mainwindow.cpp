#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 这里是窗口初始化
    ui->setupUi(this);
    setWindowTitle("SocketFileTransfer by VK");
    this->setFixedSize(this->width(),this->height());
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString FileLoc=QFileDialog::getOpenFileName(
                this,
                "选择要发送的文件",
                "../"
                ,
                "*(*.*)");

}
