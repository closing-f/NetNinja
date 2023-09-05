#ifndef DOUBLE_D_ARRAY_H_
#define DOUBLE_D_ARRAY_H_

#include <string>  
#include <iostream>  
#include <cassert>  
#include <cstdio>  

//这是一个简单的二维数组的封装类，类似于二维的vector，是用一维数组实现的  
//效率比较高，但不提供改变行数和列数的功能  
template<class Type>  
class DoubleDArray  
{  
private:  
    bool m_HaveInit;        //判断该类是否已经初始化  
    Type *m_Array;          //存放数据的数组  
    int m_Row;              //行数  
    int m_Column;           //列数  

    //将二维数据的行列转成一维数组的索引  
    int GetIndex(int row, int column) const;  

    //复制该数组  
    void CloneArray(const DoubleDArray &array);  

public:  
    DoubleDArray();  
    DoubleDArray(int row, int column);  
    DoubleDArray(const DoubleDArray &array);  
    ~DoubleDArray();  
    DoubleDArray &operator=(const DoubleDArray &array); // 赋值运算符    

    //清空数组（变成末初始化状态）  
    void ClearArray();  

    //初始化该数组, row,column分别是行数和列数  
    void InitArray(int row, int column);  

    //设置和获取数据  
    void SetValue(int row, int column, const Type &value);  
    void SetValue(int index, const Type &value);
    Type GetValue(int row, int column) const;  

    int GetRowCount() { return m_Row; }
    int GetColumnCount() { return m_Column; }
};  

template<class Type>  
DoubleDArray<Type>::DoubleDArray()  
{  
    m_HaveInit = false;  
    m_Array = NULL;  
    m_Column = m_Row = 0;  
}  

template<class Type>  
DoubleDArray<Type>::DoubleDArray(int row, int column)  
{  
    DoubleDArray();  
    InitArray(row, column);  
}  

template<class Type>  
DoubleDArray<Type>::DoubleDArray(const DoubleDArray<Type> &array)  
{  
    m_Array = NULL;  
    CloneArray(array);  
}  

template<class Type>  
DoubleDArray<Type>::~DoubleDArray()  
{  
    ClearArray();  
}  

template<class Type>  
DoubleDArray<Type> &DoubleDArray<Type>::operator=(const DoubleDArray<Type> &array)  
{  
    if( this != &array )  
        CloneArray(array);  
    return *this;  

}  

template<class Type>  
int DoubleDArray<Type>::GetIndex(int row, int column) const  
{  
    assert(m_HaveInit);  
    assert(column < m_Column);  
    assert(row < m_Row);  
    return row * m_Column + column;  
}  

template<class Type>  
void DoubleDArray<Type>::CloneArray(const DoubleDArray &array)  
{  
    ClearArray();  
    if( !array.m_HaveInit )  
        return;  
    InitArray(array.m_Row, array.m_Column);  
    int all = array.m_Row * array.m_Column;  
    for(int i=0; i<all; i++)  
        m_Array[i] = array.m_Array[i];  
}  


template<class Type>  
void DoubleDArray<Type>::ClearArray()  
{  
    if( !m_HaveInit )  
        return;  
    m_HaveInit = false;  
    m_Row = m_Column = 0;  
    if( m_Array != NULL )  
    {  
        delete [] m_Array;  
        m_Array = NULL;  
    }  
}  

template<class Type>  
void DoubleDArray<Type>::InitArray(int row, int column)  
{  
    if( m_Array != NULL )  
        delete [] m_Array;  
    m_Array = new Type[row*column];  
    m_HaveInit = true;  
    m_Row = row;  
    m_Column = column;  
}  

template<class Type>  
void DoubleDArray<Type>::SetValue(int row, int column, const Type &value)  
{  
    int index = GetIndex(row, column);  
    m_Array[index] = value;  
} 

template<class Type>  
void DoubleDArray<Type>::SetValue(int index, const Type &value)  
{  
    m_Array[index] = value;  
}  

template<class Type>  
Type DoubleDArray<Type>::GetValue(int row, int column) const  
{  
    int index = GetIndex(row, column);  
    return m_Array[index];  
}  

#endif  

/*
//测试例子  
int main()  
{  
    DoubleDArray<int> dd;  
    dd.InitArray(10, 10);  
    for(int i=0; i<10; i++)  
        for(int j=0; j<10; j++)  
            dd.SetValue(i, j, i+j);  

    for(int i=0; i<10; i++)  
    {  
        for(int j=0; j<10; j++)  
            printf("%d ", dd.GetValue(i, j));  
        printf("\n");  
    }  

    DoubleDArray<int> copy(dd);  
    copy.SetValue(5, 5, 2345);  
    printf("\n%d  %d\n", copy.GetValue(5, 5), dd.GetValue(5, 5));  
    return 0;  
}  
*/