/*
    File:    aux_expr_scaner.cpp
    Created: 20 July 2017 at 12:14 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include "../include/aux_expr_scaner.h"
#include "../include/aux_expr_lexem.h"
#include "../include/knuth_find.h"
#include "../include/belongs.h"
#include "../include/search_char.h"
#include "../include/get_init_state.h"
#include "../include/elem.h"
#include "../include/aux_expr_scaner_classes_table.h"
// #include <cstdlib>
// #include <utility>

enum class Category : uint16_t{
    Spaces,      Other,           Action_name_begin, Action_name_body,
    Delimiters,  Dollar,          Backslash,         Opened_square_br,
    After_colon, After_backslash, Begin_expr,        End_expr,
    Hat,         Percent,         Regexp_name_begin, Regexp_name_body
};

static const Segment_with_value<char32_t, uint64_t> categories_table[] = {
    {{U'b' , U'b' }, 49420}, {{U'R'   , U'R'}, 49420}, {{U'p' , U'q' }, 49164},
    {{U'\?', U'\?'}, 528  }, {{U']'   , U']'}, 512  }, {{U'l' , U'l' }, 49420},
    {{U'y' , U'z' }, 49164}, {{U'%'   , U'%'}, 8704 }, {{U'L' , U'L' }, 49420},
    {{U'[' , U'[' }, 640  }, {{U'_'   , U'_'}, 49164}, {{U'd' , U'd' }, 49420},
    {{U'n' , U'n' }, 49932}, {{U's'   , U'w'}, 49164}, {{U'|' , U'|' }, 528  },
    {{U'\"', U'\"'}, 512  }, {{U'0'   , U'9'}, 32776}, {{U'A' , U'K' }, 49164},
    {{U'M' , U'Q' }, 49164}, {{U'S'   , U'Z'}, 49164}, {{U'\\', U'\\'}, 576  },
    {{U'^' , U'^' }, 4608 }, {{U'a'   , U'a'}, 49164}, {{U'c' , U'c' }, 49164},
    {{U'e' , U'k' }, 49164}, {{U'm'   , U'm'}, 49164}, {{U'o' , U'o' }, 49420},
    {{U'r' , U'r' }, 49420}, {{U'x'   , U'x'}, 49420}, {{U'{' , U'{' }, 1552 },
    {{U'}' , U'}' }, 2576 }, {{U'\x01', U' '}, 1    }, {{U'$' , U'$' }, 544  },
    {{U'(' , U'+' }, 528  }
};

static constexpr size_t num_of_elems_in_categories_table = 34;

uint64_t get_categories_set(char32_t c)
{
    auto t = knuth_find(categories_table,
                        categories_table + num_of_elems_in_categories_table,
                        c);
    return t.first ? categories_table[t.second].value :
                     1ULL << static_cast<uint64_t>(Category::Other);
}

static inline uint64_t belongs(Category cat, uint64_t set_of_categories)
{
    return belongs(static_cast<uint64_t>(cat), set_of_categories);
}


Aux_expr_scaner::Automaton_proc Aux_expr_scaner::procs[] = {
    &Aux_expr_scaner::start_proc,     &Aux_expr_scaner::unknown_proc,
    &Aux_expr_scaner::action_proc,    &Aux_expr_scaner::delimiter_proc,
    &Aux_expr_scaner::classes_proc,   &Aux_expr_scaner::char_proc,
    &Aux_expr_scaner::hat_proc,       &Aux_expr_scaner::regexp_name_proc
};

Aux_expr_scaner::Final_proc Aux_expr_scaner::finals[] = {
    &Aux_expr_scaner::none_final_proc,
    &Aux_expr_scaner::unknown_final_proc,
    &Aux_expr_scaner::action_final_proc,
    &Aux_expr_scaner::delimiter_final_proc,
    &Aux_expr_scaner::classes_final_proc,
    &Aux_expr_scaner::char_final_proc,
    &Aux_expr_scaner::hat_final_proc,
    &Aux_expr_scaner::regexp_name_final_proc
};

// bool Aux_expr_scaner::start_proc()
// {
//     bool t = true;
//     state = -1;
//     /* For an automaton that processes a lexeme, the state with the number (-1)
//      * is the state in which this machine is initialized. */
//     if(belongs(Category::Spaces, char_categories)){
//         loc->current_line += U'\n' == ch;
//         return t;
//     }
//     lexem_begin_line = loc->current_line;
//     if(belongs(Category::Delimiters, char_categories)){
//         automaton = A_delimiter; token.code = Aux_expr_lexem_code::UnknownLexem;
//         (loc->pcurrent_char)--;
//     }else if(belongs(Category::Dollar, char_categories)){
//         automaton = A_action;    token.code = Aux_expr_lexem_code::Action;
//         buffer.clear();
//     }else if(belongs(Category::Opened_square_br, char_categories)){
//         automaton = A_class,     token.code = Aux_expr_lexem_code::Character;
//         token.c = U'[';
//     }else if(belongs(Category::Backslash, char_categories)){
//          automaton = A_char;     token.code = Aux_expr_lexem_code::Character;
//     }else if(belongs(Category::Begin_expr, char_categories)){
//         token.code = Aux_expr_lexem_code::Begin_expression; t = false;
//         (loc->pcurrent_char)++;
//     }else if(belongs(Category::End_expr, char_categories)){
//         token.code = Aux_expr_lexem_code::End_expression; t = false;
//         (loc->pcurrent_char)++;
//     }else if(belongs(Category::Hat, char_categories)){
//         automaton = A_hat,     token.code = Aux_expr_lexem_code::Character;
//         token.c   = U'^';
//     }else if(belongs(Category::Percent, char_categories)){
//         automaton = A_regexp_name;    token.code = Aux_expr_lexem_code::Regexp_name;
//         buffer.clear();
//     }else{
//         token.code = Aux_expr_lexem_code::Character; token.c = ch; t = false;
//         (loc->pcurrent_char)++;
//     }
//     return t;
// }
//
// static const char* class_strings[] = {
//     "[:Latin:]",   "[:Letter:]",  "[:Russian:]",
//     "[:bdigits:]", "[:digits:]",  "[:latin:]",
//     "[:letter:]",  "[:odigits:]", "[:russian:]",
//     "[:xdigits:]", "[:ndq:]",     "[:nsq:]"
// };
//
// static const char* line_expects = "Line %zu expects %s.\n";
//
// void Aux_expr_scaner::correct_class()
// {
//     /* This function corrects the code of the token, most likely a character class,
//      * and displays the necessary diagnostics. */
//     if(token.code >= Aux_expr_lexem_code::M_Class_Latin){
//         int y = static_cast<int>(token.code) -
//                 static_cast<int>(Aux_expr_lexem_code::M_Class_Latin);
//         printf(line_expects, loc->current_line,class_strings[y]);
//         token.code = static_cast<Aux_expr_lexem_code>(y +
//                         static_cast<int>(Aux_expr_lexem_code::Class_Latin));
//         en -> increment_number_of_errors();
//     }
// }

