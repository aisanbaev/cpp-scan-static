#include "types.hpp"

// ========== Тестирование класса fixed_string ==========
// === 1. Проверка создания из строкового литерала ===
static_assert(stdx::details::fixed_string{"hello"}.data[0] == 'h');
static_assert(stdx::details::fixed_string{"hello"}.data[4] == 'o');
static_assert(stdx::details::fixed_string{"hello"}.data[5] == '\0');

// === 2. Проверка размера ===
static_assert(sizeof(stdx::details::fixed_string{"hi"}) == sizeof(char[3]));

// === 3. Проверка конструктора от массива меньшего размера ===
static_assert(stdx::details::fixed_string<10>{"hi"}.data[2] == '\0');
static_assert(stdx::details::fixed_string<10>{"hi"}.data[9] == '\0');

// строка "ab" (длина 2 + 1 = 3) не помещается в fixed_string<2>
// static_assert(stdx::details::fixed_string<2>{"ab"}); // ошибка компиляции

// === 4. Проверка конструктора от указателей ===
constexpr const char* str = "Hello world!";
constexpr auto fs = stdx::details::fixed_string<15>(str, str + 4);
static_assert(fs.data[0] == 'H');
static_assert(fs.data[3] == 'l');
static_assert(fs.data[4] == '\0');

constexpr stdx::details::fixed_string<3> str3(str, str);
static_assert(str3.data[0] == '\0');
static_assert(str3.data[1] == '\0');
static_assert(str3.data[2] == '\0');

int main() { 
    return 0;
}