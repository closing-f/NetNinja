/*
 * @Author: closing
 * @Date: 2023-07-10 10:44:40
 * @LastEditors: closing
 * @LastEditTime: 2023-07-10 10:44:48
 * @Description: 请填写简介
 */
#include "src/application.h"

int main(int argc, char** argv) {
    server_cc::Application app;
    if(app.init(argc, argv)) {
        return app.run();
    }
    return 0;
}