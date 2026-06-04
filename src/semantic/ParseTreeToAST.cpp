#include "ParseTreeToAST.hpp"
#include <functional>
#include <iostream>

static int getLine(std::shared_ptr<ParseTreeNode> node) {
    if (!node)
        return 0;
    if (node->line > 0)
        return node->line;
    for (auto &c : node->children) {
        int l = getLine(c);
        if (l > 0)
            return l;
    }
    return 0;
}

std::string ParseTreeToAST::extractIdent(const std::string &label) {
    size_t start = label.find('(');
    size_t end = label.find(')');
    if (start != std::string::npos && end != std::string::npos &&
        end > start + 1) {
        return label.substr(start + 1, end - start - 1);
    }
    return label;
}

std::string ParseTreeToAST::extractLiteralValue(const std::string &label) {
    size_t start = label.find('(');
    size_t end = label.rfind(')');
    if (start != std::string::npos && end != std::string::npos &&
        end > start + 1) {
        return label.substr(start + 1, end - start - 1);
    }
    return label;
}

std::string ParseTreeToAST::tokenLabelToOp(const std::string &label) {
    if (label == "plus")
        return "+";
    if (label == "minus")
        return "-";
    if (label == "times")
        return "*";
    if (label == "rdiv")
        return "/";
    if (label == "idiv")
        return "div";
    if (label == "imod")
        return "mod";
    if (label == "eql")
        return "==";
    if (label == "neq")
        return "<>";
    if (label == "gtr")
        return ">";
    if (label == "geq")
        return ">=";
    if (label == "lss")
        return "<";
    if (label == "leq")
        return "<=";
    if (label == "andsy")
        return "and";
    if (label == "orsy")
        return "or";
    if (label == "notsy")
        return "not";
    return label;
}

