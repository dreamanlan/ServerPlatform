编译环境：https://github.com/nodejs/node-addon-api/blob/main/doc/setup.md

windows下的操作步骤：
0、升级nodejs前，先把旧的nodejs下的node_modules目录删除，npm-cache目录也删除，相关环境变量也清理一下（不删除可能遇到报错Class extends value undefined is not a constructor or null）
1、下载并安装nodejs最新LTS安装版并安装，另外需要python（可以自己装一个），nodejs安装包可选安装chocolatey，里面也会装python（需要自己指明目录执行，似乎不会修改环境变量）
（注意windows下用fnm安装nodejs，安装目录不是当前目录，会安装到fnm的默认目录，还是直接用msi安装比较好，参考https://nodejs.org/zh-cn/download/package-manager，
winget.exe可以直接找到路径带路径运行，装完fnm后切powershell运行）
2、打开命令行窗口，后续在命令行下执行
3、将nodejs安装目录（假设d:\nodejs）添加到path环境变量
    set path=%path%;d:\nodejs
4、安装node-gyp
    npm install -g node-gyp
5、当前目录改为node_center_client所在目录
    cd node_center_client
6、node_center_client的package.json下添加/修改依赖（*后续会替换为版本号）
    "dependencies": {
    "node-addon-api": "*"
  }
7、安装node-addon-api模块
    npm install node-addon-api
    或更新
    npm update node-addon-api
8、执行gyp配置，这里需要指明node-gyp.cmd的目录，一般在C:\Users\%USERNAME%\AppData\Roaming\npm\node-gyp.cmd
    C:\Users\%USERNAME%\AppData\Roaming\npm\node-gyp.cmd configure
9、执行gyp构建
    C:\Users\%USERNAME%\AppData\Roaming\npm\node-gyp.cmd build
10、构建完毕可以拷贝相关文件到运行环境测试了（node_center_client.node与CenterClientLibrary.dll）
