# KeyMappingsTool 方向盘按键映射工具
## 简介
本项目基于QT框架(QT6.x)开发, 实现方向盘或游戏控制器的按键映射功能
### 为什么会有这个项目

我自己目前使用的方向盘是魔爪R3, 在我使用方向盘游玩尘埃拉力赛2.0时, 发现方向盘的按键没法操作游戏菜单, 在使用vr游玩的时候每次都要摸鼠标, 很是麻烦, 就想着能不能开发一个映射工具自己用, 然后就有了本项目

此外，此项目还能让DIY玩家更方便将真车零件（如转向灯拨杆等）改装为游戏控制器，减少对嵌入式编程的需求，甚至无需接触，降低了DIY门槛

---

## 怎么编译本项目
### 依赖
- QT6.x [QT官网](https://www.qt.io/)
- 开发工具 QT Creator 或者其它
### 编译
- clone或下载本项目源码
- QT Creator打开本项目的 KeyMapping.pro文件
- 执行qmake
- 编译运行

---

## 已实现的功能
### 主要功能
- 方向盘的按键映射键盘按键, 映射鼠标上下左右移动和鼠标左键右键
- 方向盘的轴映射键盘按键
- 方向盘映射虚拟Xbox手柄, 使用[ViGEmClient](https://github.com/nefarius/ViGEmClient)配合[ViGEmBus驱动](https://github.com/nefarius/ViGEmBus)创建和使用虚拟xbox手柄
- 方向盘力反馈模拟(实验性) `可以让不支持力反馈的游戏也有力反馈的体验`
### 其它功能

- 辅助功能
  - 开机自启动
  - 欧卡2自动解除手刹 `踩油门将自动解除手刹, 通过使用欧卡2插件将遥测数据存放在共享内存, 再读取共享内存获得油门状态和手刹状态实现该功能`
  - 欧卡2的特殊按键绑定 `例如: 拨杆动作同步欧卡2的转向灯的按键绑定`
  - 打开软件自动开启全局映射
  - 方向盘力反馈模拟(实验性)

- 支持设置按键触发模式: (默认)同步模式, 延迟触发模式, 按键松开触发模式, 按下触发且松开再次触发模式

  `不同的触发模式可以满足各种需求, 例如: [延迟触发模式]可以实现欧卡转向灯自动关闭, [按下触发且松开再次触发模式]可以实现转向灯拨杆回正就关闭转向灯`

- 支持组合按键映射 `例如: 设备按键1+设备按键2 映射 键盘按键A, 只有在设备按键1和设备按键2同时按下才会映射键盘按键A`

- 支持设置映射的手柄的死区


- 支持多设备同时映射
- ......
--- 

## 社交媒体账号
哔哩哔哩: [insistonTan](https://space.bilibili.com/254052051)

---

## 感谢
- [ViGEmClient](https://github.com/nefarius/ViGEmClient) , [ViGEmBus](https://github.com/nefarius/ViGEmBus) `感谢项目的开源, 才使得方向盘能映射xbox手柄`
- [scs-sdk-plugin](https://github.com/RenCloud/scs-sdk-plugin/tree/V.1.12.1) `感谢项目的开源, 使得获取欧卡2/美卡的遥测数据更方便`
- [ETS2-KeyBinder](https://github.com/Sab1e-GitHub/ETS2-KeyBinder)`感谢项目的开源，使欧卡2的特殊按键绑定有代码参考移植`
- [m-RNA](https://github.com/m-RNA) `感谢m-RNA修复bug和新增多个功能`

---

## License
This project is licensed under the [MIT License](LICENSE).

