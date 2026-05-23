#include "SemanticAnalyzer.hpp"
#include <algorithm>
#include <cctype>

using namespace TypeSystem;

// Constructor: initialize with symbol table reference
SemanticAnalyzer::SemanticAnalyzer(SymbolTableManager &symTable)
    : symTable(symTable) {}

// Entry point: start semantic analysis by visiting AST root
void SemanticAnalyzer::analyze(std::shared_ptr<ASTNode> ast) {
    if (ast) {
        ast->accept(*this);
    }
}

void SemanticAnalyzer::printErrors() const {
    for (const auto &err : errors) {
        std::cerr << "Semantic error on line " << err.line << ": "
                  << err.message << std::endl;
    }
}

void SemanticAnalyzer::reportError(const std::string &message) {
    errors.emplace_back(currentLine, message);
}

void SemanticAnalyzer::reportError(int line, const std::string &message) {
    errors.emplace_back(line, message);
}

// Helper: verify that a condition expression evaluates to Boolean type
void SemanticAnalyzer::checkConditionType(std::shared_ptr<ASTNode> condition,
                                          const std::string &context) {
    if (!condition)
        return;
    if (condition->type != TYPE_BOOLEAN) {
        reportError(condition->line, context +
                                         " condition must be Boolean, got " +
                                         codeToTypeName(condition->type));
    }
}

int SemanticAnalyzer::resolveTypeName(const std::string &typeName) {
    int idx = symTable.lookup(typeName);
    if (idx != -1) {
        const TabEntry &e = symTable.getTab(idx);
        if (e.obj == OBJ_TYPE) {
            return e.type;
        }
    }
    // Predefined fallback
    return typeNameToCode(typeName);
}

int SemanticAnalyzer::resolveNodeType(std::shared_ptr<ASTNode> typeNode) {
    if (!typeNode)
        return TYPE_UNKNOWN;
    if (auto v = std::dynamic_pointer_cast<VariableNode>(typeNode)) {
        return resolveTypeName(v->name);
    }
    if (auto r = std::dynamic_pointer_cast<RangeNode>(typeNode)) {
        return TYPE_SUBRANGE;
    }
    if (auto a = std::dynamic_pointer_cast<ArrayTypeNode>(typeNode)) {
        return TYPE_ARRAY;
    }
    if (auto rec = std::dynamic_pointer_cast<RecordTypeNode>(typeNode)) {
        return TYPE_RECORD;
    }
    return TYPE_UNKNOWN;
}

bool SemanticAnalyzer::checkTypeCompatibility(int type1, int type2) {
    return TypeSystem::isCompatible(type1, type2);
}

bool SemanticAnalyzer::checkAssignmentCompatibility(int targetType,
                                                    int valueType) {
    return TypeSystem::isAssignmentCompatible(targetType, valueType);
}

// Infer result type of binary operation based on operand types and operator
int SemanticAnalyzer::inferBinaryOpType(const std::string &op, int leftType,
                                        int rightType) {
    // Arithmetic operators: result is Real if either operand is Real
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "div" ||
        op == "mod") {
        if (!isNumeric(leftType) || !isNumeric(rightType)) {
            return TYPE_UNKNOWN;
        }
        if (leftType == TYPE_REAL || rightType == TYPE_REAL)
            return TYPE_REAL;
        return TYPE_INTEGER;
    }
    // Relational operators: always return Boolean
    if (op == "==" || op == "<>" || op == "<" || op == "<=" || op == ">" ||
        op == ">=") {
        if (!isCompatible(leftType, rightType) &&
            !(isNumeric(leftType) && isNumeric(rightType))) {
            return TYPE_UNKNOWN;
        }
        return TYPE_BOOLEAN;
    }
    // Logical operators: require Boolean operands, return Boolean
    if (op == "and" || op == "or") {
        if (leftType != TYPE_BOOLEAN || rightType != TYPE_BOOLEAN) {
            return TYPE_UNKNOWN;
        }
        return TYPE_BOOLEAN;
    }
    return TYPE_UNKNOWN;
}

