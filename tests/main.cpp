#include "types.hpp"
#include "format_string.hpp"

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

// ========== Тестирование класса format_string ==========
using stdx::details::operator""_fs;
// === 1. Проверка исходной строки ===
static_assert("Hello {}"_fs.source.data[6] == '{');
static_assert("Hello {}"_fs.source.data[7] == '}');
static_assert("Hello {}"_fs.source.data[8] == '\0');

// === 2. Проверка числа плейсхолдеров ===
static_assert("Hello {}"_fs.number_placeholders == 1);
static_assert("Hello {} and {%d}"_fs.number_placeholders == 2);
static_assert("No placeholders here"_fs.number_placeholders == 0);
static_assert("Multiple {} {} {}"_fs.number_placeholders == 3);
static_assert("Value: {%d}, Text: {%s}, Float: {%f}"_fs.number_placeholders == 3);

// === 3. Проверка позиций плейсхолдеров ===
static_assert("Hello"_fs.number_placeholders == 0);
static_assert("Hello"_fs.placeholder_positions.size() == 0);
static_assert("Hello {}"_fs.placeholder_positions[0].first == 6);
static_assert("Hello {}"_fs.placeholder_positions[0].second == 7);

static_assert("Test {%d} and {}"_fs.placeholder_positions[0].first == 5);
static_assert("Test {%d} and {}"_fs.placeholder_positions[0].second == 8);
static_assert("Test {%d} and {}"_fs.placeholder_positions[1].first == 14);
static_assert("Test {%d} and {}"_fs.placeholder_positions[1].second == 15);

// === 4. Проверка корректности обработки плейсхолдеров без спецификаторов ===
static_assert("{}"_fs.number_placeholders == 1);
static_assert("{} and {}"_fs.number_placeholders == 2);

// === 5. Проверка строки с плейсхолдером с разными спецификаторами
static_assert("{%d}"_fs.number_placeholders == 1);
static_assert("{%s}"_fs.number_placeholders == 1);
static_assert("{%f}"_fs.number_placeholders == 1);
static_assert("{%u}"_fs.number_placeholders == 1);

// === 6. Проверка, что при неправильной строке будет ошибка компиляции ===
// static_assert("Unclosed {"_fs.number_placeholders); // Ошибка компиляции
// static_assert("{%x}"_fs.number_placeholders); // Ошибка компиляции

int main() { 
    return 0;
}