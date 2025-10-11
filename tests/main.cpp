#include "types.hpp"
#include "format_string.hpp"
#include "scan.hpp"
#include <cassert>
#include <iostream>

using stdx::details::fixed_string;
using namespace stdx::details::literals;

// ========== Тестирование класса fixed_string ==========
// === 1. Проверка создания из строкового литерала ===
static_assert(fixed_string{"hello"}.data[0] == 'h');
static_assert(fixed_string{"hello"}.data[4] == 'o');
static_assert(fixed_string{"hello"}.data[5] == '\0');

// === 2. Проверка размера ===
static_assert(sizeof(fixed_string{"hi"}) == sizeof(char[3]));

// === 3. Проверка конструктора от массива меньшего размера ===
static_assert(fixed_string<10>{"hi"}.data[2] == '\0');
static_assert(fixed_string<10>{"hi"}.data[9] == '\0');

// строка "ab" (длина 2 + 1 = 3) не помещается в fixed_string<2>
// static_assert(fixed_string<2>{"ab"}); // ошибка компиляции

// === 4. Проверка конструктора от указателей ===
constexpr const char* str = "Hello world!";
constexpr auto fs = fixed_string<15>(str, str + 4);
static_assert(fs.data[0] == 'H');
static_assert(fs.data[3] == 'l');
static_assert(fs.data[4] == '\0');

constexpr fixed_string<3> str3(str, str);
static_assert(str3.data[0] == '\0');
static_assert(str3.data[1] == '\0');
static_assert(str3.data[2] == '\0');

// ========== Тестирование класса format_string ==========
// === 1. Проверка исходной строки ===
static_assert("Hello {}"_fs.source.data[6] == '{');
static_assert("Hello {}"_fs.source.data[7] == '}');
static_assert("Hello {}"_fs.source.data[8] == '\0');

// === 2. Проверка числа плейсхолдеров ===
static_assert("Hello {}"_fs.number_placeholders == 1);
static_assert("Hello {} and {%d}"_fs.number_placeholders == 2);
static_assert("No placeholders here"_fs.number_placeholders == 0);
static_assert("Multiple {} {} {}"_fs.number_placeholders == 3);
static_assert("Value: {%d}, Text: {%s}, Unsigned: {%u}"_fs.number_placeholders == 3);

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

// === 5. Проверка строки с плейсхолдером с разными спецификаторами ===
static_assert("{%d}"_fs.number_placeholders == 1);
static_assert("{%s}"_fs.number_placeholders == 1);
static_assert("{%u}"_fs.number_placeholders == 1);

// === 6. Проверка, что при неправильной строке будет ошибка компиляции ===
// static_assert("Unclosed {"_fs.number_placeholders); // Ошибка компиляции
// static_assert("{%x}"_fs.number_placeholders); // Ошибка компиляции

// ========== Тестирование функции scan ==========
// === 1. Базовые тесты с одним плейсхолдером ===
constexpr auto test1 = stdx::scan<"{}"_fs, "hello", std::string_view>();
static_assert(std::get<0>(test1.values()) == "hello");

constexpr auto test2 = stdx::scan<"Value: {}"_fs, "Value: 42", int>();
static_assert(std::get<0>(test2.values()) == 42);

constexpr auto test3 = stdx::scan<"Number: {}"_fs, "Number: 123", unsigned int>();
static_assert(std::get<0>(test3.values()) == 123u);

// === 2. Тесты со спецификаторами ===
constexpr auto test4 = stdx::scan<"{%d}"_fs, "42", int>();
static_assert(std::get<0>(test4.values()) == 42);

constexpr auto test5 = stdx::scan<"{%u}"_fs, "123", unsigned int>();
static_assert(std::get<0>(test5.values()) == 123u);

constexpr auto test6 = stdx::scan<"{%s}"_fs, "hello world", std::string_view>();
static_assert(std::get<0>(test6.values()) == "hello world");

// === 3. Тесты с разными комбинациями форматирования ===
constexpr auto test7 = stdx::scan<"Text: {}"_fs, "Text: example", std::string_view>();
static_assert(std::get<0>(test7.values()) == "example");

constexpr auto test8 = stdx::scan<"Count: {}"_fs, "Count: 999", int>();
static_assert(std::get<0>(test8.values()) == 999);

// === 4. Тесты на граничные случаи ===
constexpr auto test9 = stdx::scan<"{}"_fs, "0", int>();
static_assert(std::get<0>(test9.values()) == 0);

constexpr auto test10 = stdx::scan<"{}"_fs, "a", std::string_view>();
static_assert(std::get<0>(test10.values()) == "a");

// ========== Тестирование функции scan с несколькими параметрами ==========
// === 1. Два параметра - строка и число ===
constexpr auto test_multi1 = stdx::scan<"{} {}"_fs, "hello 42", std::string_view, int>();
static_assert(std::get<0>(test_multi1.values()) == "hello");
static_assert(std::get<1>(test_multi1.values()) == 42);

// === 2. Два параметра - число и строка ===
constexpr auto test_multi2 = stdx::scan<"{} {}"_fs, "123 world", int, std::string_view>();
static_assert(std::get<0>(test_multi2.values()) == 123);
static_assert(std::get<1>(test_multi2.values()) == "world");

// === 3. Два параметра со спецификаторами ===
constexpr auto test_multi3 = stdx::scan<"{%d} {%s}"_fs, "456 test", int, std::string_view>();
static_assert(std::get<0>(test_multi3.values()) == 456);
static_assert(std::get<1>(test_multi3.values()) == "test");