int SemanticAnalyzer::inferUnaryOpType(const std::string &op, int operandType) {
    if (op == "not") {
        if (operandType != TYPE_BOOLEAN)
            return TYPE_UNKNOWN;
        return TYPE_BOOLEAN;
    }
    if (op == "+" || op == "-") {
        if (!isNumeric(operandType))
            return TYPE_UNKNOWN;
        return operandType;
    }
    return TYPE_UNKNOWN;
}

static int detectLiteralType(const std::string &val) {
    if (val == "true" || val == "false")
        return TYPE_BOOLEAN;
    if (!val.empty() && val.front() == '\'' && val.back() == '\'') {
        if (val.length() == 3)
            return TYPE_CHAR;
        return TYPE_STRING;
    }
    bool hasDot = false, hasExp = false;
    size_t start = (val[0] == '-') ? 1 : 0;
    for (size_t i = start; i < val.size(); ++i) {
        char c = val[i];
        if (c == '.')
            hasDot = true;
        else if (c == 'e' || c == 'E')
            hasExp = true;
        else if (!isdigit(c)) {
            // Not a pure literal - could be an identifier constant reference
            return TYPE_UNKNOWN;
        }
    }
    if (hasDot || hasExp)
        return TYPE_REAL;
    return TYPE_INTEGER;
}

// Visit program node: register program and process declarations + body
void SemanticAnalyzer::visit(ProgramNode &node) {
    currentLine = node.line;
    // Register program name in symbol table
    symTable.enter(node.name, OBJ_TYPE, TYPE_UNKNOWN);

    // Process all declarations in global scope
    for (auto &decl : node.declarations) {
        if (decl)
            decl->accept(*this);
    }

    // Process program body
    if (node.body)
        node.body->accept(*this);
}

// Visit variable declaration: resolve type and register in symbol table
void SemanticAnalyzer::visit(VarDeclNode &node) {
    currentLine = node.line;
    int typeCode = resolveTypeName(node.typeName);
    int ref = 0;

    // Handle composite types (array, record)
    if (node.typeNode) {
        node.typeNode->accept(*this);
        if (auto arr =
                std::dynamic_pointer_cast<ArrayTypeNode>(node.typeNode)) {
            typeCode = TYPE_ARRAY;
            // Register array type in atab with bounds
            int etyp = resolveNodeType(arr->elementType);
            int xtyp = resolveNodeType(arr->indexType);
            int low = 0, high = 0;
            if (auto r = std::dynamic_pointer_cast<RangeNode>(arr->indexType)) {
                if (r->low && r->high) {
                    try {
                        low = std::stoi(
                            std::dynamic_pointer_cast<LiteralNode>(r->low)
                                ->value);
                        high = std::stoi(
                            std::dynamic_pointer_cast<LiteralNode>(r->high)
                                ->value);
                    } catch (...) {
                    }
                }
                if (low > high) {
                    reportError(node.line,
                                "Array lower bound exceeds upper bound");
                }
            }
            int elsz = 1;
            int size = (high - low + 1) * elsz;
            ref = symTable.enterArray(xtyp, etyp, 0, low, high, elsz, size);
        } else if (auto rec = std::dynamic_pointer_cast<RecordTypeNode>(
                       node.typeNode)) {
            typeCode = TYPE_RECORD;
            // Register record fields in a new block
            int recBlock = symTable.enterBlock();
            for (auto &f : rec->fields) {
                if (f)
                    f->accept(*this);
            }
            ref = recBlock;
            symTable.exitBlock();
        } else if (auto r =
                       std::dynamic_pointer_cast<RangeNode>(node.typeNode)) {
            typeCode = TYPE_SUBRANGE;
            // Could validate bounds here
        }
    }

    if (typeCode == TYPE_UNKNOWN) {
        reportError(node.line, "Unknown type '" + node.typeName + "'");
    }

    int idx = symTable.enter(node.name, OBJ_VARIABLE, typeCode, ref);
    if (idx == -1) {
        reportError(node.line, "Redeclared identifier '" + node.name + "'");
    } else {
        node.type = typeCode;
        node.tabIndex = idx;
    }
}

