#pragma once

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
