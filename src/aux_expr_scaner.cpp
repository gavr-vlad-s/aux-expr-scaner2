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
#include "../include/print_char32.h"

template <class T, std::size_t N>
constexpr size_t size(const T (&array)[N]) noexcept
{
    return N;
}

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


Aux_expr_scaner::Automaton_proc Aux_expr_scaner::procs_[] = {
    &Aux_expr_scaner::start_proc,     &Aux_expr_scaner::unknown_proc,
    &Aux_expr_scaner::action_proc,    &Aux_expr_scaner::delimiter_proc,
    &Aux_expr_scaner::classes_proc,   &Aux_expr_scaner::char_proc,
    &Aux_expr_scaner::hat_proc,       &Aux_expr_scaner::regexp_name_proc
};

Aux_expr_scaner::Final_proc Aux_expr_scaner::finals_[] = {
    &Aux_expr_scaner::none_final_proc,
    &Aux_expr_scaner::unknown_final_proc,
    &Aux_expr_scaner::action_final_proc,
    &Aux_expr_scaner::delimiter_final_proc,
    &Aux_expr_scaner::classes_final_proc,
    &Aux_expr_scaner::char_final_proc,
    &Aux_expr_scaner::hat_final_proc,
    &Aux_expr_scaner::regexp_name_final_proc
};

bool Aux_expr_scaner::start_proc()
{
    bool t  = true;
    state_  = -1;
    /* For an automaton that processes a lexeme, the state with the number (-1)
     * is the state in which this machine is initialized. */
    if(belongs(Category::Spaces, char_categories_)){
        if(ch_ == U'\n'){
            loc_->pos_.line_pos_ = 1;
            (loc_->pos_.line_no_)++;
        }else{
            (loc_->pos_.line_pos_)++;
        }
        return t;
    }
    lexeme_pos_.begin_pos_ = loc_->pos_;
    lexeme_pos_.end_pos_   = loc_->pos_;
    if(belongs(Category::Delimiters, char_categories_)){
        automaton_           = A_delimiter;
        token_.lexeme_.code_ = Aux_expr_lexem_code::UnknownLexem;
        (loc_->pcurrent_char_)--;
    }else if(belongs(Category::Dollar, char_categories_)){
        automaton_           = A_action;
        token_.lexeme_.code_ = Aux_expr_lexem_code::Action;
        buffer_.clear();
    }else if(belongs(Category::Opened_square_br, char_categories_)){
        automaton_           = A_class,
        token_.lexeme_.code_ = Aux_expr_lexem_code::Character;
        token_.lexeme_.c_    = U'[';
    }else if(belongs(Category::Backslash, char_categories_)){
        automaton_           = A_char;
        token_.lexeme_.code_ = Aux_expr_lexem_code::Character;
    }else if(belongs(Category::Begin_expr, char_categories_)){
        token_.lexeme_.code_ = Aux_expr_lexem_code::Begin_expression;
        t                    = false;
        (loc_->pcurrent_char_)++;
    }else if(belongs(Category::End_expr, char_categories_)){
        token_.lexeme_.code_ = Aux_expr_lexem_code::End_expression;
        t                    = false;
        (loc_->pcurrent_char_)++;
    }else if(belongs(Category::Hat, char_categories_)){
        automaton_           = A_hat,
        token_.lexeme_.code_ = Aux_expr_lexem_code::Character;
        token_.lexeme_.c_    = U'^';
    }else if(belongs(Category::Percent, char_categories_)){
        automaton_           = A_regexp_name;
        token_.lexeme_.code_ = Aux_expr_lexem_code::Regexp_name;
        buffer_.clear();
    }else{
        token_.lexeme_.code_ = Aux_expr_lexem_code::Character;
        token_.lexeme_.c_    = ch_;
        t                    = false;
        (loc_->pcurrent_char_)++;
    }
    return t;
}