void SemanticAnalyzer::visit(ConstDeclNode &node) {
    currentLine = node.line;
    if (node.value) {
        node.value->accept(*this);
        int valType = node.value->type;
        int idx = symTable.enter(node.name, OBJ_CONSTANT, valType, 0, 1, -1, 0);
        if (idx == -1) {
            reportError(node.line, "Redeclared identifier '" + node.name + "'");
        } else {
            node.type = valType;
            node.tabIndex = idx;
            // Try to store constant value in adr
            if (auto lit = std::dynamic_pointer_cast<LiteralNode>(node.value)) {
                try {
                    symTable.getTab(idx).adr = std::stoi(lit->value);
                } catch (...) {
                    symTable.getTab(idx).adr = 0;
                }
            }
        }
    }
}

void SemanticAnalyzer::visit(TypeDeclNode &node) {
    currentLine = node.line;
    int typeCode = TYPE_UNKNOWN;
    int ref = 0;
    if (node.typeNode) {
        node.typeNode->accept(*this);
        typeCode = resolveNodeType(node.typeNode);
        if (auto arr =
                std::dynamic_pointer_cast<ArrayTypeNode>(node.typeNode)) {
            int etyp = resolveNodeType(arr->elementType);
            int xtyp = resolveNodeType(arr->indexType);
            int low = 0, high = 0;
            if (auto r = std::dynamic_pointer_cast<RangeNode>(arr->indexType)) {
                if (r->low && r->high) {
                    try {
                        low = std::stoi(
                            std::dynamic_pointer_cast<LiteralNode>(r->low)
                                ->value);
                        high = std::stoi(
                            std::dynamic_pointer_cast<LiteralNode>(r->high)
                                ->value);
                    } catch (...) {
                    }
                }
            }
            int elsz = 1;
            int size = std::max(0, (high - low + 1)) * elsz;
            ref = symTable.enterArray(xtyp, etyp, 0, low, high, elsz, size);
        } else if (auto rec = std::dynamic_pointer_cast<RecordTypeNode>(
                       node.typeNode)) {
            int recBlock = symTable.enterBlock();
            for (auto &f : rec->fields) {
                if (f)
                    f->accept(*this);
            }
            ref = recBlock;
            symTable.exitBlock();
        }
    }

    int idx = symTable.enter(node.name, OBJ_TYPE, typeCode, ref);
    if (idx == -1) {
        reportError(node.line, "Redeclared identifier '" + node.name + "'");
    } else {
        node.type = typeCode;
        node.tabIndex = idx;
    }
}

void SemanticAnalyzer::visit(ParamNode &node) {
    currentLine = node.line;
    int typeCode = resolveTypeName(node.typeName);
    int nrm = node.isVar ? 0 : 1;
    int idx = symTable.enter(node.name, OBJ_VARIABLE, typeCode, 0, nrm);
    if (idx == -1) {
        reportError(node.line, "Redeclared parameter '" + node.name + "'");
    } else {
        node.type = typeCode;
        node.tabIndex = idx;
    }
}

void SemanticAnalyzer::visit(ProcDeclNode &node) {
    currentLine = node.line;
    int idx = symTable.enter(node.name, OBJ_PROCEDURE, TYPE_UNKNOWN);
    if (idx == -1) {
        reportError(node.line, "Redeclared identifier '" + node.name + "'");
        return;
    }
    node.tabIndex = idx;

    // Enter new scope
    symTable.enterBlock();

    // Register parameters
    for (auto &p : node.params) {
        if (p)
            p->accept(*this);
    }
    symTable.finalizeParameters();

    // Update procedure's block reference
    symTable.getTab(idx).ref = symTable.currentBlock();

    // Visit local declarations
    for (auto &decl : node.localDeclarations) {
        if (decl)
            decl->accept(*this);
    }

    // Visit body
    if (node.body)
        node.body->accept(*this);

    // Exit scope
    symTable.exitBlock();
}

