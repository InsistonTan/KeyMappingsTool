# <img src="resources/app.png" align="left" width="128" height="128"/> KeyMappingsTool
![GitHub_Stars](https://img.shields.io/github/stars/InsistonTan/KeyMappingsTool?style=flat&color=ffcf49)
[![GitHub All Releases](https://img.shields.io/github/downloads/InsistonTan/KeyMappingsTool/total)](https://github.com/InsistonTan/KeyMappingsTool/releases)
[![GitHub issues by-label](https://img.shields.io/github/issues/InsistonTan/KeyMappingsTool?color=blue)](https://github.com/InsistonTan/KeyMappingsTool/issues)

方向盘按键映射工具

## 简介
本项目基于QT框架(QT6.x)开发, 实现方向盘或游戏控制器的按键映射功能, 以及方向盘力反馈模拟功能


此外，此项目还能让DIY玩家更方便将真车零件（如转向灯拨杆等）改装为游戏控制器，减少对嵌入式编程的需求，甚至无需接触，降低了DIY门槛

---

## 怎么编译本项目
### 依赖
- QT6.x MSVC2022 64bit [QT官网](https://www.qt.io/)
- MSVC v143 - VS 2022 C++ x64/x86 生成工具(使用Visual Studio Installer进行安装)
- 开发工具 QT Creator 或者 Visual Studio 
### 编译
- 1.clone或下载本项目源码
- 2.QT Creator打开本项目的 CMakeLists.txt 文件
  - 如果 QT Creator 没有自动识别到 MSVC v143(2022)编译器, 需要手动配置)
  - 如果是使用 Visual Studio, 需要手动配置QT6.x MSVC2022的安装位置, 否则无法识别Qt框架的头文件
- 3.执行cmake
- 4.编译运行

---

## 功能
### 主要功能
- 方向盘按键和轴映射键盘鼠标
- 方向盘映射虚拟Xbox手柄
- 方向盘力反馈模拟
  - 根据计算的车速模拟出动态的回正弹簧效果和阻尼效果
  - 支持自定义力反馈动态曲线
    
### 其它功能
- 支持多设备同时映射
  
- 欧卡2自动解除手刹 `踩油门将自动解除手刹, 通过使用欧卡2插件将遥测数据存放在共享内存, 再读取共享内存获得油门状态和手刹状态实现该功能`
  
- 欧卡2原生按键绑定 `例如: 绑定灯光, 转向灯拨杆, 雨刮器各个档位`
  
- 支持设置按键触发模式: (默认)同步模式, 延迟触发模式, 按键松开触发模式, 按下触发且松开再次触发模式, 保持按住(再次按下松开)模式

  `不同的触发模式可以满足各种需求, 例如: [延迟触发模式]可以实现欧卡转向灯自动关闭, [按下触发且松开再次触发模式]可以实现转向灯拨杆回正就关闭转向灯`
  
- 支持组合按键映射 `例如: 设备按键1+设备按键2 映射 键盘按键A, 只有在设备按键1和设备按键2同时按下才会映射键盘按键A`
  
- 支持设置映射的虚拟手柄的死区 `支持设置负死区来抵消游戏默认的死区`
  
- 支持映射多个键盘按键(实现键盘组合键功能)
  
- 支持设置参数保存到映射配置 `实现不同的游戏配置使用不同的参数`
- ......
---

## Q&A
Q: 如何解决方向盘映射虚拟xbox手柄时, 游戏只接收方向盘输入或者同时接收了方向盘和虚拟手柄的输入, 导致操作异常的问题?


A: 使用[HidHide](https://github.com/nefarius/HidHide)工具. 该工具可以实现只让白名单的程序识别到方向盘设备, 不在白名单的程序将无法识别到方向盘设备. 这样就可以让游戏识别不到方向盘, 也就不会有输入冲突

---


## 第三方开源项目声明
- [ViGEmClient](https://github.com/nefarius/ViGEmClient) `用于连接和使用虚拟xbox手柄`
- [ViGEmBus](https://github.com/nefarius/ViGEmBus) `xbox虚拟手柄驱动`
- [scs-sdk-plugin](https://github.com/RenCloud/scs-sdk-plugin) `用于读取欧卡2的游戏数据`
- [ETS2-KeyBinder](https://github.com/Sab1e-GitHub/ETS2-KeyBinder) `用于欧卡2原生按键绑定`

## 项目贡献者
- [m-RNA](https://github.com/m-RNA) `修复多个bug, 以及添加多个功能, 例如: 欧卡2原生按键绑定等等`

感谢所有贡献者

---

## License
This project is licensed under the [MIT License](LICENSE).

