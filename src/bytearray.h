/*
 * @Author: closing
 * @Date: 2023-05-22 15:55:45
 * @LastEditors: closing
 * @LastEditTime: 2023-05-23 10:02:30
 * @Description: byte数组类
 */
#ifndef _BYTEARRAY_H
#define _BYTEARRAY_H
#include<memory>
#include<string>
#include<vector>
#include<stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
namespace server_cc{

class ByteArray{
public:
    typedef std::shared_ptr<ByteArray> ptr;

    /**
     * @description: 节点类，用于存储数据
     */    
    struct Node{
        Node(size_t s);
        Node();
        ~Node();

        char* ptr;//指向数据的指针
        Node* next;//指向下一个节点的指针
        size_t size;//当前节点的大小
    };

    ByteArray(size_t base_size = 4096);
    ~ByteArray();
    //write
    void writeFint8(int8_t value);
    void writeFuint8(uint8_t value);
    void writeFint16(int16_t value);
    void writeFuint16(uint16_t value);
    void writeFint32(int32_t value);
    void writeFuint32(uint32_t value);
    void writeFint64(int64_t value);
    void writeFuint64(uint64_t value);


    void writeInt32(int32_t value);
    void writeUint32(uint32_t value);
    void writeInt64(int64_t value);
    void writeUint64(uint64_t value);

    void writeFloat(float value);
    void writeDouble(double value);

    void writeStringF16(const std::string& value);
    void writeStringF32(const std::string& value);
    void writeStringF64(const std::string& value);//固定长度的字符串
    void writeStringVint(const std::string& value);//可变长度的字符串

    void writeStringWithoutLength(const std::string& value);//不带长度的字符串

    //read
    int8_t readFint8();
    uint8_t readFuint8();
    int16_t readFint16();
    uint16_t readFuint16();
    int32_t readFint32();
    uint32_t readFuint32();
    int64_t readFint64();
    uint64_t readFuint64();
    
    int32_t readInt32();
    uint32_t readUint32();
    int64_t readInt64();
    uint64_t readUint64();

    float readFloat();
    double readDouble();

    //? string16 32 64 有什么区别
    std::string readStringF16();//length: int16_t,data
    std::string readStringF32();//length: int32_t,data
    std::string readStringF64();//length: int64_t,data
    std::string readStringVint();

    //内部操作
    void clear();//清空

    /**
     * @description: 
     * @param {void*} buf 用于存储数据的指针
     * @param {size_t} size 写入的长度
     * @return {*}
     */    
    void write(const void* buf,size_t size);

    /**
     * @description: 
     * @param {void*} buf 用于存储数据的指针
     * @param {size_t} size 读取的长度
     * @return {*}
     */    
    void read(void* buf,size_t size);
    
    /**
     * @description: 
     * @param {void*} buf 用于存储数据的指针
     * @param {size_t} size 读取的长度
     * @param {size_t} position 读取的位置
     * @return {*}
     */    
    void read(void* buf, size_t size, size_t position) const;
    
    //? 函数定义在头文件和cpp的区别
    size_t getPosition() const {return m_position;}
    void setPosition(size_t v);
    
    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);

    size_t getBaseSize() const {return m_baseSize;}
    size_t getReadSize() const {return m_size - m_position;}
    size_t getSize() const {return m_size;}
    


    /**
     * @description: 是否是小端
     */    
    bool isLittleEndian() ;
    void setIsLittleEndian(bool val) ;
    
    /**
     * @description: 返回可读的数据
     */
    std::string toString() const;

    std::string toHexString() const;
    
    /**
     * @description: 
     * @param {std::vector<iovec>&} buffers 用于存储数据的数组
     * @param {uint64_t} len 读取的长度
     * @return {*}
     */    
    uint64_t getReadBuffers(std::vector<iovec>& buffers,uint64_t len = ~0ull) const;

    /**
     * @description: 
     * @param {std::vector<iovec>&} buffers 用于存储数据的数组
     * @param {uint64_t} len 读取的长度
     * @param {uint64_t} position 读取的位置
     * @return {*} 
     */    
    uint64_t getReadBuffers(std::vector<iovec>& buffers,uint64_t len,uint64_t position) const;


    /**
     * @brief 获取可写入的缓存,保存成iovec数组
     * @param[out] buffers 保存可写入的内存的iovec数组
     * @param[in] len 写入的长度
     * @return 返回实际的长度
     * @post 如果(m_position + len) > m_capacity 则 m_capacity扩容N个节点以容纳len长度
     */
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);


private:
    void addCapacity(size_t size);
    size_t getCapacity() const {return m_capacity - m_position;}
private:
    size_t m_position;//当前位置
    size_t m_capacity;//容量
    size_t m_baseSize;//Node基础大小
    size_t m_size;//当前数据大小
    int8_t m_endian;//是否是小端


    Node* m_root;//根节点
    Node* m_cur;//当前节点
};
    






} // namespace server_cc




#endif