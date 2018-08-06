/*
    File:    abstract_scaner.h
    Created: 06 August 2018 at 10:30 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef ABSTRACT_SCANER_H
#define ABSTRACT_SCANER_H
#   include <cstddef>
#   include "../include/position.h"
#   include "../include/location.h"
#   include "../include/errors_and_tries.h"
namespace ascaner{
    template<typename Lexeme_type>
    struct Token{
        Position_range range_;
        Lexeme_type    lexeme_;
    };

    template<typename Lexeme_type>
    class Abstract_scaner{
    public:
        Abstract_scaner<Lexeme_type>()                       = default;
        Abstract_scaner(const Location_ptr& location, const Errors_and_tries& et);
        Abstract_scaner(const Abstract_scaner<Lexeme_type>&) = default;
        virtual ~Abstract_scaner<Lexeme_type>()              = default;

        /*  Function back() return the current lexem into the input stream. */
        void back();

        /* Function current_lexeme() returns information about current lexem,
         * i.e. returns a lexeme code and a lexeme value. */
        virtual Token<Lexeme_type> current_lexeme() = 0;
//         Position_range      lexeme_pos() const;
//
//         char32_t*           lexeme_begin_ptr() const;
    protected:
    int                          state_; /* the current state of the current automaton */
    Location_ptr                 loc_;
    char32_t*                    lexeme_begin_; /* pointer to the lexem begin */
    char32_t                     ch_;           /* current character */

//     /* set of categories for the current character */
//     uint64_t                     char_categories;
//
//     /* intermediate value of the lexem information */
//     Lexem_type                   token;
//
//     /* the line number at which the current lexem starts */
//     size_t                       lexem_begin_line;
//
//     /* a pointer to a class that counts the number of errors: */
//     std::shared_ptr<Error_count> en;
//     /* a pointer to the prefix tree for identifiers: */
//     std::shared_ptr<Char_trie>   ids;
//     /* a pointer to the prefix tree for string literals: */
//     std::shared_ptr<Char_trie>   strs;
//
//     /* buffer for writing the processed identifier or string: */
//     std::u32string               buffer;
    };
};
// #include <string>
// #include <memory>
// #include "../include/error_count.h"
// #include "../include/trie.h"
// #include "../include/char_trie.h"
// protected:
// };
//
// template<typename Lexem_type>
// Abstract_scaner<Lexem_type>::Abstract_scaner(const Location_ptr& location,
//                                              const Errors_and_tries& et)
// {
//     ids              = et.ids_trie;
//     strs             = et.strs_trie;
//     en               = et.ec;
//     loc              = location;
//     lexem_begin      = location->pcurrent_char;
//     lexem_begin_line = 1;
// }
//
// template<typename Lexem_type>
// void Abstract_scaner<Lexem_type>::back()
// {
//     loc->pcurrent_char = lexem_begin;
//     loc->current_line  = lexem_begin_line;
// }
//
// template<typename Lexem_type>
// size_t Abstract_scaner<Lexem_type>::lexem_begin_line_number() const
// {
//     return lexem_begin_line;
// }
//
// template<typename Lexem_type>
// char32_t* Abstract_scaner<Lexem_type>::lexem_begin_ptr() const
// {
//     return lexem_begin;
// }
#endif