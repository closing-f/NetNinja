#ifndef PUBLIC_MYSQL_CONTROL_H_
#define PUBLIC_MYSQL_CONTROL_H_  

#include <mysql.h>  

#include <iostream>  
#include <string>  
#include <vector>  
#include <cassert>  
#include <cstdio>  
#include <cstring>
#include <cstdarg>
#include "double_d_array.h"
#include "field.h"

enum CON_LOG_KIND
{
    CONTROL_INFO,
    CONTROL_ERROR,
};

class MysqlControl  
{  
    typedef void(*CONTROL_LOG_FUN)(CON_LOG_KIND kind, const char *msg, const char *sql);
    typedef void(*MYSQL_LOG_FUN)(int error, const char *msg);

private:  
    MYSQL *m_Connection;                //与MYSQL的连接句柄  
    MYSQL_RES *m_Result;                //上一次查询的结果集  
    MYSQL_FIELD *m_Field;
    bool m_Connected;                    //程序是否已经和MYSQL连接了  
    int m_RowCount;                        //上一次查询的结果集的行数  
    int m_ColumnCount;                    //上一次查询的结果集的列数  
    std::string m_sql;                       //sql语句的缓冲区
    CONTROL_LOG_FUN m_ControlLogFun;
    MYSQL_LOG_FUN m_MysqlLogFun;

    //发生错误时输出错误信息  
    void OutputLog(CON_LOG_KIND kind, const char *msg)
	{  
		if (CONTROL_ERROR == kind &&
			mysql_errno(m_Connection) != 0)
		{
			int err = mysql_errno(m_Connection);
			const char *strerr = mysql_error(m_Connection);
			if (NULL == m_MysqlLogFun)
				fprintf(stderr, "MYSQL ERROR : %d: %s\n", err, strerr);
			else
				m_MysqlLogFun(err, strerr);
		}

		if( NULL == m_ControlLogFun )
		{
			if( CONTROL_INFO == kind )
				printf("INFO--%s : %s\n", msg, m_sql.c_str());
			else if( CONTROL_ERROR == kind )
				printf("ERROR--%s : %s\n", msg, m_sql.c_str());
		}
		else
		{
			m_ControlLogFun(kind, msg, m_sql.c_str());
		}
	} 

    //将格式化字符串赋给string，参数和printf的参数一样，返回格式化后的string类  
	std::string StringFormat(const char *format, ...)
	{
		std::string result;
		//将字符串的长度初始化为1024  
		int tlen = 1024;
		int len = 0;
		result.resize(tlen);

		//为string赋值  
		va_list list;
		va_start(list, format);
		len = vsnprintf((char*)result.c_str(), tlen, format, list);

		//如果结果字符串长度比初始长度长，就重新设置字符串长度，然后再赋值  
		if (len >= tlen)
		{
			tlen = len + 1;
			result.resize(tlen);
			len = vsnprintf((char*)result.c_str(), tlen, format, list);
		}
		va_end(list);

		//删除字符串尾部的0字符  
		result.erase(len);
		return result;
	}

private:
	//禁止复制该类
    MysqlControl(const MysqlControl &other) { }
    MysqlControl & operator = (const MysqlControl &) { }

public:  

    MysqlControl()
	{  
		//为MYSQL连接分配空间，并对其初始化  
		m_Connection = new MYSQL;  
		mysql_init(m_Connection);  

		m_Result = NULL;  
		m_Field = NULL;
		m_Connected = false;  
		m_RowCount = m_ColumnCount = 0;  
		m_ControlLogFun = NULL;
		m_MysqlLogFun = NULL;
	} 
    ~MysqlControl()
	{  
		//清空结果集，关闭连接，释放空间  
		ClearResult();  
		mysql_close(m_Connection);  
		delete m_Connection;  
	} 

    //获取连接和结果集的指针，尽量不要使用该接口
    MYSQL *GetMysql() const { return m_Connection; }  
    MYSQL_RES *GetMysqlRes() const { return m_Result; }  

    //设置日志的回调函数
    void SetMysqlLogCallback(MYSQL_LOG_FUN fun)
    { m_MysqlLogFun = fun; }
    void SetControlLogCallback(CONTROL_LOG_FUN fun)
    { m_ControlLogFun = fun; }

    //与MYSQL建立连接  
    bool RealConnect(const char *host, const char *user,  
        const char *password, const char *db = NULL,  
        unsigned int port = 0, const char *unix_socket = NULL,  
        unsigned long client_flag = 0, bool reconnect = true)
	{  
		if( m_Connected )
			return true;

		//连接数据库，设置标志(connected)  
		m_Connection = mysql_real_connect(m_Connection, host,   
			user, password, db, port, unix_socket, client_flag);  

		if( m_Connection == NULL )  
		{  
			OutputLog(CONTROL_ERROR, "real connect fail");  
			return false;  
		}  
		OutputLog(CONTROL_INFO, "real connect success");

		//设置自动重连
		if( reconnect )
		{
			char value = 1;
			mysql_options(m_Connection, MYSQL_OPT_RECONNECT, (char *)&value);
		}
		m_Connected = true;  
		return true;  
	} 

    //改变当前的用户   
    bool ChangeUser(const char *host, const char *user,   
        const char *password, const char *db = NULL)
	{  
		assert(m_Connected);  
		int res = mysql_change_user(m_Connection, user,   
			password, db);  
		if( res != 0 )  
		{  
			OutputLog(CONTROL_ERROR, "change user fail");  
			return false;  
		}  
		OutputLog(CONTROL_INFO, "change user success");
		return true;  
	}  

