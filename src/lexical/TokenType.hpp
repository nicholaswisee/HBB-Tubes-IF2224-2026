#pragma once

#include <iostream>
#include <string_view>
#include <variant>

enum class TokenType {
    // Literals / Value
    intcon,  // Integer constant
    realcon, // Real/Float constant
    charcon, // Single character
    string,  // Sequence of characters
    ident,   // User-defined names

    // Arithmetic Operators
    plus,  // +
    minus, // -
    times, // *
    rdiv,  // /
    idiv,  // div
    imod,  // MOD

    // Logical Operators
    notsy, // NOT
    andsy, // AND
    orsy,  // OR

    // Comparison Operators
    eql, // ==
    neq, // <>
    gtr, // >
    geq, // >=
    lss, // <
    leq, // <=

    // Assignment & Punctuation
    becomes,   // :=
    lparent,   // (
    rparent,   // )
    lbrack,    // [
    rbrack,    // ]
    comma,     // ,
    semicolon, // ;
    period,    // .
    colon,     // :

    // Keywords - Declarations
    constsy,     // const
    typesy,      // type
    varsy,       // var
    functionsy,  // function
    proceduresy, // procedure
    arraysy,     // array
    recordsy,    // record
    programsy,   // program

    // Keywords - Control Flow
    beginsy,  // begin
    endsy,    // end
    ifsy,     // if
    thensy,   // then
    elsesy,   // else
    casesy,   // case
    ofsy,     // of
    repeatsy, // repeat
    untilsy,  // until
    whilesy,  // while
    dosy,     // do
    forsy,    // for
    tosy,     // to
    downtosy, // downto

    // Special
    comment,  // { ... } or (* ... *)
    eof_token // End of File (Essential for the Parser!)
};

inline std::string_view tokenTypeName(TokenType type) {
    switch (type) {
    case TokenType::intcon:
        return "intcon";
    case TokenType::realcon:
        return "realcon";
    case TokenType::charcon:
        return "charcon";
    case TokenType::string:
        return "string";
    case TokenType::ident:
        return "ident";
    case TokenType::plus:
        return "plus";
    case TokenType::minus:
        return "minus";
    case TokenType::times:
        return "times";
    case TokenType::rdiv:
        return "rdiv";
    case TokenType::idiv:
        return "idiv";
    case TokenType::imod:
        return "imod";
    case TokenType::notsy:
        return "notsy";
    case TokenType::andsy:
        return "andsy";
    case TokenType::orsy:
        return "orsy";
    case TokenType::eql:
        return "eql";
    case TokenType::neq:
        return "neq";
    case TokenType::gtr:
        return "gtr";
    case TokenType::geq:
        return "geq";
    case TokenType::lss:
        return "lss";
    case TokenType::leq:
        return "leq";
    case TokenType::becomes:
        return "becomes";
    case TokenType::lparent:
        return "lparent";
    case TokenType::rparent:
        return "rparent";
    case TokenType::lbrack:
        return "lbrack";
    case TokenType::rbrack:
        return "rbrack";
    case TokenType::comma:
        return "comma";
    case TokenType::semicolon:
        return "semicolon";
    case TokenType::period:
        return "period";
    case TokenType::colon:
        return "colon";
    case TokenType::constsy:
        return "constsy";
    case TokenType::typesy:
        return "typesy";
    case TokenType::varsy:
        return "varsy";
    case TokenType::functionsy:
        return "functionsy";
    case TokenType::proceduresy:
        return "proceduresy";
    case TokenType::arraysy:
        return "arraysy";
    case TokenType::recordsy:
        return "recordsy";
    case TokenType::programsy:
        return "programsy";
    case TokenType::beginsy:
        return "beginsy";
    case TokenType::endsy:
        return "endsy";
    case TokenType::ifsy:
        return "ifsy";
    case TokenType::thensy:
        return "thensy";
    case TokenType::elsesy:
        return "elsesy";
    case TokenType::casesy:
        return "casesy";
    case TokenType::ofsy:
        return "ofsy";
    case TokenType::repeatsy:
        return "repeatsy";
    case TokenType::untilsy:
        return "untilsy";
    case TokenType::whilesy:
        return "whilesy";
    case TokenType::dosy:
        return "dosy";
    case TokenType::forsy:
        return "forsy";
    case TokenType::tosy:
        return "tosy";
    case TokenType::downtosy:
        return "downtosy";
    case TokenType::comment:
        return "comment";
    case TokenType::eof_token:
        return "eof_token";
    }

    return "unknown";
}