ascaner::Token<Aux_expr_lexem_info> Aux_expr_scaner::current_lexeme()
{
    automaton            = A_start;
    token_.lexeme_.code_ =  Aux_expr_lexem_code::Nothing;
    lexeme_begin_ = loc_->pcurrent_char_;
    bool t = true;
    while((ch_ = *(loc_->pcurrent_char_)++)){
        char_categories_ = get_categories_set(ch);
        t = (this->*procs_[automaton_])();
        if(!t){
            /* We get here only if the lexeme has already been read. At the same time,
             * the current automaton has already read the character that follows
             * immediately after the end of the lexeme read, based on this symbol, it was
             * decided that the lexeme was read and the transition to the next character
             * was made. Therefore, in order to not miss the first character of the next
             * lexeme, you need to decrease the pcurrent_char pointer by one. */
            (loc_->pcurrent_char_)--;
            if(Aux_expr_lexem_code::Action == token_.lexeme_.code_){
                /* If the current lexeme is an identifier, then this identifier must be
                 * written to the identifier table. */
                token_.lexeme_.action_name_index_ = ids_ -> insert(buffer_);
            }else if(A_class == automaton){
                /* If you have finished processing the class of characters, you need to
                 * adjust its code, and, possibly, output diagnostics. */
                correct_class();
            }else if(Aux_expr_lexem_code::Regexp_name == token_.lexeme_.code_){
                token_.lexeme_.regexp_name_index_ = ids_ -> insert(buffer_);
            }
            return token_;
        }
    }
    /* Here we can be, only if we have already read all the processed text. In this
     * case, the pointer to the current symbol points to a character that is immediately
     * after the null character, which is a sign of the end of the text. To avoid entering
     * subsequent calls outside the text, you need to go back to the null character.*/
    (loc_->pcurrent_char_)--;
    /* Further, since we are here, the end of the current token (perhaps unexpected) has
     * not yet been processed. It is necessary to perform this processing, and, probably,
     * to display some kind of diagnostics.*/
    (this->*finals_[automaton_])();
    return token_;
}