bool ParseTreeToAST::isTerminalOperator(const std::string &label) {
    return label == "plus" || label == "minus" || label == "times" ||
           label == "rdiv" || label == "idiv" || label == "imod" ||
           label == "eql" || label == "neq" || label == "gtr" ||
           label == "geq" || label == "lss" || label == "leq" ||
           label == "andsy" || label == "orsy" || label == "notsy";
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convert(std::shared_ptr<ParseTreeNode> root) {
    if (!root)
        return nullptr;
    if (root->label == "<program>") {
        return convertProgram(root);
    }
    return nullptr;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertProgram(std::shared_ptr<ParseTreeNode> node) {
    std::string progName = "program";
    std::shared_ptr<ASTNode> body = nullptr;
    std::vector<std::shared_ptr<ASTNode>> decls;

    for (auto &child : node->children) {
        if (child->label == "<program-header>") {
            for (auto &c : child->children) {
                if (c->label.find("ident(") == 0) {
                    progName = extractIdent(c->label);
                }
            }
        } else if (child->label == "<declaration-part>") {
            decls = convertDeclarationPart(child);
        } else if (child->label == "<compound-statement>") {
            body = convertStatementList(child->children[1]); // statement-list
        }
    }

    auto program = std::make_shared<ProgramNode>(progName);
    program->declarations = decls;
    program->body = body;
    program->line = getLine(node);
    return program;
}

std::vector<std::shared_ptr<ASTNode>>
ParseTreeToAST::convertDeclarationPart(std::shared_ptr<ParseTreeNode> node) {
    std::vector<std::shared_ptr<ASTNode>> decls;
    for (auto &child : node->children) {
        if (child->label == "<const-declaration>") {
            auto v = convertConstDecl(child);
            decls.insert(decls.end(), v.begin(), v.end());
        } else if (child->label == "<var-declaration>") {
            auto v = convertVarDecl(child);
            decls.insert(decls.end(), v.begin(), v.end());
        } else if (child->label == "<type-declaration>") {
            auto v = convertTypeDecl(child);
            decls.insert(decls.end(), v.begin(), v.end());
        } else if (child->label == "<procedure-declaration>" ||
                   child->label == "<function-declaration>") {
            decls.push_back(convertSubprogramDecl(child));
        }
    }
    return decls;
}

std::vector<std::shared_ptr<ASTNode>>
ParseTreeToAST::convertConstDecl(std::shared_ptr<ParseTreeNode> node) {
    std::vector<std::shared_ptr<ASTNode>> decls;
    for (size_t i = 1; i < node->children.size(); i += 4) {
        if (i + 2 >= node->children.size())
            break;
        std::string name = extractIdent(node->children[i]->label);
        auto val = convertConstant(node->children[i + 2]);
        auto decl = std::make_shared<ConstDeclNode>(name, val);
        decl->line = getLine(node->children[i]);
        decls.push_back(decl);
    }
    return decls;
}

std::vector<std::shared_ptr<ASTNode>>
ParseTreeToAST::convertVarDecl(std::shared_ptr<ParseTreeNode> node) {
    std::vector<std::shared_ptr<ASTNode>> decls;
    for (size_t i = 0; i < node->children.size();) {
        if (node->children[i]->label == "<identifier-list>") {
            auto idents = convertIdentifierList(node->children[i]);
            std::shared_ptr<ASTNode> typeNode = nullptr;
            std::string typeName = "unknown";
            if (i + 2 < node->children.size() &&
                node->children[i + 2]->label == "<type>") {
                typeNode = convertType(node->children[i + 2]);
                // Extract type name for simple types
                if (node->children[i + 2]->children.size() == 1) {
                    auto c = node->children[i + 2]->children[0];
                    if (c->label.find("ident(") == 0) {
                        typeName = extractIdent(c->label);
                    } else if (c->label == "<array-type>") {
                        typeName = "array";
                    } else if (c->label == "<record-type>") {
                        typeName = "record";
                    }
                }
            }
            for (const auto &name : idents) {
                auto decl = std::make_shared<VarDeclNode>(name, typeName);
                decl->typeNode = typeNode;
                decl->line = getLine(node->children[i]);
                decls.push_back(decl);
            }
            i += 4;
        } else {
            i++;
        }
    }
    return decls;
}

std::vector<std::shared_ptr<ASTNode>>
ParseTreeToAST::convertTypeDecl(std::shared_ptr<ParseTreeNode> node) {
    std::vector<std::shared_ptr<ASTNode>> decls;
    for (size_t i = 1; i < node->children.size(); i += 4) {
        if (i + 2 >= node->children.size())
            break;
        std::string name = extractIdent(node->children[i]->label);
        auto typeNode = convertType(node->children[i + 2]);
        auto decl = std::make_shared<TypeDeclNode>(name, typeNode);
        decl->line = getLine(node->children[i]);
        decls.push_back(decl);
    }
    return decls;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertSubprogramDecl(std::shared_ptr<ParseTreeNode> node) {
    bool isFunc = (node->label == "<function-declaration>");
    std::string name;
    std::string returnType;
    std::vector<std::shared_ptr<ParamNode>> params;
    std::shared_ptr<ASTNode> body = nullptr;
    std::vector<std::shared_ptr<ASTNode>> localDecls;

    for (size_t i = 0; i < node->children.size(); ++i) {
        auto &child = node->children[i];
        if (child->label.find("ident(") == 0) {
            if (name.empty()) {
                name = extractIdent(child->label);
            } else if (isFunc && returnType.empty()) {
                returnType = extractIdent(child->label);
            }
        } else if (child->label == "<formal-parameter-list>") {
            params = convertFormalParams(child);
        } else if (child->label == "<block>") {
            for (auto &bc : child->children) {
                if (bc->label == "<declaration-part>") {
                    localDecls = convertDeclarationPart(bc);
                } else if (bc->label == "<compound-statement>") {
                    body = convertStatementList(bc->children[1]);
                }
            }
        }
    }

    if (isFunc) {
        auto func = std::make_shared<FuncDeclNode>(name, returnType);
        func->params = params;
        func->localDeclarations = localDecls;
        func->body = body;
        func->line = getLine(node);
        return func;
    } else {
        auto proc = std::make_shared<ProcDeclNode>(name);
        proc->params = params;
        proc->localDeclarations = localDecls;
        proc->body = body;
        proc->line = getLine(node);
        return proc;
    }
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertBlock(std::shared_ptr<ParseTreeNode> node) {
    // Used for main body only; subprogram blocks handled above
    for (auto &child : node->children) {
        if (child->label == "<compound-statement>") {
            return convertStatementList(child->children[1]);
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertType(std::shared_ptr<ParseTreeNode> node) {
    if (node->children.empty())
        return nullptr;
    auto &child = node->children[0];
    if (child->label.find("ident(") == 0) {
        auto n = std::make_shared<VariableNode>(extractIdent(child->label));
        n->line = child->line;
        return n;
    } else if (child->label == "<array-type>") {
        return convertArrayType(child);
    } else if (child->label == "<record-type>") {
        return convertRecordType(child);
    } else if (child->label == "<range>") {
        return convertRange(child);
    } else if (child->label == "<enumerated>") {
        std::vector<std::shared_ptr<ASTNode>> fields;
        for (auto &c : child->children) {
            if (c->label.find("ident(") == 0) {
                fields.push_back(
                    std::make_shared<LiteralNode>(extractIdent(c->label)));
            }
        }
        auto rec = std::make_shared<RecordTypeNode>(fields);
        rec->line = getLine(child);
        return rec;
    }
    return nullptr;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertArrayType(std::shared_ptr<ParseTreeNode> node) {
    std::shared_ptr<ASTNode> indexType = nullptr;
    std::shared_ptr<ASTNode> elemType = nullptr;
    for (auto &child : node->children) {
        if (child->label == "<range>") {
            indexType = convertRange(child);
        } else if (child->label.find("ident(") == 0) {
            indexType =
                std::make_shared<VariableNode>(extractIdent(child->label));
            indexType->line = child->line;
        } else if (child->label == "<type>") {
            elemType = convertType(child);
        }
    }
    auto arr = std::make_shared<ArrayTypeNode>(indexType, elemType);
    arr->line = getLine(node);
    return arr;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertRecordType(std::shared_ptr<ParseTreeNode> node) {
    std::vector<std::shared_ptr<ASTNode>> fields;
    for (auto &child : node->children) {
        if (child->label == "<field-list>") {
            for (auto &fp : child->children) {
                if (fp->label == "<field-part>") {
                    auto v = convertVarDecl(fp);
                    fields.insert(fields.end(), v.begin(), v.end());
                }
            }
        }
    }
    auto rec = std::make_shared<RecordTypeNode>(fields);
    rec->line = getLine(node);
    return rec;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertRange(std::shared_ptr<ParseTreeNode> node) {
    auto low = convertConstant(node->children[0]);
    auto high = convertConstant(node->children[3]);
    auto r = std::make_shared<RangeNode>(low, high);
    r->line = getLine(node);
    return r;
}

std::vector<std::shared_ptr<ParamNode>>
ParseTreeToAST::convertFormalParams(std::shared_ptr<ParseTreeNode> node) {
    std::vector<std::shared_ptr<ParamNode>> params;
    for (auto &child : node->children) {
        if (child->label == "<parameter-group>") {
            auto idents = convertIdentifierList(child->children[0]);
            std::string typeName = "unknown";
            for (auto &c : child->children) {
                if (c->label.find("ident(") == 0) {
                    typeName = extractIdent(c->label);
                } else if (c->label == "<array-type>") {
                    typeName = "array";
                }
            }
            for (const auto &name : idents) {
                params.push_back(std::make_shared<ParamNode>(name, typeName));
            }
        }
    }
    return params;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertStatement(std::shared_ptr<ParseTreeNode> node) {
    if (!node) return nullptr;
    if (node->label == "<compound-statement>") {
        return convertStatementList(node->children[1]);
    }
    if (node->children.empty()) {
        return nullptr;
    }
    auto &child = node->children[0];
    if (child->label == "<assignment-statement>") {
        auto target = convertVariable(child->children[0]);
        auto expr = convertExpression(child->children[2]);
        auto n = std::make_shared<AssignNode>(target, expr);
        n->line = getLine(child);
        return n;
    } else if (child->label == "<if-statement>") {
        auto cond = convertExpression(child->children[1]);
        auto thenStmt = convertStatement(child->children[3]);
        std::shared_ptr<ASTNode> elseStmt = nullptr;
        for (size_t i = 4; i + 1 < child->children.size(); ++i) {
            if (child->children[i]->label.find("elsesy") == 0 ||
                child->children[i]->label == "else") {
                elseStmt = convertStatement(child->children[i + 1]);
                break;
            }
        }
        auto n = std::make_shared<IfNode>(cond, thenStmt, elseStmt);
        n->line = getLine(child);
        return n;
    } else if (child->label == "<while-statement>") {
        auto cond = convertExpression(child->children[1]);
        auto body = convertStatement(child->children[3]);
        auto n = std::make_shared<WhileNode>(cond, body);
        n->line = getLine(child);
        return n;
    } else if (child->label == "<for-statement>") {
        std::string varName = extractIdent(child->children[1]->label);
        auto init = convertExpression(child->children[3]);
        std::string dir = "to";
        if (child->children[4]->label.find("tosy") == 0 ||
            child->children[4]->label == "to") {
            dir = "to";
        } else {
            dir = "downto";
        }
        auto fin = convertExpression(child->children[5]);
        auto body = convertStatement(child->children[7]);
        auto n = std::make_shared<ForNode>(varName, init, dir, fin, body);
        n->line = getLine(child);
        return n;
    } else if (child->label == "<repeat-statement>") {
        auto stmts = convertStatementList(child->children[1]);
        auto cond = convertExpression(child->children[3]);
        auto compound = std::dynamic_pointer_cast<CompoundNode>(stmts);
        std::vector<std::shared_ptr<ASTNode>> stmtVec;
        if (compound)
            stmtVec = compound->statements;
        auto n = std::make_shared<RepeatNode>(stmtVec, cond);
        n->line = getLine(child);
        return n;
    } else if (child->label == "<case-statement>") {
        auto expr = convertExpression(child->children[1]);
        std::vector<std::shared_ptr<ASTNode>> branches;
        // parseCaseBlock() is recursive: subsequent branches are nested <case-block>
        // children of the first <case-block>. Flatten using a queue.
        if (child->children.size() > 3 &&
                child->children[3]->label == "<case-block>") {
            std::vector<std::shared_ptr<ParseTreeNode>> blockQueue;
            blockQueue.push_back(child->children[3]);
            while (!blockQueue.empty()) {
                auto cbNode = blockQueue.front();
                blockQueue.erase(blockQueue.begin());
                std::vector<std::shared_ptr<ASTNode>> consts;
                std::shared_ptr<ASTNode> stmt = nullptr;
                for (size_t j = 0; j < cbNode->children.size(); ++j) {
                    auto& c = cbNode->children[j];
                    if (c->label == "<constant>") {
                        consts.push_back(convertConstant(c));
                    } else if (c->label.find("colon") == 0 || c->label == ":") {
                        if (j + 1 < cbNode->children.size()) {
                            stmt = convertStatement(cbNode->children[j + 1]);
                        }
                    } else if (c->label == "<case-block>") {
                        blockQueue.push_back(c); // nested branch → process next
                    }
                }
                if (!consts.empty()) {
                    auto branch = std::make_shared<CaseBranchNode>(consts, stmt);
                    branch->line = getLine(cbNode);
                    branches.push_back(branch);
                }
            }
        }
        auto n = std::make_shared<CaseNode>(expr, branches);
        n->line = getLine(child);
        return n;
    } else if (child->label == "<case-block>") {
        std::vector<std::shared_ptr<ASTNode>> consts;
        for (size_t i = 0; i < child->children.size(); ++i) {
            if (child->children[i]->label == "<constant>") {
                consts.push_back(convertConstant(child->children[i]));
            }
        }
        std::shared_ptr<ASTNode> stmt = nullptr;
        for (size_t i = 0; i + 1 < child->children.size(); ++i) {
            if (child->children[i]->label.find("colon") == 0 ||
                child->children[i]->label == ":") {
                stmt = convertStatement(child->children[i + 1]);
                break;
            }
        }
        auto n = std::make_shared<CaseBranchNode>(consts, stmt);
        n->line = getLine(child);
        return n;
    } else if (child->label == "<compound-statement>") {
        return convertStatementList(child->children[1]);
    } else if (child->label == "<procedure/function-call>") {
        std::string name = extractIdent(child->children[0]->label);
        std::vector<std::shared_ptr<ASTNode>> args;
        for (size_t i = 1; i < child->children.size(); ++i) {
            if (child->children[i]->label == "<parameter-list>") {
                for (auto &argNode : child->children[i]->children) {
                    if (argNode->label == "<expression>") {
                        args.push_back(convertExpression(argNode));
                    }
                }
            }
        }
        auto n = std::make_shared<ProcCallNode>(name, args);
        n->line = getLine(child);
        return n;
    }
    return nullptr;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertStatementList(std::shared_ptr<ParseTreeNode> node) {
    std::vector<std::shared_ptr<ASTNode>> stmts;
    for (auto &child : node->children) {
        if (child->label == "<statement>") {
            auto s = convertStatement(child);
            if (s)
                stmts.push_back(s);
        }
    }
    auto n = std::make_shared<CompoundNode>(stmts);
    n->line = getLine(node);
    return n;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertExpression(std::shared_ptr<ParseTreeNode> node) {
    if (node->children.size() == 1) {
        return convertSimpleExpression(node->children[0]);
    }
    auto left = convertSimpleExpression(node->children[0]);
    std::string op = tokenLabelToOp(node->children[1]->label);
    auto right = convertSimpleExpression(node->children[2]);
    auto n = std::make_shared<BinaryOpNode>(op, left, right);
    n->line = getLine(node);
    return n;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertSimpleExpression(std::shared_ptr<ParseTreeNode> node) {
    size_t i = 0;
    std::string unaryOp;
    if (i < node->children.size() && (node->children[i]->label == "plus" ||
                                      node->children[i]->label == "minus")) {
        unaryOp = tokenLabelToOp(node->children[i]->label);
        i++;
    }

    if (i >= node->children.size())
        return nullptr;

    std::shared_ptr<ASTNode> result = convertTerm(node->children[i]);
    i++;

    if (!unaryOp.empty() && result) {
        result = std::make_shared<UnaryOpNode>(unaryOp, result);
        result->line = node->children[0]->line;
    }

    while (i + 1 < node->children.size()) {
        std::string op = tokenLabelToOp(node->children[i]->label);
        auto right = convertTerm(node->children[i + 1]);
        result = std::make_shared<BinaryOpNode>(op, result, right);
        result->line = node->children[i]->line;
        i += 2;
    }

    return result;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertTerm(std::shared_ptr<ParseTreeNode> node) {
    if (node->children.empty())
        return nullptr;
    std::shared_ptr<ASTNode> result = convertFactor(node->children[0]);
    size_t i = 1;
    while (i + 1 < node->children.size()) {
        std::string op = tokenLabelToOp(node->children[i]->label);
        auto right = convertFactor(node->children[i + 1]);
        result = std::make_shared<BinaryOpNode>(op, result, right);
        result->line = node->children[i]->line;
        i += 2;
    }
    return result;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertFactor(std::shared_ptr<ParseTreeNode> node) {
    if (node->children.empty())
        return nullptr;
    auto &child = node->children[0];

    if (child->label.find("intcon(") == 0 ||
        child->label.find("realcon(") == 0 ||
        child->label.find("charcon(") == 0 ||
        child->label.find("string(") == 0) {
        auto n =
            std::make_shared<LiteralNode>(extractLiteralValue(child->label));
        n->line = child->line;
        return n;
    } else if (child->label == "<procedure/function-call>") {
        std::string name = extractIdent(child->children[0]->label);
        std::vector<std::shared_ptr<ASTNode>> args;
        for (size_t i = 1; i < child->children.size(); ++i) {
            if (child->children[i]->label == "<parameter-list>") {
                for (auto &argNode : child->children[i]->children) {
                    if (argNode->label == "<expression>") {
                        args.push_back(convertExpression(argNode));
                    }
                }
            }
        }
        auto n = std::make_shared<ProcCallNode>(name, args);
        n->line = getLine(child);
        return n;
    } else if (child->label == "<variable>") {
        return convertVariable(child);
    } else if (child->label == "<expression>") {
        return convertExpression(child);
    } else if (child->label.find("notsy") == 0 || child->label == "not") {
        if (node->children.size() > 1) {
            auto operand = convertFactor(node->children[1]);
            auto n = std::make_shared<UnaryOpNode>("not", operand);
            n->line = child->line;
            return n;
        }
    } else if (child->label == "lparent" || child->label == "(") {
        // Parenthesized expression
        if (node->children.size() > 1) {
            return convertExpression(node->children[1]);
        }
    }

    return nullptr;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertVariable(std::shared_ptr<ParseTreeNode> node) {
    if (node->children.empty())
        return nullptr;
    std::string name = extractIdent(node->children[0]->label);
    std::shared_ptr<ASTNode> result = std::make_shared<VariableNode>(name);
    result->line = node->children[0]->line;

    for (size_t i = 1; i < node->children.size(); ++i) {
        if (node->children[i]->label == "<component-variable>") {
            auto &cv = node->children[i];
            if (cv->children[0]->label == "lbrack" ||
                cv->children[0]->label == "[") {
                std::vector<std::shared_ptr<ASTNode>> indices;
                for (size_t j = 1; j < cv->children.size(); ++j) {
                    if (cv->children[j]->label == "<index-list>") {
                        for (auto &idxNode : cv->children[j]->children) {
                            if (idxNode->label.find("intcon(") == 0 ||
                                idxNode->label.find("charcon(") == 0) {
                                auto lit = std::make_shared<LiteralNode>(
                                    extractLiteralValue(idxNode->label));
                                lit->line = idxNode->line;
                                indices.push_back(lit);
                            } else if (idxNode->label.find("ident(") == 0) {
                                auto var = std::make_shared<VariableNode>(
                                    extractIdent(idxNode->label));
                                var->line = idxNode->line;
                                indices.push_back(var);
                            }
                        }
                    }
                }
                result = std::make_shared<ArrayAccessNode>(result, indices);
                result->line = getLine(cv);
            } else if (cv->children[0]->label == "period" ||
                       cv->children[0]->label == ".") {
                std::string fieldName;
                for (size_t j = 1; j < cv->children.size(); ++j) {
                    if (cv->children[j]->label.find("ident(") == 0) {
                        fieldName = extractIdent(cv->children[j]->label);
                    }
                }
                result = std::make_shared<FieldAccessNode>(result, fieldName);
                result->line = getLine(cv);
            }
        }
    }

    return result;
}

std::shared_ptr<ASTNode>
ParseTreeToAST::convertConstant(std::shared_ptr<ParseTreeNode> node) {
    if (node->children.empty())
        return nullptr;
    std::string val;
    std::string sign;
    for (auto &child : node->children) {
        if (child->label.find("charcon(") == 0 ||
            child->label.find("string(") == 0) {
            val = extractLiteralValue(child->label);
        } else if (child->label.find("intcon(") == 0 ||
                   child->label.find("realcon(") == 0) {
            val = extractLiteralValue(child->label);
        } else if (child->label.find("ident(") == 0) {
            val = extractIdent(child->label);
        } else if (child->label == "plus" || child->label == "+") {
            sign = "";
        } else if (child->label == "minus" || child->label == "-") {
            sign = "-";
        }
    }
    auto n = std::make_shared<LiteralNode>(sign + val);
    n->line = getLine(node);
    return n;
}

std::vector<std::string>
ParseTreeToAST::convertIdentifierList(std::shared_ptr<ParseTreeNode> node) {
    std::vector<std::string> idents;
    for (auto &child : node->children) {
        if (child->label.find("ident(") == 0) {
            idents.push_back(extractIdent(child->label));
        }
    }
    return idents;
}
