/*
    File:    aux-scaner-test.cpp
    Created: 12 August 2018 at 12:09 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <string>
#include <cstdio>
#include <memory>
#include "../include/get_processed_text.h"
#include "../include/location.h"
#include "../include/errors_and_tries.h"
#include "../include/char_trie.h"
#include "../include/aux_expr_scaner.h"
#include "../include/aux_expr_lexem.h"
#include "../include/abstract_scaner.h"

enum Myauka_exit_codes{
    Success, No_args, File_processing_error, Syntax_error
};

static const char* usage_str = "Usage: %s file\n";

void test_aux_scaner(const std::shared_ptr<Aux_expr_scaner>& aux_scaner)
{
    ascaner::Token<Aux_expr_lexem_info> aeli;
    Aux_expr_lexem_code                 aelic;
    do{
        aeli   = aux_scaner-> current_lexeme();
        aelic  = aeli.lexeme_.code_;
        auto s = aux_scaner->token_to_string(aeli);
        puts(s.c_str());
    }while(aelic != Aux_expr_lexem_code::Nothing);
}

int main(int argc, char* argv[])
{
    if(1 == argc){
        printf(usage_str, argv[0]);
        return No_args;
    }

    std::u32string    text     = get_processed_text(argv[1]);
    if(!text.length()){
        return File_processing_error;
    }

    char32_t*         p        = const_cast<char32_t*>(text.c_str());
    auto              loc      = std::make_shared<ascaner::Location>(p);
    Errors_and_tries  et;
    et.ec_                     = std::make_shared<Error_count>();
    et.ids_trie_               = std::make_shared<Char_trie>();
    et.strs_trie_              = std::make_shared<Char_trie>();
    auto              asc      = std::make_shared<Aux_expr_scaner>(loc, et);
    test_aux_scaner(asc);
//     while((aelic = (aeli = aux_scaner-> current_lexem()).code) !=
//           Aux_expr_lexem_code::Nothing)
//     {
//         (this->*procs[static_cast<size_t>(state)])();
//         if(State::End_class_complement == state){
//             break;
//         }
//     }

    return Success;
}