# KeyMappingsTool 方向盘按键映射工具
## 简介
本项目基于QT框架(QT6.x)开发, 实现方向盘或游戏控制器的按键映射功能
### 为什么会有这个项目

本项目起始于作者在使用魔爪R3方向盘游玩《尘埃拉力赛2.0》时遇到的问题——方向盘按键无法操作游戏菜单，在VR环境中更为不便。因此，作者开发了此工具以实现自定义按键映射，供自己使用。

此外，为了帮助DIY玩家将真车零件（如转向灯拨杆等）改装为游戏控制器，项目减少了对嵌入式编程的需求，甚至无需接触，大幅降低了DIY门槛。

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
### 其它功能
- 可设置按键触发模式: (默认)同步模式, 延迟触发模式, 按键松开触发模式, 按下触发且松开再次触发模式

  `不同的触发模式可以满足各种需求, 例如: [延迟触发模式]可以实现欧卡转向灯自动关闭, [按下触发且松开再次触发模式]可以实现转向灯拨杆回正就关闭转向灯`
  
- 辅助功能
  - 欧卡2自动解除手刹 `踩油门将自动解除手刹, 通过使用欧卡2插件将遥测数据存放在共享内存, 再读取共享内存获得油门状态和手刹状态实现该功能`
  - 打开软件自动开启全局映射

- 组合按键映射 `例如: 设备按键1+设备按键2 映射 键盘按键A, 只有在设备按键1和设备按键2同时按下才会映射键盘按键A`
- ......
--- 

## 社交媒体账号
哔哩哔哩: [insistonTan](https://space.bilibili.com/254052051)

---

## 感谢
- [ViGEmClient](https://github.com/nefarius/ViGEmClient) , [ViGEmBus](https://github.com/nefarius/ViGEmBus) `感谢项目的开源, 才使得方向盘能映射xbox手柄`
- [m-RNA](https://github.com/m-RNA) `感谢m-RNA修复bug和新增多个功能`

---

## License
This project is licensed under the [MIT License](LICENSE).

