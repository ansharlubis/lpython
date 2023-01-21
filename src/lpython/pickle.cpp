#include <string>

#include <lpython/pickle.h>
#include <lpython/pickle.h>
#include <lpython/bigint.h>
#include <lpython/python_ast.h>
#include <libasr/asr_utils.h>
#include <libasr/string_utils.h>
#include <libasr/location.h>

namespace LCompilers::LPython {

/* -----------------------------------------------------------------------*/
// ASR

class ASRPickleVisitor :
    public ASR::PickleBaseVisitor<ASRPickleVisitor>
{
public:
    bool show_intrinsic_modules;

    std::string get_str() {
        return s;
    }
    void visit_symbol(const ASR::symbol_t &x) {
        s.append(ASRUtils::symbol_parent_symtab(&x)->get_counter());
        s.append(" ");
        if (use_colors) {
            s.append(color(fg::yellow));
        }
        s.append(ASRUtils::symbol_name(&x));
        if (use_colors) {
            s.append(color(fg::reset));
        }
    }
    void visit_IntegerConstant(const ASR::IntegerConstant_t &x) {
        s.append("(");
        if (use_colors) {
            s.append(color(style::bold));
            s.append(color(fg::magenta));
        }
        s.append("IntegerConstant");
        if (use_colors) {
            s.append(color(fg::reset));
            s.append(color(style::reset));
        }
        s.append(" ");
        if (use_colors) {
            s.append(color(fg::cyan));
        }
        s.append(std::to_string(x.m_n));
        if (use_colors) {
            s.append(color(fg::reset));
        }
        s.append(" ");
        this->visit_ttype(*x.m_type);
        s.append(")");
    }
    void visit_Module(const ASR::Module_t &x) {
        if (!show_intrinsic_modules &&
                    (x.m_intrinsic || startswith(x.m_name, "lfortran_intrinsic_"))) {
            s.append("(");
            if (use_colors) {
                s.append(color(style::bold));
                s.append(color(fg::magenta));
            }
            s.append("IntrinsicModule");
            if (use_colors) {
                s.append(color(fg::reset));
                s.append(color(style::reset));
            }
            s.append(" ");
            s.append(x.m_name);
            s.append(")");
        } else {
            ASR::PickleBaseVisitor<ASRPickleVisitor>::visit_Module(x);
        };
    }
};

std::string pickle(ASR::asr_t &asr, bool colors, bool indent,
        bool show_intrinsic_modules) {
    ASRPickleVisitor v;
    v.use_colors = colors;
    v.indent = indent;
    v.show_intrinsic_modules = show_intrinsic_modules;
    v.visit_asr(asr);
    return v.get_str();
}

std::string pickle(ASR::TranslationUnit_t &asr, bool colors, bool indent, bool show_intrinsic_modules) {
    return pickle((ASR::asr_t &)asr, colors, indent, show_intrinsic_modules);
}

class ASRTreeVisitor :
    public ASR::TreeBaseVisitor<ASRTreeVisitor>
{
public:
    bool show_intrinsic_modules;

    std::string get_str() {
        return s;
    }

};

std::string pickle_tree(ASR::asr_t &asr, bool colors, bool show_intrinsic_modules) {
    ASRTreeVisitor v;
    v.use_colors = colors;
    v.show_intrinsic_modules = show_intrinsic_modules;
    v.visit_asr(asr);
    return v.get_str();
}

std::string pickle_tree(ASR::TranslationUnit_t &asr, bool colors, bool show_intrinsic_modules) {
    return pickle_tree((ASR::asr_t &)asr, colors, show_intrinsic_modules);
}

/********************** AST Pickle Json *******************/
class ASTJsonVisitor :
    public LPython::AST::JsonBaseVisitor<ASTJsonVisitor>
{
public:
    using LPython::AST::JsonBaseVisitor<ASTJsonVisitor>::JsonBaseVisitor;

    std::string get_str() {
        return s;
    }
};

std::string pickle_json(LPython::AST::ast_t &ast, LocationManager &lm) {
    ASTJsonVisitor v(lm);
    v.visit_ast(ast);
    return v.get_str();
}

std::string pickle_json(LPython::AST::Module_t &ast, LocationManager &lm) {
    return pickle_json((LPython::AST::ast_t &)ast, lm);
}

/********************** ASR Pickle Json *******************/
class ASRJsonVisitor :
    public ASR::JsonBaseVisitor<ASRJsonVisitor>
{
public:
    using ASR::JsonBaseVisitor<ASRJsonVisitor>::JsonBaseVisitor;

    std::string get_str() {
        return s;
    }

    void visit_symbol(const ASR::symbol_t &x) {
        s.append("\"");
        s.append(ASRUtils::symbol_name(&x));
        s.append(" (SymbolTable");
        s.append(ASRUtils::symbol_parent_symtab(&x)->get_counter());
        s.append(")\"");
    }

    void visit_Module(const ASR::Module_t &x) {
        s.append("{");
        inc_indent(); s.append("\n" + indtd);
        s.append("\"node\": \"Module\"");
        s.append(",\n" + indtd);
        s.append("\"fields\": {");
        inc_indent(); s.append("\n" + indtd);
        s.append("\"name\": ");
        s.append("\"" + std::string(x.m_name) + "\"");
        s.append(",\n" + indtd);
        s.append("\"dependencies\": ");
        s.append("[");
        if (x.n_dependencies > 0) {
            inc_indent(); s.append("\n" + indtd);
            for (size_t i=0; i<x.n_dependencies; i++) {
                s.append("\"" + std::string(x.m_dependencies[i]) + "\"");
                if (i < x.n_dependencies-1) {
                    s.append(",\n" + indtd);
                };
            }
            dec_indent(); s.append("\n" + indtd);
        }
        s.append("]");
        s.append(",\n" + indtd);
        s.append("\"loaded_from_mod\": ");
        if (x.m_loaded_from_mod) {
            s.append("true");
        } else {
            s.append("false");
        }
        s.append(",\n" + indtd);
        s.append("\"intrinsic\": ");
        if (x.m_intrinsic) {
            s.append("true");
        } else {
            s.append("false");
        }
        dec_indent(); s.append("\n" + indtd);
        s.append("}");
        s.append(",\n" + indtd);
        append_location(s, x.base.base.loc.first, x.base.base.loc.last);
        dec_indent(); s.append("\n" + indtd);
        s.append("}");
    }
};

std::string pickle_json(ASR::asr_t &asr, LocationManager &lm) {
    ASRJsonVisitor v(lm);
    v.visit_asr(asr);
    return v.get_str();
}

std::string pickle_json(ASR::TranslationUnit_t &asr, LocationManager &lm) {
    return pickle_json((ASR::asr_t &)asr, lm);
}

}