    //清空结果集，并将结果集的指针置为空  
    void ClearResult()
	{  
		if( m_Result == NULL )  
			return;  

		mysql_free_result(m_Result);  
		m_Result = NULL;  
		m_RowCount = 0;  
		m_ColumnCount = 0;  
	} 

    //执行一条SQL语句  
    //返回值是该表受到影响的行数  
    int Query(const char *order, ...)
	{  
		assert(m_Connected);

		va_list list;    
		va_start(list, order); 
		m_sql = StringFormat(order, list);
		va_end(list);

		//执行SQL语句  
		int res = mysql_query(m_Connection, m_sql.c_str());  
		if( res != 0 )  
		{  
			OutputLog(CONTROL_ERROR, "sql error");  
			return -1;  
		}  
		else
		{
			OutputLog(CONTROL_INFO, "query sql");
		}

		//将影响的行数作为返回值  
		res = mysql_affected_rows(m_Connection);  
		return res;  
	} 

    int Query(const std::string &order)
    { return Query(order.c_str()); }

    //执行一条SQL语句，并把返回的结果放置到结果集中  
    //返回值是结果集的行数  
    int QueryAndStore(const char *order, ...)
	{  
		assert(m_Connected);  
		//先将结果集清空  
		ClearResult();  

		va_list list;    
		va_start(list, order); 
		m_sql = StringFormat(order, list);
		va_end(list);

		//执行SQL语句  
		int res = mysql_query(m_Connection, m_sql.c_str()); 
		if( res != 0 )  
		{  
			OutputLog(CONTROL_ERROR, "sql error");  
			return -1;  
		}  
		else
		{
			OutputLog(CONTROL_INFO, "query sql");
		}

		//将my_res设为当前的结果集和行列数  
		m_Result = mysql_store_result(m_Connection);
		m_Field = mysql_fetch_field(m_Result);
		if( NULL == m_Result )  
			return -1;  
		m_RowCount = mysql_num_rows(m_Result);  
		m_ColumnCount = mysql_num_fields(m_Result);  

		return m_RowCount;  
	}  

    int QueryAndStore(const std::string &order)
    { return QueryAndStore(order.c_str()); }

    //返回结果集的行数或列数  
    int GetRowCount() { return m_RowCount; }  
    int GetColumnCount() { return m_ColumnCount; }  

    //返回由以前的INSERT或UPDATE语句为AUTO_INCREMENT列生成的值
    int GetInsertID()
    { return mysql_insert_id(m_Connection); }

    //获取某个单元格的结果，将其作为返回值  
    Field GetOneNode(int row, int column)
	{  
		Field result;
		if (NULL == m_Result)
			return result;

		if( row >= m_RowCount ||  
			column >= m_ColumnCount )  
			return result;  

		mysql_data_seek(m_Result, row);  
		MYSQL_ROW my_row = mysql_fetch_row(m_Result);  
		result.SetStructuredValue(my_row[column], m_Field[column].type);
		return result;  
	} 

    //将上一次查询的结果集存入指定的容器中  
    //获取全部结果，将其存入DoubleDArray 
    void GetAllResult(DoubleDArray<Field> &data)
	{  
		data.ClearArray();

		if( NULL == m_Result )
			return;

		MYSQL_ROW my_row;
		data.InitArray(m_RowCount, m_ColumnCount);

		//将光标设为第一行  
		mysql_data_seek(m_Result, 0);  

		for(int i=0; i<m_RowCount; i++)  
		{  
			//获取光标所在行的那一行数据（的指针）  
			my_row = mysql_fetch_row(m_Result);  
			for(int j=0; j<m_ColumnCount; j++)  
			{  
				Field field;
				field.SetStructuredValue(my_row[j], m_Field[j].type);
				data.SetValue(i, j, field);
			}  
		}  
	}  

    //获取一行结果，将其存入DoubleDArray  
    void GetOneResult(DoubleDArray<Field> &data, int row)
	{  
		data.ClearArray();

		if( NULL == m_Result )
			return;

		if( row >= m_RowCount )  
			return;  

		data.InitArray(1, m_ColumnCount); 

		//将光标放置到目标行  
		mysql_data_seek(m_Result, row);  
		MYSQL_ROW my_row = mysql_fetch_row(m_Result);  

		//将结果存入vector容器  
		for(int i=0; i<m_ColumnCount; i++)  
		{  
			Field field;
			field.SetStructuredValue(my_row[i], m_Field[i].type);
			data.SetValue(0, i, field);
		}  
	}  

    //获取结果集中的表头的名字  
    void GetHeaderName(std::vector<std::string> &vec)
	{  
		if( NULL == m_Result )
			return;

		vec.resize(m_ColumnCount);  
		MYSQL_FIELD *field;  

		//获取表头名字，存入vector容器  
		field = mysql_fetch_fields(m_Result);  
		for(int i=0; i<m_ColumnCount; i++)  
			vec[i] = field[i].name;  
	}  

    //返回SQL缓冲区的指针
    std::string GetSQLString() { return m_sql; }
};  


//提供了一个MysqlControl的单例类
class SingleMysqlControl
{
private:
    MysqlControl m_control;
    SingleMysqlControl() { }
    ~SingleMysqlControl() { }

public:
    MysqlControl *GetDate() { return &m_control; }
    static SingleMysqlControl *GetInstance()
    {
        static SingleMysqlControl instance;
        return &instance;
    }
};

#define GET_MYSQL_CONTROL SingleMysqlControl::GetInstance()->GetDate

#endif  