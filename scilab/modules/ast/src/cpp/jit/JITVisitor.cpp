/*
 *  Scilab ( http://www.scilab.org/ ) - This file is part of Scilab
 *  Copyright (C) 2014 - Scilab Enterprises - Calixte DENIZET
 *
 *  This file must be used under the terms of the CeCILL.
 *  This source file is licensed as described in the file COPYING, which
 *  you should have received as part of this distribution.  The terms
 *  are also available at
 *  http://www.cecill.info/licences/Licence_CeCILL_V2-en.txt
 *
 */

#include "JITValues.hxx"
#include "JITVisitor.hxx"
#include "jit_operations.hxx"

namespace jit
{
const bool JITVisitor::__init__ = InitializeLLVM();
llvm::Value * const JITVisitor::ONE = llvm::ConstantInt::get(getLLVMTy<int>(), int(1));
llvm::Value * const JITVisitor::TWO = llvm::ConstantInt::get(getLLVMTy<int>(), int(2));
llvm::Value * const JITVisitor::THREE = llvm::ConstantInt::get(getLLVMTy<int>(), int(3));
llvm::Value * const JITVisitor::FOUR = llvm::ConstantInt::get(getLLVMTy<int>(), int(4));

JITVisitor::JITVisitor(const analysis::AnalysisVisitor & _analysis) : ast::ConstVisitor(),
    analysis(_analysis),
    context(llvm::getGlobalContext()),
    module("JIT module", context),
    engine(InitializeEngine(&module)),
    FPM(initFPM(&module, engine)),
    function(llvm::cast<llvm::Function>(module.getOrInsertFunction("jit_main", getLLVMTy<void>(context), nullptr))),
    builder(context),
    uintptrType(getPtrAsIntTy(module, context)),
    _result(nullptr)
{
    module.setDataLayout(engine->getDataLayout()->getStringRepresentation());
    llvm::BasicBlock * BB = llvm::BasicBlock::Create(context, "EntryBlock", function);
    builder.SetInsertPoint(BB);

    symbol::Context * ctxt = symbol::Context::getInstance();
    const std::set<symbol::Symbol> & SymRead = analysis.get_read();
    for (std::set<symbol::Symbol>::const_iterator it = SymRead.cbegin(), end = SymRead.cend(); it != end; ++it)
    {
        const std::wstring & name = it->name_get();
        const std::string _name(name.begin(), name.end());
        symbol::Variable * var = ctxt->getOrCreate(*it);
        types::InternalType * pIT = symbol::Context::getInstance()->get(var);

        if (pIT)
        {
            symMap3.emplace(*it, std::shared_ptr<JITVal>(JITVal::get(*this, pIT, false, _name)));
        }
    }

    const std::set<symbol::Symbol> & SymWrite = analysis.get_write();
    for (std::set<symbol::Symbol>::const_iterator it = SymWrite.cbegin(), end = SymWrite.cend(); it != end; ++it)
    {
        const std::wstring & name = it->name_get();
        const std::string _name(name.begin(), name.end());
        symbol::Variable * var = ctxt->getOrCreate(*it);
        types::InternalType * pIT = symbol::Context::getInstance()->get(var);

        if (pIT)
        {
            symMap3.emplace(*it, std::shared_ptr<JITVal>(JITVal::get(*this, pIT, true, _name)));
        }
    }

}

void JITVisitor::run()
{
    // on reinjecte les resultats ds l'environnement a=1;jit("a=2");
    symbol::Context * ctxt = symbol::Context::getInstance();
    llvm::Value * llvmCtxt = getPointer(ctxt);
    //llvm::Value * toCall_S = getPointer(reinterpret_cast<void *>(&jit::putInContext_S<Double, double>), getLLVMPtrFuncTy<void, char *, char *, double>(context));
    llvm::Value * toCall_M = module.getOrInsertFunction("putInContext_M_D_ds", getLLVMFuncTy<void, char *, char *, double *, int , int>(context));
    //llvm::Value * toCall_M = getPointer(reinterpret_cast<void *>(&jit::putInContext_M<Double, double*>), getLLVMPtrFuncTy<void, char *, char *, double *, int, int>(context));

    llvm::Value * toCall_S = module.getOrInsertFunction("putInContext_S_D_d", getLLVMFuncTy<void, char *, char *, double>(context));

    for (JITSymbolMap::const_iterator i = symMap3.begin(), end = symMap3.end(); i != end; ++i)
    {
        symbol::Variable * var = ctxt->getOrCreate(i->first);
        llvm::Value * llvmVar = getPointer(var);
        if (i->second.get()->is_scalar())
        {
            builder.CreateCall3(toCall_S, llvmCtxt, llvmVar, i->second.get()->load(*this));
        }
        else
        {
            i->second.get()->load(*this)->dump();
            builder.CreateCall5(toCall_M, llvmCtxt, llvmVar, i->second.get()->load(*this), i->second.get()->loadR(*this), i->second.get()->loadC(*this));
        }
    }

    builder.CreateRetVoid();

    //dump();

    for (llvm::Module::iterator it = module.begin(), end = module.end(); it != end; ++it)
    {
        FPM.run(*it);
    }

    //dump();

    engine->finalizeObject();

    //getLLVMTy<const char * const>(context)->dump();
    //getLLVMTy<int (*)(double)>(context)->dump();

    //foo<int (*) (double)>(context)->dump();

    reinterpret_cast<void (*)()>(engine->getFunctionAddress("jit_main"))();
}

void JITVisitor::dump() const
{
    module.dump();
    //function->dump();
}

template<>
llvm::Value * JITVisitor::getConstant<double>(const double val)
{
    llvm::Value * v = llvm::ConstantFP::get(context, llvm::APFloat(val));
    return v;
}

void JITVisitor::visit(const ast::SimpleVar &e)
{
    /*                symbol::Symbol & sym = e.name_get();
                      std::map<symbol::Symbol, llvm::Value *>::iterator i = symMap.find(sym);
                      if (i != symMap.end())
                      {
                      if (llvm::isa<llvm::AllocaInst>(i->second))
                      {
                      llvm::LoadInst * tmp = builder.CreateLoad(llvm::cast<llvm::AllocaInst>(i->second));
                      tmp->setAlignment(sizeof(double));
                      result_set(tmp);
                      }
                      else
                      {
                      result_set(i->second);
                      }
                      }
                      else
                      {
                      std::wcout << L"que faire...=" << sym.name_get() << std::endl;
                      }
    */
    /*              symbol::Symbol & sym = e.name_get();
                    std::map<symbol::Symbol, JITVal>::iterator i = symMap2.find(sym);
                    if (i != symMap2.end())
                    {
                    llvm::Value * r = llvm::ConstantInt::get(getLLVMTy<int>(context), 1);
                    result_set(JITVal(r, r, i->second.load(builder)));
                    }
                    else
                    {
                    types::Double * pIT = static_cast<Double *>(symbol::Context::getInstance()->get(((ast::SimpleVar&)e).stack_get()));
                    llvm::Value * r = llvm::ConstantInt::get(getLLVMTy<int>(context), pIT->getRows());
                    llvm::Value * c = llvm::Cou onstantInt::get(getLLVMTy<int>(context), pIT->getCols());
                    llvm::Value * ptr = getPointer(pIT->get(), getLLVMTy<double *>(context));

                    result_set(JITVal(r, c, ptr));

                    //std::wcout << L"que faire...=" << sym.name_get() << std::endl;
                    }
    */
    symbol::Symbol & sym = e.name_get();
    JITSymbolMap::iterator i = symMap3.find(sym);
    if (i != symMap3.end())
    {
        result_set(i->second);
    }
    else
    {
        const std::wstring & name = sym.name_get();
        const std::string _name(name.begin(), name.end());
        /*types::InternalType * pIT = symbol::Context::getInstance()->get(((ast::SimpleVar&)e).stack_get());

        result_set(std::shared_ptr<JITVal>(JITVal::get(*this, pIT, _name)));*/
        throw ast::ScilabError("Variable not declared before JIT: " + _name);
    }
}

void JITVisitor::visit(const ast::DollarVar &e) //a=[1 2;3 4];b=[5 6;7 8];jit("a/b")
{

}

void JITVisitor::visit(const ast::ColonVar &e)
{

}

void JITVisitor::visit(const ast::ArrayListVar &e)
{

}

void JITVisitor::visit(const ast::IntExp &e)
{

}

void JITVisitor::visit(const ast::FloatExp &e)
{

}

void JITVisitor::visit(const ast::DoubleExp &e)
{
    result_set(std::shared_ptr<JITVal>(new JITScalarVal<double>(*this, e.value_get(), false)));
}

void JITVisitor::visit(const ast::BoolExp &e)
{

}

void JITVisitor::visit(const ast::StringExp &e)
{

}

void JITVisitor::visit(const ast::CommentExp &e)
{
    // ignored
}

void JITVisitor::visit(const ast::NilExp &e)
{

}

void JITVisitor::visit(const ast::CallExp &e)
{

}

void JITVisitor::visit(const ast::CellCallExp &e)
{

}

void JITVisitor::visit(const ast::OpExp &e)
{
    e.left_get().accept(*this);
    std::shared_ptr<JITVal> pITL = result_get();

    /*getting what to assign*/
    e.right_get().accept(*this);
    std::shared_ptr<JITVal> & pITR = result_get();

    llvm::Value * pResult = NULL;

    switch (e.oper_get())
    {
        case ast::OpExp::plus:
        {
            if (pITL.get()->is_scalar())
            {
                result_set(add_D_D(pITL, pITR, *this));
            }
            else
            {
                result_set(add_M_M(pITL, pITR, *this));
            }
            return;
        }
        case ast::OpExp::minus:
        {
            if (pITL.get()->is_scalar())
            {
                result_set(sub_D_D(pITL, pITR, *this));
            }
            else
            {
                result_set(sub_M_M(pITL, pITR, *this));
            }
            return;//a=1;b=1;for i=1:2:23;a=a+i*3+b;b=b-i*a;end;
        }
        case ast::OpExp::times:
        {
            if (pITL.get()->is_scalar())
            {
                result_set(dotmul_D_D(pITL, pITR, *this));
            }
            else
            {
                result_set(dotmul_M_M(pITL, pITR, *this));
            }
            return;
        }
        default:
            if (pITL.get()->is_scalar())
            {
                result_set(add_D_D(pITL, pITR, *this));
            }
            else
            {
                result_set(add_M_M(pITL, pITR, *this));
            }
            return;
    }

    //llvm::Value * r = llvm::ConstantInt::get(getLLVMTy<int>(context), 1);
    //result_set(JITVal(r, r, pResult));
}

void JITVisitor::visit(const ast::LogicalOpExp &e)
{

}

void JITVisitor::visit(const ast::AssignExp &e)
{
    if (e.left_exp_get().is_simple_var())
    {
        ast::SimpleVar & pVar = static_cast<ast::SimpleVar &>(e.left_exp_get());

        e.right_exp_get().accept(*this);
        std::shared_ptr<JITVal> & pITR = result_get();
        llvm::Value * alloca = nullptr;
        JITSymbolMap::const_iterator i = symMap3.find(pVar.name_get());

        if (i != symMap3.end())
        {
            i->second.get()->store(*pITR.get(), *this);
        }
        else
        {
            const std::wstring & name = pVar.name_get().name_get();
            const std::string _name(name.begin(), name.end());
            // TODO: virer ce truc... le param <double> est force...
            JITVal * jitV = new JITScalarVal<double>(*this, pITR.get(), _name);
            symMap3.emplace(pVar.name_get(), std::shared_ptr<JITVal>(jitV));
        }

        result_set(std::shared_ptr<JITVal>(nullptr));
    }
}

void JITVisitor::visit(const ast::IfExp &e)
{

}

void JITVisitor::visit(const ast::WhileExp &e)
{

}

void JITVisitor::visit(const ast::ForExp &e)
{
    //e.vardec_get().accept(*this);
    const ast::VarDec & vardec = e.vardec_get();
    symbol::Symbol & varName = vardec.name_get();
    const ast::Exp & init = vardec.init_get();

    if (init.is_list_exp())
    {
        const ast::ListExp & list = static_cast<const ast::ListExp &>(init);
        const double * list_values = list.get_values();
        llvm::Value * start = nullptr, * step, * end;
        bool use_int = false;
        bool use_uint = false;
        bool inc = true;
        bool known_step = false;

        if (!ISNAN(list_values[0]) && !ISNAN(list_values[1]) && !ISNAN(list_values[2]))
        {
            const double tstart = std::trunc(list_values[0]);
            const double tstep = std::trunc(list_values[1]);
            const double tend = std::trunc(list_values[2]);

            inc = list_values[1] >= 0;
            known_step = true;

            if ((tstart == list_values[0]) && (tstep == list_values[1]))
            {
                if (tstart >= 0 && tstep >= 0 && tstart <= list_values[2])
                {
                    // we can use an unsigned int but take care to overflow...
                    double k = std::floor(((double)std::numeric_limits<uint64_t>::max() - tstart) / tstep);
                    if ((k * tstep + tstart) >= tend)
                    {
                        // no overflow
                        start = getConstant((uint64_t)tstart);
                        step = getConstant((uint64_t)tstep);
                        end = getConstant((uint64_t)tend);

                        use_int = true;
                        use_uint = true;
                    }
                }
                else
                {
                    double k = std::floor(((double)(inc ? std::numeric_limits<int64_t>::max() : std::numeric_limits<int64_t>::min()) - tstart) / tstep);
                    if ((inc && (k * tstep + tstart) >= tend) || (!inc && (k * tstep + tstart) <= tend))
                    {
                        // no overflow
                        start = getConstant((int64_t)tstart);
                        step = getConstant((int64_t)tstep);
                        end = getConstant((int64_t)tend);

                        use_int = true;
                    }
                }
            }
            else
            {
                start = getConstant(list_values[0]);
                step = getConstant(list_values[1]);
                end = getConstant(list_values[2]);
            }
        }

        if (!start)
        {
            if (!ISNAN(list_values[0]))
            {
                start = getConstant(list_values[0]);
            }
            else
            {
                list.start_get().accept(*this);
                start = result_get().get()->load(*this);
            }

            if (!ISNAN(list_values[1]))
            {
                step = getConstant(list_values[1]);
                inc = list_values[1] >= 0;
                known_step = true;
            }
            else
            {
                list.step_get().accept(*this);
                step = result_get().get()->load(*this);
            }

            if (!ISNAN(list_values[2]))
            {
                end = getConstant(list_values[2]);
            }
            else
            {
                list.end_get().accept(*this);
                end = result_get().get()->load(*this);
            }
        }

        llvm::BasicBlock * BBBody = llvm::BasicBlock::Create(context, "for_body", function);
        llvm::BasicBlock * BBAfter = llvm::BasicBlock::Create(context, "for_after", function);

        llvm::BasicBlock * cur_block = builder.GetInsertBlock();
        llvm::Value * tmp;

        if (known_step)
        {
            if (inc)
            {
                tmp = use_int ? (use_uint ? builder.CreateICmpULE(start, end) : builder.CreateICmpSLE(start, end)) : builder.CreateFCmpOLE(start, end);
            }
            else
            {
                tmp = use_int ? (use_uint ? builder.CreateICmpUGE(start, end) : builder.CreateICmpSGE(start, end)) : builder.CreateFCmpOGE(start, end);
            }
        }
        else
        {
            //TODO: add something to handle this case
        }

        builder.CreateCondBr(tmp, BBBody, BBAfter);

        builder.SetInsertPoint(BBBody);
        llvm::PHINode * phi = use_int ? builder.CreatePHI(getLLVMTy<int64_t>(context), 2) : builder.CreatePHI(getLLVMTy<double>(context), 2);

        JITSymbolMap::const_iterator i = symMap3.find(varName);
        tmp = use_int ? (use_uint ? builder.CreateUIToFP(phi, getLLVMTy<double>(context)) : builder.CreateSIToFP(phi, getLLVMTy<double>(context))) : phi;
        i->second.get()->store(tmp, *this);

        phi->addIncoming(start, cur_block);

        builder.SetInsertPoint(BBBody);
        e.body_get().accept(*this);

        tmp = use_int ? builder.CreateAdd(phi, step) : builder.CreateFAdd(phi, step);
        phi->addIncoming(tmp, builder.GetInsertBlock());

        if (known_step)
        {
            if (inc)
            {
                tmp = use_int ? (use_uint ? builder.CreateICmpULE(tmp, end) : builder.CreateICmpSLE(tmp, end)) : builder.CreateFCmpOLE(tmp, end);
            }
            else
            {
                tmp = use_int ? (use_uint ? builder.CreateICmpUGE(tmp, end) : builder.CreateICmpSGE(tmp, end)) : builder.CreateFCmpOGE(tmp, end);
            }
        }
        else
        {
            //TODO: add something to handle this case
        }

        builder.CreateCondBr(tmp, BBBody, BBAfter);

        builder.SetInsertPoint(BBAfter);

        //llvm::AllocaInst * cur = builder.CreateAlloca(getLLVMTy<double>(context));
        //llvm::StoreInst * cur_store = builder.CreateAlignedStore(phi, cur, sizeof(double));

        //symMap3.emplace(varName, std::shared_ptr<JITVal>(new JITScalarVal<double>(*this, cur)));
    }
    else
    {
        // Should not occured...
        // Normally, if the init is an iterator the for exp itself is not jittable
        // but take care of the case for i=int32(1:2:123)...
    }

    //function->dump();
}

void JITVisitor::visit(const ast::BreakExp &e)
{

}

void JITVisitor::visit(const ast::ContinueExp &e)
{

}

void JITVisitor::visit(const ast::TryCatchExp &e)
{

}

void JITVisitor::visit(const ast::SelectExp &e)
{

}

void JITVisitor::visit(const ast::CaseExp &e)
{

}

void JITVisitor::visit(const ast::ReturnExp &e)
{

}

void JITVisitor::visit(const ast::FieldExp &e)
{

}

void JITVisitor::visit(const ast::NotExp &e)
{

}

void JITVisitor::visit(const ast::TransposeExp &e)
{

}

void JITVisitor::visit(const ast::MatrixExp &e)
{

}

void JITVisitor::visit(const ast::MatrixLineExp &e)
{

}

void JITVisitor::visit(const ast::CellExp &e)
{

}

void JITVisitor::visit(const ast::SeqExp &e)
{
    for (std::list<ast::Exp *>::const_iterator i = e.exps_get().begin(), end = e.exps_get().end(); i != end; ++i)
    {
        result_set(std::shared_ptr<JITVal>(nullptr));
        (*i)->accept(*this);
    }
}

void JITVisitor::visit(const ast::ArrayListExp &e)
{

}

void JITVisitor::visit(const ast::AssignListExp &e)
{

}

void JITVisitor::visit(const ast::VarDec &e)
{
    e.init_get().accept(*this);
}

void JITVisitor::visit(const ast::FunctionDec &e)
{

}

void JITVisitor::visit(const ast::ListExp &e)
{

}
}