// bool Aux_expr_scaner::unknown_proc()
// {
//     return belongs(Category::Other, char_categories);
// }
//
// /* This array consists of pairs of the form (state, character) and is used to initialize
//  * the character class processing automaton. The sense of the element of the array is this:
//  * if the current character in the initialization state coincides with the second component
//  * of the element, the work begins with the state that is the first component of the element.
//  * Consider, for example, the element {54, U'n '}. If the current character coincides with
//  * the second component of this element, then work begins with the state being the first
//  * component, i.e. from state 54. The array must be sorted in ascending order of the
//  * second component.*/
// static const State_for_char init_table_for_classes[] = {
//     {0, U'L'},  {14, U'R'}, {23, U'b'}, {32, U'd'}, {40, U'l'},
//     {54, U'n'}, {63, U'o'}, {72, U'r'}, {81, U'x'}
// };
//
// static const char* expects_LRbdlnorx =
//     "The line %zu expects one of the following characters: L, R, b, d, l, n, o, r, x.\n";
//
// static const char* latin_letter_expected =
//     "A Latin letter or an underscore is expected at the line %zu.\n";
//
// bool Aux_expr_scaner::classes_proc()
// {
//     bool t = false;
//     switch(state){
//         case -1:
//             if(U':' == ch){
//                 state = -2; t = true;
//             }else if(U'^' == ch){
//                 token.code = Aux_expr_lexem_code::Begin_char_class_complement;
//                 (loc->pcurrent_char)++;
//             }
//             break;
//         case -2:
//             if(belongs(Category::After_colon, char_categories)){
//                 state = get_init_state(ch, init_table_for_classes,
//                                        sizeof(init_table_for_classes)/
//                                        sizeof(State_for_char));
//                 token.code = a_classes_jump_table[state].code;
//                 t = true;
//             }else{
//                 printf(expects_LRbdlnorx, loc->current_line);
//                 en -> increment_number_of_errors();
//             }
//             break;
//         default:
//             auto elem  = a_classes_jump_table[state];
//             token.code = elem.code;
//             int y      = search_char(ch, elem.symbols);
//             if(y != THERE_IS_NO_CHAR){
//                 state = elem.first_state + y; t = true;
//             }
//     }
//     return t;
// }
//
// bool Aux_expr_scaner::char_proc()
// {
//     if(belongs(Category::After_backslash, char_categories)){
//         token.c = (U'n' == ch) ? U'\n' : ch;
//         (loc->pcurrent_char)++;
//     }else{
//         token.c = U'\\';
//     }
//     return false;
// }
//
// bool Aux_expr_scaner::delimiter_proc()
// {
//     bool t = false;
//     switch(ch){
//         case U'{':
//             token.code = Aux_expr_lexem_code::Begin_expression;
//             break;
//         case U'}':
//             token.code = Aux_expr_lexem_code::End_expression;
//             break;
//         case U'(':
//             token.code = Aux_expr_lexem_code::Opened_round_brack;
//             break;
//         case U')':
//             token.code = Aux_expr_lexem_code::Closed_round_brack;
//             break;
//         case U'|':
//             token.code = Aux_expr_lexem_code::Or;
//             break;
//         case U'*':
//             token.code = Aux_expr_lexem_code::Kleene_closure;
//             break;
//         case U'+':
//             token.code = Aux_expr_lexem_code::Positive_closure;
//             break;
//         case U'?':
//             token.code = Aux_expr_lexem_code::Optional_member;
//             break;
//     }
//     (loc->pcurrent_char)++;
//     return t;
// }
//
// bool Aux_expr_scaner::action_proc()
// {
//     bool t = true;
//     /* The variable t is true if the action name has not yet
//      * been fully read, and false otherwise. */
//     if(-1 == state){
//         if(belongs(Category::Action_name_begin, char_categories)){
//             buffer += ch; state = 0;
//         }else{
//             printf(latin_letter_expected, loc->current_line);
//             en -> increment_number_of_errors();
//             t = false;
//         }
//         return t;
//     }
//     t = belongs(Category::Action_name_body, char_categories);
//     if(t){
//         buffer += ch;
//     }
//     return t;
// }
//
// bool Aux_expr_scaner::regexp_name_proc()
// {
//     bool t = true;
//     /* The variable t is true if the regexp name has not yet
//      * been fully read, and false otherwise. */
//     if(-1 == state){
//         if(belongs(Category::Regexp_name_begin, char_categories)){
//             buffer += ch; state = 0;
//         }else{
//             printf(latin_letter_expected, loc->current_line);
//             en -> increment_number_of_errors();
//             t = false;
//         }
//         return t;
//     }
//     t = belongs(Category::Regexp_name_body, char_categories);
//     if(t){
//         buffer += ch;
//     }
//     return t;
// }
//
// bool Aux_expr_scaner::hat_proc()
// {
//     bool t = false;
//     if(ch == U']'){
//         token.code = Aux_expr_lexem_code::End_char_class_complement;
//         (loc->pcurrent_char)++;
//     }
//     return t;
// }
//
// void Aux_expr_scaner::none_final_proc()
// {
//     /* This subroutine will be called if, after reading the input text, it turned out
//      * to be in the A_start automaton. Then you do not need to do anything. */
// }
//
// void Aux_expr_scaner::unknown_final_proc()
// {
//     /* This subroutine will be called if, after reading the input text, it turned out
//      * to be in the A_unknown automaton. Then you do not need to do anything. */
// }
//
// void Aux_expr_scaner::action_final_proc()
// {
//     /* This function will be called if, after reading the input stream, they were
//      * in the action names processing automaton, the A_action automaton. Then this
//      * name should be written in the prefix tree of identifiers. */
//     token.action_name_index = ids -> insert(buffer);
// }
//
// void Aux_expr_scaner::regexp_name_final_proc()
// {
//     /* This function will be called if, after reading the input stream, they were
//      * in the action names processing automaton, the A_action automaton. Then this
//      * name should be written in the prefix tree of identifiers. */
//     token.regexp_name_index = ids -> insert(buffer);
// }
//
// void Aux_expr_scaner::delimiter_final_proc()
// {
// }
//
// void Aux_expr_scaner::classes_final_proc()
// {
//     switch(state){
//         case -1:
//             token.code = Aux_expr_lexem_code::Character;  token.c = U'[';
//             break;
//         case -2:
//             token.code = Aux_expr_lexem_code::UnknownLexem;
//             break;
//         default:
//             token.code = a_classes_jump_table[state].code;
//             correct_class();
//     }
// }
//
// void Aux_expr_scaner::char_final_proc()
// {
//     token.c = U'\\';
// }
//
// void Aux_expr_scaner::hat_final_proc()
// {
//     token.code = Aux_expr_lexem_code::Character;
//     token.c    = U'^';
// }