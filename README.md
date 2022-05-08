# Psychic

物理模拟器（完善中）

## 构建

### Windows && Visual Studio (建议 2022 版本)

```powershell
mkdir build
cd build
cmake ..
```

用 ```Visual Studio``` 打开 ```Psychic.sln``` 生成即可。

### Linux (只测试了 Manjaro)

```bash
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc
make
```

（clang 暂不可用）