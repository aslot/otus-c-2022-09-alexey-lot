Сборки программы: gcc main.c -Wall -Wextra -Wpedantic -std=c11
Программа main.c преобразует текст исходного файла из заданной кодировки в UTF-8 и сохраняет в файл.
Тестовый запуск:
./a.out koi8.txt "koi8" result
./a.out iso-8859-5.txt "iso8859" result
./a.out cp1251.txt "cp1251" result
Программа проверена в системе Ubuntu x64.