// === 4. Три параметра ===
constexpr auto test_multi4 = stdx::scan<"{} {} {}"_fs, "a 1 b", std::string_view, int, std::string_view>();
static_assert(std::get<0>(test_multi4.values()) == "a");
static_assert(std::get<1>(test_multi4.values()) == 1);
static_assert(std::get<2>(test_multi4.values()) == "b");

// === 5. Три параметра с разными типами ===
constexpr auto test_multi5 = stdx::scan<"{} {} {}"_fs, "text 789 999", std::string_view, int, unsigned int>();
static_assert(std::get<0>(test_multi5.values()) == "text");
static_assert(std::get<1>(test_multi5.values()) == 789);
static_assert(std::get<2>(test_multi5.values()) == 999u);

// === 6. Смешанные плейсхолдеры - с спецификаторами и без ===
constexpr auto test_multi6 = stdx::scan<"{} {%d} {}"_fs, "first 123 last", std::string_view, int, std::string_view>();
static_assert(std::get<0>(test_multi6.values()) == "first");
static_assert(std::get<1>(test_multi6.values()) == 123);
static_assert(std::get<2>(test_multi6.values()) == "last");

// === 7. Тест на правильное определение типов по спецификаторам ===
constexpr auto test_multi7 = stdx::scan<"{%d} {%u} {%s}"_fs, "-42 100 string", int, unsigned int, std::string_view>();
static_assert(std::get<0>(test_multi7.values()) == -42);
static_assert(std::get<1>(test_multi7.values()) == 100u);
static_assert(std::get<2>(test_multi7.values()) == "string");

// === 8. Проверка типов для многопараметровых результатов ===
static_assert(std::is_same_v<decltype(test_multi1), const stdx::details::scan_result<std::string_view, int>>);
static_assert(std::is_same_v<decltype(test_multi2), const stdx::details::scan_result<int, std::string_view>>);
static_assert(std::is_same_v<decltype(test_multi3), const stdx::details::scan_result<int, std::string_view>>);
static_assert(std::is_same_v<decltype(test_multi4), const stdx::details::scan_result<std::string_view, int, std::string_view>>);
static_assert(std::is_same_v<decltype(test_multi5), const stdx::details::scan_result<std::string_view, int, unsigned int>>);

// ========== Тесты поддерживаемых типов ==========
// Целочисленные типы со знаком
constexpr auto test_int8 = stdx::scan<"{}"_fs, "127", int8_t>();
static_assert(std::get<0>(test_int8.values()) == 127);

constexpr auto test_int16 = stdx::scan<"{}"_fs, "32767", int16_t>();
static_assert(std::get<0>(test_int16.values()) == 32767);

constexpr auto test_int32 = stdx::scan<"{}"_fs, "2147483647", int32_t>();
static_assert(std::get<0>(test_int32.values()) == 2147483647);

constexpr auto test_int64 = stdx::scan<"{}"_fs, "9223372036854775807", int64_t>();
static_assert(std::get<0>(test_int64.values()) == 9223372036854775807LL);

// Целочисленные типы без знака
constexpr auto test_uint8 = stdx::scan<"{}"_fs, "255", uint8_t>();
static_assert(std::get<0>(test_uint8.values()) == 255u);

constexpr auto test_uint16 = stdx::scan<"{}"_fs, "65535", uint16_t>();
static_assert(std::get<0>(test_uint16.values()) == 65535u);

constexpr auto test_uint32 = stdx::scan<"{}"_fs, "4294967295", uint32_t>();
static_assert(std::get<0>(test_uint32.values()) == 4294967295u);

constexpr auto test_uint64 = stdx::scan<"{}"_fs, "18446744073709551615", uint64_t>();
static_assert(std::get<0>(test_uint64.values()) == 18446744073709551615ULL);

// CV-квалифицированные версии
constexpr auto test_const_int = stdx::scan<"{}"_fs, "42", const int>();
static_assert(std::get<0>(test_const_int.values()) == 42);

constexpr auto test_const_string_view = stdx::scan<"{}"_fs, "test", const std::string_view>();
static_assert(std::get<0>(test_const_string_view.values()) == "test");

// ========== Тесты НЕ поддерживаемых типов (вызывают ошибки компиляции) ==========
// 1. std::string (должен быть string_view)
// constexpr auto test_string = stdx::scan<"{}"_fs, "test", std::string>();

// 2. Числа с плавающей точкой
// constexpr auto test_float = stdx::scan<"{}"_fs, "3.14", float>();
// constexpr auto test_double = stdx::scan<"{}"_fs, "3.14", double>();

// 3. Ссылочные типы
// constexpr auto test_int_ref = stdx::scan<"{}"_fs, "42", int&>();
// constexpr auto test_const_ref = stdx::scan<"{}"_fs, "42", const int&>();

// 4. Указатели
// constexpr auto test_int_ptr = stdx::scan<"{}"_fs, "42", int*>();
// constexpr auto test_string_ptr = stdx::scan<"{}"_fs, "test", std::string_view*>();

// 5. Пользовательские типы
// struct CustomType {};
// constexpr auto test_custom = stdx::scan<"{}"_fs, "data", CustomType>();

// 6. bool
// constexpr auto test_bool = stdx::scan<"{}"_fs, "true", bool>();

// 7. char
// constexpr auto test_char = stdx::scan<"{}"_fs, "a", char>();

int main() {
    std::cout << "All tests passed at compile-time!" << std::endl;
    return 0;
}