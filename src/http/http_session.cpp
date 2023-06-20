/*
 * @Author: closing
 * @Date: 2023-05-27 22:03:04
 * @LastEditors: closing
 * @LastEditTime: 2023-06-20 08:28:54
 * @Description: 请填写简介
 */
#include "http_session.h"
#include "http_parser.h"
namespace server_cc{
namespace http{

HttpSession::HttpSession(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner) {
}

HttpRequest::ptr HttpSession::recvRequest() {
    
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    //创建一个缓冲区
    std::shared_ptr<char> buffer(
            new char[buff_size], [](char* ptr){
                delete[] ptr;
            });
    
    char* data = buffer.get();
    int offset = 0;
    do {

        int len = read(data + offset, buff_size - offset);//len代表这次读取的字节数
        if(len <= 0) {
            close();
            return nullptr;
        }
        len += offset;//len代表目前data中还需要解析的字节数（offset为上次解析剩下的）
        //执行解析
        
        size_t nparse = parser->execute(data, len);//执行完execute后，data中的数据前nparse个字节已经被解析了
        
        if(parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;//data的前nparse个字节已经被覆盖掉，还剩(len - nparse)个字节没有解析，这些字节已被移到data的最前面，下次读的时候需要从offset后面开始读
        
        if(offset == (int)buff_size) {//如果offset等于buff_size，说明缓冲区已满，但是还没有解析完，说明请求头过长，关闭连接
            close();
            return nullptr;
        }
        if(parser->isFinished()) {
            break;
        }
    } while(true);
    
    int64_t length = parser->getContentLength();//获取请求体长度
    if(length > 0) {
        std::string body;
        body.resize(length);
        int len = 0;
        if(length >= offset) {//如果请求体长度大于等于offset，说明请求体中有一部分已经被读取到data中了
            memcpy(&body[0], data, offset);
            len = offset;
        } else {
            memcpy(&body[0], data, length);
            len = length;
        }
        
        length -= offset;
        if(length > 0) {//如果上一步读取offset个字节后，请求体还没有读完，继续读取
            if(readFixSize(&body[len], length) <= 0) {
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    //TODO 
    // parser->getData()->init();
    return parser->getData();

}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;// <<运算符已经被重载，可以将HttpResponse类转换为string
    std::string data = ss.str();
    //将data的数据发送出去
    return writeFixSize(data.c_str(), data.size());
}

}// namespace http

}// namespace server_cc