/*
 * @Author: closing
 * @Date: 2023-09-04 21:25:53
 * @LastEditors: closing
 * @LastEditTime: 2023-09-04 21:41:22
 * @Description: 请填写简介
 */
//��������

#include "database/mysql_control.h"  
#include <string>
#include <iostream>

#define TEST_HOST "localhost"
#define TEST_USER "root"
#define TEST_PASSWORD "Suibian090925."
#define TEST_DB "uav_data"

void MysqlLogCallBack(int errid, const char *msg)
{
    std::cout << "Mysql error: " << errid << "--" << msg << std::endl;
}

void ControlLogCallback(CON_LOG_KIND kind, const char *msg, const char *sql)
{

}

int main(void)
{
    MysqlControl control;
    int row, column;

    control.SetControlLogCallback(ControlLogCallback);
    control.SetMysqlLogCallback(MysqlLogCallBack);
    bool ok = control.RealConnect(TEST_HOST, TEST_USER, TEST_PASSWORD, TEST_DB);
    if (!ok)
    {
        std::cout << "connect error" << std::endl;
        return 1;
    }

    control.QueryAndStore("show tables;");
    DoubleDArray<Field> arr;
    control.GetAllResult(arr);
    for (size_t i = 0; i < arr.GetRowCount(); i++)
        std::cout << arr.GetValue(i, 0).GetString() << std::endl;
    
    return 0;
}