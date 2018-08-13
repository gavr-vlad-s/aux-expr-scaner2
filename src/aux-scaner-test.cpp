/*
    File:    aux-scaner-test.cpp
    Created: 12 August 2018 at 12:09 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <string>

struct Act_info{
    std::u32string name_;
    std::u32string body_;
};

static const Act_info added_acts[] = {
    {U"write",                      U"buffer += ch;"                                          },
    {U"add_dec_digit_to_char_code", U"char_code = char_code * 10 + digit2int(ch);"            },
    {U"add_hex_digit_to_char_code", U"char_code = char_code << 4 + digit2int(ch);"            },
    {U"add_bin_digit_to_char_code", U"char_code = char_code << 1 + digit2int(ch);"            },
    {U"add_oct_digit_to_char_code", U"char_code = char_code << 3 + digit2int(ch);"            },
    {U"write_by_code",              U"buffer += char_code;"                                   },
    {U"add_dec_digit",              U"token.int_value = token.int_value * 10 + digit2int(ch);"},
    {U"add_hex_digit",              U"token.int_value = token.int_value << 4 + digit2int(ch);"},
    {U"add_bin_digit",              U"token.int_value = token.int_value << 1 + digit2int(ch);"},
    {U"add_oct_digit",              U"token.int_value = token.int_value << 3 + digit2int(ch);"}
};

int main(int argc, char* argv[]){
    return 0;
}