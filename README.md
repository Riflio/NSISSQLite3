# NSISSQLite3

Плагин к [NSIS](https://ru.wikipedia.org/wiki/Nullsoft_Scriptable_Install_System) для выполнения запросов СУБД [SQLite3](https://en.wikipedia.org/wiki/SQLite)


## Возможности

* NSIS 3.10, SQLite 3.46.1
* INSERT/UPDATE/DELETE/CREATE/DROP/etc (WITHOUT return result from SELECT and so on)
* Import sql file

## Сборка

Для сборки под Windows необходим [MSYS2](https://www.msys2.org),
затем открыть консоль 'MSYS2 MINGW32':
```
pacman -Syu
pacman -S mingw-w64-i686-toolchain
pacman -S mingw-w64-i686-cmake

mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Готовый плагин nsissqliteplugin.dll поместить в:
* Windows: "C:\Program Files (x86)\NSIS\Pluguns\x86-unicode\"
* Linux: "/usr/share/nsis/Plugins/x86-unicode/"


## Пример использования

```
...
nsissqliteplugin::execQuery 'path/to/db.sqlite' 'INSERT INTO table (col1, col2) VALUES ("val 1", "val 2");'
...
nsissqliteplugin::importDump 'path/to/db.sqlite' 'path/to/dump.sql'
...
```
