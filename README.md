# CHIP-8 Emulator
<img src="https://github.com/JNoxik/mychip8/raw/main/images/tictactoe.png" width="600" height="400">

## Описание  
Мини-эмулятор CHIP-8 на C.

## Зависимости  
- компилятор C99 (gcc / clang / MSVC)
- SDL3

## Установка  
**Linux / macOS:**
```bash
git clone https://github.com/JNoxik/MyChip8.git
cd MyChip8
make
```

## Аргументы
```text
Usage: chip8 -f <rom.ch8>

Options:
  -f <rom.ch8>   путь до ROM файла
  -p bw | amber  палитра
  -s 20          масштабирование
  -hz 600        кол-во тактов в секунду
  -v 30          громкость звука
  -nosound       отключить звук