void SemanticAnalyzer::visit(FuncDeclNode &node) {
    currentLine = node.line;
    int retType = resolveTypeName(node.returnTypeName);
    if (retType == TYPE_UNKNOWN) {
        reportError(node.line,
                    "Unknown return type '" + node.returnTypeName + "'");
    }

    int idx = symTable.enter(node.name, OBJ_FUNCTION, retType);
    if (idx == -1) {
        reportError(node.line, "Redeclared identifier '" + node.name + "'");
        return;
    }
    node.tabIndex = idx;

    // Enter new scope
    symTable.enterBlock();

    // Register parameters
    for (auto &p : node.params) {
        if (p)
            p->accept(*this);
    }
    symTable.finalizeParameters();

    // Update function's block reference
    symTable.getTab(idx).ref = symTable.currentBlock();

    // Visit local declarations
    for (auto &decl : node.localDeclarations) {
        if (decl)
            decl->accept(*this);
    }

    // Visit body
    if (node.body)
        node.body->accept(*this);

    // Exit scope
    symTable.exitBlock();
}

// Visit assignment: check type compatibility between target and value
void SemanticAnalyzer::visit(AssignNode &node) {
    currentLine = node.line;
    if (node.target)
        node.target->accept(*this);
    if (node.value)
        node.value->accept(*this);

    int targetType = node.target ? node.target->type : TYPE_UNKNOWN;
    int valueType = node.value ? node.value->type : TYPE_UNKNOWN;

    // Validate assignment compatibility (e.g., Real := Integer is valid, reverse is not)
    if (!checkAssignmentCompatibility(targetType, valueType)) {
        reportError(node.line, "Type mismatch in assignment: expected " +
                                   codeToTypeName(targetType) + ", got " +
                                   codeToTypeName(valueType));
    }
}

// Visit if statement: validate condition is Boolean, then process branches
void SemanticAnalyzer::visit(IfNode &node) {
    currentLine = node.line;
    if (node.condition) {
        node.condition->accept(*this);
        checkConditionType(node.condition, "If");
    }
    if (node.thenBranch)
        node.thenBranch->accept(*this);
    if (node.elseBranch)
        node.elseBranch->accept(*this);
}

// Visit while loop: validate condition is Boolean, then process body
void SemanticAnalyzer::visit(WhileNode &node) {
    currentLine = node.line;
    if (node.condition) {
        node.condition->accept(*this);
        checkConditionType(node.condition, "While");
    }
    if (node.body)
        node.body->accept(*this);
}

void SemanticAnalyzer::visit(ForNode &node) {
    currentLine = node.line;
    // Lookup loop variable
    int varIdx = symTable.lookup(node.varName);
    int varType = TYPE_UNKNOWN;
    if (varIdx == -1) {
        reportError(node.line,
                    "Undeclared loop variable '" + node.varName + "'");
    } else {
        varType = symTable.getTab(varIdx).type;
    }

    if (node.initExpr) {
        node.initExpr->accept(*this);
        if (varType != TYPE_UNKNOWN &&
            !checkAssignmentCompatibility(varType, node.initExpr->type)) {
            reportError(node.line, "For loop initial value incompatible with "
                                   "loop variable: expected " +
                                       codeToTypeName(varType) + ", got " +
                                       codeToTypeName(node.initExpr->type));
        }
    }

    if (node.finalExpr) {
        node.finalExpr->accept(*this);
        if (varType != TYPE_UNKNOWN &&
            !checkAssignmentCompatibility(varType, node.finalExpr->type)) {
            reportError(node.line, "For loop final value incompatible with "
                                   "loop variable: expected " +
                                       codeToTypeName(varType) + ", got " +
                                       codeToTypeName(node.finalExpr->type));
        }
    }

    if (node.body)
        node.body->accept(*this);
}

void SemanticAnalyzer::visit(RepeatNode &node) {
    currentLine = node.line;
    for (auto &stmt : node.statements) {
        if (stmt)
            stmt->accept(*this);
    }
    if (node.condition) {
        node.condition->accept(*this);
        checkConditionType(node.condition, "Repeat-until");
    }
}