static const char* class_strings[] = {
    "[:Latin:]",   "[:Letter:]",  "[:Russian:]",
    "[:bdigits:]", "[:digits:]",  "[:latin:]",
    "[:letter:]",  "[:odigits:]", "[:russian:]",
    "[:xdigits:]", "[:ndq:]",     "[:nsq:]"
};

static const char* line_expects = "Line %zu expects %s.\n";

void Aux_expr_scaner::correct_class()
{
    /* This function corrects the code of the token, most likely a character class,
     * and displays the necessary diagnostics. */
    if(token_.lexeme_.code_ >= Aux_expr_lexem_code::M_Class_Latin){
        int y = static_cast<int>(token_.lexeme_.code_) -
                static_cast<int>(Aux_expr_lexem_code::M_Class_Latin);
        printf(line_expects, loc_->pos_.line_no_,class_strings[y]);
        token_.lexeme_.code_ = static_cast<Aux_expr_lexem_code>(y +
                               static_cast<int>(Aux_expr_lexem_code::Class_Latin));
        en_ -> increment_number_of_errors();
    }
}

ascaner::Token<Aux_expr_lexem_info> Aux_expr_scaner::current_lexeme()
{
    automaton_           = A_start;
    token_.lexeme_.code_ = Aux_expr_lexem_code::Nothing;
    lexeme_begin_        = loc_->pcurrent_char_;
    bool t               = true;
    while((ch_ = *(loc_->pcurrent_char_)++)){
        char_categories_ = get_categories_set(ch_);
        t = (this->*procs_[automaton_])();
        if(!t){
            /* We get here only if the lexeme has already been read. At the same time,
             * the current automaton has already read the character that follows
             * immediately after the end of the lexeme read, based on this symbol, it was
             * decided that the lexeme was read and the transition to the next character
             * was made. Therefore, in order to not miss the first character of the next
             * lexeme, you need to decrease the pcurrent_char pointer by one. */
            (loc_->pcurrent_char_)--;
            token_.range_ = lexeme_pos_;
            Aux_expr_lexem_code lc = token_.lexeme_.code_;
            if(Aux_expr_lexem_code::Action == lc){
                /* If the current lexeme is an identifier, then this identifier must be
                 * written to the identifier table. */
                token_.lexeme_.action_name_index_ = ids_ -> insert(buffer_);
            }else if(A_class == automaton_){
                /* If you have finished processing the class of characters, you need to
                 * adjust its code, and, possibly, output diagnostics. */
                correct_class();
            }else if(Aux_expr_lexem_code::Regexp_name == lc){
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

bool Aux_expr_scaner::unknown_proc()
{
    bool t =  belongs(Category::Other, char_categories_);
    if(t){
        lexeme_pos_.end_pos_.line_pos_++;
        (loc_->pos_.line_pos_)++;
    }
    return t;
}

/* This array consists of pairs of the form (state, character) and is used to initialize
 * the character class processing automaton. The sense of the element of the array is this:
 * if the current character in the initialization state coincides with the second component
 * of the element, the work begins with the state that is the first component of the element.
 * Consider, for example, the element {54, U'n '}. If the current character coincides with
 * the second component of this element, then work begins with the state being the first
 * component, i.e. from state 54. The array must be sorted in ascending order of the
 * second component.*/
static const State_for_char init_table_for_classes[] = {
    {0,  U'L'}, {14, U'R'}, {23, U'b'}, {32, U'd'}, {40, U'l'},
    {54, U'n'}, {63, U'o'}, {72, U'r'}, {81, U'x'}
};

static const char* expects_LRbdlnorx =
    "The line %zu expects one of the following characters: L, R, b, d, l, n, o, r, x.\n";

static const char* latin_letter_expected =
    "A Latin letter or an underscore is expected at the line %zu.\n";

bool Aux_expr_scaner::classes_proc()
{
    bool t = false;
    switch(state_){
        case -1:
            if(U':' == ch_){
                state_ = -2; t = true;
                lexeme_pos_.end_pos_.line_pos_++;
                (loc_->pos_.line_pos_)++;
            }else if(U'^' == ch_){
                token_.lexeme_.code_ = Aux_expr_lexem_code::Begin_char_class_complement;
                (loc_->pcurrent_char_)++;
                lexeme_pos_.end_pos_.line_pos_++;
                (loc_->pos_.line_pos_)++;
            }
            break;
        case -2:
            if(belongs(Category::After_colon, char_categories_)){
                state_               = get_init_state(ch_, init_table_for_classes,
                                                      size(init_table_for_classes));
                token_.lexeme_.code_ = a_classes_jump_table[state_].code;
                t = true;
            }else{
                printf(expects_LRbdlnorx, loc_->pos_.line_no_);
                en_ -> increment_number_of_errors();
            }
            break;
        default:
            auto elem            = a_classes_jump_table[state_];
            token_.lexeme_.code_ = elem.code;
            int y                = search_char(ch_, elem.symbols);
            if(y != THERE_IS_NO_CHAR){
                state_ = elem.first_state + y; t = true;
                lexeme_pos_.end_pos_.line_pos_++;
                (loc_->pos_.line_pos_)++;
            }
    }
    return t;
}

bool Aux_expr_scaner::char_proc()
{
    if(belongs(Category::After_backslash, char_categories_)){
        token_.lexeme_.c_ = (U'n' == ch_) ? U'\n' : ch_;
        (loc_->pcurrent_char_)++;
        lexeme_pos_.end_pos_.line_pos_++;
        (loc_->pos_.line_pos_)++;
    }else{
        token_.lexeme_.c_ = U'\\';
    }
    return false;
}

bool Aux_expr_scaner::delimiter_proc()
{
    bool t = false;
    switch(ch_){
        case U'{':
            token_.lexeme_.code_ = Aux_expr_lexem_code::Begin_expression;
            break;
        case U'}':
            token_.lexeme_.code_ = Aux_expr_lexem_code::End_expression;
            break;
        case U'(':
            token_.lexeme_.code_ = Aux_expr_lexem_code::Opened_round_brack;
            break;
        case U')':
            token_.lexeme_.code_ = Aux_expr_lexem_code::Closed_round_brack;
            break;
        case U'|':
            token_.lexeme_.code_ = Aux_expr_lexem_code::Or;
            break;
        case U'*':
            token_.lexeme_.code_ = Aux_expr_lexem_code::Kleene_closure;
            break;
        case U'+':
            token_.lexeme_.code_ = Aux_expr_lexem_code::Positive_closure;
            break;
        case U'?':
            token_.lexeme_.code_ = Aux_expr_lexem_code::Optional_member;
            break;
    }
    (loc_->pcurrent_char_)++;
    return t;
}

bool Aux_expr_scaner::action_proc()
{
    bool t = true;
    /* The variable t is true if the action name has not yet
     * been fully read, and false otherwise. */
    if(-1 == state_){
        if(belongs(Category::Action_name_begin, char_categories_)){
            buffer_ += ch_; state_ = 0;
        }else{
            printf(latin_letter_expected, loc_->pos_.line_no_);
            en_ -> increment_number_of_errors();
            t = false;
        }
        return t;
    }
    t = belongs(Category::Action_name_body, char_categories_);
    if(t){
        buffer_ += ch_;
        lexeme_pos_.end_pos_.line_pos_++;
        (loc_->pos_.line_pos_)++;
    }
    return t;
}

bool Aux_expr_scaner::regexp_name_proc()
{
    bool t = true;
    /* The variable t is true if the regexp name has not yet
     * been fully read, and false otherwise. */
    if(-1 == state_){
        if(belongs(Category::Regexp_name_begin, char_categories_)){
            buffer_ += ch_; state_ = 0;
        }else{
            printf(latin_letter_expected, loc_->pos_.line_no_);
            en_ -> increment_number_of_errors();
            t = false;
        }
        return t;
    }
    t = belongs(Category::Regexp_name_body, char_categories_);
    if(t){
        buffer_ += ch_;
        lexeme_pos_.end_pos_.line_pos_++;
        (loc_->pos_.line_pos_)++;
    }
    return t;
}

bool Aux_expr_scaner::hat_proc()
{
    bool t = false;
    if(ch_ == U']'){
        token_.lexeme_.code_ = Aux_expr_lexem_code::End_char_class_complement;
        (loc_->pcurrent_char_)++;
        lexeme_pos_.end_pos_.line_pos_++;
        (loc_->pos_.line_pos_)++;
    }
    return t;
}

void Aux_expr_scaner::none_final_proc()
{
    /* This subroutine will be called if, after reading the input text, it turned out
     * to be in the A_start automaton. Then you do not need to do anything. */
}

void Aux_expr_scaner::unknown_final_proc()
{
    /* This subroutine will be called if, after reading the input text, it turned out
     * to be in the A_unknown automaton. Then you do not need to do anything. */
}

void Aux_expr_scaner::action_final_proc()
{
    /* This function will be called if, after reading the input stream, they were
     * in the action names processing automaton, the A_action automaton. Then this
     * name should be written in the prefix tree of identifiers. */
    token_.lexeme_.action_name_index_ = ids_ -> insert(buffer_);
}

void Aux_expr_scaner::regexp_name_final_proc()
{
    /* This function will be called if, after reading the input stream, they were
     * in the action names processing automaton, the A_action automaton. Then this
     * name should be written in the prefix tree of identifiers. */
    token_.lexeme_.regexp_name_index_ = ids_ -> insert(buffer_);
}

void Aux_expr_scaner::delimiter_final_proc()
{
}

void Aux_expr_scaner::classes_final_proc()
{
    switch(state_){
        case -1:
            token_.lexeme_.code_ = Aux_expr_lexem_code::Character;
            token_.lexeme_.c_    = U'[';
            break;
        case -2:
            token_.lexeme_.code_ = Aux_expr_lexem_code::UnknownLexem;
            break;
        default:
            token_.lexeme_.code_ = a_classes_jump_table[state_].code;
            correct_class();
    }
}

void Aux_expr_scaner::char_final_proc()
{
    token_.lexeme_.c_ = U'\\';
}

void Aux_expr_scaner::hat_final_proc()
{
    token_.lexeme_.code_ = Aux_expr_lexem_code::Character;
    token_.lexeme_.c_    = U'^';
}

/* Данный массив состоит из строковых литералов, представляющих собой заключённые
 * в кавычки идентификаторы из перечисления Lexem_code. Строки идут в том же
 * порядке, что и соответствующие идентификаторы перечисления Lexem_code. */
static const char* lexem_names[] = {
    "Nothing",                     "UnknownLexem",              "Action",
    "Opened_round_brack",          "Closed_round_brack",        "Or",
    "Kleene_closure",              "Positive_closure",          "Optional_member",
    "Character",                   "Begin_expression",          "End_expression",
    "Class_Latin",                 "Class_Letter",              "Class_Russian",
    "Class_bdigits",               "Class_digits",              "Class_latin",
    "Class_letter",                "Class_odigits",             "Class_russian",
    "Class_xdigits",               "Class_ndq",                 "Class_nsq",
    "Begin_char_class_complement", "End_char_class_complement", "M_Class_Latin",
    "M_Class_Letter",              "M_Class_Russian",           "M_Class_bdigits",
    "M_Class_digits",              "M_Class_latin",             "M_Class_letter",
    "M_Class_odigits",             "M_Class_russian",           "M_Class_xdigits",
    "M_Class_ndq",                 "M_Class_nsq"
};

std::string Aux_expr_scaner::lexeme_to_string(const Aux_expr_lexem_info li)
{
    std::string         result;
    Aux_expr_lexem_code lc    = li.code_;
    result                    = lexem_names[static_cast<uint16_t>(lc)];
    if(Aux_expr_lexem_code::Character == lc){
        result += " " + show_char32(li.c_);
    }
    return result;
}