void SemanticAnalyzer::visit(CaseNode &node) {
    currentLine = node.line;
    if (node.expression)
        node.expression->accept(*this);
    int exprType = node.expression ? node.expression->type : TYPE_UNKNOWN;

    for (auto &branch : node.branches) {
        if (branch) {
            // Annotate branch with expression type for case branch checking
            if (auto cb = std::dynamic_pointer_cast<CaseBranchNode>(branch)) {
                for (auto &c : cb->constants) {
                    if (c)
                        c->accept(*this);
                    if (c && c->type != TYPE_UNKNOWN && c->type != exprType) {
                        reportError(c->line,
                                    "Case constant type mismatch: expected " +
                                        codeToTypeName(exprType) + ", got " +
                                        codeToTypeName(c->type));
                    }
                }
            }
            branch->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(CaseBranchNode &node) {
    currentLine = node.line;
    if (node.statement)
        node.statement->accept(*this);
}

void SemanticAnalyzer::visit(CompoundNode &node) {
    currentLine = node.line;
    for (auto &stmt : node.statements) {
        if (stmt)
            stmt->accept(*this);
    }
}

void SemanticAnalyzer::checkProcedureArgs(
    int procIdx, const std::vector<std::shared_ptr<ASTNode>> &args, int line) {
    const TabEntry &proc = symTable.getTab(procIdx);
    int blockIdx = proc.ref;
    // Skip check for predefined procedures (ref == 0 means no block assigned)
    if (blockIdx == 0 && (proc.id == "writeln" || proc.id == "readln" ||
                          proc.id == "write" || proc.id == "read")) {
        return;
    }
    if (blockIdx < 0 || blockIdx >= symTable.btabSize())
        return;

    // Count parameters by following lpar link
    std::vector<int> paramIndices;
    int p = symTable.getBTab(blockIdx).lpar;
    while (p > 0) {
        paramIndices.push_back(p);
        p = symTable.getTab(p).link;
    }
    std::reverse(paramIndices.begin(), paramIndices.end());

    if (args.size() != paramIndices.size()) {
        reportError(line, "Wrong number of arguments: expected " +
                              std::to_string(paramIndices.size()) + ", got " +
                              std::to_string(args.size()));
        return;
    }

    for (size_t i = 0; i < args.size(); ++i) {
        int expectedType = symTable.getTab(paramIndices[i]).type;
        if (!checkAssignmentCompatibility(expectedType, args[i]->type)) {
            reportError(args[i]->line, "Argument " + std::to_string(i + 1) +
                                           " type mismatch: expected " +
                                           codeToTypeName(expectedType) +
                                           ", got " +
                                           codeToTypeName(args[i]->type));
        }
    }
}

void SemanticAnalyzer::visit(ProcCallNode &node) {
    currentLine = node.line;
    int idx = symTable.lookup(node.name);
    if (idx == -1) {
        reportError(node.line, "Undeclared identifier '" + node.name + "'");
        return;
    }

    const TabEntry &entry = symTable.getTab(idx);
    if (entry.obj != OBJ_PROCEDURE && entry.obj != OBJ_FUNCTION) {
        reportError(node.line, "'" + node.name +
                                   "' is not a callable procedure or function");
    }

    node.tabIndex = idx;
    if (entry.obj == OBJ_FUNCTION) {
        node.type = entry.type;
    }

    // Visit arguments
    for (auto &arg : node.arguments) {
        if (arg)
            arg->accept(*this);
    }

    if (entry.obj == OBJ_PROCEDURE || entry.obj == OBJ_FUNCTION) {
        checkProcedureArgs(idx, node.arguments, node.line);
    }
}

// Visit binary operation: validate operand types and infer result type
void SemanticAnalyzer::visit(BinaryOpNode &node) {
    currentLine = node.line;
    if (node.left)
        node.left->accept(*this);
    if (node.right)
        node.right->accept(*this);

    int leftType = node.left ? node.left->type : TYPE_UNKNOWN;
    int rightType = node.right ? node.right->type : TYPE_UNKNOWN;

    // Validate operator-operand type compatibility and set result type
    if (node.op == "+" || node.op == "-" || node.op == "*" || node.op == "/" ||
        node.op == "div" || node.op == "mod") {
        if (!isNumeric(leftType) || !isNumeric(rightType)) {
            reportError(node.line, "Arithmetic operator '" + node.op +
                                       "' requires numeric operands, got " +
                                       codeToTypeName(leftType) + " and " +
                                       codeToTypeName(rightType));
        }
        node.type = (leftType == TYPE_REAL || rightType == TYPE_REAL)
                        ? TYPE_REAL
                        : TYPE_INTEGER;
    } else if (node.op == "==" || node.op == "<>" || node.op == "<" ||
               node.op == "<=" || node.op == ">" || node.op == ">=") {
        if (!isCompatible(leftType, rightType) &&
            !(isNumeric(leftType) && isNumeric(rightType))) {
            reportError(node.line, "Incompatible types in comparison: " +
                                       codeToTypeName(leftType) + " and " +
                                       codeToTypeName(rightType));
        }
        node.type = TYPE_BOOLEAN;
    } else if (node.op == "and" || node.op == "or") {
        if (leftType != TYPE_BOOLEAN || rightType != TYPE_BOOLEAN) {
            reportError(node.line, "Logical operator '" + node.op +
                                       "' requires Boolean operands, got " +
                                       codeToTypeName(leftType) + " and " +
                                       codeToTypeName(rightType));
        }
        node.type = TYPE_BOOLEAN;
    } else {
        reportError(node.line, "Unknown binary operator '" + node.op + "'");
        node.type = TYPE_UNKNOWN;
    }
}

void SemanticAnalyzer::visit(UnaryOpNode &node) {
    currentLine = node.line;
    if (node.operand)
        node.operand->accept(*this);

    int operandType = node.operand ? node.operand->type : TYPE_UNKNOWN;

    if (node.op == "not") {
        if (operandType != TYPE_BOOLEAN) {
            reportError(node.line, "'not' requires Boolean operand, got " +
                                       codeToTypeName(operandType));
        }
        node.type = TYPE_BOOLEAN;
    } else if (node.op == "+" || node.op == "-") {
        if (!isNumeric(operandType)) {
            reportError(node.line, "Unary '" + node.op +
                                       "' requires numeric operand, got " +
                                       codeToTypeName(operandType));
        }
        node.type = operandType;
    } else {
        reportError(node.line, "Unknown unary operator '" + node.op + "'");
        node.type = TYPE_UNKNOWN;
    }
}

void SemanticAnalyzer::visit(VariableNode &node) {
    currentLine = node.line;
    int idx = symTable.lookup(node.name);
    if (idx == -1) {
        reportError(node.line, "Undeclared identifier '" + node.name + "'");
        node.type = TYPE_UNKNOWN;
        return;
    }

    const TabEntry &entry = symTable.getTab(idx);
    node.type = entry.type;
    node.tabIndex = idx;
}

void SemanticAnalyzer::visit(LiteralNode &node) {
    currentLine = node.line;
    node.type = detectLiteralType(node.value);

    // If not a literal, maybe it's a constant identifier reference
    if (node.type == TYPE_UNKNOWN) {
        int idx = symTable.lookup(node.value);
        if (idx != -1) {
            const TabEntry &entry = symTable.getTab(idx);
            if (entry.obj == OBJ_CONSTANT) {
                node.type = entry.type;
                node.tabIndex = idx;
            }
        }
    }
}

void SemanticAnalyzer::visit(ArrayAccessNode &node) {
    currentLine = node.line;
    if (node.arrayExpr)
        node.arrayExpr->accept(*this);

    int arrType = node.arrayExpr ? node.arrayExpr->type : TYPE_UNKNOWN;
    if (arrType != TYPE_ARRAY) {
        reportError(node.line, "Array access on non-array type " +
                                   codeToTypeName(arrType));
        node.type = TYPE_UNKNOWN;
        return;
    }

    int arrTabIdx = node.arrayExpr ? node.arrayExpr->tabIndex : -1;
    if (arrTabIdx != -1) {
        int ref = symTable.getTab(arrTabIdx).ref;
        if (ref >= 0 && ref < symTable.atabSize()) {
            const ATabEntry &ate = symTable.getATab(ref);
            node.type = ate.etyp;
            // Check index types
            for (auto &idx : node.indices) {
                if (idx) {
                    idx->accept(*this);
                    if (!isAssignmentCompatible(ate.xtyp, idx->type) &&
                        idx->type != TYPE_INTEGER &&
                        idx->type != TYPE_SUBRANGE) {
                        reportError(idx->line,
                                    "Array index type mismatch: expected " +
                                        codeToTypeName(ate.xtyp) + ", got " +
                                        codeToTypeName(idx->type));
                    }
                }
            }
            return;
        }
    }
    node.type = TYPE_UNKNOWN;
}

void SemanticAnalyzer::visit(FieldAccessNode &node) {
    currentLine = node.line;
    if (node.recordExpr)
        node.recordExpr->accept(*this);

    int recType = node.recordExpr ? node.recordExpr->type : TYPE_UNKNOWN;
    if (recType != TYPE_RECORD) {
        reportError(node.line, "Field access on non-record type " +
                                   codeToTypeName(recType));
        node.type = TYPE_UNKNOWN;
        return;
    }

    int recTabIdx = node.recordExpr ? node.recordExpr->tabIndex : -1;
    if (recTabIdx != -1) {
        int ref = symTable.getTab(recTabIdx).ref;
        if (ref >= 0 && ref < symTable.btabSize()) {
            // Search field in record's block
            int fidx = symTable.lookupLocal(node.fieldName, ref);
            if (fidx != -1) {
                node.type = symTable.getTab(fidx).type;
                node.tabIndex = fidx;
                return;
            }
        }
    }

    reportError(node.line, "Unknown field '" + node.fieldName + "'");
    node.type = TYPE_UNKNOWN;
}

void SemanticAnalyzer::visit(RangeNode &node) {
    currentLine = node.line;
    if (node.low)
        node.low->accept(*this);
    if (node.high)
        node.high->accept(*this);

    int lowType = node.low ? node.low->type : TYPE_UNKNOWN;
    int highType = node.high ? node.high->type : TYPE_UNKNOWN;

    if (lowType == TYPE_UNKNOWN || highType == TYPE_UNKNOWN) {
        reportError(node.line, "Range bounds must be constant");
        return;
    }

    if (lowType != highType && !(isNumeric(lowType) && isNumeric(highType))) {
        reportError(node.line, "Range bounds must have same type, got " +
                                   codeToTypeName(lowType) + " and " +
                                   codeToTypeName(highType));
    }

    if (lowType == TYPE_REAL || highType == TYPE_REAL) {
        reportError(node.line, "Subrange cannot have type Real");
    }

    // Check low <= high for integer constants
    if (auto litLow = std::dynamic_pointer_cast<LiteralNode>(node.low)) {
        if (auto litHigh = std::dynamic_pointer_cast<LiteralNode>(node.high)) {
            try {
                int l = std::stoi(litLow->value);
                int h = std::stoi(litHigh->value);
                if (l > h) {
                    reportError(node.line, "Range lower bound (" +
                                               litLow->value +
                                               ") exceeds upper bound (" +
                                               litHigh->value + ")");
                }
            } catch (...) {
            }
        }
    }

    node.type = TYPE_SUBRANGE;
}

void SemanticAnalyzer::visit(ArrayTypeNode &node) {
    currentLine = node.line;
    if (node.indexType)
        node.indexType->accept(*this);
    if (node.elementType)
        node.elementType->accept(*this);

    int idxType = node.indexType ? node.indexType->type : TYPE_UNKNOWN;
    if (idxType == TYPE_REAL) {
        reportError(node.line, "Array index type cannot be Real");
    }
}

void SemanticAnalyzer::visit(RecordTypeNode &node) {
    currentLine = node.line;
    for (auto &f : node.fields) {
        if (f)
            f->accept(*this);
    }
    node.type = TYPE_RECORD;
}
